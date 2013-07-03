#ifndef UIDATAMANAGER_H
#define UIDATAMANAGER_H

#include "../globals.h"
#include "..\Manager.h"
#include "../Messager/Messager.h"

class InputProject;
class OutputProject;

class UIDataManager : public Manager<UIDataManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA>
{

	public:

		void DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA const & widget_request, OutputProject & project);
		void DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX const & widget_request, OutputProject & project);
		void DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project);
		void DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY const & widget_request, OutputProject & project);
		void DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project);

};

#endif
