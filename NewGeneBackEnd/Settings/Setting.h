#ifndef SETTING_H
#define SETTING_H

#include <cstdint>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "..\Messager\Messager.h"
#include "SettingsRepository.h"

class Setting;

enum SETTING_CATEGORY
{
	  SETTING_CATEGORY__GLOBAL_BACKEND
	, SETTING_CATEGORY__GLOBAL_UI
	, SETTING_CATEGORY__PROJECT_INPUT_BACKEND
	, SETTING_CATEGORY__PROJECT_INPUT_UI
	, SETTING_CATEGORY__PROJECT_OUTPUT_BACKEND
	, SETTING_CATEGORY__PROJECT_OUTPUT_UI
};

template<typename SETTING_CLASS, typename SETTING_VALUE_TYPE = SETTING_CLASS::type>
class SettingFactory
{

public:

	SETTING_CLASS * operator()(Messager & messager, SETTING_VALUE_TYPE const & initializing_val)
	{

		SETTING_CLASS * new_setting = new SETTING_CLASS(messager, initializing_val);
		new_setting->DoSpecialParse(messager);
		return new_setting;

	}

};

class Setting
{

public:

	friend class SettingRepository;

	virtual void DoSpecialParse(Messager &) {}

	// TODO: throw
	virtual SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val) { return SettingInfo(); };

protected:

	Setting() {} // must use factory function to create settings

};

template<typename DERIVED_SETTING_CLASS>
class SimpleAccessSetting_base
{
	public:
		typedef std::unique_ptr<DERIVED_SETTING_CLASS> instance;
};

template<typename DERIVED_SETTING_CLASS, typename SETTING_ENUM, SETTING_ENUM setting_enum, typename SETTINGS_MANAGER_CLASS>
class SimpleAccessSetting : public SimpleAccessSetting_base<DERIVED_SETTING_CLASS>
{
public:
	static instance get(Messager & messager)
	{
		instance derived_setting;
		std::unique_ptr<Setting> setting = static_cast<SETTINGS_MANAGER_CLASS&>(SETTINGS_MANAGER_CLASS::getManager()).getSetting(messager, setting_enum);
		try
		{
			derived_setting.reset(dynamic_cast<DERIVED_SETTING_CLASS*>(setting.release()));
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Cannot convert from UIGlobalSetting to derived setting class: %1%");
			msg % bc.what();
			messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
			return instance();
		}
		return derived_setting;
	}
};

class GlobalSetting : virtual public Setting
{
	public:
};

class ProjectSetting : virtual public Setting
{
	public:
};

class BackendSetting : virtual public Setting
{
	public:
};

class InputSetting : virtual public Setting
{
public:
};

class OutputSetting : virtual public Setting
{
public:
};

class BackendGlobalSetting : public GlobalSetting, public BackendSetting
{
	public:
		SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
		static SETTING_CATEGORY category;
};

class BackendProjectSetting : public ProjectSetting, public BackendSetting
{
	public:
};

class BackendInputSetting : virtual public BackendSetting, virtual public InputSetting
{
public:
};

class BackendOutputSetting : virtual public BackendSetting, virtual public OutputSetting
{
public:
};

class ProjectInputSetting : virtual public ProjectSetting, virtual public InputSetting
{
public:
};

class ProjectOutputSetting : virtual public ProjectSetting, virtual public OutputSetting
{
public:
};

class BackendProjectInputSetting : public BackendProjectSetting, public BackendInputSetting, public ProjectInputSetting
{
public:
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
};

class BackendProjectOutputSetting : public BackendProjectSetting, public BackendOutputSetting, public ProjectOutputSetting
{
public:
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
};

class StringSetting : virtual public Setting
{
	public:

		typedef std::string type;

		StringSetting(Messager &, std::string const & setting)
			: string_setting(setting)
		{}

		std::string getString() { return string_setting; }

	protected:
		std::string string_setting;
};

class Int32Setting : virtual public Setting
{
	public:

		typedef std::int32_t type;

		Int32Setting(Messager &, std::int32_t const & setting)
			: int32_setting(setting)
		{}

		std::int32_t getInt32() { return int32_setting; }

	protected:
		std::int32_t int32_setting;
};

#endif
