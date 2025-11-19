#-------------------------------------------------
#
# Project created by QtCreator 2025-10-13T17:33:03
#
#-------------------------------------------------

QT += core gui widgets

CONFIG += c++11

TARGET = RAM_clone
TEMPLATE = app

# The following define makes your compiler warn you if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Enable automatic MOC, UIC, RCC processing
CONFIG += automoc

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    memorymodel.cpp \
    memorytester.cpp \
    testerworker.cpp \
    thememanager.cpp \
    tableitemdelegate.cpp \
    faultinjector.cpp \
    logger.cpp \
    dataformatter.cpp \
    memorytablemanager.cpp \
    statisticsmanager.cpp \
    faultcontroller.cpp \
    testcontroller.cpp \
    resultsnavigator.cpp \
    themecontroller.cpp

# Header files
HEADERS += \
    mainwindow.h \
    memorymodel.h \
    memorytester.h \
    testerworker.h \
    thememanager.h \
    tableitemdelegate.h \
    faultinjector.h \
    types.h \
    logger.h \
    dataformatter.h \
    constants.h \
    memorytablemanager.h \
    statisticsmanager.h \
    faultcontroller.h \
    testcontroller.h \
    resultsnavigator.h \
    themecontroller.h

# UI files
FORMS += \
    mainwindow.ui
