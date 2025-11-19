#include "resultsnavigator.h"

ResultsNavigator::ResultsNavigator(QTableWidget* table, MemoryModel* mem, Logger* logger)
    : _table(table), _mem(mem), _logger(logger) {
}

void ResultsNavigator::setTableWidget(QTableWidget* table) {
    _table = table;
}

void ResultsNavigator::setMemoryModel(MemoryModel* mem) {
    _mem = mem;
}

void ResultsNavigator::setLogger(Logger* logger) {
    _logger = logger;
}

void ResultsNavigator::scrollToNextFault(const std::vector<TestResult>& results, size_t currentAddr) {
    if (!_table || !_mem || !_logger) return;
    
    // Проверяем, были ли запущены тесты
    if (results.empty()) {
        if (_logger) {
            _logger->warning("Тесты еще не запускались. Сначала запустите тест памяти.");
        }
        QMessageBox::information(qobject_cast<QWidget*>(_table->parent()), "Тесты не запущены", 
                                "Тесты еще не запускались.\nСначала запустите тест памяти.");
        return;
    }
    
    // Собираем все адреса с неисправностями в отсортированный список
    // ВАЖНО: Используем актуальные результаты из results
    std::set<size_t> faultAddresses;
    for (const auto& r : results) {
        if (!r.passed) {
            faultAddresses.insert(r.addr);
        }
    }
    
    // Если неисправностей нет
    if (faultAddresses.empty()) {
        if (_logger) {
            _logger->warning("Неисправности не найдены тестами. Все проверки прошли успешно.");
        }
        QMessageBox::information(qobject_cast<QWidget*>(_table->parent()), "Неисправности не найдены", 
                                "Тестами не обнаружено неисправностей.\nВсе проверки прошли успешно.");
        return;
    }
    
    // Получаем текущую выбранную строку
    int currentRow = _table->currentRow();
    size_t startAddr = currentAddr;
    
    // Если есть выбранная строка, используем её адрес
    if (currentRow >= 0 && currentRow < int(_mem->size())) {
        startAddr = static_cast<size_t>(currentRow);
    }
    
    size_t nextAddr = 0;
    bool found = false;
    
    // Ищем следующую неисправность, начиная со следующего адреса после текущего
    // Используем upper_bound чтобы найти первый адрес строго больше текущего
    auto it = faultAddresses.upper_bound(startAddr);
    
    if (it != faultAddresses.end()) {
        // Нашли неисправность после текущей позиции
        nextAddr = *it;
        found = true;
    } else {
        // Дошли до конца, переходим к началу (циклический поиск)
        it = faultAddresses.begin();
        if (it != faultAddresses.end()) {
            nextAddr = *it;
            // Если мы уже на неисправном адресе и есть другие неисправности, переходим к следующей
            if (nextAddr == startAddr && faultAddresses.size() > 1) {
                ++it;
                if (it != faultAddresses.end()) {
                    nextAddr = *it;
                } else {
                    // Если следующей нет, берем первую
                    nextAddr = *faultAddresses.begin();
                }
            }
            found = true;
        }
    }
    
    if (!found) {
        if (_logger) {
            _logger->warning("Не удалось найти следующую неисправность.");
        }
        return;
    }
    
    // ВАЖНО: Проверяем, что найденный адрес действительно неисправен в текущих результатах
    bool isActuallyFaulty = false;
    for (const auto& r : results) {
        if (r.addr == nextAddr && !r.passed) {
            isActuallyFaulty = true;
            break;
        }
    }
    
    if (!isActuallyFaulty) {
        if (_logger) {
            _logger->warning(QString("Адрес %1 больше не является неисправным. Обновляю таблицу...").arg(nextAddr));
        }
        // Note: Table refresh should be handled by caller
        return;
    }
    
    // Прокручиваем к найденной неисправности
    int row = int(nextAddr);
    if (row >= 0 && row < _table->rowCount()) {
        _table->setCurrentCell(row, 0);
        _table->scrollTo(_table->model()->index(row, 0), QAbstractItemView::EnsureVisible);
        _table->selectRow(row);
        if (_logger) {
            _logger->info(QString("Переход к следующей неисправности по адресу: %1").arg(nextAddr));
        }
    }
}

