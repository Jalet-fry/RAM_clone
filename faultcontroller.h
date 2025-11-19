#ifndef FAULTCONTROLLER_H
#define FAULTCONTROLLER_H

#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "types.h"
#include "memorymodel.h"
#include "logger.h"
#include "dataformatter.h"

class FaultController : public QObject {
    Q_OBJECT
public:
    explicit FaultController(MemoryModel* mem,
                            QComboBox* faultCombo,
                            QLineEdit* addrEdit,
                            QLineEdit* lenEdit,
                            QDoubleSpinBox* flipProbSpin,
                            Logger* logger,
                            QObject* parent = nullptr);
    
    void setMemoryModel(MemoryModel* mem);
    void setLogger(Logger* logger);
    
public slots:
    void injectFault();
    void resetMemory();
    void onFaultModelChanged(int index);
    
signals:
    void faultInjected();
    void memoryReset();
    
private:
    bool validateInput(size_t& addr, size_t& len);
    
    MemoryModel* _mem;
    Logger* _logger;
    
    // UI elements (not owned)
    QComboBox* _faultCombo;
    QLineEdit* _addrEdit;
    QLineEdit* _lenEdit;
    QDoubleSpinBox* _flipProbSpin;
};

#endif // FAULTCONTROLLER_H

