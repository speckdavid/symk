#-------------------------------------------------
#
# Project created by QtCreator 2016-06-19T13:15:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Visualization2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parser.cpp \
    miscforestoperations.cpp \
    drawfunctions.cpp \
    timefunctions.cpp

HEADERS  += mainwindow.h \
    parser.h \
    miscforestoperations.h \
    drawfunctions.h \
    timefunctions.h

FORMS    += mainwindow.ui
