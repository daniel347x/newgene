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
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariables.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolboxwrapper.cpp \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.cpp \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.cpp \
    CreateOutput/SelectVariables/kadcolumnselectionbox.cpp \
    CreateOutput/SelectVariables/timerangebox.cpp \
    Model/newgenemodel.cpp \
    Model/modelchangeindicator.cpp \
    Model/modelchangerequest.cpp \
    Model/modelchange.cpp \
    Model/modelvalidator.cpp \
    Model/modelchangeitem.cpp

HEADERS  += newgenemainwindow.h \
    CreateOutput/newgenecreateoutput.h \
    CreateOutput/SelectVariables/newgeneselectvariables.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.h \
    CreateOutput/newgenetabwidget.h \
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.h \
    CreateOutput/SelectVariables/Variables/newgenevariables.h \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.h \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolboxwrapper.h \
    CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.h \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.h \
    CreateOutput/SelectVariables/kadcolumnselectionbox.h \
    CreateOutput/SelectVariables/timerangebox.h \
    Model/newgenemodel.h \
    Model/modelchangeindicator.h \
    Model/modelchangerequest.h \
    Model/modelchange.h \
    Model/modelvalidator.h \
    Model/modelchangeitem.h

FORMS    += newgenemainwindow.ui \
    CreateOutput/newgenecreateoutput.ui \
    CreateOutput/SelectVariables/newgeneselectvariables.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.ui \
    CreateOutput/SelectVariables/Variables/newgenevariablegroup.ui \
    CreateOutput/SelectVariables/Variables/newgenevariables.ui \
    CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.ui \
    CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.ui \
    CreateOutput/SelectVariables/kadcolumnselectionbox.ui \
    CreateOutput/SelectVariables/timerangebox.ui


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
