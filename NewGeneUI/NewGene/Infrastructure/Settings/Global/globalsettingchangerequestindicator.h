#ifndef GLOBALSETTINGCHANGEREQUESTINDICATOR_H
#define GLOBALSETTINGCHANGEREQUESTINDICATOR_H

#include <QObject>
#include "globalsettingchangeindicator.h"
#include "settingchangerequestindicator.h"

class GlobalSettingChangeRequestIndicator : public QObject, public GlobalSettingChangeIndicator, public SettingChangeRequestIndicator
{
		Q_OBJECT
	public:
		explicit GlobalSettingChangeRequestIndicator(QObject * parent = 0);

	signals:

	public slots:

};

#endif // GLOBALSETTINGCHANGEREQUESTINDICATOR_H
