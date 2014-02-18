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
		void DoDoRandomSamplingChange(Messager & messager, WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE const & action_request, OutputProject & project);
		void DoRandomSamplingCountPerStageChange(Messager & messager, WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE const & action_request, OutputProject & project);
		void DoTimeRangeChange(Messager & messager, WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE const & action_request, OutputProject & project);
		void DoGenerateOutput(Messager & messager, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project);

		void AddDMU(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU const & action_request, InputProject & project);
		void DeleteDMU(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, InputProject & project);
		void AddDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS const & action_request, InputProject & project);
		void DeleteDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS const & action_request, InputProject & project);
		void RefreshDMUsFromFile(Messager & messager, WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE const & action_request, InputProject & project);

	protected:

		bool FailIfBusy(Messager & messager);
		void EndFailIfBusy();

		static std::recursive_mutex is_busy_mutex;
		static std::atomic<bool> is_busy;

};

#endif
