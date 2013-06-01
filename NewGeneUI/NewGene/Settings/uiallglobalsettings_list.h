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
				boost::filesystem::path model;

				try
				{
					if (files_.size() == 2)
					{
						settings = files_[0];
						model = files_[1];

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

#						ifdef Q_OS_WIN32
							if (boost::filesystem::is_directory(model.parent_path()) && boost::filesystem::windows_name(model.filename().string()))
#						else
							if (boost::filesystem::is_directory(model.parent_path()) && boost::filesystem::portable_posix_name(model.filename().string()))
#						endif
							{
								if (!boost::filesystem::exists(model))
								{
									std::ofstream _touch;
									_touch.open(model.string());
									if (_touch.is_open())
									{
										_touch.close();
									}
								}
							}

						if (boost::filesystem::is_regular_file(settings) && boost::filesystem::is_regular_file((model)))
						{
							this->files.push_back(std::make_pair<boost::filesystem::path, boost::filesystem::path>(settings.string(), model.string()));
						}
					}
				}
				catch(boost::filesystem::filesystem_error & e)
				{
					boost::format msg("Invalid path \"%1%\" or \"%2%\": %3%");
					msg % settings.string() % model.string() % e.what();
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
				}

			});
		}

		std::vector<std::pair<boost::filesystem::path, boost::filesystem::path> > files;

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

#endif // UIALLGLOBALSETTINGS_LIST_H
