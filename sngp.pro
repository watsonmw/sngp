#-------------------------------------------------
#
# Project created by QtCreator 2012-12-21T00:10:08
#
#-------------------------------------------------

QT += core gui

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

HEADERS += mainwindow.h \
    problem.h \
    navlistview.h \
    sngpworker.h \
    snode.h \
    sevalengine.h \
    sortedarray.h

FORMS += mainwindow.ui

# Turn on whole program optimization (this doesnt work
# as well as i would expect but makes some tiny improvements).
# Turn on debug symbols in release mode as well, so the
# Instruments profilers can give better hints.
macx:release {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -g -gdwarf-2 -O4
    QMAKE_LFLAGS_RELEASE += -O4
}
