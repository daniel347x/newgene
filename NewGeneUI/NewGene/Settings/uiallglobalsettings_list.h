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
			std::for_each(projects.begin(), projects.end(), [this](std::string & spath)
			{
				std::vector<std::string> files_;
				boost::split(files_, spath, boost::is_any_of(";"), boost::token_compress_on);

				if (files_.size() == 2)
				{
					std::string settings = files_[0];
					std::string model = files_[1];

					if (boost::filesystem::portable_posix_name(settings) && boost::filesystem::windows_name(settings))
					{
						if (!boost::filesystem::exists(settings))
						{
							std::fstream _touch(settings);
						}
					}

					if (boost::filesystem::portable_posix_name(model) && boost::filesystem::windows_name(model))
					{
						if (!boost::filesystem::exists(model))
						{
							std::fstream _touch(model);
						}
					}

					if (boost::filesystem::is_regular_file(settings) && boost::filesystem::is_regular_file((model)))
					{
						this->files.push_back(std::make_pair<boost::filesystem::path, boost::filesystem::path>(settings, model));
					}
				}

			});
		}

		std::vector<std::pair<boost::filesystem::path, boost::filesystem::path> > files;

};

#endif // UIALLGLOBALSETTINGS_LIST_H
