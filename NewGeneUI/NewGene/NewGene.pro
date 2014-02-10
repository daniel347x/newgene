#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T20:38:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NewGene
TEMPLATE = app

# QMAKE_CXXFLAGS_RELEASE += -Zi

QMAKE_CFLAGS_RELEASE -= -O
QMAKE_CFLAGS_RELEASE -= -O1
QMAKE_CFLAGS_RELEASE -= -O2
# add the desired -O3 if not present
QMAKE_CFLAGS_RELEASE *= -O3

macx: QMAKE_CXXFLAGS += -x c++ -arch x86_64 -std=gnu++11 -stdlib=libc++
macx: QMAKE_LFLAGS += -arch x86_64 -std=gnu++11 -stdlib=libc++
macx: QMAKE_CFLAGS += -gdwarf-2
macx: QMAKE_CXXFLAGS += -gdwarf-2
macx: ICON = ../Resources/earth.icns
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

SOURCES += main.cpp\
	Widgets/newgenemainwindow.cpp \
	Widgets/CreateOutput/newgenecreateoutput.cpp \
	Widgets/CreateOutput/SelectVariables/newgeneselectvariables.cpp \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.cpp \
	Widgets/CreateOutput/newgenetabwidget.cpp \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroup.cpp \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariables.cpp \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.cpp \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablestoolboxwrapper.cpp \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.cpp \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.cpp \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.cpp \
	Widgets/CreateOutput/SelectVariables/kadcolumnselectionbox.cpp \
	Widgets/CreateOutput/SelectVariables/timerangebox.cpp \
	Widgets/newgenewidget.cpp \
	Infrastructure/Settings/uisettingsmanager.cpp \
	Infrastructure/Model/uimodelmanager.cpp \
	Infrastructure/Documents/uidocumentmanager.cpp \
	Infrastructure/Status/uistatusmanager.cpp \
	globals.cpp \
	Infrastructure/uimanager.cpp \
	Infrastructure/Settings/Indicator/settingchangeresponseindicator.cpp \
	Infrastructure/Settings/Indicator/settingchangerequestindicator.cpp \
	Infrastructure/Settings/Indicator/settingchangeindicator.cpp \
	Infrastructure/Settings/Indicator/projectsettingchangeindicator.cpp \
	Infrastructure/Settings/Indicator/globalsettingchangeindicator.cpp \
	Infrastructure/Settings/Item/settingchangeresponseitem.cpp \
	Infrastructure/Settings/Item/settingchangerequestitem.cpp \
	Infrastructure/Settings/Item/settingchangeitem.cpp \
	Infrastructure/Settings/Item/projectsettingchangeitem.cpp \
	Infrastructure/Settings/Item/globalsettingchangeitem.cpp \
	Infrastructure/Settings/Global/globalsettingchangerequestindicator.cpp \
	Infrastructure/Settings/Global/globalsettingchangeresponseindicator.cpp \
	Infrastructure/Settings/Global/globalsettingchangerequestitem.cpp \
	Infrastructure/Settings/Global/globalsettingchangeresponseitem.cpp \
	Infrastructure/Settings/Project/projectsettingchangerequestindicator.cpp \
	Infrastructure/Settings/Project/projectsettingchangeresponseindicator.cpp \
	Infrastructure/Settings/Project/projectsettingchangerequestitem.cpp \
	Infrastructure/Settings/Project/projectsettingchangeresponseitem.cpp \
	Infrastructure/Model/Indicator/modelchangeresponse.cpp \
	Infrastructure/Model/Indicator/modelchangerequest.cpp \
	Infrastructure/Model/Indicator/modelchangeindicator.cpp \
	Infrastructure/Model/Item/modelchangeresponseitem.cpp \
	Infrastructure/Model/Item/modelchangerequestitem.cpp \
	Infrastructure/Model/Item/modelchangeitem.cpp \
	Infrastructure/Project/uiprojectmanager.cpp \
	Infrastructure/Utility/newgenefilenames.cpp \
	Infrastructure/Logging/uiloggingmanager.cpp \
	newgeneapplication.cpp \
	Infrastructure/Settings/Base/uiallsettings.cpp \
	Infrastructure/Settings/Base/uisetting.cpp \
	Infrastructure/Messager/uimessager.cpp \
	Infrastructure/Project/uiinputproject.cpp \
	Infrastructure/Project/uioutputproject.cpp \
	Infrastructure/Model/uiinputmodel.cpp \
	Infrastructure/Model/uioutputmodel.cpp \
	Infrastructure/Settings/Project/uiallprojectsettings.cpp \
	Infrastructure/Model/Base/uimodel.cpp \
	Infrastructure/Project/Base/uiproject.cpp \
	Infrastructure/Settings/uiinputprojectsettings.cpp \
	Infrastructure/Settings/uioutputprojectsettings.cpp \
	Infrastructure/Settings/uiallglobalsettings.cpp \
	Infrastructure/Settings/uiallglobalsettings_list.cpp \
	Infrastructure/Settings/uiinputprojectsettings_list.cpp \
	Infrastructure/Settings/uioutputprojectsettings_list.cpp \
	Infrastructure/Threads/uithreadmanager.cpp \
	Infrastructure/Triggers/uitriggermanager.cpp \
	Infrastructure/Settings/Base/uimodelsettings.cpp \
	Infrastructure/Settings/uiinputmodelsettings.cpp \
	Infrastructure/Settings/uioutputmodelsettings.cpp \
	Infrastructure/Threads/eventloopthreadmanager.cpp \
	Infrastructure/Threads/workqueuemanager.cpp \
	Infrastructure/Project/Base/outputprojectworkqueue_base.cpp \
	Infrastructure/Project/Base/inputprojectworkqueue_base.cpp \
	Infrastructure/Project/inputprojectworkqueue.cpp \
	Infrastructure/Project/outputprojectworkqueue.cpp \
	Infrastructure/Model/Base/inputmodelworkqueue_base.cpp \
	Infrastructure/Model/Base/outputmodelworkqueue_base.cpp \
	Infrastructure/Model/inputmodelworkqueue.cpp \
	Infrastructure/Model/outputmodelworkqueue.cpp \
	Infrastructure/Settings/Base/inputprojectsettingsworkqueue_base.cpp \
	Infrastructure/Settings/Base/outputprojectsettingsworkqueue_base.cpp \
	Infrastructure/Settings/Base/globalsettingsworkqueue_base.cpp \
	Infrastructure/Settings/Base/inputmodelsettingsworkqueue_base.cpp \
	Infrastructure/Settings/Base/outputmodelsettingsworkqueue_base.cpp \
	Infrastructure/Settings/globalsettingsworkqueue.cpp \
	Infrastructure/Settings/inputmodelsettingsworkqueue.cpp \
	Infrastructure/Settings/outputmodelsettingsworkqueue.cpp \
	Infrastructure/Settings/inputprojectsettingsworkqueue.cpp \
	Infrastructure/Settings/outputprojectsettingsworkqueue.cpp \
	Infrastructure/UIData/uiuidatamanager.cpp \
	Infrastructure/UIAction/uiuiactionmanager.cpp \
	Infrastructure/ModelAction/uimodelactionmanager.cpp \
	Infrastructure/Project/Base/uiprojectmanagerworkqueue_base.cpp \
	Infrastructure/Project/uiprojectmanagerworkqueue.cpp \
	Infrastructure/UIData/uiwidgetdatarefresh.cpp \
	Infrastructure/Messager/uimessagersingleshot.cpp \
	Infrastructure/Model/Base/modelworkqueue.cpp \
	Infrastructure/UIAction/uiaction.cpp \
	Infrastructure/UIAction/variablegroupsetmemberselectionchange.cpp \
	Widgets/CreateOutput/SelectVariables/KadWidget/kadwidgetsscrollarea.cpp \
	Widgets/CreateOutput/SelectVariables/KadWidget/kadspinbox.cpp \
	Infrastructure/UIAction/kadcountchange.cpp \
	Infrastructure/UIAction/generateoutput.cpp \
	Widgets/CreateOutput/SelectVariables/KadWidget/newgenedatetimewidget.cpp \
	Infrastructure/UIAction/timerangechange.cpp \
	Widgets/CreateOutput/GenerateOutput/newgenegenerateoutput.cpp \
	Infrastructure/UIAction/generaloptions.cpp \
    Widgets/ManageInput/newgenemanageinput.cpp \
    Widgets/ManageInput/newgenetabwidgetmanageinput.cpp

HEADERS  += Widgets/newgenemainwindow.h \
	Widgets/CreateOutput/newgenecreateoutput.h \
	Widgets/CreateOutput/SelectVariables/newgeneselectvariables.h \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.h \
	Widgets/CreateOutput/newgenetabwidget.h \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroup.h \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariables.h \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.h \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablestoolboxwrapper.h \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablestoolbox.h \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.h \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.h \
	Widgets/CreateOutput/SelectVariables/kadcolumnselectionbox.h \
	Widgets/CreateOutput/SelectVariables/timerangebox.h \
	Widgets/newgenewidget.h \
	Infrastructure/Settings/uisettingsmanager.h \
	Infrastructure/Model/uimodelmanager.h \
	Infrastructure/Documents/uidocumentmanager.h \
	Infrastructure/Status/uistatusmanager.h \
	globals.h \
	Infrastructure/uimanager.h \
	Infrastructure/Settings/Indicator/settingchangeresponseindicator.h \
	Infrastructure/Settings/Indicator/settingchangerequestindicator.h \
	Infrastructure/Settings/Indicator/settingchangeindicator.h \
	Infrastructure/Settings/Indicator/globalsettingchangeindicator.h \
	Infrastructure/Settings/Indicator/projectsettingchangeindicator.h \
	Infrastructure/Settings/Item/settingchangeresponseitem.h \
	Infrastructure/Settings/Item/settingchangerequestitem.h \
	Infrastructure/Settings/Item/settingchangeitem.h \
	Infrastructure/Settings/Item/projectsettingchangeitem.h \
	Infrastructure/Settings/Item/globalsettingchangeitem.h \
	Infrastructure/Settings/Global/globalsettingchangerequestindicator.h \
	Infrastructure/Settings/Global/globalsettingchangeresponseindicator.h \
	Infrastructure/Settings/Global/globalsettingchangerequestitem.h \
	Infrastructure/Settings/Global/globalsettingchangeresponseitem.h \
	Infrastructure/Settings/Project/projectsettingchangerequestindicator.h \
	Infrastructure/Settings/Project/projectsettingchangeresponseindicator.h \
	Infrastructure/Settings/Project/projectsettingchangerequestitem.h \
	Infrastructure/Settings/Project/projectsettingchangeresponseitem.h \
	Infrastructure/Model/Indicator/modelchangeresponse.h \
	Infrastructure/Model/Indicator/modelchangerequest.h \
	Infrastructure/Model/Indicator/modelchangeindicator.h \
	Infrastructure/Model/Item/modelchangeresponseitem.h \
	Infrastructure/Model/Item/modelchangerequestitem.h \
	Infrastructure/Model/Item/modelchangeitem.h \
	Infrastructure/Project/uiprojectmanager.h \
	Infrastructure/Utility/newgenefilenames.h \
	Infrastructure/Logging/uiloggingmanager.h \
	newgeneapplication.h \
	Infrastructure/Settings/Base/uiallsettings.h \
	Infrastructure/Settings/Base/uisetting.h \
	Infrastructure/Messager/uimessager.h \
	Infrastructure/Project/uiinputproject.h \
	Infrastructure/Project/uioutputproject.h \
	Infrastructure/Model/uiinputmodel.h \
	Infrastructure/Model/uioutputmodel.h \
	Infrastructure/Settings/Project/uiallprojectsettings.h \
	Infrastructure/Model/Base/uimodel.h \
	Infrastructure/Project/Base/uiproject.h \
	Infrastructure/Settings/uiinputprojectsettings.h \
	Infrastructure/Settings/uioutputprojectsettings.h \
	Infrastructure/Settings/uiallglobalsettings.h \
	Infrastructure/Settings/uiallglobalsettings_list.h \
	Infrastructure/Settings/uiinputprojectsettings_list.h \
	Infrastructure/Settings/uioutputprojectsettings_list.h \
	Infrastructure/Threads/uithreadmanager.h \
	Infrastructure/Triggers/uitriggermanager.h \
	Infrastructure/Settings/Base/uimodelsettings.h \
	Infrastructure/Settings/uiinputmodelsettings.h \
	Infrastructure/Settings/uioutputmodelsettings.h \
	Infrastructure/Threads/eventloopthreadmanager.h \
	Infrastructure/Threads/workqueuemanager.h \
	Infrastructure/Project/Base/outputprojectworkqueue_base.h \
	Infrastructure/Project/Base/inputprojectworkqueue_base.h \
	Infrastructure/Project/inputprojectworkqueue.h \
	Infrastructure/Project/outputprojectworkqueue.h \
	Infrastructure/Model/Base/inputmodelworkqueue_base.h \
	Infrastructure/Model/Base/outputmodelworkqueue_base.h \
	Infrastructure/Model/inputmodelworkqueue.h \
	Infrastructure/Model/outputmodelworkqueue.h \
	Infrastructure/Settings/Base/inputprojectsettingsworkqueue_base.h \
	Infrastructure/Settings/Base/outputprojectsettingsworkqueue_base.h \
	Infrastructure/Settings/Base/globalsettingsworkqueue_base.h \
	Infrastructure/Settings/Base/inputmodelsettingsworkqueue_base.h \
	Infrastructure/Settings/Base/outputmodelsettingsworkqueue_base.h \
	Infrastructure/Settings/globalsettingsworkqueue.h \
	Infrastructure/Settings/inputmodelsettingsworkqueue.h \
	Infrastructure/Settings/outputmodelsettingsworkqueue.h \
	Infrastructure/Settings/inputprojectsettingsworkqueue.h \
	Infrastructure/Settings/outputprojectsettingsworkqueue.h \
	Infrastructure/UIData/uiuidatamanager.h \
	Infrastructure/UIAction/uiuiactionmanager.h \
	Infrastructure/ModelAction/uimodelactionmanager.h \
	Infrastructure/Project/Base/uiprojectmanagerworkqueue_base.h \
	Infrastructure/Project/uiprojectmanagerworkqueue.h \
	Infrastructure/UIData/uiwidgetdatarefresh.h \
	Infrastructure/Messager/uimessagersingleshot.h \
	Infrastructure/Model/Base/modelworkqueue.h \
	Infrastructure/UIAction/uiaction.h \
	Infrastructure/UIAction/variablegroupsetmemberselectionchange.h \
	Widgets/CreateOutput/SelectVariables/KadWidget/kadwidgetsscrollarea.h \
	Widgets/CreateOutput/SelectVariables/KadWidget/kadspinbox.h \
	Infrastructure/UIAction/kadcountchange.h \
	Infrastructure/UIAction/generateoutput.h \
	Widgets/CreateOutput/SelectVariables/KadWidget/newgenedatetimewidget.h \
	Infrastructure/UIAction/timerangechange.h \
	Widgets/CreateOutput/GenerateOutput/newgenegenerateoutput.h \
	Infrastructure/UIAction/generaloptions.h \
	q_declare_metatype.h \
    Widgets/ManageInput/newgenemanageinput.h \
    Widgets/ManageInput/newgenetabwidgetmanageinput.h

FORMS    += Widgets/newgenemainwindow.ui \
	Widgets/CreateOutput/newgenecreateoutput.ui \
	Widgets/CreateOutput/SelectVariables/newgeneselectvariables.ui \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummary.ui \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroup.ui \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariables.ui \
	Widgets/CreateOutput/SelectVariables/Variables/newgenevariablegroupsscrollarea.ui \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummaryscrollarea.ui \
	Widgets/CreateOutput/SelectVariables/VariableSummary/newgenevariablesummarygroup.ui \
	Widgets/CreateOutput/SelectVariables/kadcolumnselectionbox.ui \
	Widgets/CreateOutput/SelectVariables/timerangebox.ui \
	Widgets/CreateOutput/GenerateOutput/newgenegenerateoutput.ui \
    Widgets/ManageInput/newgenemanageinput.ui \
    Widgets/ManageInput/DMUs/newgenemanagedmus.ui \
    Widgets/ManageInput/UOAs/newgenemanageuoas.ui \
    Widgets/ManageInput/VGs/newgenemanagevgs.ui \
    Widgets/ManageInput/Data/newgenemanagedata.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/release/ -lNewGeneBackEnd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/debug/ -lNewGeneBackEnd
else:macx:CONFIG(release, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/OSX/NewGene/DerivedData/NewGene/Build/Products/Release -lNewGene
else:macx:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../NewGeneBackEnd/OSX/NewGene/DerivedData/NewGene/Build/Products/Debug -lNewGene
else:unix: LIBS += -L$$PWD/../../NewGeneBackEnd/ -lNewGeneBackEnd

win32:CONFIG(release, debug|release): LIBS += -L$(BOOST_LIB) -llibboost_filesystem-vc120-mt-1_55
else:win32:CONFIG(debug, debug|release): LIBS += -L$(BOOST_LIB) -llibboost_filesystem-vc120-mt-gd-1_55
else:macx:CONFIG(release, debug|release): LIBS += -L$(BOOST_LIB) -lboost_filesystem -lboost_regex -lboost_thread -lboost_system -lboost_date_time
else:macx:CONFIG(debug, debug|release): LIBS += -L$(BOOST_LIB) -lboost_filesystem -lboost_regex -lboost_thread -lboost_system -lboost_date_time

INCLUDEPATH += $$PWD/../../NewGeneBackEnd/Release
DEPENDPATH += $$PWD/../../NewGeneBackEnd/Release
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Widgets
INCLUDEPATH += $$PWD/Widgets/CreateOutput
INCLUDEPATH += $$PWD/Widgets/CreateOutput/GenerateOutput
INCLUDEPATH += $$PWD/Widgets/CreateOutput/SelectVariables
INCLUDEPATH += $$PWD/Widgets/CreateOutput/SelectVariables/VariableSummary
INCLUDEPATH += $$PWD/Widgets/CreateOutput/SelectVariables/Variables
INCLUDEPATH += $$PWD/Widgets/CreateOutput/SelectVariables/KadWidget
INCLUDEPATH += $$PWD/Infrastructure/Model
INCLUDEPATH += $$PWD/Infrastructure/Model/Base
INCLUDEPATH += $$PWD/Infrastructure/Model/Indicator
INCLUDEPATH += $$PWD/Infrastructure/Model/Item
INCLUDEPATH += $$PWD/Infrastructure/Project
INCLUDEPATH += $$PWD/Infrastructure/Project/Base
INCLUDEPATH += $$PWD/Infrastructure/Settings
INCLUDEPATH += $$PWD/Infrastructure/Settings/Base
INCLUDEPATH += $$PWD/Infrastructure/Settings/Indicator
INCLUDEPATH += $$PWD/Infrastructure/Settings/Item
INCLUDEPATH += $$PWD/Infrastructure/Settings/Global

INCLUDEPATH += $$PWD/Infrastructure/Settings/Project
INCLUDEPATH += $$PWD/Infrastructure/Messager
INCLUDEPATH += $$PWD/Infrastructure/Documents
INCLUDEPATH += $$PWD/Infrastructure/Status
INCLUDEPATH += $$PWD/Infrastructure/Logging
INCLUDEPATH += $$PWD/Infrastructure/Triggers
INCLUDEPATH += $$PWD/Infrastructure/Threads
INCLUDEPATH += $$PWD/Infrastructure/UIData
INCLUDEPATH += $$PWD/Infrastructure/UIAction
INCLUDEPATH += $$PWD/Infrastructure/ModelAction
INCLUDEPATH += $$PWD/Infrastructure/Utility
INCLUDEPATH += $(BOOST_ROOT)

##QMAKE_LFLAGS += /ignore:4099
#QMAKE_CFLAGS += /ignore:4503 # "decorated name length exceeded" (common for template instantiations)
#QMAKE_CFLAGS += /ignore:4100 # "unreferenced formal parameter" (many "Messager & messager" parameters, with the token "messager" left on for uniformity and convenience
#QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/release/NewGeneBackEnd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../NewGeneBackEnd/debug/NewGeneBackEnd.lib

macx: CONFIG += c++11

RESOURCES += \
	../Resources/NewGeneResources.qrc
