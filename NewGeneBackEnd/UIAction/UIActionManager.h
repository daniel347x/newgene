#ifndef UIACTIONMANAGER_H
#define UIACTIONMANAGER_H

#include "../globals.h"
#include "../Manager.h"
#include "../Messager/Messager.h"
#include <atomic>
#include <mutex>

class InputProject;
class OutputProject;

class UIActionManager : public Manager<UIActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION>
{

	public:
		UIActionManager(Messager & messager_) : Manager<UIActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION>(messager_) {}

	public:

		void DoVariableGroupSetMemberSelectionChange(Messager & messager, WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project);
		void DoKAdCountChange(Messager & messager, WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE const & action_request, OutputProject & project);
		void DoDoKadSamplerChange(Messager & messager, WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE const & action_request, OutputProject & project);
		void DoKadSamplerCountPerStageChange(Messager & messager, WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE const & action_request, OutputProject & project);
		void DoConsolidateRowsChange(Messager & messager, WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE const & action_request, OutputProject & project);
		void DoTimeRangeChange(Messager & messager, WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE const & action_request, OutputProject & project);
		void DoGenerateOutput(Messager & messager, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project);
		void DoLimitDmusChange(Messager & messager, WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE const & action_request, OutputProject & project);

		void AddDMU(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU const & action_request, InputProject & project);
		void DeleteDMU(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, InputProject & project);
		void DeleteDMUOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, OutputProject & project);
		void AddDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS const & action_request, InputProject & project);
		void DeleteDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS const & action_request, InputProject & project);
		void DeleteDMUMembersOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS const & action_request, OutputProject & project);
		void RefreshDMUsFromFile(Messager & messager, WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE const & action_request, InputProject & project);
		void AddUOA(Messager & messager, WidgetActionItemRequest_ACTION_ADD_UOA const & action_request, InputProject & project);
		void DeleteUOA(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_UOA const & action_request, InputProject & project);
		void DeleteUOAOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_UOA const & action_request, OutputProject & project);
		void CreateVG(Messager & messager, WidgetActionItemRequest_ACTION_CREATE_VG const & action_request, InputProject & project);
		void DeleteVG(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_VG const & action_request, InputProject & project);
		void RefreshVG(Messager & messager, WidgetActionItemRequest_ACTION_REFRESH_VG const & action_request, InputProject & project);
		void DeleteVGOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_VG const & action_request, OutputProject & project);

	protected:

		bool FailIfBusy(Messager & messager);
		void EndFailIfBusy();

		static std::recursive_mutex is_busy_mutex;
		static std::atomic<bool> is_busy;

};

#endif
