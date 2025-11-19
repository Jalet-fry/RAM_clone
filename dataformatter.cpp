#include "dataformatter.h"

QString DataFormatter::formatBinary(Word value) {
    QString binary;
    for (int i = 31; i >= 0; --i) {
        binary += (value & (1u << i)) ? '1' : '0';
        if (i % 4 == 0 && i > 0) binary += ' ';
    }
    return binary;
}

QString DataFormatter::getFaultModelName(FaultModel model) {
    switch (model) {
        case FaultModel::None: return "Нет";
        case FaultModel::StuckAt0: return "Stuck-at-0";
        case FaultModel::StuckAt1: return "Stuck-at-1";
        case FaultModel::BitFlip: return "Bit-flip";
        case FaultModel::OpenRead: return "Open (invalid read)";
        default: return "Неизвестно";
    }
}

QString DataFormatter::getAlgorithmDescription(TestAlgorithm algo) {
    switch (algo) {
        case TestAlgorithm::WalkingOnes:
            return "Записывает единицу в каждый бит позиции и проверяет, что она сохраняется. "
                   "Обнаруживает залипания битов и ошибки чтения/записи.";
        case TestAlgorithm::WalkingZeros:
            return "Записывает ноль в каждый бит позиции и проверяет, что он сохраняется. "
                   "Обнаруживает залипания битов и ошибки чтения/записи.";
        case TestAlgorithm::MarchSimple:
            return "Простой маршевый тест: записывает все нули, проверяет нули, "
                   "записывает все единицы, проверяет единицы. Обнаруживает основные неисправности памяти.";
        default:
            return "Описание недоступно";
    }
}

