#include "UIDataManager.h"

void UIDataManager::DoRefreshInputWidget(Messager & messager, DATA_WIDGETS widget)
{

}

void UIDataManager::DoRefreshOutputWidget(Messager & messager, DATA_WIDGETS widget)
{
	// switch on widget type here
	WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA test_response;
	test_response.n = 13;
	messager.EmitOutputWidgetDataRefresh(test_response);
}
