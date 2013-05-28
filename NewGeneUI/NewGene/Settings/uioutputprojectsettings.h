#ifndef UIPROJECTOUTPUTSETTINGS_H
#define UIPROJECTOUTPUTSETTINGS_H

#include "../../../NewGeneBackEnd/Settings/OutputProjectSettings.h"
#include "../../../NewGeneBackEnd/Settings/Setting.h"
#include "../../../NewGeneBackEnd/Project/OutputProject.h"
#include "uiallprojectsettings.h"
#include "Base/UISetting.h"

namespace OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE
{

	enum OUTPUT_PROJECT_SETTINGS_UI
	{
		  SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class UIOutputProjectSettings : public QObject, public UIAllProjectSettings<OutputProjectSettings, OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE::OUTPUT_PROJECT_SETTINGS_UI, BackendProjectOutputSetting, UIProjectOutputSetting>
{

		Q_OBJECT

	signals:

	public slots:


	public:
		UIOutputProjectSettings(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIAllProjectSettings(messager, path_to_settings)
		{

		}

		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		UIProjectOutputSetting * CloneSetting(Messager & messager, UIProjectOutputSetting * current_setting, SettingInfo & setting_info) const;
		UIProjectOutputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);
};

#endif // UIPROJECTOUTPUTSETTINGS_H
