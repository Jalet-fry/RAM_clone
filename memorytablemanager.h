#ifndef MEMORYTABLEMANAGER_H
#define MEMORYTABLEMANAGER_H

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QBrush>
#include <memory>
#include <vector>
#include <unordered_map>
#include <set>
#include "types.h"
#include "memorymodel.h"
#include "thememanager.h"
#include "dataformatter.h"

class Logger;

class MemoryTableManager {
public:
    explicit MemoryTableManager(QTableWidget* table, MemoryModel* mem, Logger* logger = nullptr);
    
    void setMemoryModel(MemoryModel* mem);
    void setTestResults(const std::vector<TestResult>& results);
    void clearTestResults();  // Clear test results and remove color highlighting
    void setTheme(Theme theme);
    void setLogger(Logger* logger);
    
    void refreshTable(size_t begin, size_t end);
    void highlightAddress(size_t addr);
    void scrollToAddress(size_t addr);
    void updateProgressHighlight(size_t addr, size_t& lastHighlightedAddr);
    
private:
    QTableWidgetItem* createOrGetTableItem(int row, int col);
    void applyFailedTestHighlighting(size_t addr, const ThemeColors& colors);
    void populateTableData(size_t addr, const Word value, const InjectedFault& f,
                          const std::set<size_t>& testedAddresses,
                          const std::unordered_map<size_t, TestResult>& resultMap,
                          const ThemeColors& colors);
    void applyTableColors(size_t addr, bool isFaulty, bool isTested, bool hasFailedTest,
                         const ThemeColors& colors);
    
    QTableWidget* _table;
    MemoryModel* _mem;
    Logger* _logger;
    Theme _currentTheme;
    std::vector<TestResult> _lastResults;
    size_t _lastHighlightedAddr;
};

#endif // MEMORYTABLEMANAGER_H

