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
		explicit UISettingsManager( QObject * parent = 0 );

		static UISettingsManager & getSettingsManager();

	signals:

	public slots:

	protected:

		bool ObtainGlobalSettingsPath();

	private:

		static std::unique_ptr<UISettingsManager> settingsManager;

		UIProjectSettings * LoadDefaultProjectSettings();
		UIGlobalSettings * LoadDefaultGlobalSettings();
		UIGlobalSettings * LoadGlobalSettings();

		boost::filesystem::path globalsettingsPath;
		boost::property_tree::ptree settings;

		std::unique_ptr<UIGlobalSettings> globalSettings;

};

#endif // UISETTINGSMANAGER_H
