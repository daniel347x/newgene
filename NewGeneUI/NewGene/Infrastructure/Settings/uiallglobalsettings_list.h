#ifndef UIALLGLOBALSETTINGS_LIST_H
#define UIALLGLOBALSETTINGS_LIST_H

#include "uisettingsmanager.h"
#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/regex.hpp>
#	include <boost/algorithm/string/regex.hpp>
#endif
#include <fstream>


template<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI SETTING>
class UIGlobalSetting_Projects_Files_List : public UIGlobalSetting, public StringSetting, public SimpleAccessSetting<UIGlobalSetting_Projects_Files_List<SETTING>, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, SETTING, UISettingsManager>
{

	public:

		UIGlobalSetting_Projects_Files_List(Messager & messager, std::string const & setting)
			: Setting(messager)
			, UISetting(messager)
			, GlobalSetting(messager)
			, UIGlobalSetting(messager)
			, StringSetting(messager, setting)
		{}

		virtual void DoSpecialParse(Messager & messager)
		{
			std::vector<std::string> projects;
			boost::split_regex(projects, string_setting, boost::regex(";;"));
			std::for_each(projects.begin(), projects.end(), [this, &messager](std::string & spath)
			{
				std::vector<std::string> files_;
				boost::split(files_, spath, boost::is_any_of(";"), boost::token_compress_on);

				boost::filesystem::path settings;

				try
				{
					if (files_.size() == 1)
					{
						settings = files_[0];

#						ifdef Q_OS_WIN32
							if (boost::filesystem::is_directory(settings.parent_path()) && boost::filesystem::windows_name(settings.filename().string()))
#						else
							if (boost::filesystem::is_directory(settings.parent_path()) && boost::filesystem::portable_posix_name(settings.filename().string()))
#						endif
							{
								if (!boost::filesystem::exists(settings))
								{
									std::ofstream _touch;
									_touch.open(settings.string());
									if (_touch.is_open())
									{
										_touch.close();
									}
								}
							}

						if (boost::filesystem::is_regular_file(settings))
						{
							this->files.push_back(settings.string());
						}
					}
				}
				catch(boost::filesystem::filesystem_error & e)
				{
					boost::format msg("Invalid path \"%1%\": %2%");
					msg % settings.string() % e.what();
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
				}

			});
		}

		std::vector<boost::filesystem::path> files;

		std::string ToString() const { return StringSetting::ToString(); }

};

template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_PROJECTS_LIST>
{
public:
	typedef UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::MRU_INPUT_PROJECTS_LIST> type;
};

template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_PROJECTS_LIST>
{
public:
	typedef UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::MRU_OUTPUT_PROJECTS_LIST> type;
};

template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_INPUT_PROJECTS_LIST>
{
public:
	typedef UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST> type;
};

template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_OUTPUT_PROJECTS_LIST>
{
public:
	typedef UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST> type;
};

typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_PROJECTS_LIST>::type InputMRUFilesList;
typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_PROJECTS_LIST>::type OutputMRUFilesList;
typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_INPUT_PROJECTS_LIST>::type InputProjectFilesList;
typedef SettingClassTypeTraits<SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_OUTPUT_PROJECTS_LIST>::type OutputProjectFilesList;

#endif // UIALLGLOBALSETTINGS_LIST_H
