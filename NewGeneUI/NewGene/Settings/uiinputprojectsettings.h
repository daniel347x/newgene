#ifndef UIPROJECTINPUTSETTINGS_H
#define UIPROJECTINPUTSETTINGS_H

#include "../../../NewGeneBackEnd/Settings/InputProjectSettings.h"
#include "../../../NewGeneBackEnd/Settings/Setting.h"
#include "../../../NewGeneBackEnd/Project/InputProject.h"
#include "uiallprojectsettings.h"
#include "Base/UISetting.h"
#include "../../../NewGeneBackEnd/Settings/SettingsRepository.h"

namespace INPUT_PROJECT_SETTINGS_UI_NAMESPACE
{

	enum INPUT_PROJECT_SETTINGS_UI
	{
		  SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class UIInputProjectSettings : public QObject, public UIAllProjectSettings<InputProjectSettings, INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI, BackendProjectInputSetting, UIProjectInputSetting>
{

		Q_OBJECT

	signals:

	public slots:


	public:
		UIInputProjectSettings(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIAllProjectSettings(messager, path_to_settings)
		{

		}

		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		UIProjectInputSetting * CloneSetting(Messager & messager, UIProjectInputSetting * current_setting, SettingInfo & setting_info) const;
		UIProjectInputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);
};

#endif // UIPROJECTINPUTSETTINGS_H
