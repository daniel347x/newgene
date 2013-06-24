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
#include <mutex>

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

		typedef std::map<UUID, NewGeneWidget *> UUIDWidgetMap;

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
		virtual void DoRefreshAllWidgets() {}

		void AddWidgetUUIDToUUIDMap(NewGeneWidget * widget, UUID & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_map_mutex);
			widget_uuid_widget_map[uuid] = widget;
		}

		void RemoveWidgetFromUUIDMap(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_map_mutex);
			auto position = widget_uuid_widget_map.find(uuid);
			if (position != widget_uuid_widget_map.cend())
			{
				widget_uuid_widget_map.erase(position);
			}
		}

		NewGeneWidget * FindWidget(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_map_mutex);
			auto position = widget_uuid_widget_map.find(uuid);
			if (position != widget_uuid_widget_map.cend())
			{
				return position->second;
			}
			return nullptr;
		}

		void AddWidgetDataItemUUIDToUUIDMap(NewGeneWidget * widget, UUID & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(data_item_uuid_map_mutex);
			data_item_uuid_widget_map[uuid] = widget;
		}

		void RemoveWidgetDataItemFromUUIDMap(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(data_item_uuid_map_mutex);
			auto position = data_item_uuid_widget_map.find(uuid);
			if (position != data_item_uuid_widget_map.cend())
			{
				data_item_uuid_widget_map.erase(position);
			}
		}

		NewGeneWidget * FindWidgetFromDataItem(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(data_item_uuid_map_mutex);
			auto position = data_item_uuid_widget_map.find(uuid);
			if (position != data_item_uuid_widget_map.cend())
			{
				return position->second;
			}
			return nullptr;
		}

	protected:

		// The order of initialization is important.
		// C++ data members are initialized in the order they appear
		// in the class declaration (this file).
		// Do not reorder the declarations of these member variables.

		std::shared_ptr<UI_PROJECT_SETTINGS_CLASS> const _project_settings;
		std::shared_ptr<UI_MODEL_SETTINGS_CLASS> const _model_settings;
		std::shared_ptr<UI_MODEL_CLASS> const _model;
		std::unique_ptr<BACKEND_PROJECT_CLASS> const _backend_project;

		std::recursive_mutex widget_uuid_map_mutex;
		UUIDWidgetMap widget_uuid_widget_map;

		std::recursive_mutex data_item_uuid_map_mutex;
		UUIDWidgetMap data_item_uuid_widget_map;

};

#endif // UIPROJECT_H
