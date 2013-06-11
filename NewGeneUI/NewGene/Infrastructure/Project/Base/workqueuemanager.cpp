#include "workqueuemanager.h"

WorkQueueManager::WorkQueueManager(QObject *parent) :
    QObject(parent)
{
}

void WorkQueueManager::ReceiveTrigger()
{
    emit SendTrigger();
}
