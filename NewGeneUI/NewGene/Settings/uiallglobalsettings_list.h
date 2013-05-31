#ifndef UIALLGLOBALSETTINGS_LIST_H
#define UIALLGLOBALSETTINGS_LIST_H

#include "uisettingsmanager.h"
#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif


class UIGlobalSetting_File_List_base : public UIGlobalSetting, public StringSetting
{

	public:

		UIGlobalSetting_File_List_base(Messager & messager_, std::string const & setting)
			: UIGlobalSetting()
			, StringSetting(messager_, setting)
		{}

		virtual void DoSpecialParse(Messager & messager_)
		{
//			std::vector<std::string> strings;
//			boost::split(strings, setting, boost::is_any_of(";"), boost::token_compress_on);
//			for_each(collection.begin(), collection.end(), [](Element& e)
//			{
//			   foo(e);
//			});
		}

		std::vector<boost::filesystem::path> files;

};

class UIGlobalSetting_MRU_List_base : public UIGlobalSetting_File_List_base
{

	public:

		UIGlobalSetting_MRU_List_base(Messager & messager_, std::string const & setting)
			: UIGlobalSetting_File_List_base(messager_, setting)
		{}

		virtual void DoSpecialParse(Messager & messager)
		{
			UIGlobalSetting_File_List_base::DoSpecialParse(messager);
		}

		//std::

};

class UIGlobalSetting_MRU_Input_List : public UIGlobalSetting_MRU_List_base, public SimpleAccessSetting<UIGlobalSetting_MRU_Input_List, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, GLOBAL_SETTINGS_UI_NAMESPACE::MRU_INPUT_LIST, UISettingsManager>
{

	public:

		UIGlobalSetting_MRU_Input_List(Messager & messager_, std::string const & setting)
			: UIGlobalSetting_MRU_List_base(messager_, setting)
		{}

		virtual void DoSpecialParse(Messager & messager)
		{
			UIGlobalSetting_MRU_List_base::DoSpecialParse(messager);
		}

		//std::

};

class UIGlobalSetting_MRU_Output_List : public UIGlobalSetting_MRU_List_base, public SimpleAccessSetting<UIGlobalSetting_MRU_Output_List, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, GLOBAL_SETTINGS_UI_NAMESPACE::MRU_OUTPUT_LIST, UISettingsManager>
{

	public:

		UIGlobalSetting_MRU_Output_List(Messager & messager_, std::string const & setting)
			: UIGlobalSetting_MRU_List_base(messager_, setting)
		{}

		virtual void DoSpecialParse(Messager & messager)
		{
			UIGlobalSetting_MRU_List_base::DoSpecialParse(messager);
//			boost::format msg("Here is a message!");
//			messager.AppendMessage(new UIMessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
		}

		//std::

};

#endif // UIALLGLOBALSETTINGS_LIST_H
