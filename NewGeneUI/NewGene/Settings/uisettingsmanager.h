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

	static UISettingsManager & getSettingsManager(NewGeneMainWindow * parent = NULL);

signals:

public slots:

protected:

	bool ObtainSettingsPath();

private:

	static std::unique_ptr<UISettingsManager> globalSettings_;

	UIProjectSettings * LoadDefaultProjectSettings();
	void LoadDefaultGlobalSettings();
	UIGlobalSettings * LoadGlobalSettings();

	boost::filesystem::path settingsPath;
	boost::property_tree::ptree settings;

};

#endif // UISETTINGSMANAGER_H
