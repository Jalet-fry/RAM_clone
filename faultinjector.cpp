#include "faultinjector.h"
#include <random>

FaultInjector::FaultInjector()
    : _injected(), _rng(std::random_device{}()) {}

void FaultInjector::injectFault(const InjectedFault& f) {
    QMutexLocker locker(&_mutex);
    _injected = f;
}

InjectedFault FaultInjector::currentFault() const {
    QMutexLocker locker(&_mutex);
    return _injected;
}

void FaultInjector::reset() {
    QMutexLocker locker(&_mutex);
    _injected = InjectedFault{};
}

bool FaultInjector::_isAddrFaultyUnlocked(size_t addr) const {
    if (_injected.model == FaultModel::None) return false;
    return (addr >= _injected.addr && addr < _injected.addr + _injected.len);
}

bool FaultInjector::isAddrFaulty(size_t addr) const {
    QMutexLocker locker(&_mutex);
    return _isAddrFaultyUnlocked(addr);
}

Word FaultInjector::applyFault(size_t addr, Word storedValue) const {
    QMutexLocker locker(&_mutex);
    
    if (!_isAddrFaultyUnlocked(addr)) {
        return storedValue;
    }

    // Проверяем вероятность для адреса перед применением ошибки
    std::bernoulli_distribution addrProb(_injected.flip_probability);
    if (!addrProb(_rng)) {
        return storedValue; // Вероятность не сработала - читаем нормально
    }

    // Вероятность сработала - применяем ошибку
    // НЕИСПРАВНОСТИ ПРИМЕНЯЮТСЯ ТОЛЬКО ПРИ ЧТЕНИИ
    switch (_injected.model) {
        case FaultModel::StuckAt0: 
            return 0u;
        case FaultModel::StuckAt1: 
            return static_cast<Word>(~0u);
        case FaultModel::OpenRead: 
            return 0xFFFFFFFFu; // marker for invalid read
        case FaultModel::BitFlip: {
            Word v = storedValue; // Искажаем хранимое значение
            std::bernoulli_distribution bitProb(_injected.flip_probability);
            for (int b = 0; b < 32; ++b) 
                if (bitProb(_rng)) 
                    v ^= (1u << b);
            return v;
        }
        default: 
            return storedValue;
    }
}

