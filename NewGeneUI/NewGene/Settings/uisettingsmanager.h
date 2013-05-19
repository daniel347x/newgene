#ifndef UISETTINGSMANAGER_H
#define UISETTINGSMANAGER_H

#include "globals.h"
#include "uimanager.h"
#include "..\..\..\NewGeneBackEnd\Settings\SettingsManager.h"
#include <QString>
#ifndef Q_MOC_RUN
#   include <boost\filesystem.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/property_tree/xml_parser.hpp>
#endif
#include "Base/uisetting.h"
#include "uiallsettings.h"
#include "uiallglobalsettings.h"
#include "uiallprojectsettings.h"
#include "../../../NewGeneBackEnd/Settings/Setting.h"
#include "../../../NewGeneBackEnd/Settings/Settings.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

class NewGeneMainWindow;
class UIAllProjectSettings;
class UIAllGlobalSettings;

class UISettingsManager : public QObject, public UIManager<UISettingsManager, SettingsManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS>
{

		Q_OBJECT

	public:

		explicit UISettingsManager( QObject * parent = 0 );

		std::unique_ptr<BackendGlobalSetting> getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting);
		std::unique_ptr<BackendProjectSetting> getSetting(Messager & messager, UIProject * project, PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const which_setting);
		std::unique_ptr<UIGlobalSetting> getSetting(Messager & messager, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const which_setting);
		std::unique_ptr<UIProjectSetting> getSetting(Messager & messager, UIProject * project, PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI const which_setting);

		bool ObtainGlobalSettingsPath();
		boost::filesystem::path getGlobalSettingsPath() { return global_settings_path; }

	signals:

	public slots:

	protected:


	private:

		boost::filesystem::path global_settings_path;
		std::unique_ptr<UIAllGlobalSettings> _global_settings;

};

#endif // UISETTINGSMANAGER_H
