#ifndef MEMORYTESTER_H
#define MEMORYTESTER_H


#include <QObject>
#include <vector>
#include "memorymodel.h"


enum class TestAlgorithm {
WalkingOnes,
WalkingZeros,
MarchSimple
};


struct TestResult {
size_t addr;
Word expected;
Word read;
bool passed;
};


class MemoryTester : public QObject {
Q_OBJECT
public:
explicit MemoryTester(MemoryModel* mem, QObject* parent = nullptr);
~MemoryTester() override = default;


// blocking call — meant to run in a worker thread
void runTest(TestAlgorithm algo);


signals:
void progress(int percent);
void progressDetail(size_t addr, Word expected, Word read);
void finished(const std::vector<TestResult>& results);


private:
MemoryModel* _mem;
std::vector<TestResult> _results;
};
#endif // MEMORYTESTER_H
