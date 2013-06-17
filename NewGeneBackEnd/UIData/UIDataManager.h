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
	void DoRefreshInputWidget(Messager & messager, DATA_WIDGETS widget, InputProject & project);
	void DoRefreshOutputWidget(Messager & messager, DATA_WIDGETS widget, OutputProject & project);

};

#endif
