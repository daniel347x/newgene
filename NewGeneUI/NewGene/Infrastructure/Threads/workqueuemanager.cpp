#include "workqueuemanager.h"

WorkQueueManagerBase::WorkQueueManagerBase(bool isPool2_, QObject *parent)
  : QObject(parent)
  , isPool2(isPool2_)
{
    qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA>();
    qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_TOOLBOX>();
    qRegisterMetaType<WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>();
}
