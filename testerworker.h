#ifndef TESTERWORKER_H
#define TESTERWORKER_H

#include <QObject>
#include <QThread>
#include <memory>
#include "types.h"
#include "memorytester.h"

class TesterWorker : public QObject {
    Q_OBJECT
public:
    explicit TesterWorker(MemoryModel* mem, QObject* parent = nullptr);
    ~TesterWorker() override;

public slots:
    void run(TestAlgorithm algo);

signals:
    void progress(int percent);
    void progressDetail(size_t addr, Word expected, Word read);
    void finished(const std::vector<TestResult>& results);

private:
    QThread _thread;
    MemoryModel* _mem;
    std::unique_ptr<MemoryTester> _tester;
};

#endif // TESTERWORKER_H