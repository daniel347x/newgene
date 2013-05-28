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
	newgenewidget.cpp \
	Settings/uisettingsmanager.cpp \
	Model/uimodelmanager.cpp \
	Documents/uidocumentmanager.cpp \
	Status/uistatusmanager.cpp \
	globals.cpp \
	uimanager.cpp \
	Settings/Indicator/settingchangeresponseindicator.cpp \
	Settings/Indicator/settingchangerequestindicator.cpp \
	Settings/Indicator/settingchangeindicator.cpp \
	Settings/Indicator/projectsettingchangeindicator.cpp \
	Settings/Indicator/globalsettingchangeindicator.cpp \
	Settings/Item/settingchangeresponseitem.cpp \
	Settings/Item/settingchangerequestitem.cpp \
	Settings/Item/settingchangeitem.cpp \
	Settings/Item/projectsettingchangeitem.cpp \
	Settings/Item/globalsettingchangeitem.cpp \
	Settings/Global/globalsettingchangerequestindicator.cpp \
	Settings/Global/globalsettingchangeresponseindicator.cpp \
	Settings/Global/globalsettingchangerequestitem.cpp \
	Settings/Global/globalsettingchangeresponseitem.cpp \
	Settings/Project/projectsettingchangerequestindicator.cpp \
	Settings/Project/projectsettingchangeresponseindicator.cpp \
	Settings/Project/projectsettingchangerequestitem.cpp \
	Settings/Project/projectsettingchangeresponseitem.cpp \
	Model/Indicator/modelchangeresponse.cpp \
	Model/Indicator/modelchangerequest.cpp \
	Model/Indicator/modelchangeindicator.cpp \
	Model/Item/modelchangeresponseitem.cpp \
	Model/Item/modelchangerequestitem.cpp \
	Model/Item/modelchangeitem.cpp \
	Project/uiprojectmanager.cpp \
	Utility/newgenefilenames.cpp \
	Logging/uiloggingmanager.cpp \
	newgeneapplication.cpp \
	Settings/Base/uiallsettings.cpp \
	Settings/Base/uisetting.cpp \
	Messager/uimessager.cpp \
	Project/uiinputproject.cpp \
	Project/uioutputproject.cpp \
	Model/uiinputmodel.cpp \
	Model/uioutputmodel.cpp \
	Settings/Project/uiallprojectsettings.cpp \
	Settings/Global/uiallglobalsettings.cpp \
	Model/Base/uimodel.cpp \
	Project/Base/uiproject.cpp \
    Settings/uiinputprojectsettings.cpp \
    Settings/uioutputprojectsettings.cpp

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
	newgenewidget.h \
	Settings/uisettingsmanager.h \
	Model/uimodelmanager.h \
	Documents/uidocumentmanager.h \
	Status/uistatusmanager.h \
	globals.h \
	uimanager.h \
	Settings/Indicator/settingchangeresponseindicator.h \
	Settings/Indicator/settingchangerequestindicator.h \
	Settings/Indicator/settingchangeindicator.h \
	Settings/Indicator/globalsettingchangeindicator.h \
	Settings/Indicator/projectsettingchangeindicator.h \
	Settings/Item/settingchangeresponseitem.h \
	Settings/Item/settingchangerequestitem.h \
	Settings/Item/settingchangeitem.h \
	Settings/Item/projectsettingchangeitem.h \
	Settings/Item/globalsettingchangeitem.h \
	Settings/Global/globalsettingchangerequestindicator.h \
	Settings/Global/globalsettingchangeresponseindicator.h \
	Settings/Global/globalsettingchangerequestitem.h \
	Settings/Global/globalsettingchangeresponseitem.h \
	Settings/Project/projectsettingchangerequestindicator.h \
	Settings/Project/projectsettingchangeresponseindicator.h \
	Settings/Project/projectsettingchangerequestitem.h \
	Settings/Project/projectsettingchangeresponseitem.h \
	Model/Indicator/modelchangeresponse.h \
	Model/Indicator/modelchangerequest.h \
	Model/Indicator/modelchangeindicator.h \
	Model/Item/modelchangeresponseitem.h \
	Model/Item/modelchangerequestitem.h \
	Model/Item/modelchangeitem.h \
	Project/uiprojectmanager.h \
	Utility/newgenefilenames.h \
	Logging/uiloggingmanager.h \
	newgeneapplication.h \
	Settings/Base/uiallsettings.h \
	Settings/Base/uisetting.h \
	Messager/uimessager.h \
	Project/uiinputproject.h \
	Project/uioutputproject.h \
	Model/uiinputmodel.h \
	Model/uioutputmodel.h \
	Settings/Project/uiallprojectsettings.h \
	Settings/Global/uiallglobalsettings.h \
	Model/Base/uimodel.h \
	Project/Base/uiproject.h \
    Settings/uiinputprojectsettings.h \
    Settings/uioutputprojectsettings.h

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

win32:CONFIG(release, debug|release): LIBS += -L$(BOOST_LIB) -llibboost_filesystem-vc110-mt-1_53
else:win32:CONFIG(debug, debug|release): LIBS += -L$(BOOST_LIB) -llibboost_filesystem-vc110-mt-gd-1_53
# else:unix:

INCLUDEPATH += $$PWD/../../NewGeneBackEnd/Debug
DEPENDPATH += $$PWD/../../NewGeneBackEnd/Debug
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/CreateOutput
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables/VariableSummary
INCLUDEPATH += $$PWD/CreateOutput/SelectVariables/Variables
INCLUDEPATH += $$PWD/Model
INCLUDEPATH += $$PWD/Model/Base
INCLUDEPATH += $$PWD/Model/Indicator
INCLUDEPATH += $$PWD/Model/Item
INCLUDEPATH += $$PWD/Project
INCLUDEPATH += $$PWD/Project/Base
INCLUDEPATH += $$PWD/Settings
INCLUDEPATH += $$PWD/Settings/Base
INCLUDEPATH += $$PWD/Settings/Indicator
INCLUDEPATH += $$PWD/Settings/Item
INCLUDEPATH += $$PWD/Settings/Global
INCLUDEPATH += $$PWD/Settings/Project
INCLUDEPATH += $$PWD/Messager
INCLUDEPATH += $$PWD/Documents
INCLUDEPATH += $$PWD/Status
INCLUDEPATH += $$PWD/Logging
INCLUDEPATH += $$PWD/Utility
INCLUDEPATH += $(BOOST_ROOT)

##QMAKE_LFLAGS += /ignore:4099
#QMAKE_CFLAGS += /ignore:4503 # "decorated name length exceeded" (common for template instantiations)
#QMAKE_CFLAGS += /ignore:4100 # "unreferenced formal parameter" (many "Messager & messager" parameters, with the token "messager" left on for uniformity and convenience
#QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/release/NewGeneBackEnd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/debug/NewGeneBackEnd.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/libNewGeneBackEnd.a

RESOURCES += \
	../Resources/NewGeneResources.qrc
