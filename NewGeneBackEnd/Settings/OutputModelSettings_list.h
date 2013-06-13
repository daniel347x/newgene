#ifndef OUTPUTMODELSETTINGSLIST_H
#define OUTPUTMODELSETTINGSLIST_H

#include "OutputModelSettings.h"
#include "SettingsManager.h"

template<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS SETTING>
class OutputModelSetting__string : public OutputModelSetting, public StringSetting, public SimpleAccessProjectModelSetting<OutputModelSetting__string<SETTING>, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, SETTING, SettingsManager, OutputModelSettings>
{

public:

	OutputModelSetting__string(Messager & messager, std::string const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, OutputSetting(messager)
		, BackendModelOutputSetting(messager)
		, StringSetting(messager, setting)
	{}

};

template<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS SETTING>
class OutputModelSetting__path : public OutputModelSetting, public PathSetting, public SimpleAccessProjectModelSetting<OutputModelSetting__path<SETTING>, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, SETTING, SettingsManager, OutputModelSettings>
{

public:

	OutputModelSetting__path(Messager & messager, boost::filesystem::path const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, ModelSetting(messager)
		, OutputSetting(messager)
		, BackendModelOutputSetting(messager)
		, PathSetting(messager, setting)
	{}

};

template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_MODEL_OUTPUT_SETTING__PATH_TO_MODEL_DATABASE>
{
public:
	typedef OutputModelSetting__path<OUTPUT_MODEL_SETTINGS_NAMESPACE::PATH_TO_MODEL_DATABASE> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_MODEL_OUTPUT_SETTING__PATH_TO_MODEL_DATABASE>::type OutputModelPathToDatabase;

#endif
