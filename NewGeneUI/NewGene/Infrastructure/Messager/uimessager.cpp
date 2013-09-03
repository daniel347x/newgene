#include "uimessager.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

#include <QMetaObject>

bool UIMessager::ManagersInitialized = false;

UIMessager::UIMessager(QObject *parent) :
	QObject(parent)
  , do_not_handle_messages_on_destruction(false)
  , singleShotActive(false)
{
	if (ManagersInitialized)
	{
		QObject::connect(this, SIGNAL(PostStatus(STD_STRING, int, bool)), &statusManagerUI(), SLOT(ReceiveStatus(STD_STRING, int, bool)));
	}
}

UIMessager::~UIMessager()
{
	if (!do_not_handle_messages_on_destruction)
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
		if (_m->get()->_message_category & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
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
		//statusManagerUI().PostStatus(msg.c_str());
		emit PostStatus(msg, (int)(UIStatusManager::IMPORTANCE_STANDARD), false);
	}

	_messages.clear();

}

void UIMessager::InitializeSingleShot()
{
	singleShotActive = true;
}

void UIMessager::FinalizeSingleShot()
{
	displayStatusMessages();
	singleShotActive = false;
}

void UIMessager::EmitChangeMessage(DataChangeMessage & changes)
{
	if (changes.outp)
	{
		this->EmitOutputProjectChangeMessage(changes);
	}
	if (changes.inp)
	{
		this->EmitInputProjectChangeMessage(changes);
	}
}

UIMessagerInputProject::UIMessagerInputProject(UIInputProject * inp_, QObject * parent)
	: UIMessager(parent)
	, inp(inp_)
{
	if (ManagersInitialized)
	{
		connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
		connect(this, SIGNAL(QuestionMessageBox(STD_STRING)), get(), SLOT(QuestionMessageBox(STD_STRING)));
	}
}

UIMessagerOutputProject::UIMessagerOutputProject(UIOutputProject * outp_, QObject * parent)
	: UIMessager(parent)
	, outp(outp_)
{
	if (ManagersInitialized)
	{
		connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
		connect(this, SIGNAL(QuestionMessageBox(STD_STRING)), get(), SLOT(QuestionMessageBox(STD_STRING)));
	}
}

void UIMessagerInputProject::ShowMessageBox(std::string msg)
{
	emit DisplayMessageBox(msg);
}

void UIMessagerInputProject::ShowQuestionMessageBox(std::string msg)
{
	emit QuestionMessageBox(msg);
}

void UIMessagerInputProject::EmitInputProjectChangeMessage(DataChangeMessage & changes)
{
	get()->getQueueManager()->HandleChanges(changes);
}

void UIMessagerOutputProject::ShowMessageBox(std::string msg)
{
	emit DisplayMessageBox(msg);
}

void UIMessagerOutputProject::ShowQuestionMessageBox(std::string msg)
{
	QMetaObject::invokeMethod(get(), "QuestionMessageBox", Qt::BlockingQueuedConnection, Q_ARG( STD_STRING, msg ));
	//emit QuestionMessageBox(msg);
}

void UIMessagerOutputProject::EmitOutputProjectChangeMessage(DataChangeMessage & changes)
{
	get()->getQueueManager()->HandleChanges(changes);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}
