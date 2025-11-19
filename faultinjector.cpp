#include "faultinjector.h"
#include "constants.h"
#include <random>
#include <memory>

// Wrapper class for thread-local RNG storage with automatic cleanup
class ThreadLocalRNGWrapper {
public:
    ThreadLocalRNGWrapper() : _rng(std::random_device{}()) {}
    std::mt19937& get() { return _rng; }
private:
    std::mt19937 _rng;
};

// Thread-local storage for RNG (one per thread)
// QThreadStorage automatically calls destructor when thread ends, ensuring proper cleanup
static QThreadStorage<ThreadLocalRNGWrapper*> threadLocalRNGStorage;

static std::mt19937& getThreadLocalRNG() {
    if (!threadLocalRNGStorage.hasLocalData()) {
        threadLocalRNGStorage.setLocalData(new ThreadLocalRNGWrapper());
    }
    ThreadLocalRNGWrapper* wrapper = threadLocalRNGStorage.localData();
    if (!wrapper) {
        // Additional safety check: if localData() returns nullptr, create new wrapper
        threadLocalRNGStorage.setLocalData(new ThreadLocalRNGWrapper());
        wrapper = threadLocalRNGStorage.localData();
    }
    return wrapper->get();
}

FaultInjector::FaultInjector()
    : _injected() {}

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

std::mt19937& FaultInjector::getRNG() const {
    return getThreadLocalRNG();
}

Word FaultInjector::applyFault(size_t addr, Word storedValue) const {
    QMutexLocker locker(&_mutex);
    
    if (!_isAddrFaultyUnlocked(addr)) {
        return storedValue;
    }

    // НЕИСПРАВНОСТИ ПРИМЕНЯЮТСЯ ТОЛЬКО ПРИ ЧТЕНИИ
    switch (_injected.model) {
        case FaultModel::BitFlip: {
            // For BitFlip, probability is checked per bit, not per address
            Word v = storedValue;
            std::bernoulli_distribution bitProb(_injected.flip_probability);
            std::mt19937& rng = getRNG(); // Thread-local RNG
            for (int b = 0; b < BITS_PER_WORD; ++b) 
                if (bitProb(rng)) 
                    v ^= (1u << b);
            return v;
        }
        case FaultModel::StuckAt0: 
        case FaultModel::StuckAt1: 
        case FaultModel::OpenRead: {
            // For other models, check probability at address level
            // Note: flip_probability is used as fault application probability (not bit-flip probability)
            std::bernoulli_distribution addrProb(_injected.flip_probability);
            std::mt19937& rng = getRNG(); // Thread-local RNG
            if (!addrProb(rng)) {
                return storedValue; // Probability didn't trigger - read normally
            }
            
            // Probability triggered - apply fault
            switch (_injected.model) {
                case FaultModel::StuckAt0: 
                    return 0u;
                case FaultModel::StuckAt1: 
                    return static_cast<Word>(~0u);
                case FaultModel::OpenRead: 
                    return INVALID_READ_MARKER; // marker for invalid read
                default:
                    return storedValue;
            }
        }
        default: 
            return storedValue;
    }
}

