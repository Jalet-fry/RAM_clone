#include "statisticsmanager.h"
#include "constants.h"

StatisticsManager::StatisticsManager(QLabel* totalAddressesLabel,
                                     QLabel* testedAddressesLabel,
                                     QLabel* faultsFoundLabel,
                                     QLabel* coverageLabel,
                                     QLabel* testTimeLabel,
                                     QLabel* currentFaultModelLabel,
                                     QLabel* currentAlgorithmLabel,
                                     QLabel* faultInfoLabel,
                                     QLabel* testInfoLabel,
                                     QComboBox* algoCombo,
                                     MemoryModel* mem)
    : _mem(mem), _currentTheme(Theme::DeusEx), _testTimeMsecs(0),
      _totalAddressesLabel(totalAddressesLabel),
      _testedAddressesLabel(testedAddressesLabel),
      _faultsFoundLabel(faultsFoundLabel),
      _coverageLabel(coverageLabel),
      _testTimeLabel(testTimeLabel),
      _currentFaultModelLabel(currentFaultModelLabel),
      _currentAlgorithmLabel(currentAlgorithmLabel),
      _faultInfoLabel(faultInfoLabel),
      _testInfoLabel(testInfoLabel),
      _algoCombo(algoCombo) {
}

void StatisticsManager::setMemoryModel(MemoryModel* mem) {
    _mem = mem;
}

void StatisticsManager::setTestResults(const std::vector<TestResult>& results) {
    _lastResults = results;
}

void StatisticsManager::clearTestResults() {
    _lastResults.clear();
    _testTimeMsecs = 0;
    // Update statistics to reflect cleared state
    updateStatistics();
}

void StatisticsManager::setTestTime(int msecs) {
    _testTimeMsecs = msecs;
}

void StatisticsManager::setTheme(Theme theme) {
    _currentTheme = theme;
}

void StatisticsManager::setAlgorithmCombo(QComboBox* algoCombo) {
    _algoCombo = algoCombo;
}

void StatisticsManager::updateStatistics() {
    if (!_mem) return;
    
    _totalAddressesLabel->setText(QString("Всего адресов: %1").arg(_mem->size()));

    int testedCount = _lastResults.size();
    int faultsFound = 0;
    for (const auto& r : _lastResults) {
        if (!r.passed) ++faultsFound;
    }

    _testedAddressesLabel->setText(QString("Протестировано: %1").arg(testedCount));
    _faultsFoundLabel->setText(QString("Найдено неисправностей: %1").arg(faultsFound));

    double coverage = _mem->size() > 0 ? (testedCount * 100.0 / _mem->size()) : 0.0;
    _coverageLabel->setText(QString("Покрытие: %1%").arg(coverage, 0, 'f', 1));

    auto f = _mem->currentFault();
    _currentFaultModelLabel->setText(QString("Текущая модель: %1").arg(DataFormatter::getFaultModelName(f.model)));
    
    if (_testTimeMsecs > 0) {
        QString timeStr = QString("%1.%2 сек").arg(_testTimeMsecs / 1000).arg((_testTimeMsecs % 1000) / 100, 2, 10, QChar('0'));
        _testTimeLabel->setText(QString("Время теста: %1").arg(timeStr));
    }
    
    if (_algoCombo) {
        _currentAlgorithmLabel->setText(QString("Текущий алгоритм: %1").arg(_algoCombo->currentText()));
    }
}

void StatisticsManager::updateFaultInfo() {
    if (!_mem) return;
    
    auto f = _mem->currentFault();
    ThemeColors colors = ThemeManager::getColors(_currentTheme);
    if (f.model == FaultModel::None) {
        _faultInfoLabel->setText("Неисправность не внедрена");
        _faultInfoLabel->setStyleSheet(QString("padding: 5px; background-color: %1; border: 1px solid %2; color: %3;")
                                       .arg(colors.bgSecondary.name())
                                       .arg(colors.accent.name())
                                       .arg(colors.text.name()));
    } else {
        QString info = QString("Тип: %1\nАдрес: %2\nДлина: %3 слов")
                       .arg(DataFormatter::getFaultModelName(f.model))
                       .arg(f.addr)
                       .arg(f.len);
        if (f.model == FaultModel::BitFlip) {
            info += QString("\nВероятность инверсии: %1%").arg(f.flip_probability * PROGRESS_MAX_PERCENT, 0, 'f', 1);
        }
        _faultInfoLabel->setText(info);
        _faultInfoLabel->setStyleSheet(QString("padding: 5px; background-color: %1; border: 1px solid %2; color: %3;")
                                       .arg(colors.faultyNotTestedBg.name())
                                       .arg(colors.accent.name())
                                       .arg(colors.text.name()));
    }
    updateStatistics();
}

void StatisticsManager::updateTestInfo() {
    if (!_algoCombo) return;
    
    TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
    QString desc = DataFormatter::getAlgorithmDescription(algo);
    _testInfoLabel->setText(QString("Алгоритм: %1\n\n%2").arg(_algoCombo->currentText()).arg(desc));
}

