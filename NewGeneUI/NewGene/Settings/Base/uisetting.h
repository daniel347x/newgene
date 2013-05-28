#ifndef UISETTING_H
#define UISETTING_H

#include "uimessager.h"
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
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

class UIProjectSetting : virtual public ProjectSetting, virtual public UISetting
{
	public:
};

class UIProjectInputSetting : public UIProjectSetting, public UIInputSetting, public ProjectInputSetting
{
	public:
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

class UIProjectOutputSetting : public UIProjectSetting, public UIOutputSetting, public ProjectOutputSetting
{
	public:
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

#endif // UISETTING_H
