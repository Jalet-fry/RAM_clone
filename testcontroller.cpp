#include "testcontroller.h"
#include "constants.h"
#include <QMessageBox>
#include <QMetaObject>

TestController::TestController(TesterWorker* worker,
                               MemoryModel* mem,
                               MemoryTableManager* tableManager,
                               QComboBox* algoCombo,
                               QPushButton* startBtn,
                               QProgressBar* progress,
                               QLabel* testInfoLabel,
                               QLabel* currentAddrLabel,
                               QLabel* expectedValueLabel,
                               QLabel* readValueLabel,
                               Logger* logger,
                               QObject* parent)
    : QObject(parent), _worker(worker), _mem(mem), _tableManager(tableManager), _logger(logger),
      _testRunning(false), _lastHighlightedAddr(0), _lastTestTimeMsecs(0),
      _algoCombo(algoCombo), _startBtn(startBtn), _progress(progress),
      _testInfoLabel(testInfoLabel), _currentAddrLabel(currentAddrLabel),
      _expectedValueLabel(expectedValueLabel), _readValueLabel(readValueLabel) {
}

void TestController::setWorker(TesterWorker* worker) {
    _worker = worker;
}

void TestController::setMemoryModel(MemoryModel* mem) {
    _mem = mem;
}

void TestController::setTableManager(MemoryTableManager* tableManager) {
    _tableManager = tableManager;
}

void TestController::setLogger(Logger* logger) {
    _logger = logger;
}

void TestController::setTestRunning(bool running) {
    _testRunning = running;
}

void TestController::startTest() {
    if (!_mem || !_worker || !_logger) return;
    
    // Thread-safe check: prevent multiple test starts
    if (_testRunning) {
        if (_logger) {
            _logger->warning("Тест уже выполняется. Дождитесь завершения.");
        }
        return;
    }

    // Additional safety check: ensure worker is ready
    if (!_worker) {
        if (_logger) {
            _logger->error("Worker не инициализирован. Невозможно запустить тест.");
        }
        return;
    }

    // Additional safety checks for UI elements
    if (!_algoCombo || !_startBtn || !_currentAddrLabel || !_expectedValueLabel || !_readValueLabel) {
        if (_logger) {
            _logger->error("UI элементы не инициализированы. Невозможно запустить тест.");
        }
        return;
    }

    TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
    _testRunning = true;
    _testStartTime = QTime::currentTime();
    _startBtn->setEnabled(false);
    _lastHighlightedAddr = 0;

    // Reset progress details
    _currentAddrLabel->setText("Адрес: —");
    _expectedValueLabel->setText("Ожидается: —");
    _readValueLabel->setText("Прочитано: —");

    if (_logger) {
        _logger->info(QString("Запуск теста: %1").arg(_algoCombo->currentText()));
    }
    
    emit testStarted();
    QMetaObject::invokeMethod(_worker, "run", Qt::QueuedConnection, Q_ARG(TestAlgorithm, algo));
}

void TestController::onTestFinished(const std::vector<TestResult>& results) {
    // Thread-safe: this slot is called from main thread via Qt signal/slot mechanism
    // Reset test state atomically
    _testRunning = false;
    if (_startBtn) {
        _startBtn->setEnabled(true);
    }
    
    int fails = 0;
    for (const auto& r : results) if (!r.passed) ++fails;

    int elapsed = _testStartTime.msecsTo(QTime::currentTime());
    _lastTestTimeMsecs = elapsed;
    QString timeStr = QString("%1.%2 сек").arg(elapsed / 1000).arg((elapsed % 1000) / 100, 2, 10, QChar('0'));

    if (fails == 0) {
        if (_logger) {
            _logger->success(QString("Тест завершен успешно. Всего проверок: %1, неисправностей не обнаружено. Время: %2")
                       .arg(results.size()).arg(timeStr));
        }
    } else {
        if (_logger) {
            _logger->error(QString("Тест завершен. Всего проверок: %1, обнаружено неисправностей: %2. Время: %3")
                     .arg(results.size()).arg(fails).arg(timeStr));
        }
    }

    emit testFinished(results);
    emit testResultsUpdated(results);
}

void TestController::onAlgorithmChanged(int index) {
    // Index parameter is part of Qt signal signature
    Q_UNUSED(index);
    if (_testInfoLabel && _algoCombo) {
        TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
        QString desc = DataFormatter::getAlgorithmDescription(algo);
        _testInfoLabel->setText(QString("Алгоритм: %1\n\n%2").arg(_algoCombo->currentText()).arg(desc));
    }
}

void TestController::updateProgressDetails(size_t addr, Word expected, Word read) {
    highlightCurrentAddress(addr);
    if (_expectedValueLabel) {
        _expectedValueLabel->setText(QString("Ожидается: 0x%1").arg(expected, 8, 16, QChar('0')).toUpper());
    }
    if (_readValueLabel) {
        _readValueLabel->setText(QString("Прочитано: 0x%1").arg(read, 8, 16, QChar('0')).toUpper());
    }

    // Update table highlighting using table manager
    if (_tableManager) {
        _tableManager->updateProgressHighlight(addr, _lastHighlightedAddr);
    }
}

void TestController::highlightCurrentAddress(size_t addr) {
    if (_currentAddrLabel) {
        _currentAddrLabel->setText(QString("Адрес: %1").arg(addr));
    }
    if (_tableManager) {
        _tableManager->highlightAddress(addr);
    }
}

