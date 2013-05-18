#include "uimessager.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

UIMessager::UIMessager(QObject *parent) :
    QObject(parent)
  , do_not_handle_messages_on_desctruction(false)
{
}

UIMessager::~UIMessager()
{
	if (!do_not_handle_messages_on_desctruction)
	{
		displayStatusMessages();
	}
}

void UIMessager::displayStatusMessages()
{
	std::set<std::string> msgs;
	std::pair<std::set<std::string>::iterator,bool> insert_result;
	for (MessagesVector::const_iterator _m = _messages.cbegin(); _m != _messages.cend(); ++ _m)
	{
		if (_m->get()->_message_level & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{
			insert_result = msgs.insert(_m->get()->_message_text);
		}
	}

	std::string msg;
	bool first = true;
	for (std::set<std::string>::const_iterator _s = msgs.cbegin(); _s != msgs.cend(); ++_s)
	{
		if (first == false)
		{
			msg += " ";
		}
		msg += *_s;
		first = false;
	}

	if (!first)
	{
		statusManager().PostStatus(msg.c_str());
	}
}

UISettingsManager & UIMessager::settingsManager()
{
	return UISettingsManager::getSettingsManager();
}

UILoggingManager & UIMessager::loggingManager()
{
	return UILoggingManager::getLoggingManager();
}

UIProjectManager & UIMessager::projectManager()
{
	return UIProjectManager::getProjectManager();
}

UIModelManager & UIMessager::modelManager()
{
	return UIModelManager::getModelManager();
}

UIDocumentManager & UIMessager::documentManager()
{
	return UIDocumentManager::getDocumentManager();
}

UIStatusManager & UIMessager::statusManager()
{
	return UIStatusManager::getStatusManager();
}

//void UIMessagerErrorMessage::sendErrorMessageToBeDisplayed(std::string const msg)
//{

//}
