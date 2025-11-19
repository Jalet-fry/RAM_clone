#include "faultcontroller.h"
#include "constants.h"
#include <QMessageBox>

FaultController::FaultController(MemoryModel* mem,
                                 QComboBox* faultCombo,
                                 QLineEdit* addrEdit,
                                 QLineEdit* lenEdit,
                                 QDoubleSpinBox* flipProbSpin,
                                 Logger* logger,
                                 QObject* parent)
    : QObject(parent), _mem(mem), _logger(logger),
      _faultCombo(faultCombo), _addrEdit(addrEdit), _lenEdit(lenEdit), _flipProbSpin(flipProbSpin) {
}

void FaultController::setMemoryModel(MemoryModel* mem) {
    _mem = mem;
}

void FaultController::setLogger(Logger* logger) {
    _logger = logger;
}

bool FaultController::validateInput(size_t& addr, size_t& len) {
    if (!_mem || !_logger) return false;
    
    bool ok1 = false, ok2 = false;
    addr = _addrEdit->text().toULongLong(&ok1);
    len = _lenEdit->text().toULongLong(&ok2);

    if (!ok1 || !ok2) {
        if (_logger) {
            _logger->error("Ошибка ввода: Адрес и длина должны быть числами.");
        }
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка ввода", "Адрес и длина должны быть числами.");
        return false;
    }

    if (addr >= _mem->size()) {
        if (_logger) {
            _logger->error(QString("Ошибка: Адрес %1 выходит за пределы памяти (0-%2)").arg(addr).arg(_mem->size() - 1));
        }
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка", QString("Адрес должен быть в диапазоне 0-%1").arg(_mem->size() - 1));
        return false;
    }

    if (addr + len > _mem->size()) {
        len = _mem->size() - addr;
        if (_logger) {
            _logger->warning(QString("Длина уменьшена до %1 (выход за пределы памяти)").arg(len));
        }
    }

    return true;
}

void FaultController::injectFault() {
    if (!_mem || !_logger) return;
    
    InjectedFault f;
    f.model = static_cast<FaultModel>(_faultCombo->currentData().toInt());
    
    size_t addr, len;
    if (!validateInput(addr, len)) {
        return;
    }

    f.addr = addr;
    f.len = std::max<size_t>(1, len);
    f.flip_probability = _flipProbSpin->value();

    _mem->injectFault(f);
    if (_logger) {
        _logger->success(QString("Внедрена неисправность: %1 по адресу %2, длина=%3")
                   .arg(DataFormatter::getFaultModelName(f.model)).arg(addr).arg(f.len));
    }
    emit faultInjected();
}

void FaultController::resetMemory() {
    if (!_mem || !_logger) return;
    
    _mem->reset();
    if (_logger) {
        _logger->info("Память сброшена. Все данные очищены, неисправности удалены.");
    }
    emit memoryReset();
}

void FaultController::onFaultModelChanged(int index) {
    // Automatically set default probability based on fault model
    FaultModel model = static_cast<FaultModel>(_faultCombo->itemData(index).toInt());
    if (model == FaultModel::BitFlip) {
        _flipProbSpin->setValue(0.010);
    } else if (model != FaultModel::None) {
        // For other fault models (StuckAt0, StuckAt1, OpenRead), use 0.110
        _flipProbSpin->setValue(0.110);
    }
    // For FaultModel::None, keep current value
}

