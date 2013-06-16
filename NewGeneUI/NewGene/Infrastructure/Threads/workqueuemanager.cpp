#include "workqueuemanager.h"

WorkQueueManagerBase::WorkQueueManagerBase(bool isPool2_, QObject *parent)
  : QObject(parent)
  , isPool2(isPool2_)
{
}
