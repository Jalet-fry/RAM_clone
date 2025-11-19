#include "mainwindow.h"
#include "tableitemdelegate.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QIntValidator>
#include <algorithm>
#include <cmath>
#include <set>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), _currentTestAddr(0), _lastHighlightedAddr(0), _testRunning(false) {
    setWindowTitle("Цифровой двойник ОЗУ — 350504 Витовт Даник Маша Настя");
    resize(1400, 800);

    _mem = new MemoryModel(256, this);
    _worker = new TesterWorker(_mem);
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
    _lenEdit = new QLineEdit("100");
    _lenEdit->setToolTip("Количество слов с неисправностью");
    _lenEdit->setValidator(new QIntValidator(1, 256, this));
    lenLayout->addWidget(_lenEdit);
    faultLayout->addLayout(lenLayout);

    QHBoxLayout* probLayout = new QHBoxLayout;
    probLayout->addWidget(new QLabel("Вероятность инверсии:"));
    _flipProbSpin = new QDoubleSpinBox;
    _flipProbSpin->setRange(0.0, 1.0);
    _flipProbSpin->setSingleStep(0.01);
    _flipProbSpin->setValue(0.110);
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
    _progress->setRange(0, 100);
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

    _totalAddressesLabel = new QLabel("Всего адресов: 256");
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
        bool ok;
        int addr = _searchEdit->text().toInt(&ok);
        if (ok && addr >= 0 && addr < int(_mem->size())) {
            _table->selectRow(addr);
            _table->scrollToItem(_table->item(addr, 0), QAbstractItemView::EnsureVisible);
        }
    });
    
    // Исправленные соединения для Qt 5.5.1
    connect(_algoCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgorithmChanged(int)));
    connect(_faultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFaultInfo()));

    // Worker signals
    connect(_worker, &TesterWorker::progress, _progress, &QProgressBar::setValue);
    connect(_worker, &TesterWorker::progressDetail, this, &MainWindow::updateProgressDetails);
    connect(_worker, &TesterWorker::finished, this, &MainWindow::onTestFinished);

    // Memory signals
    connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);
    connect(_mem, &MemoryModel::faultInjected, this, &MainWindow::updateFaultInfo);

    // Initialize
    refreshTable(0, _mem->size());
    updateFaultInfo();
    updateStatistics();
    onAlgorithmChanged(0);
    
    // Apply default theme (Deus Ex)
    _currentTheme = Theme::DeusEx;
    applyTheme(Theme::DeusEx);
    
    logInfo("Программа запущена. Готов к работе.");
}

MainWindow::~MainWindow() {
    if (_worker) {
        QMetaObject::invokeMethod(_worker, "deleteLater", Qt::QueuedConnection);
        _worker = nullptr;
    }
}

void MainWindow::onInject() {
    InjectedFault f;
    f.model = static_cast<FaultModel>(_faultCombo->currentData().toInt());
    bool ok1 = false, ok2 = false;
    size_t addr = _addrEdit->text().toULongLong(&ok1);
    size_t len = _lenEdit->text().toULongLong(&ok2);

    if (!ok1 || !ok2) {
        logError("Ошибка ввода: Адрес и длина должны быть числами.");
        QMessageBox::warning(this, "Ошибка ввода", "Адрес и длина должны быть числами.");
        return;
    }

    if (addr >= _mem->size()) {
        logError(QString("Ошибка: Адрес %1 выходит за пределы памяти (0-%2)").arg(addr).arg(_mem->size() - 1));
        QMessageBox::warning(this, "Ошибка", QString("Адрес должен быть в диапазоне 0-%1").arg(_mem->size() - 1));
        return;
    }

    if (addr + len > _mem->size()) {
        len = _mem->size() - addr;
        logWarning(QString("Длина уменьшена до %1 (выход за пределы памяти)").arg(len));
    }

    f.addr = addr;
    f.len = std::max<size_t>(1, len);
    f.flip_probability = _flipProbSpin->value();

    _mem->injectFault(f);
    logSuccess(QString("Внедрена неисправность: %1 по адресу %2, длина=%3")
               .arg(getFaultModelName(f.model)).arg(addr).arg(f.len));
    updateFaultInfo();
    updateStatistics();
}

void MainWindow::onReset() {
    _mem->reset();
    _lastResults.clear();
    _testRunning = false;
    _lastHighlightedAddr = 0;
    _progress->setValue(0);
    _currentAddrLabel->setText("Адрес: —");
    _expectedValueLabel->setText("Ожидается: —");
    _readValueLabel->setText("Прочитано: —");
    logInfo("Память сброшена. Все данные очищены, неисправности удалены.");
    updateFaultInfo();
    updateStatistics();
    refreshTable(0, _mem->size());
}

void MainWindow::onStartTest() {
    if (_testRunning) {
        logWarning("Тест уже выполняется. Дождитесь завершения.");
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

    // ОТКЛЮЧАЕМ обновление таблицы во время теста для производительности
    // Таблица будет обновлена только в конце теста
    disconnect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);

    logInfo(QString("Запуск теста: %1").arg(_algoCombo->currentText()));
    updateTestInfo();

    QMetaObject::invokeMethod(_worker, "run", Qt::QueuedConnection, Q_ARG(TestAlgorithm, algo));
}

void MainWindow::onTestFinished(const std::vector<TestResult>& results) {
    _testRunning = false;
    _startBtn->setEnabled(true);
    _testTimer->stop();
    _lastResults = results;

    // ВКЛЮЧАЕМ обратно обновление таблицы после завершения теста
    connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);

    int fails = 0;
    for (const auto& r : results) if (!r.passed) ++fails;

    int elapsed = _testStartTime.msecsTo(QTime::currentTime());
    QString timeStr = QString("%1.%2 сек").arg(elapsed / 1000).arg((elapsed % 1000) / 100, 2, 10, QChar('0'));

    if (fails == 0) {
        logSuccess(QString("Тест завершен успешно. Всего проверок: %1, неисправностей не обнаружено. Время: %2")
                   .arg(results.size()).arg(timeStr));
    } else {
        logError(QString("Тест завершен. Всего проверок: %1, обнаружено неисправностей: %2. Время: %3")
                 .arg(results.size()).arg(fails).arg(timeStr));
    }

    // Refresh table to show updated values and test results
    refreshTable(0, _mem->size());

    updateStatistics();
    _testTimeLabel->setText(QString("Время теста: %1").arg(timeStr));
}

void MainWindow::refreshTable(size_t begin, size_t end) {
    Q_UNUSED(begin);
    Q_UNUSED(end);
    
    // Оптимизация: отключаем обновление виджета во время массовых изменений
    _table->setUpdatesEnabled(false);
    
    size_t n = _mem->size();
    _table->setRowCount(int(n));

    auto f = _mem->currentFault();
    std::set<size_t> testedAddresses;
    for (const auto& r : _lastResults) {
        testedAddresses.insert(r.addr);
    }

    // Получаем цвета один раз для всей таблицы (оптимизация)
    ThemeColors colors = ThemeManager::getColors(_currentTheme);

    // ЛОГИРОВАНИЕ: Подсчитываем количество неисправных адресов
    std::set<size_t> failedAddressesForLog;
    for (const auto& r : _lastResults) {
        if (!r.passed) {
            failedAddressesForLog.insert(r.addr);
        }
    }
    logInfo(QString("refreshTable: Всего результатов: %1, неисправных адресов: %2")
            .arg(_lastResults.size()).arg(failedAddressesForLog.size()));
    logInfo(QString("refreshTable: Цвета для красного выделения - фон: %1, текст: %2")
            .arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));

    for (size_t i = 0; i < n; ++i) {
        // Address
        QTableWidgetItem* addrItem = _table->item(int(i), 0);
        if (!addrItem) {
            addrItem = new QTableWidgetItem;
            _table->setItem(int(i), 0, addrItem);
        }
        addrItem->setText(QString::number(i));
        addrItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        // Начальный цвет будет перезаписан в цикле, но для непротестированных строк в DeusEx используем accent
        addrItem->setForeground(colors.tableText);

        Word v = _mem->read(i);
        bool isFaulty = (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len);
        bool isTested = testedAddresses.find(i) != testedAddresses.end();

        // HEX
        QTableWidgetItem* hexItem = _table->item(int(i), 1);
        if (!hexItem) {
            hexItem = new QTableWidgetItem;
            _table->setItem(int(i), 1, hexItem);
        }
        hexItem->setText(QString("0x%1").arg(v, 8, 16, QChar('0')).toUpper());
        hexItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hexItem->setForeground(colors.tableText);

        // Binary
        QTableWidgetItem* binItem = _table->item(int(i), 2);
        if (!binItem) {
            binItem = new QTableWidgetItem;
            _table->setItem(int(i), 2, binItem);
        }
        binItem->setText(formatBinary(v));
        binItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        binItem->setFont(QFont("Courier", 9));
        binItem->setForeground(colors.tableText);

        // Decimal
        QTableWidgetItem* decItem = _table->item(int(i), 3);
        if (!decItem) {
            decItem = new QTableWidgetItem;
            _table->setItem(int(i), 3, decItem);
        }
        decItem->setText(QString::number(v));
        decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        decItem->setForeground(colors.tableText);

        // Определяем состояние строки ДО применения цветов - проверяем ВСЕ результаты
        // ВАЖНО: Проверяем все результаты, не только первый найденный
        bool hasFailedTest = false;
        bool passed = true;
        bool foundResult = false;
        for (const auto& r : _lastResults) {
            if (r.addr == i) {
                foundResult = true;
                passed = r.passed;
                if (!r.passed) {
                    hasFailedTest = true;
                    break; // Нашли неисправность, можно прервать
                }
            }
        }
        // Если результат не найден, но адрес в testedAddresses, значит он протестирован и прошел
        if (!foundResult && isTested) {
            passed = true; // По умолчанию считаем прошедшим, если нет результата
        }

        // Status - используем правильную логику определения статуса
        QTableWidgetItem* statusItem = _table->item(int(i), 4);
        if (!statusItem) {
            statusItem = new QTableWidgetItem;
            _table->setItem(int(i), 4, statusItem);
        }
        
        // ВАЖНОЕ ИСПРАВЛЕНИЕ: Всегда сбрасываем шрифт и очищаем тултипы
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
        QTableWidgetItem* faultTypeItem = _table->item(int(i), 5);
        if (!faultTypeItem) {
            faultTypeItem = new QTableWidgetItem;
            _table->setItem(int(i), 5, faultTypeItem);
        }
        
        // ВАЖНОЕ ИСПРАВЛЕНИЕ: Всегда сбрасываем шрифт и очищаем тултипы
        faultTypeItem->setFont(font);
        faultTypeItem->setToolTip("");
        
        if (isFaulty) {
            faultTypeItem->setText(getFaultModelName(f.model));
            faultTypeItem->setForeground(colors.faultyNotTestedText);
        } else if (isTested) {
            // Check if fault was detected
            bool faultDetected = false;
            for (const auto& r : _lastResults) {
                if (r.addr == i && !r.passed) {
                    faultDetected = true;
                    faultTypeItem->setToolTip(QString("Ожидалось: 0x%1, Прочитано: 0x%2")
                                              .arg(r.expected, 8, 16, QChar('0'))
                                              .arg(r.read, 8, 16, QChar('0')));
                    break;
                }
            }
            if (!faultDetected) {
                faultTypeItem->setText("—");
                faultTypeItem->setForeground(colors.tableText);
            } else {
                faultTypeItem->setText("Обнаружена");
                faultTypeItem->setForeground(colors.failedTestText);
            }
        } else {
            faultTypeItem->setText("—");
            faultTypeItem->setForeground(colors.statusUntestedText);
        }

        // Применяем цвета в зависимости от состояния строки
        // ВАЖНО: Сначала применяем все обычные цвета, потом КРАСНОЕ выделение в ОТДЕЛЬНОМ цикле
        for (int col = 0; col < _table->columnCount(); ++col) {
            QTableWidgetItem* item = _table->item(int(i), col);
            if (!item) continue;

            // Сбрасываем жирный шрифт
            QFont itemFont = item->font();
            itemFont.setBold(false);
            item->setFont(itemFont);

            // Очищаем тултипы (кроме колонки типа неисправности)
            if (col != 5) {
                item->setToolTip("");
            }

            // Применяем обычные цвета ТОЛЬКО если нет неисправности
            // НЕ применяем красное выделение здесь - оно будет применено отдельно
            if (col != 4) {
                if (!hasFailedTest) {
                    // Сбрасываем флаг неисправности для делегата
                    item->setData(Qt::UserRole + 1, QVariant(false));
                    
                    // Золотистый для неисправной области (еще не тестировалась)
                    if (isFaulty && !isTested && col != 5) {
                        item->setBackground(colors.faultyNotTestedBg);
                        item->setForeground(colors.faultyNotTestedText);
                    }
                    // Зеленый для протестированных и исправных
                    else if (isTested && col != 5) {
                        item->setBackground(colors.passedTestBg);
                        item->setForeground(colors.passedTestText);
                    }
                    // Темный для непротестированных
                    else if (!isTested && col != 5) {
                        if (i % 2 == 0) {
                            item->setBackground(colors.untestedBgEven);
                        } else {
                            item->setBackground(colors.untestedBgOdd);
                        }
                        // Используем цвет для непротестированных из темы
                        item->setForeground(colors.statusUntestedText);
                    }
                    // Стандартные цвета для первых 4 колонок (fallback)
                    else if (col < 4) {
                        if (i % 2 == 0) {
                            item->setBackground(colors.tableBgEven);
                        } else {
                            item->setBackground(colors.tableBgOdd);
                        }
                        item->setForeground(colors.tableText);
                    }
                }
                // Если есть неисправность, НЕ применяем обычные цвета - они будут перезаписаны красным
            }
        }

        // КРАСНОЕ ВЫДЕЛЕНИЕ - АБСОЛЮТНЫЙ ПРИОРИТЕТ (применяется в ОТДЕЛЬНОМ цикле ПОСЛЕ всех остальных)
        // Применяется ко всем колонкам кроме статуса (колонка 4) и типа неисправности (колонка 5)
        // ВАЖНО: Это должно быть ПОСЛЕ основного цикла, чтобы перезаписать любые другие цвета
        if (hasFailedTest) {
            // ЛОГИРОВАНИЕ: Логируем применение красного выделения
            logInfo(QString("refreshTable: Адрес %1 - hasFailedTest=true, применяю красное выделение (фон: %2, текст: %3)")
                    .arg(i).arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));
            
            // Применяем красное выделение ко всем колонкам строки (кроме статуса)
            for (int col = 0; col < _table->columnCount(); ++col) {
                if (col == 4) continue; // Пропускаем колонку статуса
                QTableWidgetItem* item = _table->item(int(i), col);
                if (!item) {
                    // Если элемента нет, создаем его
                    item = new QTableWidgetItem;
                    _table->setItem(int(i), col, item);
                }
                // Принудительно устанавливаем красное выделение
                QBrush redBrush(colors.failedTestBg);
                QBrush whiteBrush(colors.failedTestText);
                // Устанавливаем через setData для гарантированной установки
                item->setData(Qt::BackgroundRole, redBrush);
                item->setData(Qt::ForegroundRole, whiteBrush);
                // Также устанавливаем через setBackground/setForeground
                item->setBackground(redBrush);
                item->setForeground(whiteBrush);
                // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Помечаем элемент как неисправный для делегата
                // Делегат будет использовать этот флаг для правильной отрисовки красного выделения
                item->setData(Qt::UserRole + 1, QVariant(true)); // Помечаем как неисправный элемент
            }
        } else if (failedAddressesForLog.count(i) > 0) {
            // ЛОГИРОВАНИЕ: Если адрес в списке неисправных, но hasFailedTest=false - это ошибка!
            logError(QString("refreshTable: ОШИБКА! Адрес %1 в списке неисправных, но hasFailedTest=false!")
                    .arg(i));
        }
    }

    // ВАЖНО: Применяем красное выделение ЕЩЕ РАЗ после всех обновлений
    // Это гарантирует, что оно не будет перезаписано
    // Сначала собираем все адреса с неисправностями
    std::set<size_t> failedAddresses;
    for (const auto& r : _lastResults) {
        if (!r.passed) {
            failedAddresses.insert(r.addr);
        }
    }
    
    // Теперь применяем красное выделение ко всем неисправным адресам
    logInfo(QString("refreshTable: Второй проход - применяю красное выделение к %1 адресам")
            .arg(failedAddresses.size()));
    for (size_t addr : failedAddresses) {
        if (addr >= n) continue; // Проверка границ
        
        // Применяем красное выделение ко всем колонкам строки (кроме статуса)
        for (int col = 0; col < _table->columnCount(); ++col) {
            if (col == 4) continue; // Пропускаем колонку статуса
            QTableWidgetItem* item = _table->item(int(addr), col);
            if (!item) {
                // Если элемента нет, создаем его
                item = new QTableWidgetItem;
                _table->setItem(int(addr), col, item);
            }
            // Принудительно устанавливаем красное выделение
            QBrush redBrush(colors.failedTestBg);
            QBrush whiteBrush(colors.failedTestText);
            // Устанавливаем через setData для гарантированной установки
            item->setData(Qt::BackgroundRole, redBrush);
            item->setData(Qt::ForegroundRole, whiteBrush);
            // Также устанавливаем через setBackground/setForeground
            item->setBackground(redBrush);
            item->setForeground(whiteBrush);
            // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Помечаем элемент как неисправный для делегата
            // Делегат будет использовать этот флаг для правильной отрисовки красного выделения
            item->setData(Qt::UserRole + 1, QVariant(true)); // Помечаем как неисправный элемент
        }
    }

    // Включаем обновление виджета обратно
    _table->setUpdatesEnabled(true);
    
    // ЛОГИРОВАНИЕ: Проверяем фактические цвета элементов после применения
    int verifiedCount = 0;
    int mismatchCount = 0;
    for (size_t addr : failedAddresses) {
        if (addr >= n) continue;
        bool allCorrect = true;
        for (int col = 0; col < _table->columnCount(); ++col) {
            if (col == 4) continue; // Пропускаем колонку статуса
            QTableWidgetItem* item = _table->item(int(addr), col);
            if (item) {
                QColor actualBg = item->background().color();
                QColor expectedBg = colors.failedTestBg;
                if (actualBg.rgb() != expectedBg.rgb()) {
                    allCorrect = false;
                    logError(QString("refreshTable: НЕСООТВЕТСТВИЕ! Адрес %1, колонка %2 - ожидался фон %3, фактический %4")
                            .arg(addr).arg(col).arg(expectedBg.name()).arg(actualBg.name()));
                    mismatchCount++;
                }
            }
        }
        if (allCorrect) {
            verifiedCount++;
        }
    }
    logInfo(QString("refreshTable: Проверка завершена - правильно окрашено: %1, несоответствий: %2")
            .arg(verifiedCount).arg(mismatchCount));
    
    // Оптимизация: resizeColumnsToContents очень медленный, вызываем только при необходимости
    // Можно вызывать реже или только при изменении размера окна
    // _table->resizeColumnsToContents(); // Отключено для производительности
}

void MainWindow::clearLog() {
    _log->clear();
    logInfo("Журнал очищен.");
}

void MainWindow::exportResults() {
    if (_lastResults.empty()) {
        logWarning("Нет результатов для экспорта. Сначала запустите тест.");
        QMessageBox::information(this, "Экспорт", "Нет результатов для экспорта. Сначала запустите тест.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт результатов", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logError(QString("Не удалось открыть файл для записи: %1").arg(fileName));
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
    logSuccess(QString("Результаты экспортированы в файл: %1").arg(fileName));
}

void MainWindow::scrollToNextFault() {
    // Проверяем, были ли запущены тесты
    if (_lastResults.empty()) {
        logWarning("Тесты еще не запускались. Сначала запустите тест памяти.");
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
        logWarning("Неисправности не найдены тестами. Все проверки прошли успешно.");
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
        logWarning("Не удалось найти следующую неисправность.");
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
        logWarning(QString("Адрес %1 больше не является неисправным. Обновляю таблицу...").arg(nextAddr));
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
        logInfo(QString("Переход к следующей неисправности по адресу: %1").arg(nextAddr));
    }
}

void MainWindow::onAlgorithmChanged(int index) {
    Q_UNUSED(index);
    _currentAlgorithmLabel->setText(QString("Текущий алгоритм: %1").arg(_algoCombo->currentText()));
    updateTestInfo();
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
    _currentFaultModelLabel->setText(QString("Текущая модель: %1").arg(getFaultModelName(f.model)));
}

void MainWindow::logInfo(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    _log->setTextColor(colors.logInfo);
    _log->append(QString("[%1] INFO: %2").arg(timestamp).arg(message));
}

void MainWindow::logWarning(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    _log->setTextColor(colors.logWarning);
    _log->append(QString("[%1] WARNING: %2").arg(timestamp).arg(message));
}

void MainWindow::logError(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    _log->setTextColor(colors.logError);
    _log->append(QString("[%1] ERROR: %2").arg(timestamp).arg(message));
}

void MainWindow::logSuccess(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    _log->setTextColor(colors.logSuccess);
    _log->append(QString("[%1] SUCCESS: %2").arg(timestamp).arg(message));
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
                       .arg(getFaultModelName(f.model))
                       .arg(f.addr)
                       .arg(f.len);
        if (f.model == FaultModel::BitFlip) {
            info += QString("\nВероятность инверсии: %1%").arg(f.flip_probability * 100, 0, 'f', 1);
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
    QString desc = getAlgorithmDescription(algo);
    _testInfoLabel->setText(QString("Алгоритм: %1\n\n%2").arg(_algoCombo->currentText()).arg(desc));
}

void MainWindow::updateMemoryCellStatus(size_t addr) {
    // This method can be used for future enhancements
    Q_UNUSED(addr);
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
                    
                    // ЛОГИРОВАНИЕ: Логируем, если перезаписываем цвет
                    if (!isSpecialColor && col != 4) {
                        // Проверяем, является ли этот адрес неисправным
                        bool isFailedAddr = false;
                        for (const auto& r : _lastResults) {
                            if (r.addr == addr && !r.passed) {
                                isFailedAddr = true;
                                break;
                            }
                        }
                        if (isFailedAddr) {
                            logWarning(QString("updateProgressDetails: ПЕРЕЗАПИСЬ! Адрес %1, колонка %2 - текущий фон %3, но адрес неисправен!")
                                    .arg(addr).arg(col).arg(currentBg.name()));
                        }
                    }
                    
                    if (!isSpecialColor) {
                        // Используем стандартные цвета таблицы из темы
                        if (addr % 2 == 0) {
                            item->setBackground(colors.tableBgEven);
                        } else {
                            item->setBackground(colors.tableBgOdd);
                        }
                        item->setForeground(colors.tableText);
                    } else {
                        // ЛОГИРОВАНИЕ: Логируем, что сохраняем специальный цвет
                        logInfo(QString("updateProgressDetails: Адрес %1, колонка %2 - сохраняю специальный цвет %3")
                                .arg(addr).arg(col).arg(currentBg.name()));
                    }
                }
            }
            _lastHighlightedAddr = addr;

            // Прокручиваем только каждые 20 адресов для производительности
            if (addr % 20 == 0) {
                _table->scrollToItem(_table->item(int(addr), 0), QAbstractItemView::EnsureVisible);
            }
        }
    }
}

QString MainWindow::formatBinary(Word value) {
    QString binary;
    for (int i = 31; i >= 0; --i) {
        binary += (value & (1u << i)) ? '1' : '0';
        if (i % 4 == 0 && i > 0) binary += ' ';
    }
    return binary;
}

QString MainWindow::getFaultModelName(FaultModel model) {
    switch (model) {
        case FaultModel::None: return "Нет";
        case FaultModel::StuckAt0: return "Stuck-at-0";
        case FaultModel::StuckAt1: return "Stuck-at-1";
        case FaultModel::BitFlip: return "Bit-flip";
        case FaultModel::OpenRead: return "Open (invalid read)";
        default: return "Неизвестно";
    }
}

QString MainWindow::getAlgorithmDescription(TestAlgorithm algo) {
    switch (algo) {
        case TestAlgorithm::WalkingOnes:
            return "Записывает единицу в каждый бит позиции и проверяет, что она сохраняется. "
                   "Обнаруживает залипания битов и ошибки чтения/записи.";
        case TestAlgorithm::WalkingZeros:
            return "Записывает ноль в каждый бит позиции и проверяет, что он сохраняется. "
                   "Обнаруживает залипания битов и ошибки чтения/записи.";
        case TestAlgorithm::MarchSimple:
            return "Простой маршевый тест: записывает все нули, проверяет нули, "
                   "записывает все единицы, проверяет единицы. Обнаруживает основные неисправности памяти.";
        default:
            return "Описание недоступно";
    }
}

QString MainWindow::getThemeStylesheet(Theme theme) {
    return ThemeManager::getStylesheet(theme);
}

void MainWindow::applyTheme(Theme theme) {
    _currentTheme = theme;
    
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