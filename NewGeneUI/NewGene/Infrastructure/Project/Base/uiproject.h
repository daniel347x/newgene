#ifndef UIPROJECT_H
#define UIPROJECT_H

//#include "globals.h"
#include <QObject>
#include <memory>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "../../../NewGeneBackEnd/Project/Project.h"
#include "../Model/Base/uimodel.h"
#include "eventloopthreadmanager.h"
#include <QMessageBox>

class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UIProjectManager;
class NewGeneMainWindow;
class UILoggingManager;
class UIProjectManager;

template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class UIProject : public EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>
{
	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		UIProject(std::shared_ptr<UI_PROJECT_SETTINGS_CLASS> const & ui_settings,
				  std::shared_ptr<UI_MODEL_SETTINGS_CLASS> const & ui_model_settings,
				  std::shared_ptr<UI_MODEL_CLASS> const & ui_model,
				  QObject * parent = NULL)
			: EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>(number_worker_threads)
			, _project_settings(ui_settings)
			, _model_settings(ui_model_settings)
			, _model(ui_model)
			, _backend_project( new BACKEND_PROJECT_CLASS(_project_settings->getBackendSettingsSharedPtr(), _model_settings->getBackendSettingsSharedPtr(), _model->getBackendModelSharedPtr()) )
		{
		}

		~UIProject()
		{
		}

		QObject * GetProjectSettingsConnector()
		{
			return projectSettings().getConnector();
		}

		QObject * GetModelSettingsConnector()
		{
			return modelSettings().getConnector();
		}

		QObject * GetModelConnector()
		{
			return model().getConnector();
		}

		// TODO: Test for validity
		UI_PROJECT_SETTINGS_CLASS & projectSettings()
		{
			return *_project_settings;
		}

		// TODO: Test for validity
		UI_MODEL_SETTINGS_CLASS & modelSettings()
		{
			return *_model_settings;
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

		virtual void UpdateConnections() {}

	protected:

		// The order of initialization is important.
		// C++ data members are initialized in the order they appear
		// in the class declaration (this file).
		// Do not reorder the declarations of these member variables.

		std::shared_ptr<UI_PROJECT_SETTINGS_CLASS> const _project_settings;
		std::shared_ptr<UI_MODEL_SETTINGS_CLASS> const _model_settings;
		std::shared_ptr<UI_MODEL_CLASS> const _model;
		std::unique_ptr<BACKEND_PROJECT_CLASS> const _backend_project;

};

#endif // UIPROJECT_H
