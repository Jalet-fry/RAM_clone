#include "mainwindow.h"
#include <QtWidgets>
#include <QMessageBox>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Digital Twin — RAM Testbench");
    resize(1000, 600);

    _mem = new MemoryModel(256, this);
    _worker = new TesterWorker(_mem);

    QWidget* central = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(central);

    // Controls
    QHBoxLayout* controls = new QHBoxLayout;
    _faultCombo = new QComboBox;
    _faultCombo->addItem("None", (int)FaultModel::None);
    _faultCombo->addItem("Stuck-at-0", (int)FaultModel::StuckAt0);
    _faultCombo->addItem("Stuck-at-1", (int)FaultModel::StuckAt1);
    _faultCombo->addItem("Bit-flip", (int)FaultModel::BitFlip);
    _faultCombo->addItem("Open (invalid read)", (int)FaultModel::OpenRead);

    controls->addWidget(new QLabel("Fault model:"));
    controls->addWidget(_faultCombo);

    _addrEdit = new QLineEdit("0");
    _lenEdit = new QLineEdit("1");
    controls->addWidget(new QLabel("Address (word):"));
    controls->addWidget(_addrEdit);
    controls->addWidget(new QLabel("Length (words):"));
    controls->addWidget(_lenEdit);

    _flipProbSpin = new QDoubleSpinBox;
    _flipProbSpin->setRange(0.0, 1.0);
    _flipProbSpin->setSingleStep(0.01);
    _flipProbSpin->setValue(0.01);
    controls->addWidget(new QLabel("Flip prob (for Bit-flip):"));
    controls->addWidget(_flipProbSpin);

    _injectBtn = new QPushButton("Inject Fault");
    controls->addWidget(_injectBtn);
    _resetBtn = new QPushButton("Reset Memory");
    controls->addWidget(_resetBtn);

    lay->addLayout(controls);

    // Test controls
    QHBoxLayout* testRow = new QHBoxLayout;
    _algoCombo = new QComboBox;
    _algoCombo->addItem("Walking 1s", (int)TestAlgorithm::WalkingOnes);
    _algoCombo->addItem("Walking 0s", (int)TestAlgorithm::WalkingZeros);
    _algoCombo->addItem("March - simple", (int)TestAlgorithm::MarchSimple);
    testRow->addWidget(new QLabel("Test algorithm:"));
    testRow->addWidget(_algoCombo);

    _startBtn = new QPushButton("Start Test");
    testRow->addWidget(_startBtn);

    _progress = new QProgressBar;
    _progress->setRange(0, 100);
    _progress->setValue(0);
    testRow->addWidget(_progress);

    lay->addLayout(testRow);

    // Table + log
    QHBoxLayout* bottom = new QHBoxLayout;
    _table = new QTableWidget;
    _table->setColumnCount(2);
    _table->setHorizontalHeaderLabels({"Address", "Value (hex)"});
    _table->verticalHeader()->setVisible(false);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bottom->addWidget(_table, 3);

    _log = new QTextEdit;
    _log->setReadOnly(true);
    bottom->addWidget(_log, 2);

    lay->addLayout(bottom);

    setCentralWidget(central);

    connect(_injectBtn, &QPushButton::clicked, this, &MainWindow::onInject);
    connect(_resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(_startBtn, &QPushButton::clicked, this, &MainWindow::onStartTest);

    // connect worker signals (queued automatically across threads)
    connect(_worker, &TesterWorker::progress, _progress, &QProgressBar::setValue);
    connect(_worker, &TesterWorker::finished, this, &MainWindow::onTestFinished);

    connect(_mem, &MemoryModel::dataChanged, this, &MainWindow::refreshTable);

    refreshTable(0, _mem->size());
}

MainWindow::~MainWindow()
{
    if (_worker) {
        // deleteLater нужно выполнить в потоке, где живёт объект.
        QMetaObject::invokeMethod(_worker, "deleteLater", Qt::QueuedConnection);
        _worker = nullptr;
    }
}

void MainWindow::onInject() {
    InjectedFault f;
    f.model = static_cast<FaultModel>(_faultCombo->currentData().toInt());
    bool ok1 = false, ok2 = false;
    size_t addr = _addrEdit->text().toULongLong(&ok1);
    size_t len = _lenEdit->text().toULongLong(&ok2);
    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "Input error", "Address/Length must be integer.");
        return;
    }
    f.addr = addr;
    f.len = std::max<size_t>(1, len);
    f.flip_probability = _flipProbSpin->value();
    _mem->injectFault(f);
    _log->append(QString("Injected fault: %1 at %2 len=%3").arg(_faultCombo->currentText()).arg((qulonglong)addr).arg((qulonglong)len));
}

void MainWindow::onReset() {
    _mem->reset();
    _log->append("Memory reset.");
    _progress->setValue(0);
}

void MainWindow::onStartTest() {
    TestAlgorithm algo = static_cast<TestAlgorithm>(_algoCombo->currentData().toInt());
    _log->append("Starting test: " + _algoCombo->currentText());
    // invoke worker slot in its thread (queued connection because object lives in worker thread)
    QMetaObject::invokeMethod(_worker, "run", Qt::QueuedConnection, Q_ARG(TestAlgorithm, algo));
}

void MainWindow::onTestFinished(const std::vector<TestResult>& results) {
    int fails = 0;
    for (const auto& r : results) if (!r.passed) ++fails;
    _log->append(QString("Test finished. Results: total=%1 fails=%2").arg((int)results.size()).arg(fails));

    // color table rows for failed addresses
    for (const auto& r : results) {
        if (!r.passed) {
            int row = int(r.addr);
            if (row < _table->rowCount() && row >= 0) {
                auto item = _table->item(row, 1);
                if (item) {
                    item->setBackground(Qt::red);
                    item->setToolTip(QString("Expected: 0x%1 Read: 0x%2").arg(r.expected, 0, 16).arg(r.read, 0, 16));
                }
            }
        }
    }
}

void MainWindow::refreshTable(size_t /*begin*/, size_t /*end*/) {
    size_t n = _mem->size();
    _table->setRowCount(int(n));
    for (size_t i = 0; i < n; ++i) {
        QTableWidgetItem* addrItem = _table->item(int(i), 0);
        if (!addrItem) { addrItem = new QTableWidgetItem; _table->setItem(int(i), 0, addrItem); }
        addrItem->setText(QString::number((qulonglong)i));
        QTableWidgetItem* valItem = _table->item(int(i), 1);
        if (!valItem) { valItem = new QTableWidgetItem; _table->setItem(int(i), 1, valItem); }
        Word v = _mem->read(i);
        auto f = _mem->currentFault();
        if (f.model != FaultModel::None && i >= f.addr && i < f.addr + f.len) {
            valItem->setBackground(QColor(255, 240, 200));
        } else {
            valItem->setBackground(Qt::white);
        }
        valItem->setText(QString("0x%1").arg(v, 8, 16, QChar('0')).toUpper());
    }
    _table->resizeColumnsToContents();
}
