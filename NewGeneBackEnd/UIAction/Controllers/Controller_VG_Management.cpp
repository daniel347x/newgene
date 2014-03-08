#include "../UIActionManager.h"

#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif
#include "../../Utilities/TimeRangeHelper.h"

/************************************************************************/
// ACTION_CREATE_VG
// ACTION_DELETE_VG
// ACTION_REFRESH_VG
/************************************************************************/

void UIActionManager::CreateVG(Messager & messager, WidgetActionItemRequest_ACTION_CREATE_VG const & action_request, InputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		boost::format msg("There are no new VGs to add.");
		messager.ShowMessageBox(msg.str());
		return;
	}

	InputModel & input_model = project.model();

	if (!action_request.items || action_request.items->size() == 0)
	{
		boost::format msg("There are no new VGs to add.");
		messager.ShowMessageBox(msg.str());
		return;
	}

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{
					if (!instanceActionItem.second)
					{
						boost::format msg("Missing the new VG to create.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetInstanceIdentifier const & uoa_to_use = instanceActionItem.first;
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__StringVector const & actionItemStrings = static_cast<WidgetActionItem__StringVector const &>(actionItem);
					std::vector<std::string> const & vg_strings = actionItemStrings.getValue();
					if (vg_strings.size() == 0)
					{
						boost::format msg("Incorrect internal data format for VG creation.");
						messager.ShowMessageBox(msg.str());
						return;
					}
					std::string const & new_vg_code = vg_strings[0];
					std::string const & vg_description = vg_strings[1];

					bool vg_already_exists = input_model.t_vgp_identifiers.ExistsByCode(input_model.getDb(), input_model, new_vg_code);

					if (vg_already_exists)
					{
						boost::format msg("The VG code '%1%' already exists.");
						msg % boost::to_upper_copy(new_vg_code);
						messager.ShowMessageBox(msg.str());
						return;
					}

					bool vg_successfully_created = input_model.t_vgp_identifiers.CreateNewVG(input_model.getDb(), input_model, new_vg_code, vg_description, uoa_to_use);

					if (!vg_successfully_created)
					{
						boost::format msg("Unable to execute INSERT statement to create a new VG category.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					boost::format msg("Variable group '%1%' successfully created.");
					msg % boost::to_upper_copy(new_vg_code);
					messager.ShowMessageBox(msg.str());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifier newIdentifier;
					bool found_newly_created_vg = input_model.t_vgp_identifiers.getIdentifierFromStringCode(new_vg_code, newIdentifier);

					if (!found_newly_created_vg || !newIdentifier.uuid || newIdentifier.uuid->empty() || !newIdentifier.code || newIdentifier.code->empty() || !newIdentifier.identifier_parent)
					{
						boost::format msg("Unable to find newly created VG.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
					DataChange change(type, intention, newIdentifier, WidgetInstanceIdentifiers());

					change_response.changes.push_back(change);

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteVG(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_VG const & action_request, InputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					WidgetInstanceIdentifier vg = instanceActionItem.first;

					if (!vg.uuid || vg.uuid->empty() || !vg.code || vg.code->empty())
					{
						boost::format msg("Missing the VG to delete.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					std::string vg_to_delete_display_text = Table_VG_CATEGORY::GetVgDisplayText(vg);

					bool vg_already_exists = input_model.t_vgp_identifiers.Exists(input_model.getDb(), input_model, vg);

					if (!vg_already_exists)
					{
						boost::format msg("The VG '%1%' is already absent.");
						msg % boost::to_upper_copy(vg_to_delete_display_text);
						messager.ShowMessageBox(msg.str());
						return;
					}

					bool vg_successfully_deleted = input_model.t_vgp_identifiers.DeleteVG(input_model.getDb(), &input_model, vg, change_response);

					if (!vg_successfully_deleted)
					{
						boost::format msg("Unable to delete the VG.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					boost::format msg("VG '%1%' successfully deleted.");
					msg % Table_VG_CATEGORY::GetVgDisplayText(vg);
					messager.ShowMessageBox(msg.str());

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteVGOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_VG const & action_request, OutputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	OutputModel & output_model = project.model();
	InputModel & input_model = output_model.getInputModel();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&output_model, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					WidgetInstanceIdentifier dmu = instanceActionItem.first;

					if (!dmu.code || !dmu.uuid)
					{
						boost::format msg("Missing the UOA to delete.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					std::string dmu_to_delete_code = *dmu.code;
					std::string dmu_to_delete_uuid = *dmu.uuid;

					// The INPUT model does the bulk of the deleting,
					// and informs the UI.
					// Here, we just want to clear a couple things in the output database.

					output_model.t_kad_count.Remove(output_model.getDb(), *dmu.code);

					WidgetInstanceIdentifiers uoas = input_model.t_uoa_setmemberlookup.RetrieveUOAsGivenDMU(input_model.getDb(), &input_model, dmu);
					std::for_each(uoas.cbegin(), uoas.cend(), [&](WidgetInstanceIdentifier const & uoa)
					{
						if (uoa.uuid)
						{
							WidgetInstanceIdentifiers vgs(input_model.t_vgp_identifiers.RetrieveVGsFromUOA(input_model.getDb(), &input_model, *uoa.uuid));
							std::for_each(vgs.cbegin(), vgs.cend(), [&](WidgetInstanceIdentifier const & vg)
							{
								if (vg.code)
								{
									output_model.t_variables_selected_identifiers.RemoveAllfromVG(output_model.getDb(), *vg.code);
								}
							});
						}
					});

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::RefreshVG(Messager & messager, WidgetActionItemRequest_ACTION_REFRESH_VG const & action_request, InputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					if (!instanceActionItem.second)
					{
						boost::format msg("Missing VG refresh information.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__ImportVariableGroup const & actionItemData = static_cast<WidgetActionItem__ImportVariableGroup const &>(actionItem);

					WidgetInstanceIdentifier variable_group = actionItemData.getVG();
					std::vector<std::string> timeRangeColumnNames = actionItemData.getTimeRangeColNames();
					std::vector<std::pair<WidgetInstanceIdentifier, std::string>> dmusAndColumnNames = actionItemData.getDmusAndColNames();
					boost::filesystem::path filePathName = actionItemData.getFilePathName();
					TIME_GRANULARITY time_granularity = actionItemData.getTimeGranularity();

					if (!variable_group.uuid || variable_group.uuid->empty() || !variable_group.code || variable_group.code->empty())
					{
						boost::format msg("Missing the VG to refresh.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					std::string errorMsg;
					std::unique_ptr<Table_VariableGroupData> new_table(new Table_VariableGroupData(*variable_group.code));
					if (new_table == nullptr)
					{
						boost::format msg("Out of memory.  Cannot import the variable group data.");
						messager.ShowMessageBox(msg.str());
						return;
					}
					ImportDefinition import_definition;
					bool success = new_table->BuildImportDefinition(input_model.getDb(), &input_model, variable_group, timeRangeColumnNames, dmusAndColumnNames, filePathName, time_granularity, import_definition, errorMsg);
					if (!success)
					{
						new_table->DeleteDataTable(input_model.getDb(), &input_model);
						boost::format msg("Failed to build the import definition: %1%");
						msg % errorMsg;
						messager.ShowMessageBox(msg.str());
						return;
					}

					// Add the metadata for the new table to the VG_DATA_METADATA__DATETIME_COLUMNS table
					errorMsg.clear();
					success = input_model.t_vgp_data_metadata__datetime_columns.AddDataTable(input_model.getDb(), &input_model, variable_group, errorMsg);
					if (!success)
					{
						new_table->DeleteDataTable(input_model.getDb(), &input_model);
						boost::format msg("%1%");
						if (errorMsg.empty())
						{
							msg % errorMsg;
						}
						else
						{
							msg % "Unable to create date/time column entries for the variable group.";
						}
						messager.ShowMessageBox(msg.str());
						return;
					}

					// Add the metadata for the new table to the VG_DATA_METADATA__PRIMARY_KEYS table
					errorMsg.clear();
					success = input_model.t_vgp_data_metadata__primary_keys.AddDataTable(input_model.getDb(), &input_model, variable_group, import_definition.primary_keys_info, errorMsg);
					if (!success)
					{
						new_table->DeleteDataTable(input_model.getDb(), &input_model);
						boost::format msg("%1%");
						if (errorMsg.empty())
						{
							msg % errorMsg;
						}
						else
						{
							msg % "Unable to create primary key metadata column entries for the variable group.";
						}
						messager.ShowMessageBox(msg.str());
						return;
					}

					Importer table_importer(import_definition, &input_model, new_table.get(), Importer::INSERT_OR_UPDATE, variable_group, InputModelImportTableFn);
					success = table_importer.DoImport();
					if (!success)
					{
						new_table->DeleteDataTable(input_model.getDb(), &input_model);
						boost::format msg("Unable to import or refresh the variable group from the file.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// Success!  Turn the pointer over to the input model
					input_model.t_vgp_data_vector.push_back(std::move(new_table));

					boost::format msg("VG '%1%' successfully refreshed from file.");
					msg % Table_VG_CATEGORY::GetVgDisplayText(variable_group);
					messager.ShowMessageBox(msg.str());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifiers vg_members = input_model.t_vgp_setmembers.getIdentifiers(*variable_group.uuid);

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention, variable_group, vg_members);
					change_response.changes.push_back(change);

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}
