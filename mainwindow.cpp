#include "mainwindow.h"
#include "tableitemdelegate.h"
#include "constants.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QIntValidator>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <set>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), _currentTestAddr(0), _lastHighlightedAddr(0), _testRunning(false), _dataChangedConnected(true) {
    setWindowTitle("Цифровой двойник ОЗУ — 350504 Витовт Даник Маша Настя");
    resize(1400, 800);

    _mem = new MemoryModel(DEFAULT_MEMORY_SIZE, this);
    _worker = std::unique_ptr<TesterWorker>(new TesterWorker(_mem));
    _testTimer = new QTimer(this);

    // Main layout with splitter
    QWidget* central = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // Top section: Control panels
    QSplitter* topSplitter = new QSplitter(Qt::Horizontal);

    // Fault Management Group
    _faultGroup = new QGroupBox("Управление неисправностями");
    QVBoxLayout* faultLayout = new QVBoxLayout;

    QHBoxLayout* faultModelLayout = new QHBoxLayout;
    faultModelLayout->addWidget(new QLabel("Модель неисправности:"));
    _faultCombo = new QComboBox;
    _faultCombo->addItem("Нет", (int)FaultModel::None);
    _faultCombo->addItem("Stuck-at-0", (int)FaultModel::StuckAt0);
    _faultCombo->addItem("Stuck-at-1", (int)FaultModel::StuckAt1);
    _faultCombo->addItem("Bit-flip", (int)FaultModel::BitFlip);
    _faultCombo->addItem("Open (invalid read)", (int)FaultModel::OpenRead);
    _faultCombo->setCurrentIndex(3); // Bit-flip по умолчанию
    _faultCombo->setToolTip("Выберите модель неисправности для внедрения в память");
    faultModelLayout->addWidget(_faultCombo);
    faultLayout->addLayout(faultModelLayout);

    QHBoxLayout* addrLayout = new QHBoxLayout;
    addrLayout->addWidget(new QLabel("Адрес (слово):"));
    _addrEdit = new QLineEdit("0");
    _addrEdit->setToolTip("Адрес начала области с неисправностью (0-255)");
    _addrEdit->setValidator(new QIntValidator(0, 255, this));
    addrLayout->addWidget(_addrEdit);
    faultLayout->addLayout(addrLayout);

    QHBoxLayout* lenLayout = new QHBoxLayout;
    lenLayout->addWidget(new QLabel("Длина (слов):"));
    _lenEdit = new QLineEdit(QString::number(DEFAULT_FAULT_LENGTH));
    _lenEdit->setToolTip("Количество слов с неисправностью");
    _lenEdit->setValidator(new QIntValidator(1, static_cast<int>(MAX_MEMORY_SIZE), this));
    lenLayout->addWidget(_lenEdit);
    faultLayout->addLayout(lenLayout);

    QHBoxLayout* probLayout = new QHBoxLayout;
    probLayout->addWidget(new QLabel("Вероятность инверсии:"));
    _flipProbSpin = new QDoubleSpinBox;
    _flipProbSpin->setRange(0.0, 1.0);
    _flipProbSpin->setSingleStep(0.01);
    // Default probability: 0.010 for Bit-flip, 0.110 for other fault models
    _flipProbSpin->setValue(0.010); // Bit-flip is selected by default (index 3)
    _flipProbSpin->setDecimals(3);
    _flipProbSpin->setToolTip("Вероятность инверсии бита для модели Bit-flip (0.0 - 1.0)");
    probLayout->addWidget(_flipProbSpin);
    faultLayout->addLayout(probLayout);

    QHBoxLayout* faultBtnLayout = new QHBoxLayout;
    _injectBtn = new QPushButton("Внедрить неисправность");
    _injectBtn->setToolTip("Внедрить выбранную неисправность в память");
    _resetBtn = new QPushButton("Сбросить память");
    _resetBtn->setToolTip("Очистить память и удалить все неисправности");
    faultBtnLayout->addWidget(_injectBtn);
    faultBtnLayout->addWidget(_resetBtn);
    faultLayout->addLayout(faultBtnLayout);

    _faultInfoLabel = new QLabel("Неисправность не внедрена");
    _faultInfoLabel->setWordWrap(true);
    faultLayout->addWidget(_faultInfoLabel);

    _faultGroup->setLayout(faultLayout);
    topSplitter->addWidget(_faultGroup);

    // Testing Group
    _testGroup = new QGroupBox("Тестирование");
    QVBoxLayout* testLayout = new QVBoxLayout;

    QHBoxLayout* algoLayout = new QHBoxLayout;
    algoLayout->addWidget(new QLabel("Алгоритм тестирования:"));
    _algoCombo = new QComboBox;
    _algoCombo->addItem("Walking 1s", (int)TestAlgorithm::WalkingOnes);
    _algoCombo->addItem("Walking 0s", (int)TestAlgorithm::WalkingZeros);
    _algoCombo->addItem("March - simple", (int)TestAlgorithm::MarchSimple);
    _algoCombo->setToolTip("Выберите алгоритм тестирования памяти");
    algoLayout->addWidget(_algoCombo);
    testLayout->addLayout(algoLayout);

    _testInfoLabel = new QLabel("");
    _testInfoLabel->setWordWrap(true);
    testLayout->addWidget(_testInfoLabel);

    QHBoxLayout* testBtnLayout = new QHBoxLayout;
    _startBtn = new QPushButton("Запустить тест");
    _startBtn->setToolTip("Запустить выбранный тест");
    testBtnLayout->addWidget(_startBtn);
    testLayout->addLayout(testBtnLayout);

    _progress = new QProgressBar;
    _progress->setRange(0, PROGRESS_MAX_PERCENT);
    _progress->setValue(0);
    _progress->setFormat("%p%");
    testLayout->addWidget(_progress);

    QHBoxLayout* progressDetailsLayout = new QHBoxLayout;
    _currentAddrLabel = new QLabel("Адрес: —");
    _expectedValueLabel = new QLabel("Ожидается: —");
    _readValueLabel = new QLabel("Прочитано: —");
    progressDetailsLayout->addWidget(_currentAddrLabel);
    progressDetailsLayout->addWidget(_expectedValueLabel);
    progressDetailsLayout->addWidget(_readValueLabel);
    testLayout->addLayout(progressDetailsLayout);

    _testGroup->setLayout(testLayout);
    topSplitter->addWidget(_testGroup);

    // Statistics Group
    _statsGroup = new QGroupBox("Статистика");
    QVBoxLayout* statsLayout = new QVBoxLayout;

    _totalAddressesLabel = new QLabel(QString("Всего адресов: %1").arg(DEFAULT_MEMORY_SIZE));
    _testedAddressesLabel = new QLabel("Протестировано: 0");
    _faultsFoundLabel = new QLabel("Найдено неисправностей: 0");
    _coverageLabel = new QLabel("Покрытие: 0%");
    _testTimeLabel = new QLabel("Время теста: —");
    _currentFaultModelLabel = new QLabel("Текущая модель: Нет");
    _currentAlgorithmLabel = new QLabel("Текущий алгоритм: —");

    statsLayout->addWidget(_totalAddressesLabel);
    statsLayout->addWidget(_testedAddressesLabel);
    statsLayout->addWidget(_faultsFoundLabel);
    statsLayout->addWidget(_coverageLabel);
    statsLayout->addWidget(_testTimeLabel);
    statsLayout->addWidget(_currentFaultModelLabel);
    statsLayout->addWidget(_currentAlgorithmLabel);

    _statsGroup->setLayout(statsLayout);
    topSplitter->addWidget(_statsGroup);

    topSplitter->setStretchFactor(0, 1);
    topSplitter->setStretchFactor(1, 1);
    topSplitter->setStretchFactor(2, 1);

    mainLayout->addWidget(topSplitter, 0);

    // Bottom section: Memory table and log
    QSplitter* bottomSplitter = new QSplitter(Qt::Horizontal);

    // Memory Table
    QWidget* memoryWidget = new QWidget;
    QVBoxLayout* memoryLayout = new QVBoxLayout(memoryWidget);

    QHBoxLayout* tableControlsLayout = new QHBoxLayout;
    tableControlsLayout->addWidget(new QLabel("Поиск адреса:"));
    _searchEdit = new QLineEdit;
    _searchEdit->setPlaceholderText("Введите адрес (0-255)");
    _searchEdit->setValidator(new QIntValidator(0, 255, this));
    _searchBtn = new QPushButton("Найти");
    _scrollToNextFaultBtn = new QPushButton("Следующая неисправность");
    _scrollToNextFaultBtn->setToolTip("Перейти к следующей обнаруженной неисправности (с циклическим поиском)");
    tableControlsLayout->addWidget(_searchEdit);
    tableControlsLayout->addWidget(_searchBtn);
    tableControlsLayout->addWidget(_scrollToNextFaultBtn);
    tableControlsLayout->addStretch();
    memoryLayout->addLayout(tableControlsLayout);

    _table = new QTableWidget;
    _table->setColumnCount(6);
    _table->setHorizontalHeaderLabels({"Адрес", "HEX", "Двоичное", "Десятичное", "Статус", "Тип неисправности"});
    _table->verticalHeader()->setVisible(false);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setAlternatingRowColors(false); // Отключаем автоматическое чередование, управляем цветами вручную
    _table->setSortingEnabled(false);
    _table->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем кастомный делегат для правильной отрисовки красного выделения
    _tableDelegate = new TableItemDelegate(this);
    _tableDelegate->setTheme(Theme::DeusEx); // Устанавливаем начальную тему
    _table->setItemDelegate(_tableDelegate);
    memoryLayout->addWidget(_table);

    bottomSplitter->addWidget(memoryWidget);

    // Log
    QWidget* logWidget = new QWidget;
    QVBoxLayout* logLayout = new QVBoxLayout(logWidget);

    QHBoxLayout* logControlsLayout = new QHBoxLayout;
    logControlsLayout->addWidget(new QLabel("Журнал событий:"));
    logControlsLayout->addStretch();
    _clearLogBtn = new QPushButton("Очистить");
    _exportBtn = new QPushButton("Экспорт результатов");
    logControlsLayout->addWidget(_clearLogBtn);
    logControlsLayout->addWidget(_exportBtn);
    logLayout->addLayout(logControlsLayout);

    _log = new QTextEdit;
    _log->setReadOnly(true);
    _log->setFont(QFont("Courier", 9));
    logLayout->addWidget(_log);
    
    // Initialize logger
    _logger = std::unique_ptr<Logger>(new Logger(_log, _currentTheme));

    bottomSplitter->addWidget(logWidget);
    bottomSplitter->setStretchFactor(0, 3);
    bottomSplitter->setStretchFactor(1, 2);

    mainLayout->addWidget(bottomSplitter, 1);

    setCentralWidget(central);

    // Create menu
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* helpMenu = menuBar->addMenu("Справка");
    QAction* aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "О программе",
            "Цифровой двойник ОЗУ — Тестбенч\n\n"
            "Программа для моделирования неисправностей в оперативной памяти\n"
            "и тестирования различных алгоритмов обнаружения неисправностей.\n\n"
            "Версия 1.0");
    });

    QAction* algorithmsAction = helpMenu->addAction("Описание алгоритмов");
    connect(algorithmsAction, &QAction::triggered, this, [this]() {
        QString desc = "Описание алгоритмов тестирования:\n\n";
        desc += "Walking 1s: Записывает единицу в каждый бит и проверяет чтение.\n";
        desc += "Walking 0s: Записывает ноль в каждый бит и проверяет чтение.\n";
        desc += "March - simple: Простой маршевый тест (запись 0, проверка 0, запись 1, проверка 1).\n";
        QMessageBox::information(this, "Описание алгоритмов", desc);
    });

    // Theme menu
    QMenu* viewMenu = menuBar->addMenu("Вид");
    _themeGroup = new QActionGroup(this);
    _themeGroup->setExclusive(true);
    
    _deusExAction = new QAction("Deus Ex", this);
    _deusExAction->setCheckable(true);
    _deusExAction->setChecked(true);
    _themeGroup->addAction(_deusExAction);
    viewMenu->addAction(_deusExAction);
    
    _matrixAction = new QAction("Matrix", this);
    _matrixAction->setCheckable(true);
    _themeGroup->addAction(_matrixAction);
    viewMenu->addAction(_matrixAction);
    
    _gurrenLagannAction = new QAction("Gurren Lagann", this);
    _gurrenLagannAction->setCheckable(true);
    _themeGroup->addAction(_gurrenLagannAction);
    viewMenu->addAction(_gurrenLagannAction);
    
    _cyberpunkAction = new QAction("Cyberpunk", this);
    _cyberpunkAction->setCheckable(true);
    _themeGroup->addAction(_cyberpunkAction);
    viewMenu->addAction(_cyberpunkAction);
    
    connect(_themeGroup, &QActionGroup::triggered, this, &MainWindow::onThemeChanged);

    // Connections
    connect(_injectBtn, &QPushButton::clicked, this, &MainWindow::onInject);
    connect(_resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(_startBtn, &QPushButton::clicked, this, &MainWindow::onStartTest);
    connect(_clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLog);
    connect(_exportBtn, &QPushButton::clicked, this, &MainWindow::exportResults);
    connect(_scrollToNextFaultBtn, &QPushButton::clicked, this, &MainWindow::scrollToNextFault);
    connect(_searchBtn, &QPushButton::clicked, this, [this]() {
        if (!_mem || !_table) return;
        bool ok;
        int addr = _searchEdit->text().toInt(&ok);
        if (ok && addr >= 0 && addr < int(_mem->size())) {
            _table->selectRow(addr);
            QTableWidgetItem* item = _table->item(addr, 0);
            if (item) {
                _table->scrollToItem(item, QAbstractItemView::EnsureVisible);
            }
        }
    });
    
    // Исправленные соединения для Qt 5.5.1
    connect(_algoCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgorithmChanged(int)));
    connect(_faultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onFaultModelChanged(int)));
    connect(_faultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFaultInfo()));

    // Worker signals
    connect(_worker.get(), &TesterWorker::progress, _progress, &QProgressBar::setValue);
    connect(_worker.get(), &TesterWorker::progressDetail, this, &MainWindow::updateProgressDetails);
    connect(_worker.get(), &TesterWorker::finished, this, &MainWindow::onTestFinished);

    // Memory signals
    connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);
    connect(_mem, &MemoryModel::faultInjected, this, &MainWindow::updateFaultInfo);
    connect(_mem, &MemoryModel::errorOccurred, this, [this](const QString& message) {
        if (_logger) {
            _logger->error(message);
        }
    });

    // Initialize
    refreshTable(0, _mem->size());
    updateFaultInfo();
    updateStatistics();
    onAlgorithmChanged(0);
    
    // Apply default theme (Deus Ex)
    _currentTheme = Theme::DeusEx;
    applyTheme(Theme::DeusEx);
    
    _logger->info("Программа запущена. Готов к работе.");
}

MainWindow::~MainWindow() {
    if (_worker) {
        // Stop any running test
        _testRunning = false;
        
        // Restore dataChanged connection if it was disconnected
        if (!_dataChangedConnected && _mem) {
            connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);
            _dataChangedConnected = true;
        }
        
        // Disconnect signals to prevent callbacks during destruction
        disconnect(_worker.get(), nullptr, this, nullptr);
        
        // Move worker back to main thread for safe deletion
        _worker->moveToThread(QThread::currentThread());
        
        // Reset worker - its destructor will wait for thread to finish
        _worker.reset();
    }
}

void MainWindow::onInject() {
    if (!_mem || !_logger) return;
    
    InjectedFault f;
    f.model = static_cast<FaultModel>(_faultCombo->currentData().toInt());
    bool ok1 = false, ok2 = false;
    size_t addr = _addrEdit->text().toULongLong(&ok1);
    size_t len = _lenEdit->text().toULongLong(&ok2);

    if (!ok1 || !ok2) {
        if (_logger) {
            _logger->error("Ошибка ввода: Адрес и длина должны быть числами.");
        }
        QMessageBox::warning(this, "Ошибка ввода", "Адрес и длина должны быть числами.");
        return;
    }

    if (addr >= _mem->size()) {
        _logger->error(QString("Ошибка: Адрес %1 выходит за пределы памяти (0-%2)").arg(addr).arg(_mem->size() - 1));
        QMessageBox::warning(this, "Ошибка", QString("Адрес должен быть в диапазоне 0-%1").arg(_mem->size() - 1));
        return;
    }

    if (addr + len > _mem->size()) {
        len = _mem->size() - addr;
        _logger->warning(QString("Длина уменьшена до %1 (выход за пределы памяти)").arg(len));
    }

    f.addr = addr;
    f.len = std::max<size_t>(1, len);
    f.flip_probability = _flipProbSpin->value();

    _mem->injectFault(f);
    _logger->success(QString("Внедрена неисправность: %1 по адресу %2, длина=%3")
               .arg(DataFormatter::getFaultModelName(f.model)).arg(addr).arg(f.len));
    updateFaultInfo();
    updateStatistics();
}

void MainWindow::onReset() {
    if (!_mem || !_logger) return;
    _mem->reset();
    _lastResults.clear();
    _testRunning = false;
    _lastHighlightedAddr = 0;
    _progress->setValue(0);
    _currentAddrLabel->setText("Адрес: —");
    _expectedValueLabel->setText("Ожидается: —");
    _readValueLabel->setText("Прочитано: —");
    _logger->info("Память сброшена. Все данные очищены, неисправности удалены.");
    updateFaultInfo();
    updateStatistics();
    refreshTable(0, _mem->size());
}

void MainWindow::onStartTest() {
    if (!_mem || !_worker || !_logger) return;
    
    // Thread-safe check: prevent multiple test starts
    if (_testRunning) {
        if (_logger) {
            _logger->warning("Тест уже выполняется. Дождитесь завершения.");
        }
        return;
    }

    // Additional safety check: ensure worker is ready
    if (!_worker) {
        if (_logger) {
            _logger->error("Worker не инициализирован. Невозможно запустить тест.");
        }
        return;
    }

    TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
    _testRunning = true;
    _testStartTime = QTime::currentTime();
    _testTimer->start();
    _startBtn->setEnabled(false);
    _lastResults.clear();

    // Reset progress details
    _currentAddrLabel->setText("Адрес: —");
    _expectedValueLabel->setText("Ожидается: —");
    _readValueLabel->setText("Прочитано: —");

    // Disable table updates during test for performance
    // Table will be updated only at the end of test
    if (_dataChangedConnected) {
        disconnect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);
        _dataChangedConnected = false;
    }

    _logger->info(QString("Запуск теста: %1").arg(_algoCombo->currentText()));
    updateTestInfo();

    QMetaObject::invokeMethod(_worker.get(), "run", Qt::QueuedConnection, Q_ARG(TestAlgorithm, algo));
}

void MainWindow::onTestFinished(const std::vector<TestResult>& results) {
    // Thread-safe: this slot is called from main thread via Qt signal/slot mechanism
    // Reset test state atomically
    _testRunning = false;
    _startBtn->setEnabled(true);
    _testTimer->stop();
    _lastResults = results;

    // Re-enable table updates after test completion
    if (!_dataChangedConnected) {
        connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);
        _dataChangedConnected = true;
    }

    int fails = 0;
    for (const auto& r : results) if (!r.passed) ++fails;

    int elapsed = _testStartTime.msecsTo(QTime::currentTime());
    QString timeStr = QString("%1.%2 сек").arg(elapsed / 1000).arg((elapsed % 1000) / 100, 2, 10, QChar('0'));

    if (fails == 0) {
        _logger->success(QString("Тест завершен успешно. Всего проверок: %1, неисправностей не обнаружено. Время: %2")
                   .arg(results.size()).arg(timeStr));
    } else {
        _logger->error(QString("Тест завершен. Всего проверок: %1, обнаружено неисправностей: %2. Время: %3")
                 .arg(results.size()).arg(fails).arg(timeStr));
    }

    // Refresh table to show updated values and test results
    refreshTable(0, _mem->size());

    updateStatistics();
    _testTimeLabel->setText(QString("Время теста: %1").arg(timeStr));
}

QTableWidgetItem* MainWindow::createOrGetTableItem(int row, int col) {
    if (!_table) return nullptr;
    QTableWidgetItem* item = _table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem;
        _table->setItem(row, col, item);
    }
    return item;
}

void MainWindow::refreshTable(size_t begin, size_t end) {
    // Parameters are part of Qt signal signature but not used in implementation
    // Table is refreshed completely regardless of range
    Q_UNUSED(begin);
    Q_UNUSED(end);
    
    if (!_mem || !_table) return;
    
    // Оптимизация: отключаем обновление виджета во время массовых изменений
    _table->setUpdatesEnabled(false);
    
    size_t n = _mem->size();
    _table->setRowCount(int(n));

    auto f = _mem->currentFault();
    
    // Single pass through _lastResults to collect all needed data
    std::set<size_t> testedAddresses;
    std::set<size_t> failedAddresses;
    std::unordered_map<size_t, TestResult> resultMap; // Map address to test result for quick lookup
    
    for (const auto& r : _lastResults) {
        testedAddresses.insert(r.addr);
        if (!r.passed) {
            failedAddresses.insert(r.addr);
        }
        // Store result for quick lookup (last result for each address wins)
        resultMap[r.addr] = r;
    }

    // Получаем цвета один раз для всей таблицы (оптимизация)
    ThemeColors colors = ThemeManager::getColors(_currentTheme);

#ifdef DEBUG
    // Debug logging: Count faulty addresses
    std::set<size_t> failedAddressesForLog;
    for (const auto& r : _lastResults) {
        if (!r.passed) {
            failedAddressesForLog.insert(r.addr);
        }
    }
    _logger->info(QString("refreshTable: Всего результатов: %1, неисправных адресов: %2")
            .arg(_lastResults.size()).arg(failedAddressesForLog.size()));
    _logger->info(QString("refreshTable: Цвета для красного выделения - фон: %1, текст: %2")
            .arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));
#endif

    for (size_t i = 0; i < n; ++i) {
        Word v = _mem->read(i);
        bool isFaulty = (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len);
        bool isTested = testedAddresses.find(i) != testedAddresses.end();
        bool hasFailedTest = (failedAddresses.find(i) != failedAddresses.end());

        // Populate table data (address, hex, binary, decimal, status, fault type)
        populateTableData(i, v, f, testedAddresses, resultMap, colors);

        // Apply colors to table cells
        applyTableColors(i, isFaulty, isTested, hasFailedTest, colors);

        // Apply red highlighting for failed tests (highest priority)
        if (hasFailedTest) {
#ifdef DEBUG
            _logger->info(QString("refreshTable: Адрес %1 - hasFailedTest=true, применяю красное выделение (фон: %2, текст: %3)")
                    .arg(i).arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));
#endif
            applyFailedTestHighlighting(i, colors);
        }
#ifdef DEBUG
        else {
            if (failedAddresses.count(i) > 0) {
                _logger->error(QString("refreshTable: ОШИБКА! Адрес %1 в списке неисправных, но hasFailedTest=false!")
                        .arg(i));
            }
        }
#endif
    }

    // IMPORTANT: Apply red highlighting AGAIN after all updates
    // This guarantees it won't be overwritten
    // failedAddresses already collected in single pass above
    
    // Now apply red highlighting to all faulty addresses
#ifdef DEBUG
    _logger->info(QString("refreshTable: Второй проход - применяю красное выделение к %1 адресам")
            .arg(failedAddresses.size()));
#endif
    // Only apply red highlighting if there are failed addresses
    // If failedAddresses is empty (e.g., after reset), red highlighting should already be cleared
    // by applyTableColors setting hasFailedTest=false
    for (size_t addr : failedAddresses) {
        if (addr >= n) continue; // Boundary check
        // Apply red highlighting using helper method
        applyFailedTestHighlighting(addr, colors);
    }
    
    // IMPORTANT: If failedAddresses is empty, ensure all red highlighting is removed
    // This handles the case when memory is reset and _lastResults is cleared
    if (failedAddresses.empty()) {
        // Explicitly clear red highlighting for all addresses
        for (size_t i = 0; i < n; ++i) {
            bool isFaulty = (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len);
            bool isTested = testedAddresses.find(i) != testedAddresses.end();
            
            for (int col = 0; col < _table->columnCount(); ++col) {
                QTableWidgetItem* item = _table->item(int(i), col);
                if (!item) continue;
                
                // Reset failed test flag for all columns
                item->setData(Qt::UserRole + 1, QVariant(false));
                
                if (col == 4) {
                    // Status column - reset to proper color based on state
                    if (isTested) {
                        // If tested, use appropriate status color
                        item->setBackground(colors.statusPassedBg);
                        item->setForeground(colors.statusPassedText);
                    } else {
                        item->setBackground(colors.statusUntestedBg);
                        item->setForeground(colors.statusUntestedText);
                    }
                } else if (col == 5) {
                    // Fault type column - reset to proper color based on state
                    if (isFaulty) {
                        item->setForeground(colors.faultyNotTestedText);
                    } else if (isTested) {
                        item->setForeground(colors.tableText);
                    } else {
                        item->setForeground(colors.statusUntestedText);
                    }
                    // Reset background to default
                    item->setBackground(QBrush()); // Clear background
                } else {
                    // Data columns (0-3) - explicitly reset colors to remove any red highlighting
                    if (isFaulty && !isTested) {
                        item->setBackground(colors.faultyNotTestedBg);
                        item->setForeground(colors.faultyNotTestedText);
                    } else if (isTested) {
                        item->setBackground(colors.passedTestBg);
                        item->setForeground(colors.passedTestText);
                    } else {
                        if (i % 2 == 0) {
                            item->setBackground(colors.untestedBgEven);
                        } else {
                            item->setBackground(colors.untestedBgOdd);
                        }
                        item->setForeground(colors.statusUntestedText);
                    }
                }
            }
        }
    }

    // Re-enable widget updates
    _table->setUpdatesEnabled(true);
    
#ifdef DEBUG
    // Debug logging: Check actual colors of items after application
    int verifiedCount = 0;
    int mismatchCount = 0;
    for (size_t addr : failedAddresses) {
        if (addr >= n) continue;
        bool allCorrect = true;
        for (int col = 0; col < _table->columnCount(); ++col) {
            if (col == 4) continue; // Skip status column
            QTableWidgetItem* item = _table->item(int(addr), col);
            if (item) {
                QColor actualBg = item->background().color();
                QColor expectedBg = colors.failedTestBg;
                if (actualBg.rgb() != expectedBg.rgb()) {
                    allCorrect = false;
                    _logger->error(QString("refreshTable: НЕСООТВЕТСТВИЕ! Адрес %1, колонка %2 - ожидался фон %3, фактический %4")
                            .arg(addr).arg(col).arg(expectedBg.name()).arg(actualBg.name()));
                    mismatchCount++;
                }
            }
        }
        if (allCorrect) {
            verifiedCount++;
        }
    }
    _logger->info(QString("refreshTable: Проверка завершена - правильно окрашено: %1, несоответствий: %2")
            .arg(verifiedCount).arg(mismatchCount));
#endif
    
    // Оптимизация: resizeColumnsToContents очень медленный, вызываем только при необходимости
    // Можно вызывать реже или только при изменении размера окна
    // _table->resizeColumnsToContents(); // Отключено для производительности
}

void MainWindow::populateTableData(size_t addr, const Word value, const InjectedFault& f,
                                  const std::set<size_t>& testedAddresses,
                                  const std::unordered_map<size_t, TestResult>& resultMap,
                                  const ThemeColors& colors) {
    if (!_table) return;
    
    int row = int(addr);
    bool isFaulty = (f.model != FaultModel::None && addr >= f.addr && addr < f.addr + f.len);
    bool isTested = testedAddresses.find(addr) != testedAddresses.end();
    
    // Address
    QTableWidgetItem* addrItem = createOrGetTableItem(row, 0);
    addrItem->setText(QString::number(addr));
    addrItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addrItem->setForeground(colors.tableText);

    // HEX
    QTableWidgetItem* hexItem = createOrGetTableItem(row, 1);
    hexItem->setText(QString("0x%1").arg(value, 8, 16, QChar('0')).toUpper());
    hexItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hexItem->setForeground(colors.tableText);

    // Binary
    QTableWidgetItem* binItem = createOrGetTableItem(row, 2);
    binItem->setText(DataFormatter::formatBinary(value));
    binItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    binItem->setFont(QFont("Courier", 9));
    binItem->setForeground(colors.tableText);

    // Decimal
    QTableWidgetItem* decItem = createOrGetTableItem(row, 3);
    decItem->setText(QString::number(value));
    decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    decItem->setForeground(colors.tableText);

    // Determine test result
    bool passed = true;
    auto resultIt = resultMap.find(addr);
    if (resultIt != resultMap.end()) {
        passed = resultIt->second.passed;
    } else if (isTested) {
        passed = true;
    }

    // Status
    QTableWidgetItem* statusItem = createOrGetTableItem(row, 4);
    if (!statusItem) return;
    
    QFont font = statusItem->font();
    font.setBold(false);
    statusItem->setFont(font);
    statusItem->setToolTip("");
    
    if (isTested) {
        if (passed) {
            statusItem->setText("Исправна");
            statusItem->setBackground(colors.statusPassedBg);
            statusItem->setForeground(colors.statusPassedText);
        } else {
            statusItem->setText("Неисправна");
            statusItem->setBackground(colors.statusFailedBg);
            statusItem->setForeground(colors.statusFailedText);
        }
    } else {
        statusItem->setText("Не протестировано");
        statusItem->setBackground(colors.statusUntestedBg);
        statusItem->setForeground(colors.statusUntestedText);
    }

    // Fault Type
    QTableWidgetItem* faultTypeItem = createOrGetTableItem(row, 5);
    if (!faultTypeItem) return;
    
    faultTypeItem->setFont(font);
    faultTypeItem->setToolTip("");
    
    // Note: Fault type column can also get red highlighting when test fails
    // The flag will be set by applyFailedTestHighlighting if needed
    
    if (isFaulty) {
        faultTypeItem->setText(DataFormatter::getFaultModelName(f.model));
        faultTypeItem->setForeground(colors.faultyNotTestedText);
        faultTypeItem->setBackground(QBrush()); // Clear background
    } else if (isTested) {
        auto resultIt2 = resultMap.find(addr);
        if (resultIt2 != resultMap.end() && !resultIt2->second.passed) {
            faultTypeItem->setText("Обнаружена");
            faultTypeItem->setForeground(colors.failedTestText);
            faultTypeItem->setBackground(QBrush()); // Clear background - text color indicates fault
            faultTypeItem->setToolTip(QString("Ожидалось: 0x%1, Прочитано: 0x%2")
                                      .arg(resultIt2->second.expected, 8, 16, QChar('0'))
                                      .arg(resultIt2->second.read, 8, 16, QChar('0')));
        } else {
            faultTypeItem->setText("—");
            faultTypeItem->setForeground(colors.tableText);
            faultTypeItem->setBackground(QBrush()); // Clear background
        }
    } else {
        faultTypeItem->setText("—");
        faultTypeItem->setForeground(colors.statusUntestedText);
        faultTypeItem->setBackground(QBrush()); // Clear background
    }
}

void MainWindow::applyTableColors(size_t addr, bool isFaulty, bool isTested, bool hasFailedTest,
                                 const ThemeColors& colors) {
    if (!_table) return;
    
    // Apply colors to data columns (0-3) only
    // Status (4) and fault type (5) columns are handled in populateTableData
    // Red highlighting for failed tests is applied separately with higher priority to ALL columns
    for (int col = 0; col < 4; ++col) { // Only process data columns (Address, HEX, Binary, Decimal)
        QTableWidgetItem* item = _table->item(int(addr), col);
        if (!item) continue;

        // Reset bold font
        QFont itemFont = item->font();
        itemFont.setBold(false);
        item->setFont(itemFont);

        // Clear tooltips
        item->setToolTip("");

        // IMPORTANT: Always reset the failed test flag first
        // This ensures red highlighting is removed when hasFailedTest is false
        item->setData(Qt::UserRole + 1, QVariant(hasFailedTest));

        // Apply colors only if not a failed test (failed tests get red highlighting separately)
        if (!hasFailedTest) {
            // Explicitly reset background and foreground to remove any red highlighting
            if (isFaulty && !isTested) {
                // Golden/yellow for faulty area (not yet tested)
                item->setBackground(colors.faultyNotTestedBg);
                item->setForeground(colors.faultyNotTestedText);
            } else if (isTested) {
                // Green for tested and passed
                item->setBackground(colors.passedTestBg);
                item->setForeground(colors.passedTestText);
            } else {
                // Dark for untested
                if (addr % 2 == 0) {
                    item->setBackground(colors.untestedBgEven);
                } else {
                    item->setBackground(colors.untestedBgOdd);
                }
                item->setForeground(colors.statusUntestedText);
            }
        }
        // If hasFailedTest is true, red highlighting will be applied in applyFailedTestHighlighting
    }
}

void MainWindow::clearLog() {
    _logger->clear();
    _logger->info("Журнал очищен.");
}

void MainWindow::exportResults() {
    if (_lastResults.empty()) {
        _logger->warning("Нет результатов для экспорта. Сначала запустите тест.");
        QMessageBox::information(this, "Экспорт", "Нет результатов для экспорта. Сначала запустите тест.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт результатов", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        _logger->error(QString("Не удалось открыть файл для записи: %1").arg(fileName));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "\xEF\xBB\xBF"; // UTF-8 BOM for Excel compatibility
    out << "Address,Expected (HEX),Read (HEX),Expected (Decimal),Read (Decimal),Status\n";

    for (const auto& r : _lastResults) {
        out << r.addr << ","
            << QString("0x%1").arg(r.expected, 8, 16, QChar('0')).toUpper() << ","
            << QString("0x%1").arg(r.read, 8, 16, QChar('0')).toUpper() << ","
            << r.expected << ","
            << r.read << ","
            << (r.passed ? "PASS" : "FAIL") << "\n";
    }

    file.close();
    _logger->success(QString("Результаты экспортированы в файл: %1").arg(fileName));
}

void MainWindow::scrollToNextFault() {
    // Проверяем, были ли запущены тесты
    if (_lastResults.empty()) {
        _logger->warning("Тесты еще не запускались. Сначала запустите тест памяти.");
        QMessageBox::information(this, "Тесты не запущены", 
                                "Тесты еще не запускались.\nСначала запустите тест памяти.");
        return;
    }
    
    // Собираем все адреса с неисправностями в отсортированный список
    // ВАЖНО: Используем актуальные результаты из _lastResults
    std::set<size_t> faultAddresses;
    for (const auto& r : _lastResults) {
        if (!r.passed) {
            faultAddresses.insert(r.addr);
        }
    }
    
    // Если неисправностей нет
    if (faultAddresses.empty()) {
        _logger->warning("Неисправности не найдены тестами. Все проверки прошли успешно.");
        QMessageBox::information(this, "Неисправности не найдены", 
                                "Тестами не обнаружено неисправностей.\nВсе проверки прошли успешно.");
        return;
    }
    
    // Получаем текущую выбранную строку
    int currentRow = _table->currentRow();
    size_t startAddr = 0;
    
    // Если есть выбранная строка, используем её адрес
    if (currentRow >= 0 && currentRow < int(_mem->size())) {
        startAddr = static_cast<size_t>(currentRow);
    }
    
    size_t nextAddr = 0;
    bool found = false;
    
    // Ищем следующую неисправность, начиная со следующего адреса после текущего
    // Используем upper_bound чтобы найти первый адрес строго больше текущего
    auto it = faultAddresses.upper_bound(startAddr);
    
    if (it != faultAddresses.end()) {
        // Нашли неисправность после текущей позиции
        nextAddr = *it;
        found = true;
    } else {
        // Дошли до конца, переходим к началу (циклический поиск)
        it = faultAddresses.begin();
        if (it != faultAddresses.end()) {
            nextAddr = *it;
            // Если мы уже на неисправном адресе и есть другие неисправности, переходим к следующей
            if (nextAddr == startAddr && faultAddresses.size() > 1) {
                ++it;
                if (it != faultAddresses.end()) {
                    nextAddr = *it;
                } else {
                    // Если следующей нет, берем первую
                    nextAddr = *faultAddresses.begin();
                }
            }
            found = true;
        }
    }
    
    if (!found) {
        _logger->warning("Не удалось найти следующую неисправность.");
        return;
    }
    
    // ВАЖНО: Проверяем, что найденный адрес действительно неисправен в текущих результатах
    bool isActuallyFaulty = false;
    for (const auto& r : _lastResults) {
        if (r.addr == nextAddr && !r.passed) {
            isActuallyFaulty = true;
            break;
        }
    }
    
    if (!isActuallyFaulty) {
        _logger->warning(QString("Адрес %1 больше не является неисправным. Обновляю таблицу...").arg(nextAddr));
        refreshTable(0, _mem->size());
        // Пытаемся найти следующую неисправность снова
        return scrollToNextFault();
    }
    
    // Прокручиваем к найденной неисправности
    int row = int(nextAddr);
    if (row >= 0 && row < _table->rowCount()) {
        _table->setCurrentCell(row, 0);
        _table->scrollTo(_table->model()->index(row, 0), QAbstractItemView::EnsureVisible);
        _table->selectRow(row);
        _logger->info(QString("Переход к следующей неисправности по адресу: %1").arg(nextAddr));
    }
}

void MainWindow::onAlgorithmChanged(int index) {
    // Index parameter is part of Qt signal signature
    Q_UNUSED(index);
    _currentAlgorithmLabel->setText(QString("Текущий алгоритм: %1").arg(_algoCombo->currentText()));
    updateTestInfo();
}

void MainWindow::onFaultModelChanged(int index) {
    // Automatically set default probability based on fault model
    FaultModel model = static_cast<FaultModel>(_faultCombo->itemData(index).toInt());
    if (model == FaultModel::BitFlip) {
        _flipProbSpin->setValue(0.010);
    } else if (model != FaultModel::None) {
        // For other fault models (StuckAt0, StuckAt1, OpenRead), use 0.110
        _flipProbSpin->setValue(0.110);
    }
    // For FaultModel::None, keep current value
}

void MainWindow::updateStatistics() {
    _totalAddressesLabel->setText(QString("Всего адресов: %1").arg(_mem->size()));

    int testedCount = _lastResults.size();
    int faultsFound = 0;
    for (const auto& r : _lastResults) {
        if (!r.passed) ++faultsFound;
    }

    _testedAddressesLabel->setText(QString("Протестировано: %1").arg(testedCount));
    _faultsFoundLabel->setText(QString("Найдено неисправностей: %1").arg(faultsFound));

    double coverage = _mem->size() > 0 ? (testedCount * 100.0 / _mem->size()) : 0.0;
    _coverageLabel->setText(QString("Покрытие: %1%").arg(coverage, 0, 'f', 1));

    auto f = _mem->currentFault();
    _currentFaultModelLabel->setText(QString("Текущая модель: %1").arg(DataFormatter::getFaultModelName(f.model)));
}


void MainWindow::updateFaultInfo() {
    auto f = _mem->currentFault();
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    if (f.model == FaultModel::None) {
        _faultInfoLabel->setText("Неисправность не внедрена");
        _faultInfoLabel->setStyleSheet(QString("padding: 5px; background-color: %1; border: 1px solid %2; color: %3;")
                                       .arg(colors.bgSecondary.name())
                                       .arg(colors.accent.name())
                                       .arg(colors.text.name()));
    } else {
        QString info = QString("Тип: %1\nАдрес: %2\nДлина: %3 слов")
                       .arg(DataFormatter::getFaultModelName(f.model))
                       .arg(f.addr)
                       .arg(f.len);
        if (f.model == FaultModel::BitFlip) {
            info += QString("\nВероятность инверсии: %1%").arg(f.flip_probability * PROGRESS_MAX_PERCENT, 0, 'f', 1);
        }
        _faultInfoLabel->setText(info);
        _faultInfoLabel->setStyleSheet(QString("padding: 5px; background-color: %1; border: 1px solid %2; color: %3;")
                                       .arg(colors.faultyNotTestedBg.name())
                                       .arg(colors.accent.name())
                                       .arg(colors.text.name()));
    }
    updateStatistics();
}

void MainWindow::updateTestInfo() {
    TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
    QString desc = DataFormatter::getAlgorithmDescription(algo);
    _testInfoLabel->setText(QString("Алгоритм: %1\n\n%2").arg(_algoCombo->currentText()).arg(desc));
}


void MainWindow::highlightCurrentAddress(size_t addr) {
    _currentTestAddr = addr;
    _currentAddrLabel->setText(QString("Адрес: %1").arg(addr));
    if (addr < _mem->size() && int(addr) < _table->rowCount()) {
        _table->selectRow(int(addr));
        _table->scrollToItem(_table->item(int(addr), 0), QAbstractItemView::EnsureVisible);
    }
}

void MainWindow::updateProgressDetails(size_t addr, Word expected, Word read) {
    highlightCurrentAddress(addr);
    _expectedValueLabel->setText(QString("Ожидается: 0x%1").arg(expected, 8, 16, QChar('0')).toUpper());
    _readValueLabel->setText(QString("Прочитано: 0x%1").arg(read, 8, 16, QChar('0')).toUpper());

    // Оптимизация: обновляем таблицу реже и не прокручиваем при каждом обновлении
    // Прокручиваем только каждые 20 адресов или если адрес близок к краю видимой области
    if (addr < _mem->size() && int(addr) < _table->rowCount()) {
        // Обновляем выделение только если адрес изменился значительно
        if (addr != _lastHighlightedAddr) {
            // Clear previous bold highlighting (but keep background colors)
            if (_lastHighlightedAddr < _mem->size() && int(_lastHighlightedAddr) < _table->rowCount()) {
                for (int col = 0; col < _table->columnCount(); ++col) {
                    QTableWidgetItem* item = _table->item(int(_lastHighlightedAddr), col);
                    if (item) {
                        QFont font = item->font();
                        font.setBold(false);
                        item->setFont(font);
                    }
                }
            }

            // Highlight current address with bold font
            ThemeColors colors = ThemeManager::getColors(_currentTheme);
            for (int col = 0; col < _table->columnCount(); ++col) {
                QTableWidgetItem* item = _table->item(int(addr), col);
                if (item) {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                    // Don't change background if it's a failed test or faulty area
                    // ИСПРАВЛЕНИЕ: Используем rgb() для сравнения цветов вместо прямого сравнения
                    QColor currentBg = item->background().color();
                    QRgb currentBgRgb = currentBg.rgb();
                    QRgb failedTestBgRgb = colors.failedTestBg.rgb();
                    QRgb faultyNotTestedBgRgb = colors.faultyNotTestedBg.rgb();
                    bool isSpecialColor = (currentBgRgb == failedTestBgRgb || currentBgRgb == faultyNotTestedBgRgb);
                    
#ifdef DEBUG
                    // Debug logging: Log if overwriting color
                    if (!isSpecialColor && col != 4) {
                        // Check if this address is faulty
                        bool isFailedAddr = false;
                        for (const auto& r : _lastResults) {
                            if (r.addr == addr && !r.passed) {
                                isFailedAddr = true;
                                break;
                            }
                        }
                        if (isFailedAddr) {
                            _logger->warning(QString("updateProgressDetails: ПЕРЕЗАПИСЬ! Адрес %1, колонка %2 - текущий фон %3, но адрес неисправен!")
                                    .arg(addr).arg(col).arg(currentBg.name()));
                        }
                    }
#endif
                    
                    if (!isSpecialColor) {
                        // Use standard table colors from theme
                        if (addr % 2 == 0) {
                            item->setBackground(colors.tableBgEven);
                        } else {
                            item->setBackground(colors.tableBgOdd);
                        }
                        item->setForeground(colors.tableText);
                    }
#ifdef DEBUG
                    else {
                        // Debug logging: Log that we're preserving special color
                        _logger->info(QString("updateProgressDetails: Адрес %1, колонка %2 - сохраняю специальный цвет %3")
                                .arg(addr).arg(col).arg(currentBg.name()));
                    }
#endif
                }
            }
            _lastHighlightedAddr = addr;

            // Scroll only every N addresses for performance
            if (addr % TABLE_SCROLL_INTERVAL == 0) {
                _table->scrollToItem(_table->item(int(addr), 0), QAbstractItemView::EnsureVisible);
            }
        }
    }
}


QString MainWindow::getThemeStylesheet(Theme theme) {
    return ThemeManager::getStylesheet(theme);
}

void MainWindow::applyTheme(Theme theme) {
    _currentTheme = theme;
    
    // Update logger theme
    if (_logger) {
        _logger->setTheme(theme);
    }
    
    // Apply stylesheet to main window
    setStyleSheet(getThemeStylesheet(theme));
    
    // Update table delegate theme
    if (_tableDelegate) {
        _tableDelegate->setTheme(theme);
    }
    
    // Update fault info label with theme-specific colors
    updateFaultInfo();
    
    // Update test info label with theme-specific colors
    ThemeColors colors = ThemeManager::getColors(theme);
    _testInfoLabel->setStyleSheet(QString("padding: 5px; background-color: %1; border: 1px solid %2; color: %3;")
                                  .arg(colors.bgSecondary.name())
                                  .arg(colors.accent.name())
                                  .arg(colors.text.name()));
    
    // Устанавливаем базовый цвет текста для журнала и перекрашиваем весь существующий текст
    QColor logTextColor = colors.logInfo;
    
    // Сохраняем весь текст из журнала
    QString logText = _log->toPlainText();
    
    // Перекрашиваем весь текст в журнале
    if (!logText.isEmpty()) {
        // Перекрашиваем сообщения по типам
        QStringList lines = logText.split('\n');
        _log->clear();
        _log->setTextColor(logTextColor); // Базовый цвет
        
        for (const QString& line : lines) {
            if (line.isEmpty()) continue;
            
            // Определяем тип сообщения по содержимому строки
            if (line.contains("INFO:")) {
                _log->setTextColor(colors.logInfo);
            } else if (line.contains("WARNING:")) {
                _log->setTextColor(colors.logWarning);
            } else if (line.contains("ERROR:")) {
                _log->setTextColor(colors.logError);
            } else if (line.contains("SUCCESS:")) {
                _log->setTextColor(colors.logSuccess);
            } else {
                _log->setTextColor(logTextColor); // Базовый цвет для неизвестных типов
            }
            _log->append(line);
        }
    } else {
        // Если журнал пуст, просто устанавливаем базовый цвет
        _log->setTextColor(logTextColor);
    }
    
    // Refresh table to apply theme colors
    refreshTable(0, _mem->size());
}

void MainWindow::applyFailedTestHighlighting(size_t addr, const ThemeColors& colors) {
    if (!_table) return;
    // Apply red highlighting to ALL columns in the row when test fails
    for (int col = 0; col < _table->columnCount(); ++col) {
        QTableWidgetItem* item = createOrGetTableItem(int(addr), col);
        if (!item) continue;
        // Force red highlighting for all columns
        QBrush redBrush(colors.failedTestBg);
        QBrush whiteBrush(colors.failedTestText);
        // Set via setData for guaranteed setting
        item->setData(Qt::BackgroundRole, redBrush);
        item->setData(Qt::ForegroundRole, whiteBrush);
        // Also set via setBackground/setForeground
        item->setBackground(redBrush);
        item->setForeground(whiteBrush);
        // Mark item as failed for delegate
        item->setData(Qt::UserRole + 1, QVariant(true)); // Mark as failed item
    }
}

void MainWindow::onThemeChanged() {
    QAction* action = _themeGroup->checkedAction();
    if (action == _deusExAction) {
        applyTheme(Theme::DeusEx);
    } else if (action == _matrixAction) {
        applyTheme(Theme::Matrix);
    } else if (action == _gurrenLagannAction) {
        applyTheme(Theme::GurrenLagann);
    } else if (action == _cyberpunkAction) {
        applyTheme(Theme::Cyberpunk);
    }
}