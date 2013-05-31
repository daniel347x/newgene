#ifndef PROJECT_H
#define PROJECT_H

#include <memory>
#include "../Settings/ProjectSettings.h"

template<typename PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS, typename MODEL_CLASS>
class Project
{

	public:

		Project(Messager & messager, std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > const & settings, std::shared_ptr<MODEL_CLASS> const & model)
			: _settings(settings)
			, _model(model)
		{

		}

		ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> & settings()
		{
			return *_settings;
		}

		MODEL_CLASS & model()
		{
			return *_model;
		}

	protected:

		std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > const _settings;
		std::shared_ptr<MODEL_CLASS> const _model;

};

#endif
