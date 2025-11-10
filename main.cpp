#include "mainwindow.h"
#include <QApplication>
#include <memorytester.h>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    qRegisterMetaType<TestAlgorithm>("TestAlgorithm");
    qRegisterMetaType<std::vector<TestResult>>("std::vector<TestResult>");
    qRegisterMetaType<TestResult>("TestResult");
    MainWindow w;
    w.show();
    return app.exec();
}
