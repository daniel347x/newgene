#ifndef WORKQUEUEMANAGER_H
#define WORKQUEUEMANAGER_H

#include <QObject>

class WorkQueueManager : public QObject
{
        Q_OBJECT
    public:
        explicit WorkQueueManager(QObject *parent = 0);

    signals:

    public slots:

};

#endif // WORKQUEUEMANAGER_H
