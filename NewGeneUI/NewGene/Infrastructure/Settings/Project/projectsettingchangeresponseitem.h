#ifndef PROJECTSETTINGCHANGERESPONSEITEM_H
#define PROJECTSETTINGCHANGERESPONSEITEM_H

#include <QObject>
#include "projectsettingchangeitem.h"
#include "settingchangeresponseitem.h"

class ProjectSettingChangeResponseItem : public QObject, public ProjectSettingChangeItem, public SettingChangeResponseItem
{
		Q_OBJECT
	public:
		explicit ProjectSettingChangeResponseItem( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // PROJECTSETTINGCHANGERESPONSEITEM_H
