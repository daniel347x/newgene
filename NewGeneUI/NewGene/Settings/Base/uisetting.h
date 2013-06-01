#ifndef UISETTING_H
#define UISETTING_H

#include "uimessager.h"
#include "../../../NewGeneBackEnd/Settings/Setting.h"

class UISetting : virtual public Setting
{
	public:
		UISetting(Messager & messager) : Setting(messager) {}
};

class UIInputSetting : virtual public UISetting, virtual public InputSetting
{
	public:
		UIInputSetting(Messager & messager) : Setting(messager), UISetting(messager), InputSetting(messager) {}
};

class UIOutputSetting : virtual public UISetting, virtual public OutputSetting
{
	public:
		UIOutputSetting(Messager & messager) : Setting(messager), UISetting(messager), OutputSetting(messager) {}
};

class UIGlobalSetting : virtual public UISetting, virtual public GlobalSetting
{
	public:
		UIGlobalSetting(Messager & messager) : Setting(messager), UISetting(messager), GlobalSetting(messager) {}
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

class UIProjectSetting : virtual public UISetting, virtual public ProjectSetting
{
	public:
		UIProjectSetting(Messager & messager) : Setting(messager), UISetting(messager), ProjectSetting(messager) {}
};

class UIProjectInputSetting : public UIProjectSetting, public UIInputSetting, public ProjectInputSetting
{
	public:
		UIProjectInputSetting(Messager & messager) : Setting(messager), UISetting(messager), InputSetting(messager), ProjectSetting(messager), UIProjectSetting(messager), UIInputSetting(messager), ProjectInputSetting(messager) {}
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

class UIProjectOutputSetting : public UIProjectSetting, public UIOutputSetting, public ProjectOutputSetting
{
	public:
		UIProjectOutputSetting(Messager & messager) : Setting(messager), UISetting(messager), OutputSetting(messager), ProjectSetting(messager), UIProjectSetting(messager), UIOutputSetting(messager), ProjectOutputSetting(messager) {}
		SettingInfo GetSettingInfoFromEnum(Messager & messager_, int const enum_val);
		static SETTING_CATEGORY category;
};

#endif // UISETTING_H
