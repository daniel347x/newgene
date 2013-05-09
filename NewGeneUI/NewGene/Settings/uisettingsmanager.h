#ifndef UISETTINGSMANAGER_H
#define UISETTINGSMANAGER_H

#include "globals.h"
#include "uimanager.h"
#include <QString>
#ifndef Q_MOC_RUN
#   include <boost\filesystem.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/property_tree/xml_parser.hpp>
#endif

class NewGeneMainWindow;
class UIProjectSettings;
class UIGlobalSettings;

class UISettingsManager : public UIManager
{

	Q_OBJECT

public:
	explicit UISettingsManager(NewGeneMainWindow *parent = 0);

	static UISettingsManager * getSettingsManager(NewGeneMainWindow * parent = NULL);

	static QString settingsFileName;

signals:

public slots:

protected:

	bool ObtainSettingsPath();

private:

	static UISettingsManager * settings_;

	UIProjectSettings * LoadDefaultProjectSettings(bool const initializing);
	void LoadDefaultGlobalSettings(bool const initializing);
	UIGlobalSettings * LoadGlobalSettings(bool const initializing);

	boost::filesystem::path settingsPath;
	boost::property_tree::ptree settings;

};

#endif // UISETTINGSMANAGER_H
