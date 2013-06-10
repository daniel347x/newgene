#ifndef WORKQUEUEMANAGER_H
#define WORKQUEUEMANAGER_H

#include <QObject>

class WorkQueueManager : public QObject
{
		Q_OBJECT
	public:
		explicit WorkQueueManager(QObject *parent = 0);

	signals:
		void SendTrigger();

	public slots:
		void ReceiveTrigger();

};

#endif // WORKQUEUEMANAGER_H
