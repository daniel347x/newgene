#ifndef PROJECTSETTINGCHANGERESPONSEINDICATOR_H
#define PROJECTSETTINGCHANGERESPONSEINDICATOR_H

#include <QObject>
#include "projectsettingchangeindicator.h"
#include "settingchangeresponseindicator.h"

class ProjectSettingChangeResponseIndicator : public QObject, public ProjectSettingChangeIndicator, public SettingChangeResponseIndicator
{
		Q_OBJECT
	public:
		explicit ProjectSettingChangeResponseIndicator( QObject * parent = 0 );

	signals:

	public slots:

};

#endif // PROJECTSETTINGCHANGERESPONSEINDICATOR_H
