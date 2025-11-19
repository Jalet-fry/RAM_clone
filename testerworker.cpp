#include "testerworker.h"
#include <QMetaObject>
#include <functional>


TesterWorker::TesterWorker(MemoryModel* mem, QObject* parent)
: QObject(parent), _thread(), _mem(mem), _tester(std::unique_ptr<MemoryTester>(new MemoryTester(mem))) {
// move this object to the internal thread so its slots run there
this->moveToThread(&_thread);
_thread.start();
// Connect tester signals once during construction
connect(_tester.get(), &MemoryTester::progress, this, &TesterWorker::progress);
connect(_tester.get(), &MemoryTester::progressDetail, this, &TesterWorker::progressDetail);
connect(_tester.get(), &MemoryTester::finished, this, &TesterWorker::finished);
}


TesterWorker::~TesterWorker() {
_thread.quit();
_thread.wait();
}


void TesterWorker::run(TestAlgorithm algo) {
// This slot runs in the worker thread (because object was moved).
// _tester is a member variable, so it lives as long as TesterWorker
_tester->runTest(algo);
}
