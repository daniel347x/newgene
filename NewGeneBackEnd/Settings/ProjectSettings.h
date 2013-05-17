#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

enum PROJECT_SETTINGS_BACKEND
{
	PROJECT_SETTING_BACKEND__LAST
};

class ProjectSettings : public Settings<PROJECT_SETTINGS_BACKEND, BackendProjectSetting>
{

	public:

		ProjectSettings();
		ProjectSettings(boost::filesystem::path const project_settings_path);
		virtual ~ProjectSettings() {}

	private:

		std::map<PROJECT_SETTINGS_BACKEND, BackendProjectSetting> _ui_settings;

};

#endif
