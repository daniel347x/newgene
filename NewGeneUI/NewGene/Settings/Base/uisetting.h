#ifndef UISETTING_H
#define UISETTING_H

#include "../../../NewGeneBackEnd/Settings/Setting.h"

class UISetting : virtual public Setting
{
	public:
		UISetting();
};

class UIInputSetting : virtual public UISetting, virtual public InputSetting
{
	public:
};

class UIOutputSetting : virtual public UISetting, virtual public OutputSetting
{
	public:
};

class UIGlobalSetting : public GlobalSetting, public UISetting
{
	public:
};

class UIProjectSetting : virtual public ProjectSetting, virtual public UISetting
{
	public:
};

class UIProjectInputSetting : public UIProjectSetting, public UIInputSetting, public ProjectInputSetting
{
	public:
};

class UIProjectOutputSetting : public UIProjectSetting, public UIOutputSetting, public ProjectOutputSetting
{
	public:
};

#endif // UISETTING_H
