#-------------------------------------------------
#
# Project created by QtCreator 2014-11-06T22:10:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
            $$PWD/src

TARGET = CloneQtProject
TEMPLATE = app

SOURCES += \
            src/main.cpp\
            src/main/mainwindow.cpp \
            src/threads/clonethread.cpp

HEADERS  += \
            src/main/mainwindow.h \
            src/threads/clonethread.h

FORMS    += \
            src/main/mainwindow.ui
