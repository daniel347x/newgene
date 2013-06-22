#ifndef UIDATAMANAGER_H
#define UIDATAMANAGER_H

#include "..\Manager.h"
#include "../Messager/Messager.h"
#include "DataWidgets.h"

class InputProject;
class OutputProject;

class UIDataManager : public Manager<UIDataManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA>
{

public:

	void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA const & widget_request, OutputProject & project);
	void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX const & widget_request, OutputProject & project);

};

#endif
