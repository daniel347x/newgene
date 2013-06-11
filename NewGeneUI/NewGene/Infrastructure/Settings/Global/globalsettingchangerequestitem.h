#ifndef GLOBALSETTINGCHANGEREQUESTITEM_H
#define GLOBALSETTINGCHANGEREQUESTITEM_H

#include <QObject>
#include "globalsettingchangeitem.h"
#include "settingchangerequestitem.h"

class GlobalSettingChangeRequestItem : public QObject, public GlobalSettingChangeItem, public SettingChangeRequestItem
{
		Q_OBJECT
	public:
		explicit GlobalSettingChangeRequestItem( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // GLOBALSETTINGCHANGEREQUESTITEM_H
