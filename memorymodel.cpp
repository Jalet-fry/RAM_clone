
#include "memorymodel.h"
#include <algorithm>
#include <chrono>

MemoryModel::MemoryModel(size_t words, QObject* parent)
    : QObject(parent), _words(words, 0), _injected(), _rng(std::random_device{}()) {}

size_t MemoryModel::size() const {
    QMutexLocker locker(&_mutex);
    return _words.size();
}

void MemoryModel::reset() {
    QMutexLocker locker(&_mutex);
    std::fill(_words.begin(), _words.end(), 0u);
    _injected = InjectedFault{};
    locker.unlock();
    emit dataChanged(0, _words.size());
}

bool MemoryModel::_isAddrFaulty(size_t addr) const {
    if (_injected.model == FaultModel::None) return false;
    return (addr >= _injected.addr && addr < _injected.addr + _injected.len);
}

Word MemoryModel::read(size_t addr) {
    QMutexLocker locker(&_mutex);
    if (addr >= _words.size()) return 0u;
    if (!_isAddrFaulty(addr)) return _words[addr];

    switch (_injected.model) {
        case FaultModel::StuckAt0: return 0u;
        case FaultModel::StuckAt1: return static_cast<Word>(~0u);
        case FaultModel::OpenRead: return 0xFFFFFFFFu; // marker for invalid read
        case FaultModel::BitFlip: {
            Word v = _words[addr];
            std::bernoulli_distribution d(_injected.flip_probability);
            // generate flips using single RNG; cost is small
            for (int b = 0; b < 32; ++b) if (d(_rng)) v ^= (1u << b);
            return v;
        }
        default: return _words[addr];
    }
}

void MemoryModel::write(size_t addr, Word value) {
    QMutexLocker locker(&_mutex);
    if (addr >= _words.size()) return;
    if (!_isAddrFaulty(addr)) {
        _words[addr] = value;
    } else {
        switch (_injected.model) {
            case FaultModel::StuckAt0: _words[addr] = 0u; break;
            case FaultModel::StuckAt1: _words[addr] = ~0u; break;
            case FaultModel::OpenRead:
                // OpenRead models an issue at read—write may succeed normally
                _words[addr] = value; break;
            case FaultModel::BitFlip: {
                Word v = value;
                std::bernoulli_distribution d(_injected.flip_probability);
                for (int b = 0; b < 32; ++b) if (d(_rng)) v ^= (1u << b);
                _words[addr] = v; break;
            }
            default: _words[addr] = value; break;
        }
    }
    locker.unlock();
    emit dataChanged(addr, addr + 1);
}

void MemoryModel::writeDirect(size_t addr, Word value) {
    QMutexLocker locker(&_mutex);
    if (addr >= _words.size()) return;
    _words[addr] = value;
    locker.unlock();
    emit dataChanged(addr, addr + 1);
}

void MemoryModel::injectFault(const InjectedFault& f) {
    QMutexLocker locker(&_mutex);
    _injected = f;
    locker.unlock();
    emit faultInjected();
    emit dataChanged(0, _words.size());
}

InjectedFault MemoryModel::currentFault() const {
    QMutexLocker locker(&_mutex);
    return _injected;
}
