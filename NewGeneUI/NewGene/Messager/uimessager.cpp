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
	std::string msg;
	bool first = true;
	for (MessagesVector::const_iterator _m = _messages.cbegin(); _m != _messages.cend(); ++ _m)
	{
		if (_m->get()->_message_level & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{
			if (first == false)
			{
				msg += " ";
			}
			msg += _m->get()->_message_text;
			first = false;
		}
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
