#ifndef SETTING_H
#define SETTING_H

#include <cstdint>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "..\Messager\Messager.h"

class Setting;

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

	virtual void DoSpecialParse(Messager &) {}

protected:

	Setting() {} // must use factory function to create settings

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

class BackendGlobalSetting : public GlobalSetting, public BackendSetting
{
	public:
};

class BackendProjectSetting : public ProjectSetting, public BackendSetting
{
	public:
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
