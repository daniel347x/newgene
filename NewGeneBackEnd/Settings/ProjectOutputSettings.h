#ifndef PROJECTOUTPUTSETTINGS_H
#define PROJECTOUTPUTSETTINGS_H

#include "ProjectSettings.h"

//namespace PROJECT_SETTINGS_BACKEND_NAMESPACE
//{
//
//	enum PROJECT_SETTINGS_BACKEND
//	{
//		SETTING_FIRST = 0
//		, SETTING_LAST
//	};
//
//}

//template<>
//SettingInfo GetSettingInfoFromEnum<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND>(Messager & messager, PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const value_);

class ProjectOutputSettings : public ProjectSettings
{

public:

	ProjectOutputSettings(Messager & messager, boost::filesystem::path const project_settings_path)
		: ProjectSettings(messager, project_settings_path)
	{}
	virtual ~ProjectOutputSettings() {}

};

#endif
