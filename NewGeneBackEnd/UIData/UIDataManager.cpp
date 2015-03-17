#include "UIDataManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

#include <algorithm>
#include <iterator>

/************************************************************************/
// VARIABLE_GROUPS_SCROLL_AREA
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA const & widget_request, OutputProject & project)
{

}
/************************************************************************/
// VARIABLE_GROUPS_TOOLBOX
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX const & widget_request, OutputProject & project)
{
	if (sqlite3_threadsafe() == 0)
	{
		messager.ShowMessageBox("SQLite is not threadsafe!");
		return;
	}

	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_TOOLBOX variable_groups(widget_request);
	variable_groups.identifiers = input_model.t_vgp_identifiers.getIdentifiers();
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	OutputModel & output_model = project.model();
	WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE variable_group(widget_request);

	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		WidgetInstanceIdentifiers identifiers = input_model.t_vgp_setmembers.getIdentifiers(*widget_request.identifier->uuid);
		WidgetInstanceIdentifiers selectedIdentifiers = output_model.t_variables_selected_identifiers.getIdentifiers(*widget_request.identifier->uuid);
		std::for_each(identifiers.cbegin(), identifiers.cend(), [&selectedIdentifiers, &variable_group](WidgetInstanceIdentifier const & identifier)
		{
			bool found = false;
			std::for_each(selectedIdentifiers.cbegin(), selectedIdentifiers.cend(), [&identifier, &found, &variable_group](WidgetInstanceIdentifier const & selectedIdentifier)
			{
				if (identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, selectedIdentifier))
				{
					found = true;
					return; // from lambda
				}
			});

			if (found)
			{
				variable_group.identifiers.push_back(std::make_pair(identifier, true));
			}
			else
			{
				variable_group.identifiers.push_back(std::make_pair(identifier, false));
			}
		});
	}

	std::sort(variable_group.identifiers.begin(), variable_group.identifiers.end());
	messager.EmitOutputWidgetDataRefresh(variable_group);
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_SCROLL_AREA
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA variable_groups(widget_request);
	variable_groups.identifiers = input_model.t_vgp_identifiers.getIdentifiers();
	std::sort(variable_groups.identifiers.begin(), variable_groups.identifiers.end());
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE const & widget_request,
		OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE variable_group(widget_request);

	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		variable_group.identifiers = output_model.t_variables_selected_identifiers.getIdentifiers(*widget_request.identifier->uuid);
	}

	messager.EmitOutputWidgetDataRefresh(variable_group);
}

/************************************************************************/
// KAD_SPIN_CONTROLS_AREA
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_KAD_SPIN_CONTROLS_AREA variable_groups(widget_request);
	variable_groups.active_dmus = project.model().t_variables_selected_identifiers.GetActiveDMUs(&project.model(), &project.model().getInputModel());
	variable_groups.identifiers = input_model.t_dmu_category.getIdentifiers();
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// KAD_SPIN_CONTROL_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_KAD_SPIN_CONTROL_WIDGET kad_spincontrol(widget_request);

	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		WidgetInstanceIdentifier_Int_Pair spinControlData = output_model.t_kad_count.getIdentifier(*widget_request.identifier->uuid);
		kad_spincontrol.count = spinControlData.second;
	}

	messager.EmitOutputWidgetDataRefresh(kad_spincontrol);
}

/************************************************************************/
// TIMERANGE_REGION_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_TIMERANGE_REGION_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_TIMERANGE_REGION_WIDGET timerange_region(widget_request);
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	std::tuple<bool, std::int64_t, bool, bool> info = output_model.t_general_options.getKadSamplerInfo(project.model().getDb());
	timerange_region.do_random_sampling = std::get<0>(info) != 0;
	timerange_region.random_sampling_count_per_stage = std::get<1>(info);
	timerange_region.consolidate_rows = std::get<2>(info) != 0;
	timerange_region.display_absolute_time_columns = std::get<3>(info) != 0;
	messager.EmitOutputWidgetDataRefresh(timerange_region);
}

/************************************************************************/
// DATETIME_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_DATETIME_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_DATETIME_WIDGET timerange_datetimecontrol(widget_request);

	if (widget_request.identifier && widget_request.identifier->code && *widget_request.identifier->code == "0")
	{
		if (widget_request.identifier->flags == "s")
		{
			WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
			bool found = output_model.t_time_range.getIdentifierFromStringCodeAndFlags("0", "s", timerange_start_identifier);

			if (found)
			{
				timerange_datetimecontrol.the_date_time = timerange_start_identifier.second;
			}
		}
		else if (widget_request.identifier->flags == "e")
		{
			WidgetInstanceIdentifier_Int64_Pair timerange_end_identifier;
			bool found = output_model.t_time_range.getIdentifierFromStringCodeAndFlags("0", "e", timerange_end_identifier);

			if (found)
			{
				timerange_datetimecontrol.the_date_time = timerange_end_identifier.second;
			}
		}
	}

	messager.EmitOutputWidgetDataRefresh(timerange_datetimecontrol);
}

/************************************************************************/
// GENERATE_OUTPUT_TAB
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_GENERATE_OUTPUT_TAB const & widget_request, OutputProject & project)
{
	//OutputModel & output_model = project.model();
	WidgetDataItem_GENERATE_OUTPUT_TAB generate_output_tab_data(widget_request);
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	messager.EmitOutputWidgetDataRefresh(generate_output_tab_data);
}

/************************************************************************/
// LIMIT_DMUS_TAB
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_LIMIT_DMUS_TAB const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_LIMIT_DMUS_TAB dmu_limit_info(widget_request);

	std::vector<dmu_category_limit_members_info_tuple> & dmu_category_limit_members_info = dmu_limit_info.dmu_category_limit_members_info;

	WidgetInstanceIdentifiers dmu_categories = project.model().getInputModel().t_dmu_category.getIdentifiers();

	for (auto & dmu_category : dmu_categories)
	{
		WidgetInstanceIdentifiers dmu_set_members__all = project.model().getInputModel().t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);
		bool is_limited = project.model().t_limit_dmus_categories.Exists(project.model().getDb(), project.model(), project.model().getInputModel(), *dmu_category.code);
		WidgetInstanceIdentifiers dmu_set_members__limited = project.model().t_limit_dmus_set_members.getIdentifiers(*dmu_category.code);
		std::sort(dmu_set_members__all.begin(), dmu_set_members__all.end());
		std::sort(dmu_set_members__limited.begin(), dmu_set_members__limited.end());

		WidgetInstanceIdentifiers dmu_set_members_not_limited;
		std::set_difference(dmu_set_members__all.cbegin(), dmu_set_members__all.cend(), dmu_set_members__limited.cbegin(), dmu_set_members__limited.cend(),
							std::inserter(dmu_set_members_not_limited, dmu_set_members_not_limited.begin()));

		dmu_category_limit_members_info.emplace_back(std::make_tuple(dmu_category, is_limited, dmu_set_members__all, dmu_set_members_not_limited, dmu_set_members__limited));
	}

	messager.EmitOutputWidgetDataRefresh(dmu_limit_info);
}

/************************************************************************/
// MANAGE_DMUS_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshInputWidget(Messager & messager, WidgetDataItemRequest_MANAGE_DMUS_WIDGET const & widget_request, InputProject & project)
{
	InputModel & input_model = project.model();
	WidgetDataItem_MANAGE_DMUS_WIDGET dmu_management(widget_request);
	WidgetInstanceIdentifiers dmus = input_model.t_dmu_category.getIdentifiers();
	std::for_each(dmus.cbegin(), dmus.cend(), [this, &dmu_management, &input_model](WidgetInstanceIdentifier const & single_dmu)
	{
		WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*single_dmu.uuid);
		dmu_management.dmus_and_members.push_back(std::make_pair(single_dmu, dmu_members));
	});
	messager.EmitInputWidgetDataRefresh(dmu_management);
}

/************************************************************************/
// MANAGE_UOAS_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshInputWidget(Messager & messager, WidgetDataItemRequest_MANAGE_UOAS_WIDGET const & widget_request, InputProject & project)
{
	InputModel & input_model = project.model();
	WidgetDataItem_MANAGE_UOAS_WIDGET uoa_management(widget_request);
	WidgetInstanceIdentifiers uoas = input_model.t_uoa_category.getIdentifiers();
	std::for_each(uoas.cbegin(), uoas.cend(), [this, &uoa_management, &input_model](WidgetInstanceIdentifier const & single_uoa)
	{
		if (!single_uoa.uuid || single_uoa.uuid->empty())
		{
			boost::format msg("Bad UOA in action handler.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		WidgetInstanceIdentifiers dmu_categories = input_model.t_uoa_category.RetrieveDMUCategories(input_model.getDb(), &input_model, *single_uoa.uuid);
		uoa_management.uoas_and_dmu_categories.push_back(std::make_pair(single_uoa, dmu_categories));
	});
	messager.EmitInputWidgetDataRefresh(uoa_management);
}

/************************************************************************/
// MANAGE_VGS_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshInputWidget(Messager & messager, WidgetDataItemRequest_MANAGE_VGS_WIDGET const & widget_request, InputProject & project)
{
	InputModel & input_model = project.model();
	WidgetDataItem_MANAGE_VGS_WIDGET vg_management(widget_request);
	WidgetInstanceIdentifiers vgs = input_model.t_vgp_identifiers.getIdentifiers();
	std::for_each(vgs.cbegin(), vgs.cend(), [this, &vg_management, &input_model](WidgetInstanceIdentifier const & single_vg)
	{
		if (!single_vg.uuid || single_vg.uuid->empty() || !single_vg.identifier_parent)
		{
			boost::format msg("Bad VG in action handler.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		vg_management.vgs_and_uoa.push_back(std::make_pair(single_vg, *single_vg.identifier_parent));
	});
	messager.EmitInputWidgetDataRefresh(vg_management);
}
