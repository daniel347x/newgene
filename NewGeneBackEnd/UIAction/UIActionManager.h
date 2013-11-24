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

		void DoVariableGroupSetMemberSelectionChange(Messager & messager, WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project);
		void DoKAdCountChange(Messager & messager, WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE const & action_request, OutputProject & project);
		void DoDoRandomSamplingChange(Messager & messager, WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE const & action_request, OutputProject & project);
		void DoRandomSamplingCountPerStageChange(Messager & messager, WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE const & action_request, OutputProject & project);
		void DoTimeRangeChange(Messager & messager, WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE const & action_request, OutputProject & project);
		void DoGenerateOutput(Messager & messager, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project);

	protected:

		bool FailIfBusy(Messager & messager);
		void EndFailIfBusy();

		static std::recursive_mutex is_busy_mutex;
		static std::atomic<bool> is_busy;

};

#endif
