#-------------------------------------------------
#
# Project created by QtCreator 2012-12-21T00:10:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sngp
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    problem.cpp \
    navlistview.cpp \
    sngpworker.cpp \
    snode.cpp \
    sevalengine.cpp

HEADERS  += mainwindow.h \
    problem.h \
    navlistview.h \
    sngpworker.h \
    snode.h \
    sevalengine.h

FORMS    += mainwindow.ui
