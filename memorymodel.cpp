#include "memorymodel.h"
#include <algorithm>

MemoryModel::MemoryModel(size_t words, QObject* parent)
    : QObject(parent), _words(words, 0), _faultInjector(std::unique_ptr<FaultInjector>(new FaultInjector())) {}

MemoryModel::~MemoryModel() = default;

size_t MemoryModel::size() const {
    QMutexLocker locker(&_mutex);
    return _words.size();
}

void MemoryModel::reset() {
    size_t size;
    {
        QMutexLocker locker(&_mutex);
        std::fill(_words.begin(), _words.end(), 0u);
        size = _words.size(); // Save size while mutex is locked
    } // Mutex automatically unlocked here
    _faultInjector->reset();
    emit dataChanged(0, size);
}

Word MemoryModel::read(size_t addr) {
    Word stored_value;
    {
        QMutexLocker locker(&_mutex);
        if (addr >= _words.size()) {
            // Out of bounds - emit error signal and return 0 for backward compatibility
            emit errorOccurred(QString("MemoryModel::read: Address %1 is out of bounds (size: %2)")
                              .arg(addr).arg(_words.size()));
            return 0u;
        }
        stored_value = _words[addr]; // Always read the actual stored value
    } // Mutex automatically unlocked here
    return _faultInjector->applyFault(addr, stored_value);
}

void MemoryModel::write(size_t addr, Word value) {
    {
        QMutexLocker locker(&_mutex);
        if (addr >= _words.size()) {
            // Out of bounds - emit error signal and return
            emit errorOccurred(QString("MemoryModel::write: Address %1 is out of bounds (size: %2)")
                              .arg(addr).arg(_words.size()));
            return;
        }
        
        // WRITE ALWAYS STORES EXACT VALUE (faults are not applied during write)
        _words[addr] = value;
    } // Mutex automatically unlocked here
    emit dataChanged(addr, addr + 1);
}

void MemoryModel::writeDirect(size_t addr, Word value) {
    {
        QMutexLocker locker(&_mutex);
        if (addr >= _words.size()) {
            // Out of bounds - emit error signal and return
            emit errorOccurred(QString("MemoryModel::writeDirect: Address %1 is out of bounds (size: %2)")
                              .arg(addr).arg(_words.size()));
            return;
        }
        _words[addr] = value;
    } // Mutex automatically unlocked here
    emit dataChanged(addr, addr + 1);
}

void MemoryModel::injectFault(const InjectedFault& f) {
    _faultInjector->injectFault(f);
    emit faultInjected();
    emit dataChanged(0, _words.size());
}

InjectedFault MemoryModel::currentFault() const {
    return _faultInjector->currentFault();
}