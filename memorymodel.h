#ifndef MEMORYMODEL_H
#define MEMORYMODEL_H

#include <QObject>
#include <vector>
#include <cstdint>
#include <QMutex>
#include "faultinjector.h"

using Word = uint32_t;

class MemoryModel : public QObject {
Q_OBJECT
public:
explicit MemoryModel(size_t words = 256, QObject* parent = nullptr);
~MemoryModel() override;

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
mutable QMutex _mutex;
std::vector<Word> _words;
FaultInjector* _faultInjector;
};

#endif // MEMORYMODEL_H
