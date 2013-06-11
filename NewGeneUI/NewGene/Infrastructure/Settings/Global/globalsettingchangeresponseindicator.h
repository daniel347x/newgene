#ifndef GLOBALSETTINGCHANGERESPONSEINDICATOR_H
#define GLOBALSETTINGCHANGERESPONSEINDICATOR_H

#include <QObject>
#include "globalsettingchangeindicator.h"
#include "settingchangeresponseindicator.h"

class GlobalSettingChangeResponseIndicator : public QObject, public GlobalSettingChangeIndicator, public SettingChangeResponseIndicator
{
		Q_OBJECT
	public:
		explicit GlobalSettingChangeResponseIndicator( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // GLOBALSETTINGCHANGERESPONSEINDICATOR_H
