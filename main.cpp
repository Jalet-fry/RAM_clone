#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <memorytester.h>
#include "types.h"
#include "thememanager.h"

int main(int argc, char** argv) {
    // Set up debug message pattern for better diagnostics
    qSetMessagePattern("[%{time h:mm:ss.zzz} %{type} %{file}:%{line}] %{message}");
    
    qDebug() << "Application starting...";
    QApplication app(argc, argv);
    qDebug() << "QApplication created";
    qRegisterMetaType<TestAlgorithm>("TestAlgorithm");
    qRegisterMetaType<std::vector<TestResult>>("std::vector<TestResult>");
    qRegisterMetaType<TestResult>("TestResult");
    // Register size_t for use in queued connections
    qRegisterMetaType<size_t>("size_t");
    // Register Word type for use in queued connections (used in progressDetail signal)
    qRegisterMetaType<Word>("Word");
    // Register Theme enum for use in queued connections (used in themeChanged signal)
    qRegisterMetaType<Theme>("Theme");
    qDebug() << "Meta types registered";
    
    qDebug() << "Creating MainWindow...";
    MainWindow w;
    qDebug() << "MainWindow created";
    
    qDebug() << "Showing MainWindow...";
    w.show();
    qDebug() << "MainWindow shown, entering event loop";
    
    return app.exec();
}
