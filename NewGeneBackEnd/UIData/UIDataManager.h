#ifndef UIDATAMANAGER_H
#define UIDATAMANAGER_H

#include "..\Manager.h"
#include "../Messager/Messager.h"
#include "DataWidgets.h"

class UIDataManager : public Manager<UIDataManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA>
{
public:
	void DoRefreshInputWidget(Messager & messager, DATA_WIDGETS widget);
	void DoRefreshOutputWidget(Messager & messager, DATA_WIDGETS widget);

};

#endif
