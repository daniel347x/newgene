#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

#if 0
//#include <boost/predef.h> // not until Boost 1.55
#if __APPLE__ // ... but this is defined for Clang anyways
#   include <stdint.h>
    namespace std
    {
        typedef ::int8_t int8_t;
        typedef ::int16_t int16_t;
        typedef ::int32_t int32_t;
        typedef ::int64_t int64_t;
    }
#else
#   include <cstdint>
#endif
#endif

#include <cstdint>

#include <QtGlobal>

#include "../../NewGeneBackEnd/UIAction/ActionChanges.h"

#if 0
#if __APPLE__
#   include <boost/preprocessor/stringize.hpp>
#   pragma message "start message"
#   ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#       if (__MAC_OS_X_VERSION_MAX_ALLOWED == 1080)
#           pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED == 1080"
#       else
#           pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED != 1080"
#           pragma message BOOST_PP_STRINGIZE(__MAC_OS_X_VERSION_MAX_ALLOWED)
#       endif
#   else
#       pragma message "__MAC_OS_X_VERSION_MAX_ALLOWED is not defined"
#   endif
#   ifdef __MAC_10_8
#       if (__MAC_10_8 == 1080)
#           pragma message "__MAC_10_8 == 1080"
#       else
#           pragma message "__MAC_10_8 != 1080"
#           pragma message BOOST_PP_STRINGIZE(__MAC_10_8)
#       endif
#   else
#       pragma message "__MAC_10_8 is not defined"
#   endif
#   pragma message "done message"
#  endif
#endif
#endif

typedef std::string STD_STRING;
typedef std::int64_t STD_INT64;

#include "../../NewGeneBackEnd/globals.h"
#include <QMessageBox>
#include "newgenefilenames.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
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

Q_DECLARE_METATYPE(STD_STRING)
Q_DECLARE_METATYPE(STD_INT64)

//Q_DECLARE_METATYPE(QVector<int>);

Q_DECLARE_METATYPE(WidgetChangeMessage)
Q_DECLARE_METATYPE(WidgetChangeMessages)

class WidgetInstanceIdentifier;
Q_DECLARE_METATYPE(WidgetInstanceIdentifier)

class WidgetInstanceIdentifier_Bool_Pair;
Q_DECLARE_METATYPE(WidgetInstanceIdentifier_Bool_Pair)

// Widget refresh request data
class WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA;
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA)

class WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX;
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)

class WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE;
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)

class WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA;
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA)

class WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE;
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)

class WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA;
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)

class WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)

class WidgetDataItemRequest_TIMERANGE_REGION_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)

class WidgetDataItemRequest_DATETIME_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItemRequest_DATETIME_WIDGET)

class WidgetDataItemRequest_GENERATE_OUTPUT_TAB;
Q_DECLARE_METATYPE(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)

// Widget refresh data
class WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA;
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)

class WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)

class WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE;
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)

class WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA;
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA)

class WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE;
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)

class WidgetDataItem_KAD_SPIN_CONTROLS_AREA;
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)

class WidgetDataItem_KAD_SPIN_CONTROL_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)

class WidgetDataItem_TIMERANGE_REGION_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItem_TIMERANGE_REGION_WIDGET)

class WidgetDataItem_DATETIME_WIDGET;
Q_DECLARE_METATYPE(WidgetDataItem_DATETIME_WIDGET)

class WidgetDataItem_GENERATE_OUTPUT_TAB;
Q_DECLARE_METATYPE(WidgetDataItem_GENERATE_OUTPUT_TAB)

// Widget action data
class WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)

class WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)

class WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)

class WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)

class WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)

class WidgetActionItemRequest_ACTION_GENERATE_OUTPUT;
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)

#endif // GLOBALS_H
