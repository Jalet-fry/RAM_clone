#include "testerworker.h"
#include <QMetaObject>
#include <QTimer>
#include <QDebug>
#include <functional>


TesterWorker::TesterWorker(MemoryModel* mem, QObject* parent)
: QObject(parent), _thread(), _mem(mem), _tester(nullptr) {
    // DO NOT create MemoryTester in constructor - this causes crashes
    // MemoryTester will be created in initialize() method, already in the target thread
    // This ensures proper thread affinity and avoids undefined behavior
    qDebug() << "TesterWorker constructor: completed";
}

void TesterWorker::initialize() {
    qDebug() << "TesterWorker::initialize: start";
    // Start thread first
    _thread.start();
    qDebug() << "TesterWorker::initialize: thread started";
    
    // Move this object to the worker thread FIRST
    // This ensures we're in the correct thread context before creating child objects
    this->moveToThread(&_thread);
    qDebug() << "TesterWorker::initialize: moved to thread";
    
    // Use QTimer::singleShot to create MemoryTester in the worker thread's event loop
    // This avoids deadlock issues with BlockingQueuedConnection
    QTimer::singleShot(0, this, [this]() {
        // This lambda runs in the worker thread context
        qDebug() << "TesterWorker::initialize: creating MemoryTester in worker thread";
        if (!_tester && _mem) {
            _tester.reset(new MemoryTester(_mem));
            qDebug() << "TesterWorker::initialize: MemoryTester created";
            
            // Now that both objects are in the same thread, connect signals with DirectConnection
            // for better performance
            connect(_tester.get(), &MemoryTester::progress, this, &TesterWorker::progress, Qt::DirectConnection);
            connect(_tester.get(), &MemoryTester::progressDetail, this, &TesterWorker::progressDetail, Qt::DirectConnection);
            connect(_tester.get(), &MemoryTester::finished, this, &TesterWorker::finished, Qt::DirectConnection);
            qDebug() << "TesterWorker::initialize: signals connected";
        } else {
            qDebug() << "TesterWorker::initialize: MemoryTester creation skipped (already exists or _mem is null)";
        }
    });
    qDebug() << "TesterWorker::initialize: QTimer::singleShot scheduled, returning";
}

TesterWorker::~TesterWorker() {
    // Disconnect all signals to prevent queued calls during destruction
    disconnect(this, nullptr, nullptr, nullptr);
    if (_tester) {
        disconnect(_tester.get(), nullptr, nullptr, nullptr);
    }
    
    // Stop any running operations
    if (_thread.isRunning()) {
        // Request thread to quit
        _thread.quit();
        
        // Wait for thread to finish, but with timeout to avoid hanging
        // Important: wait() must be called from a different thread than the one being waited on
        if (!_thread.wait(5000)) {
            // If thread doesn't finish in 5 seconds, terminate it
            _thread.terminate();
            _thread.wait();
        }
    }
}

void TesterWorker::run(TestAlgorithm algo) {
    // This slot runs in the worker thread (because object was moved).
    // _tester is a member variable, so it lives as long as TesterWorker
    if (_tester) {
        _tester->runTest(algo);
    }
}
