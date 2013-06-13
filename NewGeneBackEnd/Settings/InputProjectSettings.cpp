#include "InputProjectSettings.h"
#include "InputProjectSettings_list.h"

#define S_PATH_TO_MODEL__1 PATH_TO_MODEL
#define S_PATH_TO_MODEL__2 SETTING_CLASS_BACKEND_PROJECT_INPUT_SETTING__PATH_TO_MODEL_SETTINGS
#define S_PATH_TO_MODEL__3 "PATH_TO_MODEL"
#define S_PATH_TO_MODEL__4 "L:\\daniel347x\\__DanExtras\\NewGene\\Projects\\Input\\TestInputModelSettings.xml"
#define S_PATH_TO_MODEL__5 InputProjectBackendSetting__path<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_MODEL>

std::string newgene_input_project_backend_root_node("newgene.project.input.backend.");

#define GET_INPUT_PROJECT_BACKEND_SETTING_INFO(INPUT_PROJECT_BACKEND_SETTING_ENUM, SETTING_INFO_ENUM, SETTING_STRING, SETTING_DEFAULT) \
case INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_BACKEND_SETTING_ENUM: \
	{ \
		return SettingInfo(SettingInfo::SETTING_INFO_ENUM, \
		static_cast<int>(INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_BACKEND_SETTING_ENUM), \
		newgene_input_project_backend_root_node + SETTING_STRING, \
		SETTING_DEFAULT); \
	} \
	break; \


#define INPUT_PROJECT_BACKEND_SET_MAP_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string); \
		_settings_map[static_cast<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND>(setting_info.enum_index)] = std::unique_ptr<BackendProjectInputSetting>(SettingFactory<SETTING_CLASS>()(messager, string_setting)); \
	} \
	break; \


#define INPUT_PROJECT_BACKEND_CLONE_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS * setting = dynamic_cast<SETTING_CLASS*>(current_setting); \
		return SettingFactory<SETTING_CLASS>()(messager, setting->getString()); \
	} \
	break; \


#define INPUT_PROJECT_BACKEND_NEW_SETTING__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		std::string string_setting = setting_info.default_val_string; \
		if (setting_value_void) \
		{ \
			string_setting = *((std::string *)(setting_value_void)); \
		} \
		return SettingFactory<SETTING_CLASS>()(messager, string_setting); \
	} \
	break; \


#define INPUT_PROJECT_BACKEND_SET_PTREE_ENTRY__STRING(SETTING_INFO_ENUM, SETTING_CLASS) \
case SettingInfo::SETTING_INFO_ENUM: \
	{ \
		SETTING_CLASS const * setting = static_cast<SETTING_CLASS const *>(_settings_map[which_setting].get()); \
		if (setting) \
		{ \
			pt.put(setting_info.text, setting->getString()); \
		} \
	} \
	break; \


SettingInfo BackendProjectInputSetting::GetSettingInfoFromEnum(Messager & messager, int const value__)
{

	INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const value_ = static_cast<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const>(value__);

#undef G_

	switch (value_)
	{

		#define G_(Y) S_PATH_TO_MODEL##Y
		GET_INPUT_PROJECT_BACKEND_SETTING_INFO ( G_(__1), G_(__2), G_(__3), G_(__4) )
		#undef G_

	default:
		{
			boost::format msg("Settings information is not available for INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND value %1%.  Using empty setting.");
			msg % value_;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
		}

	}

	return SettingInfo();

}

void InputProjectSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL##Y
		INPUT_PROJECT_BACKEND_SET_MAP_ENTRY__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown UI input project backend setting \"%1%\" (\"%2%\") being loaded.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}

BackendProjectInputSetting * InputProjectSettings::CloneSetting(Messager & messager, BackendProjectInputSetting * current_setting, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

			#define G_(Y) S_PATH_TO_MODEL##Y
			INPUT_PROJECT_BACKEND_CLONE_SETTING__STRING ( G_(__2), G_(__5) )
			#undef G_

		default:
			{
				boost::format msg("Unknown input project backend setting \"%1%\" (\"%2%\") being requested.");
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

BackendProjectInputSetting * InputProjectSettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL##Y
		INPUT_PROJECT_BACKEND_NEW_SETTING__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown input project backend setting \"%1%\" (\"%2%\") being updated.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

	return NULL;

}

void InputProjectSettings::SetPTreeEntry(Messager & messager, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND which_setting, boost::property_tree::ptree & pt)
{

	SettingsMap::const_iterator theSetting = _settings_map.find(which_setting);
	if (theSetting == _settings_map.cend())
	{
		return;
	}

	SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);

	switch (setting_info.setting_class)
	{

		#define G_(Y) S_PATH_TO_MODEL##Y
		INPUT_PROJECT_BACKEND_SET_PTREE_ENTRY__STRING ( G_(__2), G_(__5) )
		#undef G_

	default:
		{
			boost::format msg("Unknown input project backend setting \"%1%\" (\"%2%\") being set.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}
