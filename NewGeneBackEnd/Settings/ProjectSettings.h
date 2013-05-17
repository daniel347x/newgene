#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "Settings.h"

class ProjectSettings : public Settings
{

	public:
		ProjectSettings(boost::filesystem::path const global_settings_path);
		virtual ~ProjectSettings() {}

};

#endif
