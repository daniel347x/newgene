#include "Setting.h"
#include "InputModelSettings.h"
#include "InputModelSettings_list.h"

#define S_PATH_TO_MODEL_DATABASE__1 PATH_TO_MODEL_DATABASE
#define S_PATH_TO_MODEL_DATABASE__2 SETTING_CLASS_MODEL_INPUT_SETTING__PATH_TO_MODEL_DATABASE
#define S_PATH_TO_MODEL_DATABASE__3 "PATH_TO_MODEL_DATABASE"
#define S_PATH_TO_MODEL_DATABASE__4 "."
#define S_PATH_TO_MODEL_DATABASE__5 InputModelSetting__path<INPUT_MODEL_SETTINGS_NAMESPACE::PATH_TO_MODEL_DATABASE>

std::string newgene_input_model_root_node("newgene.model.input.");

#define GET_INPUT_MODEL_SETTING_INFO(INPUT_MODEL_SETTING_ENUM, SETTING_INFO_ENUM, SETTING_STRING, SETTING_DEFAULT) \
case INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTING_ENUM: \
	{ \
		return SettingInfo(SettingInfo::SETTING_INFO_ENUM, \
		static_cast<int>(INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTING_ENUM), \
		newgene_input_model_root_node + SETTING_STRING, \
		SETTING_DEFAULT); \
	} \
	break; \


#define INPUT_MODEL_SET_MAP_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string); \
		_settings_map[static_cast<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS>(setting_info.enum_index)] = std::unique_ptr<InputModelSetting>(SettingFactory<SETTING_CLASS>()(messager, string_setting)); \
	} \
	break; \


#define INPUT_MODEL_CLONE_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS * setting = dynamic_cast<SETTING_CLASS*>(current_setting); \
		return SettingFactory<SETTING_CLASS>()(messager, setting->getString()); \
	} \
	break; \


#define INPUT_MODEL_NEW_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = setting_info.default_val_string; \
		string_setting = setting_value_string; \
		return SettingFactory<SETTING_CLASS>()(messager, string_setting); \
	} \
	break; \


#define INPUT_MODEL_SET_PTREE_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS const * setting = static_cast<SETTING_CLASS const *>(_settings_map[which_setting].get()); \
		if (setting) \
		{ \
			pt.put(setting_info.text, setting->getString()); \
		} \
	} \
	break; \


SettingInfo BackendModelInputSetting::GetSettingInfoFromEnum(Messager & messager, int const value__)
{

	INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const value_ = static_cast<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const>(value__);

#undef G_

	switch (value_)
	{

		#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
		GET_INPUT_MODEL_SETTING_INFO ( G_(__1), G_(__2), G_(__3), G_(__4) )
		#undef G_

	default:
		{
			boost::format msg("Settings information is not available for INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS value %1%.  Using empty setting.");
			msg % value_;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
		}

	}

	return SettingInfo();

}

void InputModelSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
		INPUT_MODEL_SET_MAP_ENTRY__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown UI input model setting \"%1%\" (\"%2%\") being loaded.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}

InputModelSetting * InputModelSettings::CloneSetting(Messager & messager, InputModelSetting * current_setting, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

			#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
			INPUT_MODEL_CLONE_SETTING__STRING ( G_(__2), G_(__5) )
			#undef G_

		default:
			{
				boost::format msg("Unknown input model backend setting \"%1%\" (\"%2%\") being requested.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

		}

	}
	catch (std::bad_cast & e)
	{
		boost::format msg("Unable to retrieve setting \"%1%\" (\"%2%\"): %3%.");
		msg % setting_info.text % setting_info.setting_class % e.what();
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
	}

	return NULL;

}

InputModelSetting * InputModelSettings::NewSetting(Messager & messager, SettingInfo & setting_info, std::string const & setting_value_string)
{

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
		INPUT_MODEL_NEW_SETTING__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown input model setting \"%1%\" (\"%2%\") being updated.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

	return NULL;

}

void InputModelSettings::SetPTreeEntry(Messager & messager, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS which_setting, boost::property_tree::ptree & pt)
{

	SettingsMap::const_iterator theSetting = _settings_map.find(which_setting);
	if (theSetting == _settings_map.cend())
	{
		return;
	}

	SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
		INPUT_MODEL_SET_PTREE_ENTRY__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown input model backend setting \"%1%\" (\"%2%\") being set.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}
