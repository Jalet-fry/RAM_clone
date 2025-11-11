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
#include <QSplitter>
#include <QLabel>
#include <QTime>
#include <QTimer>
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
void updateStatistics();


private:
MemoryModel* _mem;
TesterWorker* _worker;

// Helper methods
void logInfo(const QString& message);
void logWarning(const QString& message);
void logError(const QString& message);
void logSuccess(const QString& message);
void updateFaultInfo();
void updateTestInfo();
void updateMemoryCellStatus(size_t addr);
void highlightCurrentAddress(size_t addr);
void updateProgressDetails(size_t addr, Word expected, Word read);
QString formatBinary(Word value);
QString getFaultModelName(FaultModel model);
QString getAlgorithmDescription(TestAlgorithm algo);

// UI - Fault Management
QGroupBox* _faultGroup;
QComboBox* _faultCombo;
QLineEdit* _addrEdit;
QLineEdit* _lenEdit;
QDoubleSpinBox* _flipProbSpin;
QPushButton* _injectBtn;
QPushButton* _resetBtn;
QLabel* _faultInfoLabel;

// UI - Testing
QGroupBox* _testGroup;
QComboBox* _algoCombo;
QPushButton* _startBtn;
QProgressBar* _progress;
QLabel* _testInfoLabel;
QLabel* _currentAddrLabel;
QLabel* _expectedValueLabel;
QLabel* _readValueLabel;

// UI - Statistics
QGroupBox* _statsGroup;
QLabel* _totalAddressesLabel;
QLabel* _testedAddressesLabel;
QLabel* _faultsFoundLabel;
QLabel* _coverageLabel;
QLabel* _testTimeLabel;
QLabel* _currentFaultModelLabel;
QLabel* _currentAlgorithmLabel;

// UI - Memory Table
QTableWidget* _table;
QPushButton* _scrollToFaultBtn;
QLineEdit* _searchEdit;
QPushButton* _searchBtn;

// UI - Log
QTextEdit* _log;
QPushButton* _clearLogBtn;

// UI - Additional
QPushButton* _exportBtn;
QTimer* _testTimer;
QTime _testStartTime;

// State
std::vector<TestResult> _lastResults;
size_t _currentTestAddr;
size_t _lastHighlightedAddr;
bool _testRunning;
};
#endif // MAINWINDOW_H