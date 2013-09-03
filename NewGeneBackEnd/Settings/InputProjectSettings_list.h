#ifndef INPUTPROJECTSETTINGSLIST_H
#define INPUTPROJECTSETTINGSLIST_H

#include "InputProjectSettings.h"
#include "SettingsManager.h"

template<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND SETTING>
class InputProjectBackendSetting__string : public BackendProjectInputSetting, public StringSetting, public SimpleAccessProjectModelSetting<InputProjectBackendSetting__string<SETTING>, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, SETTING, SettingsManager, InputProjectSettings>
{

public:

	InputProjectBackendSetting__string(Messager & messager, std::string const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, InputSetting(messager)
		, BackendProjectInputSetting(messager)
		, StringSetting(messager, setting)
	{}

};

template<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND SETTING>
class InputProjectBackendSetting__path : public BackendProjectInputSetting, public PathSetting, public SimpleAccessProjectModelSetting<InputProjectBackendSetting__path<SETTING>, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, SETTING, SettingsManager, InputProjectSettings>
{

public:

	InputProjectBackendSetting__path(Messager & messager, boost::filesystem::path const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, InputSetting(messager)
		, BackendProjectInputSetting(messager)
		, PathSetting(messager, setting)
	{}

};


/************************************************************************************************************************/
// PATH_TO_MODEL
/************************************************************************************************************************/
template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_INPUT_SETTING__PATH_TO_MODEL_SETTINGS>
{
public:
	typedef InputProjectBackendSetting__path<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_MODEL> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_INPUT_SETTING__PATH_TO_MODEL_SETTINGS>::type InputProjectPathToModel;


#endif
