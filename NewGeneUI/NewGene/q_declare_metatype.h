#ifndef Q_DECLARE_METATYPE_H
#define Q_DECLARE_METATYPE_H

//#include "Widgets/newgenewidget.h"
#include "../../NewGeneBackEnd/Utilities/WidgetIdentifier.h"
#include "./Widgets/Utilities/qsortfilterproxymodel_numberslast.h"

#define QEVENT_NONE_HINT                                           (QEvent::User)
#define QEVENT_PROMPT_FOR_VG_REFRESH_HINT                          (QEvent::User + 1)
#define QEVENT_CLICK_VG_REFRESH_HINT                               (QEvent::User + 2)
#define QEVENT_PROMPT_FOR_DMU_REFRESH_HINT                          (QEvent::User + 3)
#define QEVENT_CLICK_DMU_REFRESH_HINT                               (QEvent::User + 4)

extern QEvent::Type QEVENT_NONE;
extern QEvent::Type QEVENT_PROMPT_FOR_VG_REFRESH;
extern QEvent::Type QEVENT_CLICK_VG_REFRESH;
extern QEvent::Type QEVENT_PROMPT_FOR_DMU_REFRESH;
extern QEvent::Type QEVENT_CLICK_DMU_REFRESH;

typedef std::pair<NewGeneWidget *, DataChangeMessage> WidgetChangeMessage;
typedef std::vector<WidgetChangeMessage> WidgetChangeMessages;

Q_DECLARE_METATYPE(STD_STRING)
Q_DECLARE_METATYPE(STD_INT64)
Q_DECLARE_METATYPE(STD_VECTOR_STRING)
Q_DECLARE_METATYPE(STD_VECTOR_WIDGETIDENTIFIER)

Q_DECLARE_METATYPE(WidgetChangeMessage)
Q_DECLARE_METATYPE(WidgetChangeMessages)

Q_DECLARE_METATYPE(WidgetInstanceIdentifier)
Q_DECLARE_METATYPE(WidgetInstanceIdentifier_Bool_Pair)

// Widget refresh request data

Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA)
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA)
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)
Q_DECLARE_METATYPE(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItemRequest_DATETIME_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)
Q_DECLARE_METATYPE(WidgetDataItemRequest_MANAGE_DMUS_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItemRequest_MANAGE_UOAS_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItemRequest_MANAGE_VGS_WIDGET)

// Widget refresh data

Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA)
Q_DECLARE_METATYPE(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)
Q_DECLARE_METATYPE(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItem_TIMERANGE_REGION_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItem_DATETIME_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItem_GENERATE_OUTPUT_TAB)
Q_DECLARE_METATYPE(WidgetDataItem_MANAGE_DMUS_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItem_MANAGE_UOAS_WIDGET)
Q_DECLARE_METATYPE(WidgetDataItem_MANAGE_VGS_WIDGET)

// Widget action data

Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_ADD_DMU)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DELETE_DMU)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_ADD_UOA)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DELETE_UOA)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_CREATE_VG)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_DELETE_VG)
Q_DECLARE_METATYPE(WidgetActionItemRequest_ACTION_REFRESH_VG)

#endif // Q_DECLARE_METATYPE_H
