#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T20:38:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NewGene
TEMPLATE = app


SOURCES += main.cpp\
        newgenemainwindow.cpp

HEADERS  += newgenemainwindow.h

FORMS    += newgenemainwindow.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/release/ -lNewGeneBackEnd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/debug/ -lNewGeneBackEnd
else:unix: LIBS += -L$$PWD/../../NewGeneBackEnd/ -lNewGeneBackEnd

INCLUDEPATH += $$PWD/../../NewGeneBackEnd/Debug
DEPENDPATH += $$PWD/../../NewGeneBackEnd/Debug

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/release/NewGeneBackEnd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/debug/NewGeneBackEnd.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/libNewGeneBackEnd.a
