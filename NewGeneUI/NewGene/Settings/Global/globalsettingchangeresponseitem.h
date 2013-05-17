#ifndef GLOBALSETTINGCHANGERESPONSEITEM_H
#define GLOBALSETTINGCHANGERESPONSEITEM_H

#include <QObject>
#include "globalsettingchangeitem.h"
#include "settingchangeresponseitem.h"

class GlobalSettingChangeResponseItem : public QObject, public GlobalSettingChangeItem, public SettingChangeResponseItem
{
		Q_OBJECT
	public:
		explicit GlobalSettingChangeResponseItem( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // GLOBALSETTINGCHANGERESPONSEITEM_H
