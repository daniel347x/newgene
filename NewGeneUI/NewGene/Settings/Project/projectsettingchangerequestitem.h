#ifndef PROJECTSETTINGCHANGEREQUESTITEM_H
#define PROJECTSETTINGCHANGEREQUESTITEM_H

#include <QObject>
#include "projectsettingchangeitem.h"
#include "settingchangerequestitem.h"

class ProjectSettingChangeRequestItem : public QObject, public ProjectSettingChangeItem, public SettingChangeRequestItem
{
		Q_OBJECT
	public:
		explicit ProjectSettingChangeRequestItem(QObject *parent = 0);

	signals:

	public slots:

};

#endif // PROJECTSETTINGCHANGEREQUESTITEM_H
