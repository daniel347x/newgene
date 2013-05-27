#ifndef PROJECT_H
#define PROJECT_H

#include <memory>
#include "../Settings/ProjectSettings.h"

template<typename PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS, typename MODEL_CLASS>
class Project
{
public:
	Project(Messager & messager)
	{

	}
	std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > _settings;
	std::shared_ptr<MODEL_CLASS> _model;
};

#endif
