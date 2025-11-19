#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>

// Базовый тип для слова памяти
using Word = uint32_t;

// Модели неисправностей
enum class FaultModel {
    None,
    StuckAt0,
    StuckAt1,
    BitFlip,
    OpenRead
};

// Структура для описания внедренной неисправности
struct InjectedFault {
    FaultModel model = FaultModel::None;
    size_t addr = 0;
    size_t len = 1;
    double flip_probability = 0.01;
};

// Алгоритмы тестирования памяти
enum class TestAlgorithm {
    WalkingOnes,
    WalkingZeros,
    MarchSimple
};

// Результат тестирования одного адреса
struct TestResult {
    size_t addr;
    Word expected;
    Word read;
    bool passed;
};

#endif // TYPES_H

