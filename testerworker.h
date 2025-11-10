#ifndef TESTERWORKER_H
#define TESTERWORKER_H

#include <QObject>
#include <QThread>
#include "memorytester.h"


// TesterWorker is a small wrapper that lives in its own QThread.
class TesterWorker : public QObject {
Q_OBJECT
public:
explicit TesterWorker(MemoryModel* mem, QObject* parent = nullptr);
~TesterWorker() override;


public slots:
void run(TestAlgorithm algo); // queued: runs testers in worker thread


signals:
void progress(int percent);
void finished(const std::vector<TestResult>& results);


private:
QThread _thread;
MemoryModel* _mem;
};
#endif // TESTERWORKER_H
