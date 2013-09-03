#ifndef INPUTMODELSETTINGSLIST_H
#define INPUTMODELSETTINGSLIST_H

#include "InputModelSettings.h"
#include "SettingsManager.h"

template<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS SETTING>
class InputModelSetting__string : public InputModelSetting, public StringSetting, public SimpleAccessProjectModelSetting<InputModelSetting__string<SETTING>, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, SETTING, SettingsManager, InputModelSettings>
{

public:

	InputModelSetting__string(Messager & messager, std::string const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, InputSetting(messager)
		, BackendModelInputSetting(messager)
		, StringSetting(messager, setting)
	{}

};

template<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS SETTING>
class InputModelSetting__path : public InputModelSetting, public PathSetting, public SimpleAccessProjectModelSetting<InputModelSetting__path<SETTING>, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, SETTING, SettingsManager, InputModelSettings>
{

public:

	InputModelSetting__path(Messager & messager, boost::filesystem::path const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, InputSetting(messager)
		, BackendModelInputSetting(messager)
		, PathSetting(messager, setting)
	{}

};


/************************************************************************************************************************/
// PATH_TO_MODEL_DATABASE
/************************************************************************************************************************/
template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_MODEL_INPUT_SETTING__PATH_TO_MODEL_DATABASE>
{
public:
	typedef InputModelSetting__path<INPUT_MODEL_SETTINGS_NAMESPACE::PATH_TO_MODEL_DATABASE> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_MODEL_INPUT_SETTING__PATH_TO_MODEL_DATABASE>::type InputModelPathToDatabase;

#endif
