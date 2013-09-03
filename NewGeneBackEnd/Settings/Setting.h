#ifndef SETTING_H
#define SETTING_H

#include <cstdint>
#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
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
	, SETTING_CATEGORY__MODEL_INPUT
	, SETTING_CATEGORY__MODEL_OUTPUT
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

	virtual std::string ToString() const = 0;

	virtual void DoSpecialParse(Messager &) {}

	// TODO: throw
	virtual SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val) { return SettingInfo(); };

protected:

	Setting(Messager &) {} // must use factory function to create settings

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
			boost::format msg("Cannot convert to derived setting class: %1%");
			msg % bc.what();
			messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
			return instance();
		}
		return derived_setting;
	}
};

template<typename DERIVED_SETTING_CLASS, typename SETTING_ENUM, SETTING_ENUM setting_enum, typename SETTINGS_MANAGER_CLASS, typename SETTINGS_CLASS>
class SimpleAccessProjectModelSetting : public SimpleAccessSetting_base<DERIVED_SETTING_CLASS>
{
public:
	static instance get(Messager & messager, SETTINGS_CLASS & settings)
	{
		instance derived_setting;
		std::unique_ptr<Setting> setting = static_cast<SETTINGS_MANAGER_CLASS&>(SETTINGS_MANAGER_CLASS::getManager()).getSetting(messager, settings, setting_enum);
		try
		{
			derived_setting.reset(dynamic_cast<DERIVED_SETTING_CLASS*>(setting.release()));
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Cannot convert to derived setting class: %1%");
			msg % bc.what();
			messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
			return instance();
		}
		return derived_setting;
	}
};

class BackendSetting : virtual public Setting
{
	public:
		BackendSetting(Messager & messager)
			: Setting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class InputSetting : virtual public Setting
{
	public:
		InputSetting(Messager & messager)
			: Setting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class OutputSetting : virtual public Setting
{
	public:
		OutputSetting(Messager & messager)
			: Setting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class GlobalSetting : virtual public Setting
{
	public:
		GlobalSetting(Messager & messager)
			: Setting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class ProjectSetting : virtual public Setting
{
	public:
		ProjectSetting(Messager & messager)
			: Setting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class ModelSetting : virtual public Setting
{
public:
	ModelSetting(Messager & messager)
		: Setting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class BackendGlobalSetting : virtual public BackendSetting, virtual public GlobalSetting
{
	public:
		BackendGlobalSetting(Messager & messager)
			: Setting(messager)
			, BackendSetting(messager)
			, GlobalSetting(messager)
		{

		}
		SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
		static SETTING_CATEGORY category;
		std::string ToString() const { return ""; }
};

class BackendProjectSetting : virtual public BackendSetting, virtual public ProjectSetting
{
	public:
		BackendProjectSetting(Messager & messager)
			: Setting(messager)
			, BackendSetting(messager)
			, ProjectSetting(messager)
		{

		}
		std::string ToString() const { return ""; }
};

class BackendModelSetting : virtual public BackendSetting, virtual public ModelSetting
{
public:
	BackendModelSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class BackendInputSetting : virtual public BackendSetting, virtual public InputSetting
{
public:
	BackendInputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, InputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class BackendOutputSetting : virtual public BackendSetting, virtual public OutputSetting
{
public:
	BackendOutputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, OutputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class ProjectInputSetting : virtual public ProjectSetting, virtual public InputSetting
{
public:
	ProjectInputSetting(Messager & messager)
		: Setting(messager)
		, ProjectSetting(messager)
		, InputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class ProjectOutputSetting : virtual public ProjectSetting, virtual public OutputSetting
{
public:
	ProjectOutputSetting(Messager & messager)
		: Setting(messager)
		, ProjectSetting(messager)
		, OutputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

// Regarding the underscore: Disambiguate from InputModelSetting to avoid careless, hard-to-debug mistakes
class ModelInputSetting_ : virtual public ModelSetting, virtual public InputSetting
{
public:
	ModelInputSetting_(Messager & messager)
		: Setting(messager)
		, ModelSetting(messager)
		, InputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

// Regarding the underscore: Disambiguate from OutputModelSetting to avoid careless, hard-to-debug mistakes
class ModelOutputSetting_ : virtual public ModelSetting, virtual public OutputSetting
{
public:
	ModelOutputSetting_(Messager & messager)
		: Setting(messager)
		, ModelSetting(messager)
		, OutputSetting(messager)
	{

	}
	std::string ToString() const { return ""; }
};

class BackendProjectInputSetting : public BackendProjectSetting, public BackendInputSetting, public ProjectInputSetting
{
public:
	BackendProjectInputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, InputSetting(messager)
		, BackendProjectSetting(messager)
		, BackendInputSetting(messager)
		, ProjectInputSetting(messager)
	{

	}
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
	std::string ToString() const { return ""; }
};

class BackendProjectOutputSetting : public BackendProjectSetting, public BackendOutputSetting, public ProjectOutputSetting
{
public:
	BackendProjectOutputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, OutputSetting(messager)
		, BackendProjectSetting(messager)
		, BackendOutputSetting(messager)
		, ProjectOutputSetting(messager)
	{

	}
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
	std::string ToString() const { return ""; }
};

class BackendModelInputSetting : public BackendModelSetting, public BackendInputSetting, public ModelInputSetting_
{
public:
	BackendModelInputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, InputSetting(messager)
		, BackendModelSetting(messager)
		, BackendInputSetting(messager)
		, ModelInputSetting_(messager)
	{

	}
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
	std::string ToString() const { return ""; }
};

class BackendModelOutputSetting : public BackendModelSetting, public BackendOutputSetting, public ModelOutputSetting_
{
public:
	BackendModelOutputSetting(Messager & messager)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, OutputSetting(messager)
		, BackendModelSetting(messager)
		, BackendOutputSetting(messager)
		, ModelOutputSetting_(messager)
	{

	}
	SettingInfo GetSettingInfoFromEnum(Messager & messager, int const enum_val);
	static SETTING_CATEGORY category;
	std::string ToString() const { return ""; }
};

typedef BackendModelInputSetting InputModelSetting;
typedef BackendModelOutputSetting OutputModelSetting;

class StringSetting : virtual public Setting
{
	public:

		typedef std::string type;

		StringSetting(Messager & messager, std::string const & setting)
			: Setting(messager)
			, string_setting(setting)
		{}

		std::string getString() const { return string_setting; }
		std::string ToString() const { return getString(); }
		void * getDefaultValue()
		{
			return (void*)(&string_setting);
		}

	protected:
		std::string string_setting;
};

class Int32Setting : virtual public Setting
{
	public:

		typedef std::int32_t type;

		Int32Setting(Messager & messager, std::int32_t const & setting)
			: Setting(messager)
			, int32_setting(setting)
		{}

		std::int32_t getInt32() const { return int32_setting; }
		void * getDefaultValue()
		{
			return (void*)(&int32_setting);
		}
		std::string ToString() const { return boost::lexical_cast<std::string>(int32_setting); }

	protected:
		std::int32_t int32_setting;
};

class PathSetting : public StringSetting
{
public:

	typedef boost::filesystem::path type;

	PathSetting(Messager & messager, boost::filesystem::path const & setting)
		: Setting(messager)
		, StringSetting(messager, setting.string())
	{}

	boost::filesystem::path getPath() const { return path_setting; }
	std::string ToString() const { return getPath().string(); }

	void * getDefaultValue()
	{
		return (void*)(&path_setting);
	}

	virtual void DoSpecialParse(Messager & messager)
	{
		try
		{
			if (string_setting.empty())
			{
				return;
			}

			path_setting = string_setting;

		}
		catch(boost::filesystem::filesystem_error & e)
		{
			boost::format msg("Invalid path \"%1%\": %2%");
			msg % string_setting % e.what();
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
		}
	}

protected:
	boost::filesystem::path path_setting;
};

#endif
