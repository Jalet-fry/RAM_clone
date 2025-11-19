#include "memorytablemanager.h"
#include "logger.h"
#include "constants.h"
#include "tableitemdelegate.h"
#include <QFont>
#include <QBrush>
#include <QColor>

MemoryTableManager::MemoryTableManager(QTableWidget* table, MemoryModel* mem, Logger* logger)
    : _table(table), _mem(mem), _logger(logger), _currentTheme(Theme::DeusEx), _lastHighlightedAddr(0) {
}

void MemoryTableManager::setMemoryModel(MemoryModel* mem) {
    _mem = mem;
}

void MemoryTableManager::setTestResults(const std::vector<TestResult>& results) {
    _lastResults = results;
}

void MemoryTableManager::clearTestResults() {
    _lastResults.clear();
    // Refresh table to remove color highlighting from test results
    if (_mem && _table) {
        refreshTable(0, _mem->size());
    }
}

void MemoryTableManager::setTheme(Theme theme) {
    _currentTheme = theme;
}

void MemoryTableManager::setLogger(Logger* logger) {
    _logger = logger;
}

QTableWidgetItem* MemoryTableManager::createOrGetTableItem(int row, int col) {
    if (!_table) return nullptr;
    QTableWidgetItem* item = _table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem;
        _table->setItem(row, col, item);
    }
    return item;
}

void MemoryTableManager::refreshTable(size_t begin, size_t end) {
    // Parameters are part of Qt signal signature but not used in implementation
    // Table is refreshed completely regardless of range
    Q_UNUSED(begin);
    Q_UNUSED(end);
    
    if (!_mem || !_table) return;
    
    // Оптимизация: отключаем обновление виджета во время массовых изменений
    _table->setUpdatesEnabled(false);
    
    size_t n = _mem->size();
    _table->setRowCount(int(n));

    auto f = _mem->currentFault();
    
    // Single pass through _lastResults to collect all needed data
    std::set<size_t> testedAddresses;
    std::set<size_t> failedAddresses;
    std::unordered_map<size_t, TestResult> resultMap; // Map address to test result for quick lookup
    
    for (const auto& r : _lastResults) {
        testedAddresses.insert(r.addr);
        if (!r.passed) {
            failedAddresses.insert(r.addr);
        }
        // Store result for quick lookup (last result for each address wins)
        resultMap[r.addr] = r;
    }

    // Получаем цвета один раз для всей таблицы (оптимизация)
    ThemeColors colors = ThemeManager::getColors(_currentTheme);

#ifdef DEBUG
    // Debug logging: Count faulty addresses
    if (_logger) {
        std::set<size_t> failedAddressesForLog;
        for (const auto& r : _lastResults) {
            if (!r.passed) {
                failedAddressesForLog.insert(r.addr);
            }
        }
        _logger->info(QString("refreshTable: Всего результатов: %1, неисправных адресов: %2")
                .arg(_lastResults.size()).arg(failedAddressesForLog.size()));
        _logger->info(QString("refreshTable: Цвета для красного выделения - фон: %1, текст: %2")
                .arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));
    }
#endif

    for (size_t i = 0; i < n; ++i) {
        Word v = _mem->read(i);
        bool isFaulty = (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len);
        bool isTested = testedAddresses.find(i) != testedAddresses.end();
        bool hasFailedTest = (failedAddresses.find(i) != failedAddresses.end());

        // Populate table data (address, hex, binary, decimal, status, fault type)
        populateTableData(i, v, f, testedAddresses, resultMap, colors);

        // Apply colors to table cells
        applyTableColors(i, isFaulty, isTested, hasFailedTest, colors);

        // Apply red highlighting for failed tests (highest priority)
        if (hasFailedTest) {
#ifdef DEBUG
            if (_logger) {
                _logger->info(QString("refreshTable: Адрес %1 - hasFailedTest=true, применяю красное выделение (фон: %2, текст: %3)")
                        .arg(i).arg(colors.failedTestBg.name()).arg(colors.failedTestText.name()));
            }
#endif
            applyFailedTestHighlighting(i, colors);
        }
#ifdef DEBUG
        else {
            if (_logger && failedAddresses.count(i) > 0) {
                _logger->error(QString("refreshTable: ОШИБКА! Адрес %1 в списке неисправных, но hasFailedTest=false!")
                        .arg(i));
            }
        }
#endif
    }

    // IMPORTANT: Apply red highlighting AGAIN after all updates
    // This guarantees it won't be overwritten
    // failedAddresses already collected in single pass above
    
    // Now apply red highlighting to all faulty addresses
#ifdef DEBUG
    if (_logger) {
        _logger->info(QString("refreshTable: Второй проход - применяю красное выделение к %1 адресам")
                .arg(failedAddresses.size()));
    }
#endif
    // Only apply red highlighting if there are failed addresses
    // If failedAddresses is empty (e.g., after reset), red highlighting should already be cleared
    // by applyTableColors setting hasFailedTest=false
    for (size_t addr : failedAddresses) {
        if (addr >= n) continue; // Boundary check
        // Apply red highlighting using helper method
        applyFailedTestHighlighting(addr, colors);
    }
    
    // IMPORTANT: If failedAddresses is empty, ensure all red highlighting is removed
    // This handles the case when memory is reset and _lastResults is cleared
    if (failedAddresses.empty()) {
        // Explicitly clear red highlighting for all addresses
        for (size_t i = 0; i < n; ++i) {
            bool isFaulty = (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len);
            bool isTested = testedAddresses.find(i) != testedAddresses.end();
            
            for (int col = 0; col < _table->columnCount(); ++col) {
                QTableWidgetItem* item = _table->item(int(i), col);
                if (!item) continue;
                
                // Reset failed test flag for all columns
                item->setData(Qt::UserRole + 1, QVariant(false));
                
                if (col == 4) {
                    // Status column - reset to proper color based on state
                    if (isTested) {
                        // If tested, use appropriate status color
                        item->setBackground(colors.statusPassedBg);
                        item->setForeground(colors.statusPassedText);
                    } else {
                        item->setBackground(colors.statusUntestedBg);
                        item->setForeground(colors.statusUntestedText);
                    }
                } else if (col == 5) {
                    // Fault type column - reset to proper color based on state
                    if (isFaulty) {
                        item->setForeground(colors.faultyNotTestedText);
                    } else if (isTested) {
                        item->setForeground(colors.tableText);
                    } else {
                        item->setForeground(colors.statusUntestedText);
                    }
                    // Reset background to default
                    item->setBackground(QBrush()); // Clear background
                } else {
                    // Data columns (0-3) - explicitly reset colors to remove any red highlighting
                    if (isFaulty && !isTested) {
                        item->setBackground(colors.faultyNotTestedBg);
                        item->setForeground(colors.faultyNotTestedText);
                    } else if (isTested) {
                        item->setBackground(colors.passedTestBg);
                        item->setForeground(colors.passedTestText);
                    } else {
                        if (i % 2 == 0) {
                            item->setBackground(colors.untestedBgEven);
                        } else {
                            item->setBackground(colors.untestedBgOdd);
                        }
                        item->setForeground(colors.statusUntestedText);
                    }
                }
            }
        }
    }

    // Re-enable widget updates
    _table->setUpdatesEnabled(true);
    
#ifdef DEBUG
    // Debug logging: Check actual colors of items after application
    if (_logger) {
        int verifiedCount = 0;
        int mismatchCount = 0;
        for (size_t addr : failedAddresses) {
            if (addr >= n) continue;
            bool allCorrect = true;
            for (int col = 0; col < _table->columnCount(); ++col) {
                if (col == 4) continue; // Skip status column
                QTableWidgetItem* item = _table->item(int(addr), col);
                if (item) {
                    QColor actualBg = item->background().color();
                    QColor expectedBg = colors.failedTestBg;
                    if (actualBg.rgb() != expectedBg.rgb()) {
                        allCorrect = false;
                        _logger->error(QString("refreshTable: НЕСООТВЕТСТВИЕ! Адрес %1, колонка %2 - ожидался фон %3, фактический %4")
                                .arg(addr).arg(col).arg(expectedBg.name()).arg(actualBg.name()));
                        mismatchCount++;
                    }
                }
            }
            if (allCorrect) {
                verifiedCount++;
            }
        }
        _logger->info(QString("refreshTable: Проверка завершена - правильно окрашено: %1, несоответствий: %2")
                .arg(verifiedCount).arg(mismatchCount));
    }
#endif
    
    // Оптимизация: resizeColumnsToContents очень медленный, вызываем только при необходимости
    // Можно вызывать реже или только при изменении размера окна
    // _table->resizeColumnsToContents(); // Отключено для производительности
}

void MemoryTableManager::populateTableData(size_t addr, const Word value, const InjectedFault& f,
                                  const std::set<size_t>& testedAddresses,
                                  const std::unordered_map<size_t, TestResult>& resultMap,
                                  const ThemeColors& colors) {
    if (!_table) return;
    
    int row = int(addr);
    bool isFaulty = (f.model != FaultModel::None && addr >= f.addr && addr < f.addr + f.len);
    bool isTested = testedAddresses.find(addr) != testedAddresses.end();
    
    // Address
    QTableWidgetItem* addrItem = createOrGetTableItem(row, 0);
    addrItem->setText(QString::number(addr));
    addrItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addrItem->setForeground(colors.tableText);

    // HEX
    QTableWidgetItem* hexItem = createOrGetTableItem(row, 1);
    hexItem->setText(QString("0x%1").arg(value, 8, 16, QChar('0')).toUpper());
    hexItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hexItem->setForeground(colors.tableText);

    // Binary
    QTableWidgetItem* binItem = createOrGetTableItem(row, 2);
    binItem->setText(DataFormatter::formatBinary(value));
    binItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    binItem->setFont(QFont("Courier", 9));
    binItem->setForeground(colors.tableText);

    // Decimal
    QTableWidgetItem* decItem = createOrGetTableItem(row, 3);
    decItem->setText(QString::number(value));
    decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    decItem->setForeground(colors.tableText);

    // Determine test result
    bool passed = true;
    auto resultIt = resultMap.find(addr);
    if (resultIt != resultMap.end()) {
        passed = resultIt->second.passed;
    } else if (isTested) {
        passed = true;
    }

    // Status
    QTableWidgetItem* statusItem = createOrGetTableItem(row, 4);
    if (!statusItem) return;
    
    QFont font = statusItem->font();
    font.setBold(false);
    statusItem->setFont(font);
    statusItem->setToolTip("");
    
    if (isTested) {
        if (passed) {
            statusItem->setText("Исправна");
            statusItem->setBackground(colors.statusPassedBg);
            statusItem->setForeground(colors.statusPassedText);
        } else {
            statusItem->setText("Неисправна");
            statusItem->setBackground(colors.statusFailedBg);
            statusItem->setForeground(colors.statusFailedText);
        }
    } else {
        statusItem->setText("Не протестировано");
        statusItem->setBackground(colors.statusUntestedBg);
        statusItem->setForeground(colors.statusUntestedText);
    }

    // Fault Type
    QTableWidgetItem* faultTypeItem = createOrGetTableItem(row, 5);
    if (!faultTypeItem) return;
    
    faultTypeItem->setFont(font);
    faultTypeItem->setToolTip("");
    
    // Note: Fault type column can also get red highlighting when test fails
    // The flag will be set by applyFailedTestHighlighting if needed
    
    if (isFaulty) {
        faultTypeItem->setText(DataFormatter::getFaultModelName(f.model));
        faultTypeItem->setForeground(colors.faultyNotTestedText);
        faultTypeItem->setBackground(QBrush()); // Clear background
    } else if (isTested) {
        auto resultIt2 = resultMap.find(addr);
        if (resultIt2 != resultMap.end() && !resultIt2->second.passed) {
            faultTypeItem->setText("Обнаружена");
            faultTypeItem->setForeground(colors.failedTestText);
            faultTypeItem->setBackground(QBrush()); // Clear background - text color indicates fault
            faultTypeItem->setToolTip(QString("Ожидалось: 0x%1, Прочитано: 0x%2")
                                      .arg(resultIt2->second.expected, 8, 16, QChar('0'))
                                      .arg(resultIt2->second.read, 8, 16, QChar('0')));
        } else {
            faultTypeItem->setText("—");
            faultTypeItem->setForeground(colors.tableText);
            faultTypeItem->setBackground(QBrush()); // Clear background
        }
    } else {
        faultTypeItem->setText("—");
        faultTypeItem->setForeground(colors.statusUntestedText);
        faultTypeItem->setBackground(QBrush()); // Clear background
    }
}

void MemoryTableManager::applyTableColors(size_t addr, bool isFaulty, bool isTested, bool hasFailedTest,
                                 const ThemeColors& colors) {
    if (!_table) return;
    
    // Apply colors to data columns (0-3) only
    // Status (4) and fault type (5) columns are handled in populateTableData
    // Red highlighting for failed tests is applied separately with higher priority to ALL columns
    for (int col = 0; col < 4; ++col) { // Only process data columns (Address, HEX, Binary, Decimal)
        QTableWidgetItem* item = _table->item(int(addr), col);
        if (!item) continue;

        // Reset bold font
        QFont itemFont = item->font();
        itemFont.setBold(false);
        item->setFont(itemFont);

        // Clear tooltips
        item->setToolTip("");

        // IMPORTANT: Always reset the failed test flag first
        // This ensures red highlighting is removed when hasFailedTest is false
        item->setData(Qt::UserRole + 1, QVariant(hasFailedTest));

        // Apply colors only if not a failed test (failed tests get red highlighting separately)
        if (!hasFailedTest) {
            // Explicitly reset background and foreground to remove any red highlighting
            if (isFaulty && !isTested) {
                // Golden/yellow for faulty area (not yet tested)
                item->setBackground(colors.faultyNotTestedBg);
                item->setForeground(colors.faultyNotTestedText);
            } else if (isTested) {
                // Green for tested and passed
                item->setBackground(colors.passedTestBg);
                item->setForeground(colors.passedTestText);
            } else {
                // Dark for untested
                if (addr % 2 == 0) {
                    item->setBackground(colors.untestedBgEven);
                } else {
                    item->setBackground(colors.untestedBgOdd);
                }
                item->setForeground(colors.statusUntestedText);
            }
        }
        // If hasFailedTest is true, red highlighting will be applied in applyFailedTestHighlighting
    }
}

void MemoryTableManager::applyFailedTestHighlighting(size_t addr, const ThemeColors& colors) {
    if (!_table) return;
    // Apply red highlighting to ALL columns in the row when test fails
    for (int col = 0; col < _table->columnCount(); ++col) {
        QTableWidgetItem* item = createOrGetTableItem(int(addr), col);
        if (!item) continue;
        // Force red highlighting for all columns
        QBrush redBrush(colors.failedTestBg);
        QBrush whiteBrush(colors.failedTestText);
        // Set via setData for guaranteed setting
        item->setData(Qt::BackgroundRole, redBrush);
        item->setData(Qt::ForegroundRole, whiteBrush);
        // Also set via setBackground/setForeground
        item->setBackground(redBrush);
        item->setForeground(whiteBrush);
        // Mark item as failed for delegate
        item->setData(Qt::UserRole + 1, QVariant(true)); // Mark as failed item
    }
}

void MemoryTableManager::highlightAddress(size_t addr) {
    if (!_table || !_mem) return;
    
    if (addr < _mem->size() && int(addr) < _table->rowCount()) {
        _table->selectRow(int(addr));
        _table->scrollToItem(_table->item(int(addr), 0), QAbstractItemView::EnsureVisible);
    }
}

void MemoryTableManager::scrollToAddress(size_t addr) {
    if (!_table || !_mem) return;
    
    if (addr < _mem->size() && int(addr) < _table->rowCount()) {
        QTableWidgetItem* item = _table->item(int(addr), 0);
        if (item) {
            _table->scrollToItem(item, QAbstractItemView::EnsureVisible);
        }
    }
}

void MemoryTableManager::updateProgressHighlight(size_t addr, size_t& lastHighlightedAddr) {
    if (!_table || !_mem) return;
    
    if (addr < _mem->size() && int(addr) < _table->rowCount()) {
        // Обновляем выделение только если адрес изменился значительно
        if (addr != lastHighlightedAddr) {
            // Clear previous bold highlighting (but keep background colors)
            if (lastHighlightedAddr < _mem->size() && int(lastHighlightedAddr) < _table->rowCount()) {
                for (int col = 0; col < _table->columnCount(); ++col) {
                    QTableWidgetItem* item = _table->item(int(lastHighlightedAddr), col);
                    if (item) {
                        QFont font = item->font();
                        font.setBold(false);
                        item->setFont(font);
                    }
                }
            }

            // Highlight current address with bold font
            ThemeColors colors = ThemeManager::getColors(_currentTheme);
            for (int col = 0; col < _table->columnCount(); ++col) {
                QTableWidgetItem* item = _table->item(int(addr), col);
                if (item) {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                    // Don't change background if it's a failed test or faulty area
                    QColor currentBg = item->background().color();
                    QRgb currentBgRgb = currentBg.rgb();
                    QRgb failedTestBgRgb = colors.failedTestBg.rgb();
                    QRgb faultyNotTestedBgRgb = colors.faultyNotTestedBg.rgb();
                    bool isSpecialColor = (currentBgRgb == failedTestBgRgb || currentBgRgb == faultyNotTestedBgRgb);
                    
                    if (!isSpecialColor) {
                        // Use standard table colors from theme
                        if (addr % 2 == 0) {
                            item->setBackground(colors.tableBgEven);
                        } else {
                            item->setBackground(colors.tableBgOdd);
                        }
                        item->setForeground(colors.tableText);
                    }
                }
            }
            lastHighlightedAddr = addr;

            // Scroll only every N addresses for performance
            if (addr % TABLE_SCROLL_INTERVAL == 0) {
                _table->scrollToItem(_table->item(int(addr), 0), QAbstractItemView::EnsureVisible);
            }
        }
    }
}

