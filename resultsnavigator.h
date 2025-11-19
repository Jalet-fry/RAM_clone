#ifndef RESULTSNAVIGATOR_H
#define RESULTSNAVIGATOR_H

#include <QTableWidget>
#include <QMessageBox>
#include <vector>
#include <set>
#include "types.h"
#include "memorymodel.h"
#include "logger.h"

class ResultsNavigator {
public:
    explicit ResultsNavigator(QTableWidget* table, MemoryModel* mem, Logger* logger);
    
    void setTableWidget(QTableWidget* table);
    void setMemoryModel(MemoryModel* mem);
    void setLogger(Logger* logger);
    
    void scrollToNextFault(const std::vector<TestResult>& results, size_t currentAddr = 0);
    
private:
    QTableWidget* _table;
    MemoryModel* _mem;
    Logger* _logger;
};

#endif // RESULTSNAVIGATOR_H

