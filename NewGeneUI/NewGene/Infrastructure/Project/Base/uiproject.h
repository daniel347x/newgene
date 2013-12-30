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

		class DataChangeInterest
		{
			public:
				DATA_CHANGE_TYPE type;
				bool match_uuid;
				UUID uuid_to_match;
				DataChangeInterest(DATA_CHANGE_TYPE type_, bool match_uuid_, UUID const uuid_to_match_ = "")
					: type(type_)
					, match_uuid(match_uuid_)
					, uuid_to_match(uuid_to_match_)
				{
					if (match_uuid && uuid_to_match.empty())
					{
						match_uuid = false;
					}
				}
				DataChangeInterest(DataChangeInterest const & rhs)
					: type(rhs.type)
					, match_uuid(rhs.match_uuid)
					, uuid_to_match(rhs.uuid_to_match)
				{

				}
		};

		typedef std::map<UUID, NewGeneWidget *> UUIDWidgetMap; // uuid_parent, uuid_child
		typedef std::map<std::pair<UUID, UUID>, NewGeneWidget *> ParentWidgetUUIDAndChildDataUUID_to_WidgetPointer_Map; // uuid_parent, uuid_child
		typedef std::multimap<NewGeneWidget * const, DataChangeInterest const> WidgetDataChangeInterestMap;

		static int const number_worker_threads = 2; // For now, single thread only in pool

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
			Q_UNUSED(parent);
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

		void AddWidgetUUIDToUUIDMap(NewGeneWidget * widget, UUID const & uuid)
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

		void AddWidgetDataItemUUIDToUUIDMap(NewGeneWidget * widget, UUID const & parent_widget_uuid, UUID const & child_data_uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map__mutex);
			widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map[std::make_pair(parent_widget_uuid, child_data_uuid)] = widget;
		}

		void RemoveWidgetDataItemFromUUIDMap(UUID const & parent_widget_uuid, UUID const & child_data_uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map__mutex);
			auto position = widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map.find(std::make_pair(parent_widget_uuid, child_data_uuid));
			if (position != widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map.cend())
			{
				widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map.erase(position);
			}
		}

		NewGeneWidget * FindWidgetFromDataItem(UUID const & parent_widget_uuid, UUID const & child_data_uuid)
		{
			std::lock_guard<std::recursive_mutex> widget_map_guard(widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map__mutex);
			auto position = widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map.find(std::make_pair(parent_widget_uuid, child_data_uuid));
			if (position != widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map.cend())
			{
				return position->second;
			}
			return nullptr;
		}

		void RegisterInterestInChange(NewGeneWidget * widget, DATA_CHANGE_TYPE const type, bool const match_uuid, UUID const & uuid_to_match)
		{
			std::lock_guard<std::recursive_mutex> change_map_guard(data_change_interest_map_mutex);
			data_change_interest_map.insert(std::make_pair(widget, DataChangeInterest(type, match_uuid, uuid_to_match)));
		}

		void UnregisterInterestInChanges(NewGeneWidget * widget)
		{
			std::lock_guard<std::recursive_mutex> change_map_guard(data_change_interest_map_mutex);
			if (data_change_interest_map.find(widget) != data_change_interest_map.cend())
			{
				data_change_interest_map.erase(widget);
			}
		}

		virtual void PassChangeMessageToWidget(NewGeneWidget *, DataChangeMessage const &) {}

		// ********************************************************** //
		// This function is called in the context of the UI thread.
		// ********************************************************** //
		void DisplayChanges(WidgetChangeMessages widget_change_messages)
		{
			std::for_each(widget_change_messages.cbegin(), widget_change_messages.cend(), [this](WidgetChangeMessage const & widget_change_message)
			{
				{
					NewGeneWidget * const & widget = widget_change_message.first;
					DataChangeMessage const & change_message = widget_change_message.second;

					if (!change_message.changes.empty())
					{
						std::lock_guard<std::recursive_mutex> change_map_guard(data_change_interest_map_mutex);
#						ifdef _WIN32
						// Obnoxious Visual Studio bug: http://stackoverflow.com/questions/20847637/clang-os-x-requires-template-keyword-in-a-particular-nested-declaration-whi
						WidgetDataChangeInterestMap::const_iterator found_iterator = data_change_interest_map.find(widget);
#						else
						typename WidgetDataChangeInterestMap::const_iterator found_iterator = data_change_interest_map.find(widget);
#						endif
						if (found_iterator != data_change_interest_map.cend())
						{
							// The widget is still alive
							// (it will only be destroyed by this current thread,
							// so it can't be destroyed right now)
							this->PassChangeMessageToWidget(widget, change_message);
						}
					}

				}

			});
		}

		// ********************************************************** //
		// This function is called in the context of the work queue.
		// ********************************************************** //
		WidgetChangeMessages HandleChanges(DataChangeMessage & changes)
		{

			// 'changes' has a list of change items, each with a DATA_CHANGE_TYPE.
			// 'data_change_interest_map' has a list of widgets, each interested in a list of changes.
			// Todo: Send only one message, with only and all the changes of interest, to each widget.

			std::map<NewGeneWidget *, DataChangeMessage> widget_change_message_map;

			{
				std::lock_guard<std::recursive_mutex> change_map_guard(data_change_interest_map_mutex);
				std::for_each(data_change_interest_map.cbegin(), data_change_interest_map.cend(), [&changes, &widget_change_message_map](std::pair<NewGeneWidget * const, DataChangeInterest const> const & pair_)
				{
					// This function does the following:
					// create widget-to-uuid map, populate it above at same time as reverse map,
					// get uuid from map here in this function using the following widget pointer,
					// if found then test against all uuid's in the DataChange message (parent and all children)
					// for a match and only add to subset_of_changes_message if there's a match,
					// then below only push_back the change message if there are more than 0 changes in it.
					NewGeneWidget * const & widget = pair_.first;
					DataChangeInterest const & interest = pair_.second;
					DataChangeMessage & subset_of_changes_message = widget_change_message_map[widget];

					std::for_each(changes.changes.cbegin(), changes.changes.cend(), [&widget, &interest, &subset_of_changes_message](DataChange const & change)
					{
						if (change.change_type == interest.type)
						{
							if (interest.match_uuid)
							{
								// send only if a UUID matches
								bool matches = false;
								if (change.parent_identifier.uuid && *change.parent_identifier.uuid == interest.uuid_to_match)
								{
									matches = true;
								}
								else
								{
									std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&interest, &matches, &change](WidgetInstanceIdentifier const &)
									{
										if (change.parent_identifier.uuid && *change.parent_identifier.uuid == interest.uuid_to_match)
										{
											matches = true;
										}
									});
								}
								if (matches)
								{
									// Only send if a UUID matches
									subset_of_changes_message.changes.push_back(change);
								}
							}
							else
							{
								// Always send
								subset_of_changes_message.changes.push_back(change);
							}
						}
					});

				});
			}

			WidgetChangeMessages widget_change_messages;
			std::for_each(widget_change_message_map.cbegin(), widget_change_message_map.cend(), [&widget_change_messages](std::pair<NewGeneWidget *, DataChangeMessage> const pair_)
			{
				NewGeneWidget * const & widget = pair_.first;
				DataChangeMessage const & change_message = pair_.second;
				widget_change_messages.push_back(std::make_pair(widget, change_message));
			});
			return widget_change_messages;

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

		std::recursive_mutex widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map__mutex;
		ParentWidgetUUIDAndChildDataUUID_to_WidgetPointer_Map widget_uuid_plus_child_data_item_uuid__to__widget_pointer_map;

		std::recursive_mutex data_change_interest_map_mutex;
		WidgetDataChangeInterestMap data_change_interest_map;

};

#endif // UIPROJECT_H
