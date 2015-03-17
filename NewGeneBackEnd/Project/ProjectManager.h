#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "../Manager.h"
#include <map>
#include <string>
#include <memory>
#include "../Utilities/Semaphore.h"

class ProjectManager : public Manager<ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>
{


	public:

		enum PROJECT_TYPE
		{
			PROJECT_TYPE__INPUT
			, PROJECT_TYPE__OUTPUT
		};

		bool input_project_is_open;
		bool output_project_is_open;

		ProjectManager(Messager & messager_) : Manager<ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>(messager_)
			, input_project_is_open(false)
			, output_project_is_open(false)
		{
			InitializeTasks();
		}

		bool LetMeRunTask(PROJECT_TYPE const project_type, long const widget_action_item_id, std::string const & task_name, std::string & errorMsg)
		{

			semaphore * the_semaphore = nullptr;
			bool wait_on_semaphore = false;

			{

				std::lock_guard<std::recursive_mutex> data_lock(task_retrieval_mutex);

				task_class_info & task_info = task_class_infos[task_name];

				if (task_info.task_name.empty())
				{
					boost::format msg("There is no task named %1%.");
					msg % task_name;
					errorMsg = msg.str();
					return false;
				}

				task_instance_identifier const task_identifier(task_name, widget_action_item_id);

				// If it exists, the current one will be returned with its state unchanged.
				// If it doesn't exist, a new one will be created, with state set to TASK_STATUS__PENDING_FIRST_REQUEST
				task_instance & task = task_instances[task_identifier];

				if (!task_info.task_semaphore && !task.task_semaphore)
				{
					// the task is currently being processed by a different action, as indicated
					// by the NULL semaphore pointer in both the task_info object, and the current task_instance object
					// So, just return false...
					boost::format msg("There is currently another identical %1% action being performed.  Please try again.");
					msg % task_name;
					errorMsg = msg.str();
					return false;
				}

				bool semaphore_was_available = false;

				if (task_info.task_semaphore)
				{
					// Because the semaphore is currently held by the singleton task_info object,
					// it means we're free to use it and transfer it to the task_instance object
					semaphore_was_available = true;
				}

				if (semaphore_was_available)
				{
					// Transfer ownership of the semaphore to the task
					task.task_semaphore = std::move(task_info.task_semaphore);
				}

				the_semaphore = task.task_semaphore.get();

				// handle the case that either input or output project is not open
				int number_projects_open = 0;

				if (input_project_is_open)
				{
					++number_projects_open;
				}

				if (output_project_is_open)
				{
					++number_projects_open;
				}

				if (number_projects_open < 2)
				{
					task.task_status = TASK_STATUS__ONLY_ONE_PROJECT_IS_OPEN;
					return true;
				}

				TASK_ORDER const task_order = task_info.task_order;

				switch (project_type)
				{

					case PROJECT_TYPE__INPUT:
						{

							switch (task.task_status)
							{

								case TASK_STATUS__PENDING_FIRST_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST;
											return true;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST;
											wait_on_semaphore = true;
										}
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED;
											return true;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											boost::format msg("Output request should not be on hold when processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											boost::format msg("Incorrect order of processing: input request received while output project is processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED;
											wait_on_semaphore = true;
										}
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											boost::format msg("Incorrect order of processing: input request received after output project completed processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE;
											return true;
										}
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__COMPLETED:
									{
										boost::format msg("Received multiple input requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								default:
									{
										boost::format msg("Unknown state when receiving input request to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

							}

						}
						break;

					case PROJECT_TYPE__OUTPUT:
						{

							switch (task.task_status)
							{

								case TASK_STATUS__PENDING_FIRST_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST;
											wait_on_semaphore = true;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST;
											return true;
										}
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											boost::format msg("Input request should not be on hold when processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED;
											return true;
										}
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED;
											wait_on_semaphore = true;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											boost::format msg("Incorrect order of processing: output request received while input project is processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST:
									{
										if (task_order == TASK_ORDER__INPUT_THEN_OUTPUT)
										{
											task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE;
											return true;
										}
										else if (task_order == TASK_ORDER__OUTPUT_THEN_INPUT)
										{
											boost::format msg("Incorrect order of processing: output request received after input project completed processing task %1%");
											msg % task_name;
											errorMsg = msg.str();
											return false;
										}
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								case TASK_STATUS__COMPLETED:
									{
										boost::format msg("Received multiple output requests to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

								default:
									{
										boost::format msg("Unknown state when receiving output request to process task %1%");
										msg % task_name;
										errorMsg = msg.str();
										return false;
									}
									break;

							}
						}
						break;

					default:
						{
							boost::format msg("Invalid logic in LetMeRunTask for task %1%");
							msg % task_name;
							errorMsg = msg.str();
							return false;
						}
						break;

				}

			}

			if (!wait_on_semaphore)
			{
				boost::format msg("Logic error when receiving request to process task %1%");
				msg % task_name;
				errorMsg = msg.str();
				return false;
			}

			the_semaphore->wait();

			return true;

		}

		bool TaskCompleted(PROJECT_TYPE const project_type, long const widget_action_item_id, std::string const & task_name, std::string & errorMsg)
		{

			bool free_semaphore = false;
			bool notify = false;

			std::lock_guard<std::recursive_mutex> data_lock(task_retrieval_mutex);

			task_class_info & task_info = task_class_infos[task_name];

			if (task_info.task_name.empty())
			{
				boost::format msg("There is no task named %1%.");
				msg % task_name;
				errorMsg = msg.str();
				return false;
			}

			task_instance_identifier const task_identifier(task_name, widget_action_item_id);

			// If it exists, the current one will be returned with its state unchanged.
			// If it doesn't exist, a new one will be created, with state set to TASK_STATUS__PENDING_FIRST_REQUEST
			task_instance & task = task_instances[task_identifier];

			if (!task_info.task_semaphore && !task.task_semaphore)
			{
				// the task is currently being processed by a different action, as indicated
				// by the NULL semaphore pointer in both the task_info object, and the current task_instance object
				// So, just return false...
				boost::format msg("There is currently another identical %1% action being performed.");
				msg % task_name;
				errorMsg = msg.str();
				return false;
			}

			// handle the case that either input or output project is not open
			int number_projects_open = 0;

			if (input_project_is_open)
			{
				++number_projects_open;
			}

			if (output_project_is_open)
			{
				++number_projects_open;
			}

			if (number_projects_open < 2)
			{
				task.task_status = TASK_STATUS__COMPLETED;
				task_info.task_semaphore = std::move(task.task_semaphore);
				return true;
			}

			TASK_ORDER const task_order = task_info.task_order;

			switch (project_type)
			{

				case PROJECT_TYPE__INPUT:
					{

						switch (task.task_status)
						{

							case TASK_STATUS__PENDING_FIRST_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST:
								{
									task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED:
								{
									task.task_status = TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE;
									notify = true;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE:
								{
									// free semaphore
									task.task_status = TASK_STATUS__COMPLETED;
									free_semaphore = true;
								}
								break;

							case TASK_STATUS__COMPLETED:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							default:
								{
									boost::format msg("Received input completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

						}

					}
					break;

				case PROJECT_TYPE__OUTPUT:
					{

						switch (task.task_status)
						{

							case TASK_STATUS__PENDING_FIRST_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST:
								{
									task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED:
								{
									task.task_status = TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE;
									notify = true;
								}
								break;

							case TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE:
								{
									// free semaphore
									task.task_status = TASK_STATUS__COMPLETED;
									free_semaphore = true;
								}
								break;

							case TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							case TASK_STATUS__COMPLETED:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

							default:
								{
									boost::format msg("Received output completed at invalid time for task %1%");
									msg % task_name;
									errorMsg = msg.str();
									return false;
								}
								break;

						}
					}
					break;

				default:
					{
						boost::format msg("Invalid logic in TaskCompleted for task %1%");
						msg % task_name;
						errorMsg = msg.str();
						return false;
					}
					break;

			}

			if (notify)
			{
				task.task_semaphore->notify();
			}

			if (free_semaphore)
			{
				task_info.task_semaphore = std::move(task.task_semaphore);
				task_instances.erase(task_identifier);
			}

			return true;

		}

	private:

		enum TASK_ORDER
		{
			TASK_ORDER__UNDEFINED = 0
			, TASK_ORDER__INPUT_THEN_OUTPUT
			, TASK_ORDER__OUTPUT_THEN_INPUT
		};

		enum TASK_STATUS
		{
			TASK_STATUS__PENDING_FIRST_REQUEST
			, TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__OUTPUT_REQUEST
			, TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ON_HOLD__WAITING_ON__INPUT_REQUEST
			, TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__OUTPUT_REQUEST
			, TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__WAITING_ON__INPUT_REQUEST
			, TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__OUTPUT_REQUEST
			, TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__WAITING_ON__INPUT_REQUEST
			, TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_ACTIVE__OUTPUT_REQUEST_RECEIVED
			, TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_ACTIVE__INPUT_REQUEST_RECEIVED
			, TASK_STATUS__INPUT_REQUEST_RECEIVED_AND_COMPLETED__OUTPUT_REQUEST_ACTIVE
			, TASK_STATUS__OUTPUT_REQUEST_RECEIVED_AND_COMPLETED__INPUT_REQUEST_ACTIVE
			, TASK_STATUS__COMPLETED
			, TASK_STATUS__ONLY_ONE_PROJECT_IS_OPEN
		};

		struct task_instance_identifier
		{

			task_instance_identifier(std::string const & task_name_, long const task_id_)
				: task_name(task_name_)
				, task_id(task_id_)
			{}

			task_instance_identifier(task_instance_identifier const & rhs)
				: task_name(rhs.task_name)
				, task_id(rhs.task_id)
			{}

			bool operator<(task_instance_identifier const & rhs) const
			{
				if (task_name != rhs.task_name)
				{
					return task_name < rhs.task_name;
				}

				return task_id < rhs.task_id;
			}

			std::string const task_name;
			long const task_id;

		};

		struct task_instance
		{

			task_instance()
				: task_status(TASK_STATUS__PENDING_FIRST_REQUEST)
			{}

			// Owns the semaphore when active
			std::unique_ptr<semaphore> task_semaphore;

			TASK_STATUS task_status;

		};

		class task_class_info
		{

			public:

				task_class_info()
					: task_order(TASK_ORDER__UNDEFINED)
				{}

				task_class_info(std::string const & task_name_, TASK_ORDER const task_order_, semaphore * task_semaphore_)
					: task_name(task_name_)
					, task_order(task_order_)
					, task_semaphore(task_semaphore_)
				{}

				std::string task_name;
				TASK_ORDER task_order;

				// Owns the semaphore when no task is active or pending
				std::unique_ptr<semaphore> task_semaphore;

		};

		void InitializeTasks()
		{

			static char const * task_names[]
			{
				"delete_dmu"
				, "delete_uoa"
				, "delete_vg"
			};

			static TASK_ORDER task_orders[]
			{
				TASK_ORDER__OUTPUT_THEN_INPUT
				, TASK_ORDER__OUTPUT_THEN_INPUT
				, TASK_ORDER__OUTPUT_THEN_INPUT
			};

			static size_t number_tasks = sizeof(task_names) / sizeof(char *);

			for (size_t task_number = 0; task_number < number_tasks; ++task_number)
			{
				char const * task_name = task_names[task_number];
				TASK_ORDER task_order = task_orders[task_number];
				task_class_infos.emplace(std::piecewise_construct, std::forward_as_tuple(std::string(task_name)), std::forward_as_tuple(std::string(task_name), task_order,
										 new semaphore));
			}

		}

		// global info about the tasks
		std::map<std::string, task_class_info> task_class_infos;

		// a set of pending or active tasks - only one can own the corresponding semaphore at a time
		std::map<task_instance_identifier, task_instance> task_instances;

		std::recursive_mutex task_retrieval_mutex;

};

#endif
