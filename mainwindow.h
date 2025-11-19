#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QActionGroup>
#include <QAction>
#include <memory>
#include <unordered_map>
#include <set>
#include <vector>
#include "types.h"
#include "memorymodel.h"
#include "testerworker.h"
#include "thememanager.h"
#include "tableitemdelegate.h"
#include "logger.h"
#include "dataformatter.h"
#include "memorytablemanager.h"
#include "statisticsmanager.h"
#include "faultcontroller.h"
#include "testcontroller.h"
#include "resultsnavigator.h"
#include "themecontroller.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void clearLog();
    void onTableDataChanged(size_t begin, size_t end);
    void onFaultInjected();
    void onMemoryReset();
    void onTestStarted();
    void onTestFinished(const std::vector<TestResult>& results);
    void onTestResultsUpdated(const std::vector<TestResult>& results);
    void onThemeChanged(Theme theme);

private:

    MemoryModel* _mem;  // Owned by Qt parent (this)
    std::unique_ptr<TesterWorker> _worker;
    std::unique_ptr<Logger> _logger;
    
    // Managers
    std::unique_ptr<MemoryTableManager> _tableManager;
    std::unique_ptr<StatisticsManager> _statisticsManager;
    std::unique_ptr<FaultController> _faultController;
    std::unique_ptr<TestController> _testController;
    std::unique_ptr<ResultsNavigator> _resultsNavigator;
    std::unique_ptr<ThemeController> _themeController;

    // UI elements
    QGroupBox* _faultGroup;
    QGroupBox* _testGroup;
    QGroupBox* _statsGroup;

    QComboBox* _faultCombo;
    QLineEdit* _addrEdit;
    QLineEdit* _lenEdit;
    QDoubleSpinBox* _flipProbSpin;
    QPushButton* _injectBtn;
    QPushButton* _resetBtn;
    QLabel* _faultInfoLabel;

    QComboBox* _algoCombo;
    QPushButton* _startBtn;
    QProgressBar* _progress;
    QLabel* _testInfoLabel;
    QLabel* _currentAddrLabel;
    QLabel* _expectedValueLabel;
    QLabel* _readValueLabel;

    QLabel* _totalAddressesLabel;
    QLabel* _testedAddressesLabel;
    QLabel* _faultsFoundLabel;
    QLabel* _coverageLabel;
    QLabel* _testTimeLabel;
    QLabel* _currentFaultModelLabel;
    QLabel* _currentAlgorithmLabel;

    QTableWidget* _table;
    TableItemDelegate* _tableDelegate;
    QTextEdit* _log;
    QLineEdit* _searchEdit;
    QPushButton* _searchBtn;
    QPushButton* _scrollToNextFaultBtn;
    QPushButton* _clearLogBtn;

    // Theme menu
    QActionGroup* _themeGroup;
    QAction* _deusExAction;
    QAction* _matrixAction;
    QAction* _gurrenLagannAction;
    QAction* _cyberpunkAction;

    std::vector<TestResult> _lastResults;
    bool _dataChangedConnected;  // Track connection state
};

#endif // MAINWINDOW_H