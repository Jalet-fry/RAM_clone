#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <QLabel>
#include <QComboBox>
#include <memory>
#include <vector>
#include "types.h"
#include "memorymodel.h"
#include "thememanager.h"
#include "dataformatter.h"

class StatisticsManager {
public:
    StatisticsManager(QLabel* totalAddressesLabel,
                     QLabel* testedAddressesLabel,
                     QLabel* faultsFoundLabel,
                     QLabel* coverageLabel,
                     QLabel* testTimeLabel,
                     QLabel* currentFaultModelLabel,
                     QLabel* currentAlgorithmLabel,
                     QLabel* faultInfoLabel,
                     QLabel* testInfoLabel,
                     QComboBox* algoCombo,
                     MemoryModel* mem);
    
    void setMemoryModel(MemoryModel* mem);
    void setTestResults(const std::vector<TestResult>& results);
    void clearTestResults();  // Clear test results and reset statistics
    void setTestTime(int msecs);
    void setTheme(Theme theme);
    void setAlgorithmCombo(QComboBox* algoCombo);
    
    void updateStatistics();
    void updateFaultInfo();
    void updateTestInfo();
    
private:
    MemoryModel* _mem;
    std::vector<TestResult> _lastResults;
    Theme _currentTheme;
    int _testTimeMsecs;
    
    // UI elements (not owned)
    QLabel* _totalAddressesLabel;
    QLabel* _testedAddressesLabel;
    QLabel* _faultsFoundLabel;
    QLabel* _coverageLabel;
    QLabel* _testTimeLabel;
    QLabel* _currentFaultModelLabel;
    QLabel* _currentAlgorithmLabel;
    QLabel* _faultInfoLabel;
    QLabel* _testInfoLabel;
    QComboBox* _algoCombo;
};

#endif // STATISTICSMANAGER_H

