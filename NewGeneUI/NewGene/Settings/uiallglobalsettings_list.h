#ifndef UIALLGLOBALSETTINGS_LIST_H
#define UIALLGLOBALSETTINGS_LIST_H

#include "uisettingsmanager.h"

class UIGlobalSetting_MRUList : public UIGlobalSetting, public StringSetting, public SimpleAccessSetting<UIGlobalSetting_MRUList, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST, UISettingsManager>
{

	public:

		UIGlobalSetting_MRUList(Messager & messager_, std::string const & setting)
			: UIGlobalSetting()
			, StringSetting(messager_, setting)
		{}

		virtual void DoSpecialParse(Messager & messager_)
		{
//			boost::format msg("Here is a message!");
//			messager.AppendMessage(new UIMessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
		}

};

#endif // UIALLGLOBALSETTINGS_LIST_H
