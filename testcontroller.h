#ifndef TESTCONTROLLER_H
#define TESTCONTROLLER_H

#include <QObject>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTime>
#include <memory>
#include <vector>
#include "types.h"
#include "testerworker.h"
#include "memorymodel.h"
#include "logger.h"
#include "dataformatter.h"
#include "memorytablemanager.h"

class TestController : public QObject {
    Q_OBJECT
public:
    explicit TestController(TesterWorker* worker,
                           MemoryModel* mem,
                           MemoryTableManager* tableManager,
                           QComboBox* algoCombo,
                           QPushButton* startBtn,
                           QProgressBar* progress,
                           QLabel* testInfoLabel,
                           QLabel* currentAddrLabel,
                           QLabel* expectedValueLabel,
                           QLabel* readValueLabel,
                           Logger* logger,
                           QObject* parent = nullptr);
    
    void setWorker(TesterWorker* worker);
    void setMemoryModel(MemoryModel* mem);
    void setTableManager(MemoryTableManager* tableManager);
    void setLogger(Logger* logger);
    void setTestRunning(bool running);
    bool isTestRunning() const { return _testRunning; }
    int getLastTestTime() const { return _lastTestTimeMsecs; }
    
public slots:
    void startTest();
    void onTestFinished(const std::vector<TestResult>& results);
    void onAlgorithmChanged(int index);
    void updateProgressDetails(size_t addr, Word expected, Word read);
    
signals:
    void testStarted();
    void testFinished(const std::vector<TestResult>& results);
    void testResultsUpdated(const std::vector<TestResult>& results);
    
private:
    void highlightCurrentAddress(size_t addr);
    
    TesterWorker* _worker;
    MemoryModel* _mem;
    MemoryTableManager* _tableManager;
    Logger* _logger;
    bool _testRunning;
    QTime _testStartTime;
    size_t _lastHighlightedAddr;
    int _lastTestTimeMsecs;
    
    // UI elements (not owned)
    QComboBox* _algoCombo;
    QPushButton* _startBtn;
    QProgressBar* _progress;
    QLabel* _testInfoLabel;
    QLabel* _currentAddrLabel;
    QLabel* _expectedValueLabel;
    QLabel* _readValueLabel;
};

#endif // TESTCONTROLLER_H

