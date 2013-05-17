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
class UIAllProjectSettings;
class UIAllGlobalSettings;

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
		boost::filesystem::path getGlobalSettingsPath() { return global_settings_path; }

	private:

		static std::unique_ptr<UISettingsManager> settingsManager;


		boost::filesystem::path global_settings_path;
		std::unique_ptr<UIAllGlobalSettings> _global_settings;

};

#endif // UISETTINGSMANAGER_H
