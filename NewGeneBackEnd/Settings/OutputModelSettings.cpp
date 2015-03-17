#include "Setting.h"
#include "OutputModelSettings.h"
#include "OutputModelSettings_list.h"

#define S_PATH_TO_MODEL_DATABASE__1 PATH_TO_MODEL_DATABASE
#define S_PATH_TO_MODEL_DATABASE__2 SETTING_CLASS_MODEL_OUTPUT_SETTING__PATH_TO_MODEL_DATABASE
#define S_PATH_TO_MODEL_DATABASE__3 "PATH_TO_MODEL_DATABASE"
#define S_PATH_TO_MODEL_DATABASE__4 "."
#define S_PATH_TO_MODEL_DATABASE__5 OutputModelSetting__path<OUTPUT_MODEL_SETTINGS_NAMESPACE::PATH_TO_MODEL_DATABASE>

std::string newgene_output_model_root_node("newgene.model.output.");

#define GET_OUTPUT_MODEL_SETTING_INFO(OUTPUT_MODEL_SETTING_ENUM, SETTING_INFO_ENUM, SETTING_STRING, SETTING_DEFAULT) \
	case OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTING_ENUM: \
	{ \
		return SettingInfo(SettingInfo::SETTING_INFO_ENUM, \
						   static_cast<int>(OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTING_ENUM), \
						   newgene_output_model_root_node + SETTING_STRING, \
						   SETTING_DEFAULT); \
	} \
	break; \


#define OUTPUT_MODEL_SET_MAP_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
	case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string); \
		_settings_map[static_cast<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS>(setting_info.enum_index)] = std::unique_ptr<OutputModelSetting>(SettingFactory<SETTING_CLASS>()(messager, string_setting)); \
	} \
	break; \


#define OUTPUT_MODEL_CLONE_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
	case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS * setting = dynamic_cast<SETTING_CLASS*>(current_setting); \
		return SettingFactory<SETTING_CLASS>()(messager, setting->getString()); \
	} \
	break; \


#define OUTPUT_MODEL_NEW_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
	case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = setting_info.default_val_string; \
		string_setting = setting_value_string; \
		return SettingFactory<SETTING_CLASS>()(messager, string_setting); \
	} \
	break; \


#define OUTPUT_MODEL_SET_PTREE_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
	case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS const * setting = static_cast<SETTING_CLASS const *>(_settings_map[which_setting].get()); \
		if (setting) \
		{ \
			pt.put(setting_info.text, setting->getString()); \
		} \
	} \
	break; \


	SettingInfo BackendModelOutputSetting::GetSettingInfoFromEnum(Messager & messager, int const value__)
	{

		OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const value_ = static_cast<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const>(value__);

#undef G_

		switch (value_)
		{

#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
				GET_OUTPUT_MODEL_SETTING_INFO(G_(__1), G_(__2), G_(__3), G_(__4))
#undef G_

			default:
				{
					boost::format msg("Settings information is not available for OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS value %1%.  Using empty setting.");
					msg % value_;
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
				}

		}

		return SettingInfo();

	}

	void OutputModelSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
	{

		switch (setting_info.setting_class)
		{

#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
				OUTPUT_MODEL_SET_MAP_ENTRY__STRING(G_(__2), G_(__5))
#undef G_

			default:
				{
					boost::format msg("Unknown UI output model setting \"%1%\" (\"%2%\") being loaded.");
					msg % setting_info.text % setting_info.setting_class;
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
				}
				break;

		}

	}

	OutputModelSetting * OutputModelSettings::CloneSetting(Messager & messager, OutputModelSetting * current_setting, SettingInfo & setting_info) const
	{

		try
		{

			switch (setting_info.setting_class)
			{

#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
					OUTPUT_MODEL_CLONE_SETTING__STRING(G_(__2), G_(__5))
#undef G_

				default:
					{
						boost::format msg("Unknown output model backend setting \"%1%\" (\"%2%\") being requested.");
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

	OutputModelSetting * OutputModelSettings::NewSetting(Messager & messager, SettingInfo & setting_info, std::string const & setting_value_string)
	{

		switch (setting_info.setting_class)
		{

#define G_(Y) S_PATH_TO_MODEL_DATABASE##Y
				OUTPUT_MODEL_NEW_SETTING__STRING(G_(__2), G_(__5))
#undef G_

			default:
				{
					boost::format msg("Unknown output model setting \"%1%\" (\"%2%\") being updated.");
					msg % setting_info.text % setting_info.setting_class;
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
				}
				break;

		}

		return NULL;

	}

	void OutputModelSettings::SetPTreeEntry(Messager & messager, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS which_setting, boost::property_tree::ptree & pt)
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
				OUTPUT_MODEL_SET_PTREE_ENTRY__STRING(G_(__2), G_(__5))
#undef G_

			default:
				{
					boost::format msg("Unknown output model backend setting \"%1%\" (\"%2%\") being set.");
					msg % setting_info.text % setting_info.setting_class;
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
				}
				break;

		}

	}
