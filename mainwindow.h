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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onInject();
    void onReset();
    void onStartTest();
    void onTestFinished(const std::vector<TestResult>& results);
    void refreshTable(size_t begin, size_t end);
    void clearLog();
    void exportResults();
    void scrollToNextFault();
    void onAlgorithmChanged(int index);
    void onFaultModelChanged(int index);
    void updateFaultInfo();
    void updateStatistics();
    void updateTestInfo();
    void updateProgressDetails(size_t addr, Word expected, Word read);
    void onThemeChanged();

private:
    void highlightCurrentAddress(size_t addr);
    void applyTheme(Theme theme);
    QString getThemeStylesheet(Theme theme);
    
    // Helper methods for refreshTable
    QTableWidgetItem* createOrGetTableItem(int row, int col);
    void applyFailedTestHighlighting(size_t addr, const ThemeColors& colors);
    void populateTableData(size_t addr, const Word value, const InjectedFault& f, 
                          const std::set<size_t>& testedAddresses, 
                          const std::unordered_map<size_t, TestResult>& resultMap,
                          const ThemeColors& colors);
    void applyTableColors(size_t addr, bool isFaulty, bool isTested, bool hasFailedTest,
                         const ThemeColors& colors);

    MemoryModel* _mem;  // Owned by Qt parent (this)
    std::unique_ptr<TesterWorker> _worker;
    QTimer* _testTimer;  // Owned by Qt parent (this)
    QTime _testStartTime;
    std::unique_ptr<Logger> _logger;

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
    QPushButton* _exportBtn;

    std::vector<TestResult> _lastResults;
    size_t _currentTestAddr;
    size_t _lastHighlightedAddr;
    bool _testRunning;
    bool _dataChangedConnected;  // Track connection state

    // Theme management
    Theme _currentTheme;
    QActionGroup* _themeGroup;
    QAction* _deusExAction;
    QAction* _matrixAction;
    QAction* _gurrenLagannAction;
    QAction* _cyberpunkAction;
};

#endif // MAINWINDOW_H