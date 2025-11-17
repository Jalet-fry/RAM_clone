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
#include "memorymodel.h"
#include "testerworker.h"

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
    void scrollToFault();
    void onAlgorithmChanged(int index);
    void updateFaultInfo();
    void updateStatistics();
    void updateTestInfo();
    void updateProgressDetails(size_t addr, Word expected, Word read);

private:
    void logInfo(const QString& message);
    void logWarning(const QString& message);
    void logError(const QString& message);
    void logSuccess(const QString& message);
    void highlightCurrentAddress(size_t addr);
    void updateMemoryCellStatus(size_t addr);
    QString formatBinary(Word value);
    QString getFaultModelName(FaultModel model);
    QString getAlgorithmDescription(TestAlgorithm algo);

    MemoryModel* _mem;
    TesterWorker* _worker;
    QTimer* _testTimer;
    QTime _testStartTime;

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
    QTextEdit* _log;
    QLineEdit* _searchEdit;
    QPushButton* _searchBtn;
    QPushButton* _scrollToFaultBtn;
    QPushButton* _clearLogBtn;
    QPushButton* _exportBtn;

    std::vector<TestResult> _lastResults;
    size_t _currentTestAddr;
    size_t _lastHighlightedAddr;
    bool _testRunning;
};

#endif // MAINWINDOW_H