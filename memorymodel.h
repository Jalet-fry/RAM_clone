#ifndef MEMORYMODEL_H
#define MEMORYMODEL_H


#include <QObject>
#include <vector>
#include <random>
#include <cstdint>
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


class MemoryModel : public QObject {
Q_OBJECT
public:
explicit MemoryModel(size_t words = 256, QObject* parent = nullptr);
~MemoryModel() override = default;


size_t size() const;
void reset();


// thread-safe read/write
Word read(size_t addr);
void write(size_t addr, Word value);
void writeDirect(size_t addr, Word value);


void injectFault(const InjectedFault& f);
InjectedFault currentFault() const;


signals:
void dataChanged(size_t begin, size_t end);
void faultInjected();


private:
bool _isAddrFaulty(size_t addr) const;


mutable QMutex _mutex;
std::vector<Word> _words;
InjectedFault _injected;
mutable std::mt19937 _rng;
};

#endif // MEMORYMODEL_H
