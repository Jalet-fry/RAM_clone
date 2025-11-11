#include "memorytester.h"
#include <QThread>
#include <cmath>

MemoryTester::MemoryTester(MemoryModel* mem, QObject* parent)
    : QObject(parent), _mem(mem) {}

void MemoryTester::runTest(TestAlgorithm algo) {
    _results.clear();
    size_t n = _mem->size();
    if (n == 0) { emit progress(100); emit finished(_results); return; }

    if (algo == TestAlgorithm::WalkingOnes) {
        for (size_t a = 0; a < n; ++a) {
            Word pattern = (1u << (a % 32));
            _mem->write(a, pattern);
        }
        for (size_t a = 0; a < n; ++a) {
            Word expected = (1u << (a % 32));
            Word r = _mem->read(a);
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            int percent = int(((a + 1) * 100.0) / n);
            emit progress(percent);
            emit progressDetail(a, expected, r);
            QThread::msleep(2);
        }
    } else if (algo == TestAlgorithm::WalkingZeros) {
        for (size_t a = 0; a < n; ++a) {
            Word pattern = ~(1u << (a % 32));
            _mem->write(a, pattern);
        }
        for (size_t a = 0; a < n; ++a) {
            Word expected = ~(1u << (a % 32));
            Word r = _mem->read(a);
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            int percent = int(((a + 1) * 100.0) / n);
            emit progress(percent);
            emit progressDetail(a, expected, r);
            QThread::msleep(2);
        }
    } else if (algo == TestAlgorithm::MarchSimple) {
        // write 0s
        for (size_t a = 0; a < n; ++a) _mem->write(a, 0u);
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == 0u);
            _results.push_back({a, 0u, r, pass});
            int percent = int(((a + 1) * 50.0) / n);
            emit progress(percent);
            emit progressDetail(a, 0u, r);
            QThread::msleep(1);
        }
        // write 1s
        for (size_t a = 0; a < n; ++a) _mem->write(a, ~0u);
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == ~0u);
            _results.push_back({a, ~0u, r, pass});
            int percent = 50 + int(((a + 1) * 50.0) / n);
            emit progress(percent);
            emit progressDetail(a, ~0u, r);
            QThread::msleep(1);
        }
    }

    emit finished(_results);
}