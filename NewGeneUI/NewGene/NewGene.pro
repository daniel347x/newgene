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
        newgenemainwindow.cpp \
    CreateOutput/newgenecreateoutput.cpp \
    CreateOutput/SelectVariables/newgeneselectvariables.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.cpp \
    CreateOutput/newgenetabwidget.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroupbox.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryholder.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariables.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.cpp

HEADERS  += newgenemainwindow.h \
    CreateOutput/newgenecreateoutput.h \
    CreateOutput/SelectVariables/newgeneselectvariables.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.h \
    CreateOutput/newgenetabwidget.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroupbox.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryholder.h \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.h \
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.h \
    CreateOutput/SelectVariables/Variables/newgenevariables.h \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.h

FORMS    += newgenemainwindow.ui \
    CreateOutput/newgenecreateoutput.ui \
    CreateOutput/SelectVariables/newgeneselectvariables.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroupbox.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryholder.ui \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.ui \
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.ui \
    CreateOutput/SelectVariables/Variables/newgenevariables.ui \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/release/ -lNewGeneBackEnd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/debug/ -lNewGeneBackEnd
else:unix: LIBS += -L$$PWD/../../NewGeneBackEnd/ -lNewGeneBackEnd

INCLUDEPATH += $$PWD/../../NewGeneBackEnd/Debug
DEPENDPATH += $$PWD/../../NewGeneBackEnd/Debug
INCLUDEPATH += $$PWD/CreateOutput
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables/VariableSummary
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables/Variables

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/release/NewGeneBackEnd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/debug/NewGeneBackEnd.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/libNewGeneBackEnd.a

RESOURCES += \
    ../Resources/NewGeneResources.qrc
