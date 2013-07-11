#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
typedef std::string STD_STRING;

#include "../../NewGeneBackEnd/globals.h"
#include <QMessageBox>
#include "newgenefilenames.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Infrastructure/Messager/uimessager.h"

class NewGeneWidget;
typedef std::pair<NewGeneWidget *, DataChangeMessage> WidgetChangeMessage;
typedef std::vector<WidgetChangeMessage> WidgetChangeMessages;

class NewGeneMainWindow;

class UIProjectManager;
class UISettingsManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;
class UITriggerManager;
class UIThreadManager;
class UIUIDataManager;
class UIUIActionManager;
class UIModelActionManager;

extern NewGeneMainWindow * theMainWindow;

UIProjectManager & projectManagerUI();
UISettingsManager & settingsManagerUI();
UILoggingManager & loggingManagerUI();
UIModelManager & modelManagerUI();
UIDocumentManager & documentManagerUI();
UIStatusManager & statusManagerUI();
UITriggerManager & triggerManagerUI();
UIThreadManager & threadManagerUI();
UIUIDataManager & uidataManagerUI();
UIUIActionManager & uiactionManagerUI();
UIModelActionManager & modelactionManagerUI();

Q_DECLARE_METATYPE(STD_STRING);

Q_DECLARE_METATYPE(QVector<int>);

Q_DECLARE_METATYPE(WidgetChangeMessage);
Q_DECLARE_METATYPE(WidgetChangeMessages);

Q_DECLARE_METATYPE(WidgetInstanceIdentifier);

// Widget refresh request data
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA);
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX);
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA);
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE);
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA);
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET);

// Widget refresh data
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA);
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX);
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA);
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE);
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROLS_AREA);
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET);

// Widget action data
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED);

#endif // GLOBALS_H
