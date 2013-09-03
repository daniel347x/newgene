#ifndef OUTPUTPROJECTSETTINGSLIST_H
#define OUTPUTPROJECTSETTINGSLIST_H

#include "OutputProjectSettings.h"
#include "SettingsManager.h"

template<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND SETTING>
class OutputProjectBackendSetting__string : public BackendProjectOutputSetting, public StringSetting, public SimpleAccessProjectModelSetting<OutputProjectBackendSetting__string<SETTING>, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, SETTING, SettingsManager, OutputProjectSettings>
{

public:

	OutputProjectBackendSetting__string(Messager & messager, std::string const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, OutputSetting(messager)
		, BackendProjectOutputSetting(messager)
		, StringSetting(messager, setting)
	{}

};

template<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND SETTING>
class OutputProjectBackendSetting__path : public BackendProjectOutputSetting, public PathSetting, public SimpleAccessProjectModelSetting<OutputProjectBackendSetting__path<SETTING>, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, SETTING, SettingsManager, OutputProjectSettings>
{

public:

	OutputProjectBackendSetting__path(Messager & messager, boost::filesystem::path const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ProjectSetting(messager)
		, OutputSetting(messager)
		, BackendProjectOutputSetting(messager)
		, PathSetting(messager, setting)
	{}

	std::string ToString() { return PathSetting::ToString(); }

};


/************************************************************************************************************************/
// PATH_TO_MODEL
/************************************************************************************************************************/
template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_MODEL_SETTINGS>
{
public:
	typedef OutputProjectBackendSetting__path<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_MODEL> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_MODEL_SETTINGS>::type OutputProjectPathToModel;


/************************************************************************************************************************/
// PATH_TO_KAD_OUTPUT_FILE
/************************************************************************************************************************/
template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_KAD_OUTPUT_FILE>
{
public:
	typedef OutputProjectBackendSetting__path<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_KAD_OUTPUT_FILE>::type OutputProjectPathToKadOutputFile;

#endif
