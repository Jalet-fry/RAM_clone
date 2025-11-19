#ifndef FAULTINJECTOR_H
#define FAULTINJECTOR_H

#include <cstdint>
#include <random>
#include <QMutex>

using Word = uint32_t;

enum class FaultModel {
    None,
    StuckAt0,
    StuckAt1,
    BitFlip,
    OpenRead
};

struct InjectedFault {
    FaultModel model = FaultModel::None;
    size_t addr = 0;
    size_t len = 1;
    double flip_probability = 0.01;
};

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

    mutable QMutex _mutex;
    InjectedFault _injected;
    mutable std::mt19937 _rng;
};

#endif // FAULTINJECTOR_H

