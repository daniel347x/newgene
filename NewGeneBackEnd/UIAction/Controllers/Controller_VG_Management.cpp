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
		boost::format msg("There are no new UOAs to add.");
		messager.ShowMessageBox(msg.str());
		return;
	}

	InputModel & input_model = project.model();

	if (!action_request.items || action_request.items->size() == 0)
	{
		boost::format msg("There are no new UOAs to add.");
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
																 boost::format msg("Missing the new UOA to create.");
																 messager.ShowMessageBox(msg.str());
																 return;
															 }

															 // ************************************* //
															 // Retrieve data sent by user interface
															 // ************************************* //
															 WidgetActionItem const & actionItem = *instanceActionItem.second;
															 WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int const & actionItemWsStringStringInt = static_cast<WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int const &>(actionItem);
															 WidgetInstanceIdentifiers const & dmu_categories = actionItemWsStringStringInt.getValue();
															 std::string const & new_uoa_code = actionItemWsStringStringInt.getValueString();
															 std::string const & uoa_description = actionItemWsStringStringInt.getValueString2();
															 int const & the_time_granularity = actionItemWsStringStringInt.getValueInt();
															 TIME_GRANULARITY time_granularity = (TIME_GRANULARITY)the_time_granularity;

															 if (dmu_categories.size() == 0)
															 {
																 boost::format msg("At least one DMU category is required to create a new UOA.");
																 messager.ShowMessageBox(msg.str());
																 return;
															 }

															 bool uoa_already_exists = input_model.t_uoa_category.ExistsByCode(input_model.getDb(), input_model, new_uoa_code);

															 if (uoa_already_exists)
															 {
																 boost::format msg("The UOA code '%1%' already exists.");
																 msg % boost::to_upper_copy(new_uoa_code);
																 messager.ShowMessageBox(msg.str());
																 return;
															 }

															 bool uoa_successfully_created = input_model.t_uoa_category.CreateNewUOA(input_model.getDb(), input_model, new_uoa_code, uoa_description, dmu_categories, time_granularity);

															 if (!uoa_successfully_created)
															 {
																 boost::format msg("Unable to execute INSERT statement to create a new UOA category.");
																 throw NewGeneException() << newgene_error_description(msg.str());
															 }

															 boost::format msg("UOA '%1%' successfully created.");
															 msg % boost::to_upper_copy(new_uoa_code);
															 messager.ShowMessageBox(msg.str());

															 // ***************************************** //
															 // Prepare data to send back to user interface
															 // ***************************************** //

															 WidgetInstanceIdentifier newIdentifier;
															 bool found_newly_created_uoa = input_model.t_uoa_category.getIdentifierFromStringCode(new_uoa_code, newIdentifier);

															 if (!found_newly_created_uoa || !newIdentifier.uuid || newIdentifier.uuid->empty())
															 {
																 boost::format msg("Unable to find newly created UOA.");
																 throw NewGeneException() << newgene_error_description(msg.str());
															 }

															 DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE;
															 DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
															 DataChange change(type, intention, newIdentifier, dmu_categories);

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

																if (!instanceActionItem.second)
																{
																	boost::format msg("The DMU identifiers are invalid.");
																	messager.ShowMessageBox(msg.str());
																	return;
																}

																WidgetInstanceIdentifier uoa_category = instanceActionItem.first;
																WidgetActionItem const & actionItem = *instanceActionItem.second;
																WidgetActionItem__WidgetInstanceIdentifiers const & actionItemDMUs = static_cast<WidgetActionItem__WidgetInstanceIdentifiers const &>(actionItem);
																WidgetInstanceIdentifiers dmu_categories = actionItemDMUs.getValue();

																if (!uoa_category.uuid || uoa_category.uuid->empty())
																{
																	boost::format msg("Missing the UOA to delete.");
																	messager.ShowMessageBox(msg.str());
																	return;
																}

																// ************************************* //
																// Retrieve data sent by user interface
																// ************************************* //
																std::string uoa_to_delete_display_text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa_category, dmu_categories);

																bool uoa_already_exists = input_model.t_uoa_category.Exists(input_model.getDb(), input_model, uoa_category);

																if (!uoa_already_exists)
																{
																	boost::format msg("The UOA '%1%' is already absent.");
																	msg % boost::to_upper_copy(uoa_to_delete_display_text);
																	messager.ShowMessageBox(msg.str());
																	return;
																}

																bool uoa_successfully_deleted = input_model.t_uoa_category.DeleteUOA(input_model.getDb(), input_model, uoa_category, change_response);

																if (!uoa_successfully_deleted)
																{
																	boost::format msg("Unable to delete the UOA.");
																	throw NewGeneException() << newgene_error_description(msg.str());
																}

																boost::format msg("UOA '%1%' successfully deleted.");
																msg % boost::to_upper_copy(uoa_to_delete_display_text);
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

															 WidgetInstanceIdentifier dmu_category = instanceActionItem.first;

															 if (!dmu_category.code || !dmu_category.uuid || dmu_category.code->empty() || dmu_category.uuid->empty())
															 {
																 boost::format msg("Missing the DMU category to refresh.");
																 messager.ShowMessageBox(msg.str());
																 return;
															 }

															 if (!instanceActionItem.second)
															 {
																 boost::format msg("Missing DMU refresh information.");
																 messager.ShowMessageBox(msg.str());
																 return;
															 }

															 // ************************************* //
															 // Retrieve data sent by user interface
															 // ************************************* //
															 WidgetActionItem const & actionItem = *instanceActionItem.second;
															 WidgetActionItem__StringVector const & actionItemString = static_cast<WidgetActionItem__StringVector const &>(actionItem);
															 std::vector<std::string> dmu_refresh_strings = actionItemString.getValue();

															 std::string dmu_refresh_file_pathname = dmu_refresh_strings[0];
															 std::vector<std::string> dmu_refresh_column_labels(dmu_refresh_strings.cbegin() + 1, dmu_refresh_strings.cend());

															 bool success = input_model.t_dmu_setmembers.RefreshFromFile(input_model.getDb(), input_model, dmu_category, boost::filesystem::path(dmu_refresh_file_pathname), dmu_refresh_column_labels);

															 if (!success)
															 {
																 return;
															 }

															 boost::format msg("DMU category '%1%' successfully refreshed from file.");
															 msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
															 messager.ShowMessageBox(msg.str());

															 // ***************************************** //
															 // Prepare data to send back to user interface
															 // ***************************************** //

															 WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);

															 DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
															 DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;
															 DataChange change(type, intention, dmu_category, dmu_members);

															 change_response.changes.push_back(change);

														 });

														 messager.EmitChangeMessage(change_response);

	}
		break;

	default:
		break;

	}

}
