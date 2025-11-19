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

// Memory configuration
constexpr size_t DEFAULT_MEMORY_SIZE = 256;
constexpr size_t MAX_MEMORY_SIZE = 256;

// Word configuration
constexpr int BITS_PER_WORD = 32;
constexpr unsigned int INVALID_READ_MARKER = 0xFFFFFFFFu;

// UI configuration
constexpr int PROGRESS_MAX_PERCENT = 100;
constexpr int DEFAULT_FAULT_LENGTH = 100;
constexpr int TABLE_SCROLL_INTERVAL = 20;  // Scroll table every N addresses

#endif // CONSTANTS_H

