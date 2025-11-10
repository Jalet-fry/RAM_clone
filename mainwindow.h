#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include "memorymodel.h"
#include "testerworker.h"


class MainWindow : public QMainWindow {
Q_OBJECT
public:
explicit MainWindow(QWidget* parent = nullptr);
~MainWindow();


private slots:
void onInject();
void onReset();
void onStartTest();
void onTestFinished(const std::vector<TestResult>& results);
void refreshTable(size_t begin, size_t end);


private:
MemoryModel* _mem;
TesterWorker* _worker;



// UI
QComboBox* _faultCombo;
QLineEdit* _addrEdit;
QLineEdit* _lenEdit;
QDoubleSpinBox* _flipProbSpin;
QPushButton* _injectBtn;
QPushButton* _resetBtn;


QComboBox* _algoCombo;
QPushButton* _startBtn;
QProgressBar* _progress;


QTableWidget* _table;
QTextEdit* _log;
};
#endif // MAINWINDOW_H
