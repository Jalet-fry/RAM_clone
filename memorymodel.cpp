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
    
    Word stored_value = _words[addr]; // Всегда читаем фактически хранимое значение
    
    if (!_isAddrFaulty(addr)) 
        return stored_value;

    // Проверяем вероятность для адреса перед применением ошибки
    std::bernoulli_distribution addrProb(_injected.flip_probability);
    if (!addrProb(_rng)) {
        return stored_value; // Вероятность не сработала - читаем нормально
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
            Word v = stored_value; // Искажаем хранимое значение
            std::bernoulli_distribution bitProb(_injected.flip_probability);
            for (int b = 0; b < 32; ++b) 
                if (bitProb(_rng)) 
                    v ^= (1u << b);
            return v;
        }
        default: 
            return stored_value;
    }
}

void MemoryModel::write(size_t addr, Word value) {
    QMutexLocker locker(&_mutex);
    if (addr >= _words.size()) return;
    
    // ЗАПИСЬ ВСЕГДА СОХРАНЯЕТ ТОЧНОЕ ЗНАЧЕНИЕ (не применяем неисправности при записи)
    _words[addr] = value;
    
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