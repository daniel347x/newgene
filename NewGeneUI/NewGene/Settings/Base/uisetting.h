#ifndef UISETTING_H
#define UISETTING_H

#include "../../../NewGeneBackEnd/Settings/Setting.h"

class UISetting : virtual public Setting
{
	public:
		UISetting();
};

class UIGlobalSetting : public GlobalSetting, public UISetting
{
	public:
};

class UIProjectSetting : public ProjectSetting, public UISetting
{
	public:
};

#endif // UISETTING_H
