#ifndef CONSTANTS_H
#define CONSTANTS_H

// Задержка для визуализации прогресса тестирования (миллисекунды)
constexpr int VISUALIZATION_DELAY_MS = 60;

// Интервал обновления прогресса (каждые N адресов)
constexpr int PROGRESS_UPDATE_INTERVAL = 10;

// Процент прогресса для фазы записи/чтения в тестах WalkingOnes и WalkingZeros
constexpr double PROGRESS_PHASE_PERCENT = 50.0;

// Процент прогресса для каждого шага в MarchSimple тесте
constexpr double PROGRESS_MARCH_PERCENT = 25.0;

#endif // CONSTANTS_H

