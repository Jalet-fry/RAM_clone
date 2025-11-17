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
        // Фаза 1: Запись эталонных данных
        for (size_t a = 0; a < n; ++a) {
            Word pattern = (1u << (a % 32));
            _mem->writeDirect(a, pattern); // Гарантированная запись без искажений
            int percent = int((a * 50.0) / n);
            emit progress(percent);
            QThread::msleep(1);
        }
        
        // Фаза 2: Чтение и проверка (здесь применяются неисправности)
        for (size_t a = 0; a < n; ++a) {
            Word expected = (1u << (a % 32)); // То, что мы записали
            Word r = _mem->read(a);           // То, что прочитали (возможно с неисправностью)
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            int percent = 50 + int(((a + 1) * 50.0) / n);
            emit progress(percent);
            QThread::msleep(2);
        }
        
    } else if (algo == TestAlgorithm::WalkingZeros) {
        // Фаза 1: Запись эталонных данных
        for (size_t a = 0; a < n; ++a) {
            Word pattern = ~(1u << (a % 32));
            _mem->writeDirect(a, pattern);
            int percent = int((a * 50.0) / n);
            emit progress(percent);
            QThread::msleep(1);
        }
        
        // Фаза 2: Чтение и проверка
        for (size_t a = 0; a < n; ++a) {
            Word expected = ~(1u << (a % 32));
            Word r = _mem->read(a);
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            int percent = 50 + int(((a + 1) * 50.0) / n);
            emit progress(percent);
            QThread::msleep(2);
        }
        
    } else if (algo == TestAlgorithm::MarchSimple) {
        // Марш-тест: запись 0 → чтение 0 → запись 1 → чтение 1
        
        // Шаг 1: Запись всех 0
        for (size_t a = 0; a < n; ++a) {
            _mem->writeDirect(a, 0u);
            emit progress(int((a * 25.0) / n));
            QThread::msleep(1);
        }
        
        // Шаг 2: Чтение всех 0 (ожидаем 0)
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == 0u);
            _results.push_back({a, 0u, r, pass});
            emit progress(25 + int(((a + 1) * 25.0) / n));
            QThread::msleep(1);
        }
        
        // Шаг 3: Запись всех 1
        for (size_t a = 0; a < n; ++a) {
            _mem->writeDirect(a, ~0u);
            emit progress(50 + int((a * 25.0) / n));
            QThread::msleep(1);
        }
        
        // Шаг 4: Чтение всех 1 (ожидаем 1)
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == ~0u);
            _results.push_back({a, ~0u, r, pass});
            emit progress(75 + int(((a + 1) * 25.0) / n));
            QThread::msleep(1);
        }
    }

    emit finished(_results);
}