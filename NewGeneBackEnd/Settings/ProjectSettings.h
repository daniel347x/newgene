#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

namespace PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum PROJECT_SETTINGS_BACKEND
	{
		SETTING_LAST
	};

}

class ProjectSettings : public Settings<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND, BackendProjectSetting>
{

	public:

		ProjectSettings(Messager & messager);
		ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path);
		virtual ~ProjectSettings() {}


	protected:

		void LoadDefaultSettings(Messager & messager);


	private:

		std::map<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND, BackendProjectSetting> _ui_settings;

};

#endif
