#ifndef DATAFORMATTER_H
#define DATAFORMATTER_H

#include <QString>
#include "types.h"

class DataFormatter {
public:
    // Форматирование двоичного представления
    static QString formatBinary(Word value);
    
    // Получение имени модели неисправности
    static QString getFaultModelName(FaultModel model);
    
    // Получение описания алгоритма тестирования
    static QString getAlgorithmDescription(TestAlgorithm algo);
};

#endif // DATAFORMATTER_H

