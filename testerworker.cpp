#include "testerworker.h"
#include <QMetaObject>
#include <functional>


TesterWorker::TesterWorker(MemoryModel* mem, QObject* parent)
: QObject(parent), _thread(), _mem(mem) {
// move this object to the internal thread so its slots run there
this->moveToThread(&_thread);
_thread.start();
// When thread finishes, deleteLater will be called on this object if desired from outside
}


TesterWorker::~TesterWorker() {
_thread.quit();
_thread.wait();
}


void TesterWorker::run(TestAlgorithm algo) {
// This slot runs in the worker thread (because object was moved).
MemoryTester tester(_mem);
connect(&tester, &MemoryTester::progress, this, &TesterWorker::progress);
connect(&tester, &MemoryTester::progressDetail, this, &TesterWorker::progressDetail);
connect(&tester, &MemoryTester::finished, this, &TesterWorker::finished);
tester.runTest(algo);
}
