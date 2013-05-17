#include "uimessager.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

UIMessager::UIMessager(QObject *parent) :
	QObject(parent)
{
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
				first = true;
			}
			msg += _m->get()->_message_text;
		}
	}
	statusManager().PostStatus(msg.c_str());
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
