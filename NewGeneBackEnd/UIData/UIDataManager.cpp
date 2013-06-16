#include "UIDataManager.h"

void UIDataManager::DoRefreshInputWidget(Messager & messager, DATA_WIDGETS widget)
{

}

void UIDataManager::DoRefreshOutputWidget(Messager & messager, DATA_WIDGETS widget)
{
	messager.ShowMessageBox("Successfully called a message box from within the BACKEND worker thread!");
	WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA test_response;
	test_response.n = 12;
	messager.EmitOutputWidgetDataRefresh(test_response);
}
