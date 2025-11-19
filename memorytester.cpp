#include "memorytester.h"
#include "constants.h"
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
            // Обновляем прогресс реже для производительности
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                int percent = int((a * PROGRESS_PHASE_PERCENT) / n);
                emit progress(percent);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        
        // Фаза 2: Чтение и проверка (здесь применяются неисправности)
        for (size_t a = 0; a < n; ++a) {
            Word expected = (1u << (a % 32)); // То, что мы записали
            Word r = _mem->read(a);           // То, что прочитали (возможно с неисправностью)
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            // Обновляем прогресс и детали реже для производительности
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                int percent = int(PROGRESS_PHASE_PERCENT) + int(((a + 1) * PROGRESS_PHASE_PERCENT) / n);
                emit progress(percent);
                emit progressDetail(a, expected, r);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        emit progress(100); // Гарантируем 100% в конце
        
    } else if (algo == TestAlgorithm::WalkingZeros) {
        // Фаза 1: Запись эталонных данных
        for (size_t a = 0; a < n; ++a) {
            Word pattern = ~(1u << (a % 32));
            _mem->writeDirect(a, pattern);
            // Обновляем прогресс реже для производительности
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                int percent = int((a * PROGRESS_PHASE_PERCENT) / n);
                emit progress(percent);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        
        // Фаза 2: Чтение и проверка
        for (size_t a = 0; a < n; ++a) {
            Word expected = ~(1u << (a % 32));
            Word r = _mem->read(a);
            bool pass = (expected == r);
            _results.push_back({a, expected, r, pass});
            // Обновляем прогресс и детали реже для производительности
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                int percent = int(PROGRESS_PHASE_PERCENT) + int(((a + 1) * PROGRESS_PHASE_PERCENT) / n);
                emit progress(percent);
                emit progressDetail(a, expected, r);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        emit progress(100); // Гарантируем 100% в конце
        
    } else if (algo == TestAlgorithm::MarchSimple) {
        // Марш-тест: запись 0 → чтение 0 → запись 1 → чтение 1
        
        // Шаг 1: Запись всех 0
        for (size_t a = 0; a < n; ++a) {
            _mem->writeDirect(a, 0u);
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                emit progress(int((a * PROGRESS_MARCH_PERCENT) / n));
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        
        // Шаг 2: Чтение всех 0 (ожидаем 0)
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == 0u);
            _results.push_back({a, 0u, r, pass});
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                emit progress(int(PROGRESS_MARCH_PERCENT) + int(((a + 1) * PROGRESS_MARCH_PERCENT) / n));
                emit progressDetail(a, 0u, r);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        
        // Шаг 3: Запись всех 1
        for (size_t a = 0; a < n; ++a) {
            _mem->writeDirect(a, ~0u);
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                emit progress(int(PROGRESS_MARCH_PERCENT * 2) + int((a * PROGRESS_MARCH_PERCENT) / n));
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        
        // Шаг 4: Чтение всех 1 (ожидаем 1)
        for (size_t a = 0; a < n; ++a) {
            Word r = _mem->read(a);
            bool pass = (r == ~0u);
            _results.push_back({a, ~0u, r, pass});
            if (a % PROGRESS_UPDATE_INTERVAL == 0 || a == n - 1) {
                emit progress(int(PROGRESS_MARCH_PERCENT * 3) + int(((a + 1) * PROGRESS_MARCH_PERCENT) / n));
                emit progressDetail(a, ~0u, r);
                // Добавляем небольшую задержку для визуализации
                QThread::msleep(VISUALIZATION_DELAY_MS);
            }
        }
        emit progress(100); // Гарантируем 100% в конце
    }

    emit finished(_results);
}