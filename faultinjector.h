#ifndef FAULTINJECTOR_H
#define FAULTINJECTOR_H

#include <random>
#include <QMutex>
#include <QThreadStorage>
#include "types.h"

class FaultInjector {
public:
    FaultInjector();
    ~FaultInjector() = default;

    // Управление неисправностями
    void injectFault(const InjectedFault& f);
    InjectedFault currentFault() const;
    void reset();

    // Применение неисправностей
    Word applyFault(size_t addr, Word storedValue) const;
    bool isAddrFaulty(size_t addr) const;

private:
    bool _isAddrFaultyUnlocked(size_t addr) const; // Без блокировки мьютекса
    std::mt19937& getRNG() const; // Thread-local RNG accessor

    mutable QMutex _mutex;
    InjectedFault _injected;
};

#endif // FAULTINJECTOR_H

