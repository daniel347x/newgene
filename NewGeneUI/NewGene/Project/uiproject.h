#ifndef UIPROJECT_H
#define UIPROJECT_H

#include "globals.h"
#include <QObject>
#include <memory>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "../../../NewGeneBackEnd/Project/Project.h"
#include "../Model/uimodel.h"

class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UIProjectManager;
class NewGeneMainWindow;
class UILoggingManager;
class UIProjectManager;

template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_CLASS>
class UIProject
{
	public:
		UIProject(Messager & messager, boost::filesystem::path const path_to_settings)
			: _backend_project( new BACKEND_PROJECT_CLASS(messager) )
		{

		}

		void apply_settings(UI_PROJECT_SETTINGS_CLASS * ui_settings)
		{
			_project_settings.reset(ui_settings);
			backend()._settings = _project_settings->getBackendSettingsSharedPtr(); // share the pointer to the backend settings with the backend project
		}

		void apply_model(UI_MODEL_CLASS * ui_model)
		{
			_model.reset(ui_model);
		}

		// TODO: Test for validity
		UI_PROJECT_SETTINGS_CLASS & settings()
		{
			return *_project_settings;
		}

		// TODO: Test for validity
		UI_MODEL_CLASS & model()
		{
			return *_model;
		}

		// TODO: Test for validity
		BACKEND_PROJECT_CLASS & backend()
		{
			return *_backend_project;
		}

	protected:

		// The model represents the data and metadata itself.
		// The model and the settings (below) form an unseparable pair
		// that together define a single NewGene "project".
		// The model definition is contained within a single database file,
		// although the project may contain many database files.
		std::unique_ptr<UI_MODEL_CLASS> _model;

		// The backend project is the backend equivalent of this UIProject,
		// with the exception that this UIProject carries a shared_ptr to the backend project,
		// but not vice-versa (the backend project should not be, and is not,
		// affected by the state of the user interface).
		//
		// The order of initialization is important.
		// C++ data members are initialized in the order they appear
		// in the class declaration (this file).
		// Do not reorder the declarations of these member variables,
		// as the backend project must be completely instantiated
		// before the project settings, below, are loaded,
		// because the project settings internally must know
		// which project they correspond to (i.e., a pointer
		// to the backend project is passed to the constructor
		// of the project settings when this UIProject is initialized).
		std::shared_ptr<BACKEND_PROJECT_CLASS > _backend_project;

		// The settings are distinguished from the model only by the fact
		// that they are stored permanently in human-readable XML or text files
		// in order to be more accessible to end-users, and hence readable and
		// editable by knowledgeable users.
		std::unique_ptr<UI_PROJECT_SETTINGS_CLASS> _project_settings;

};

#endif // UIPROJECT_H
