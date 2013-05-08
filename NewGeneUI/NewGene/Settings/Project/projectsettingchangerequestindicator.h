#ifndef PROJECTSETTINGCHANGEREQUESTINDICATOR_H
#define PROJECTSETTINGCHANGEREQUESTINDICATOR_H

#include <QObject>
#include "projectsettingchangeindicator.h"
#include "settingchangerequestindicator.h"

class ProjectSettingChangeRequestIndicator : public QObject, public ProjectSettingChangeIndicator, public SettingChangeRequestIndicator
{
		Q_OBJECT
	public:
		explicit ProjectSettingChangeRequestIndicator(QObject *parent = 0);

	signals:

	public slots:

};

#endif // PROJECTSETTINGCHANGEREQUESTINDICATOR_H
