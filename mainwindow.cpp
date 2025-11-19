#include "mainwindow.h"
#include "tableitemdelegate.h"
#include "constants.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QIntValidator>
#include <QThread>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <set>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), _dataChangedConnected(true) {
    qDebug() << "MainWindow constructor: start";
    setWindowTitle("Цифровой двойник ОЗУ — 350504 Витовт Даник Маша Настя");
    resize(1400, 800);
    qDebug() << "MainWindow: window title and size set";

    qDebug() << "MainWindow: creating MemoryModel...";
    _mem = new MemoryModel(DEFAULT_MEMORY_SIZE, this);
    qDebug() << "MainWindow: MemoryModel created";
    
    qDebug() << "MainWindow: creating TesterWorker...";
    _worker = std::unique_ptr<TesterWorker>(new TesterWorker(_mem));
    qDebug() << "MainWindow: TesterWorker created";

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
    logControlsLayout->addWidget(_clearLogBtn);
    logLayout->addLayout(logControlsLayout);

    _log = new QTextEdit;
    _log->setReadOnly(true);
    _log->setFont(QFont("Courier", 9));
    logLayout->addWidget(_log);
    
    // Initialize logger
    Theme initialTheme = Theme::DeusEx;
    _logger = std::unique_ptr<Logger>(new Logger(_log, initialTheme));
    
    // Initialize worker thread - must be done after all objects are created
    // This moves TesterWorker to worker thread after full initialization
    _worker->initialize();
    
    // Create managers
    _tableManager = std::unique_ptr<MemoryTableManager>(new MemoryTableManager(_table, _mem, _logger.get()));
    _tableManager->setTheme(initialTheme);
    
    _statisticsManager = std::unique_ptr<StatisticsManager>(new StatisticsManager(
        _totalAddressesLabel, _testedAddressesLabel, _faultsFoundLabel, _coverageLabel,
        _testTimeLabel, _currentFaultModelLabel, _currentAlgorithmLabel,
        _faultInfoLabel, _testInfoLabel, _algoCombo, _mem));
    _statisticsManager->setTheme(initialTheme);
    
    _faultController = std::unique_ptr<FaultController>(new FaultController(
        _mem, _faultCombo, _addrEdit, _lenEdit, _flipProbSpin, _logger.get(), this));
    
    _testController = std::unique_ptr<TestController>(new TestController(
        _worker.get(), _mem, _tableManager.get(), _algoCombo, _startBtn, _progress,
        _testInfoLabel, _currentAddrLabel, _expectedValueLabel, _readValueLabel,
        _logger.get(), this));
    
    _resultsNavigator = std::unique_ptr<ResultsNavigator>(new ResultsNavigator(_table, _mem, _logger.get()));

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
    
    // Create ThemeController after menu is created
    _themeController = std::unique_ptr<ThemeController>(new ThemeController(
        this, _themeGroup, _deusExAction, _matrixAction, _gurrenLagannAction, _cyberpunkAction,
        _log, _logger.get(), this));
    
    connect(_themeGroup, &QActionGroup::triggered, _themeController.get(), &ThemeController::onThemeChanged);

    // Connections - UI buttons
    connect(_injectBtn, &QPushButton::clicked, _faultController.get(), &FaultController::injectFault);
    connect(_resetBtn, &QPushButton::clicked, _faultController.get(), &FaultController::resetMemory);
    connect(_startBtn, &QPushButton::clicked, _testController.get(), &TestController::startTest);
    connect(_clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLog);
    connect(_scrollToNextFaultBtn, &QPushButton::clicked, this, [this]() {
        _resultsNavigator->scrollToNextFault(_lastResults);
    });
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
    connect(_algoCombo, SIGNAL(currentIndexChanged(int)), _testController.get(), SLOT(onAlgorithmChanged(int)));
    connect(_faultCombo, SIGNAL(currentIndexChanged(int)), _faultController.get(), SLOT(onFaultModelChanged(int)));

    // Worker signals - use QueuedConnection since _worker is in a different thread
    connect(_worker.get(), &TesterWorker::progress, _progress, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(_worker.get(), &TesterWorker::progressDetail, _testController.get(), &TestController::updateProgressDetails, Qt::QueuedConnection);
    connect(_worker.get(), &TesterWorker::finished, _testController.get(), &TestController::onTestFinished, Qt::QueuedConnection);

    // Memory signals
    connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::onTableDataChanged);
    connect(_mem, &MemoryModel::faultInjected, this, &MainWindow::onFaultInjected);
    connect(_mem, &MemoryModel::errorOccurred, this, [this](const QString& message) {
        if (_logger) {
            _logger->error(message);
        }
    });
    
    // Manager signals
    connect(_faultController.get(), &FaultController::faultInjected, this, &MainWindow::onFaultInjected);
    connect(_faultController.get(), &FaultController::memoryReset, this, &MainWindow::onMemoryReset);
    connect(_testController.get(), &TestController::testStarted, this, &MainWindow::onTestStarted);
    connect(_testController.get(), &TestController::testFinished, this, &MainWindow::onTestFinished);
    connect(_testController.get(), &TestController::testResultsUpdated, this, &MainWindow::onTestResultsUpdated);
    connect(_themeController.get(), &ThemeController::themeChanged, this, &MainWindow::onThemeChanged);

    // Initialize
    _tableManager->refreshTable(0, _mem->size());
    _statisticsManager->updateFaultInfo();
    _statisticsManager->updateStatistics();
    _testController->onAlgorithmChanged(0);
    
    // Apply default theme (Deus Ex)
    qDebug() << "MainWindow: applying default theme...";
    _themeController->applyTheme(initialTheme);
    qDebug() << "MainWindow: theme applied";
    
    qDebug() << "MainWindow: logging startup message...";
    _logger->info("Программа запущена. Готов к работе.");
    qDebug() << "MainWindow constructor: completed successfully";
}

MainWindow::~MainWindow() {
    if (_worker) {
        // Stop any running test
        if (_testController) {
            _testController->setTestRunning(false);
        }
        
        // Restore dataChanged connection if it was disconnected
        if (!_dataChangedConnected && _mem) {
            connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::onTableDataChanged);
            _dataChangedConnected = true;
        }
        
        // Disconnect signals to prevent callbacks during destruction
        // This prevents any signals from worker thread reaching this (partially destroyed) object
        disconnect(_worker.get(), nullptr, this, nullptr);
        
        // Worker's destructor will handle thread cleanup (quit and wait)
        // No need to move back to main thread - Qt handles this automatically
        _worker.reset();
    }
}

void MainWindow::onTableDataChanged(size_t begin, size_t end) {
    if (_tableManager) {
        _tableManager->refreshTable(begin, end);
    }
}

void MainWindow::onFaultInjected() {
    if (_statisticsManager) {
        _statisticsManager->updateFaultInfo();
    }
    if (_tableManager) {
        _tableManager->refreshTable(0, _mem->size());
    }
}

void MainWindow::onMemoryReset() {
    _lastResults.clear();
    _progress->setValue(0);
    _currentAddrLabel->setText("Адрес: —");
    _expectedValueLabel->setText("Ожидается: —");
    _readValueLabel->setText("Прочитано: —");
    
    // Clear test results in managers (they handle their own internal state)
    if (_tableManager) {
        _tableManager->clearTestResults();
    }
    if (_statisticsManager) {
        _statisticsManager->clearTestResults();
        _statisticsManager->updateFaultInfo();
    }
}

void MainWindow::onTestStarted() {
    _lastResults.clear();
    // Disable table updates during test for performance
    if (_dataChangedConnected) {
        disconnect(_mem, &MemoryModel::dataChanged, this, &MainWindow::onTableDataChanged);
        _dataChangedConnected = false;
    }
}

void MainWindow::onTestFinished(const std::vector<TestResult>& results) {
    _lastResults = results;
    
    // Re-enable table updates after test completion
    if (!_dataChangedConnected) {
        connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::onTableDataChanged);
        _dataChangedConnected = true;
    }
    
    // Update managers with results
    if (_tableManager) {
        _tableManager->setTestResults(results);
        _tableManager->refreshTable(0, _mem->size());
    }
    if (_statisticsManager) {
        _statisticsManager->setTestResults(results);
        if (_testController) {
            int testTime = _testController->getLastTestTime();
            _statisticsManager->setTestTime(testTime);
            _statisticsManager->updateStatistics();
        }
    }
}

void MainWindow::onTestResultsUpdated(const std::vector<TestResult>& results) {
    _lastResults = results;
    
    // Update statistics with test time from test controller
    if (_testController && _statisticsManager) {
        int testTime = _testController->getLastTestTime();
        _statisticsManager->setTestTime(testTime);
        _statisticsManager->updateStatistics();
    }
}

void MainWindow::onThemeChanged(Theme theme) {
    qDebug() << "MainWindow::onThemeChanged: start, theme =" << static_cast<int>(theme);
    // NOTE: Do NOT call applyTheme here - this slot is called FROM applyTheme!
    // We just need to update the managers with the new theme
    
    if (_tableManager) {
        qDebug() << "MainWindow::onThemeChanged: updating table manager theme";
        _tableManager->setTheme(theme);
        _tableManager->refreshTable(0, _mem->size());
    }
    if (_statisticsManager) {
        qDebug() << "MainWindow::onThemeChanged: updating statistics manager theme";
        _statisticsManager->setTheme(theme);
        _statisticsManager->updateFaultInfo();
    }
    if (_tableDelegate) {
        qDebug() << "MainWindow::onThemeChanged: updating table delegate theme";
        _tableDelegate->setTheme(theme);
    }
    qDebug() << "MainWindow::onThemeChanged: completed";
}

void MainWindow::clearLog() {
    if (_logger) {
        _logger->clear();
        _logger->info("Журнал очищен.");
    }
}