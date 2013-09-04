#include "uimessager.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

#include <QMetaObject>

bool UIMessager::ManagersInitialized = false;
int UIMessager::next_messager_id = 1;

UIMessager::UIMessager(QObject *parent) :
	QObject(parent)
  , do_not_handle_messages_on_destruction(false)
  , singleShotActive(false)
{
	current_messager_id = next_messager_id;
	++next_messager_id;
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
		if (get())
		{
			connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
			connect(this, SIGNAL(QuestionMessageBox(STD_STRING)), get(), SLOT(QuestionMessageBox(STD_STRING)));
			if (get()->mainWindowObject)
			{
				connect(this, SIGNAL(SignalStartProgressBar(int, std::int64_t const, std::int64_t const)), get()->mainWindowObject, SLOT(ReceiveSignalStartProgressBar(int, std::int64_t const, std::int64_t const)));
				connect(this, SIGNAL(SignalEndProgressBar(int)), get()->mainWindowObject, SLOT(ReceiveSignalStopProgressBar(int)));
				connect(this, SIGNAL(SignalUpdateProgressBarValue(int, std::int64_t const)), get()->mainWindowObject, SLOT(ReceiveSignalUpdateProgressBarValue(int, std::int64_t const)));
				connect(this, SIGNAL(SignalUpdateStatusBarText(int, STD_STRING const &)), get()->mainWindowObject, SLOT(ReceiveSignalUpdateStatusBarText(int, STD_STRING const)));
			}
		}
	}
}

UIMessagerOutputProject::UIMessagerOutputProject(UIOutputProject * outp_, QObject * parent)
	: UIMessager(parent)
	, outp(outp_)
{
	if (ManagersInitialized)
	{
		if (get())
		{
			connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
			connect(this, SIGNAL(QuestionMessageBox(STD_STRING)), get(), SLOT(QuestionMessageBox(STD_STRING)));
			if (get()->mainWindowObject)
			{
				connect(this, SIGNAL(SignalStartProgressBar(int, std::int64_t const, std::int64_t const)), get()->mainWindowObject, SLOT(ReceiveSignalStartProgressBar(int, std::int64_t const, std::int64_t const)));
				connect(this, SIGNAL(SignalEndProgressBar(int)), get()->mainWindowObject, SLOT(ReceiveSignalStopProgressBar(int)));
				connect(this, SIGNAL(SignalUpdateProgressBarValue(int, std::int64_t const)), get()->mainWindowObject, SLOT(ReceiveSignalUpdateProgressBarValue(int, std::int64_t const)));
				connect(this, SIGNAL(SignalUpdateStatusBarText(int, STD_STRING const &)), get()->mainWindowObject, SLOT(ReceiveSignalUpdateStatusBarText(int, STD_STRING const)));
			}
		}
	}
}

void UIMessagerInputProject::ShowMessageBox(std::string msg)
{
	emit DisplayMessageBox(msg);
}

bool UIMessagerInputProject::ShowQuestionMessageBox(std::string msg_title, std::string msg_text)
{
	bool yes = false;
	QMetaObject::invokeMethod(get(), "QuestionMessageBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG( bool, yes ), Q_ARG( STD_STRING, msg_title ), Q_ARG( STD_STRING, msg_text ));
	return yes;
}

void UIMessagerInputProject::StartProgressBar(std::int64_t const min_value, std::int64_t const max_value)
{
	emit SignalStartProgressBar(current_messager_id, min_value, max_value);
}

void UIMessagerInputProject::EndProgressBar()
{
	emit SignalEndProgressBar(current_messager_id);
}

void UIMessagerInputProject::UpdateProgressBarValue(std::int64_t const the_value)
{
	emit SignalUpdateProgressBarValue(current_messager_id, the_value);
}

void UIMessagerInputProject::UpdateStatusBarText(std::string const & the_text)
{
	emit SignalUpdateStatusBarText(current_messager_id, the_text);
}

void UIMessagerInputProject::EmitInputProjectChangeMessage(DataChangeMessage & changes)
{
	get()->getQueueManager()->HandleChanges(changes);
}

void UIMessagerOutputProject::ShowMessageBox(std::string msg)
{
	emit DisplayMessageBox(msg);
}

bool UIMessagerOutputProject::ShowQuestionMessageBox(std::string msg_title, std::string msg_text)
{
	bool yes = false;
	QMetaObject::invokeMethod(get(), "QuestionMessageBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG( bool, yes ), Q_ARG( STD_STRING, msg_title ), Q_ARG( STD_STRING, msg_text ));
	return yes;
}

void UIMessagerOutputProject::StartProgressBar(std::int64_t const min_value, std::int64_t const max_value)
{
	emit SignalStartProgressBar(current_messager_id, min_value, max_value);
}

void UIMessagerOutputProject::EndProgressBar()
{
	emit SignalEndProgressBar(current_messager_id);
}

void UIMessagerOutputProject::UpdateProgressBarValue(std::int64_t const the_value)
{
	emit SignalUpdateProgressBarValue(current_messager_id, the_value);
}

void UIMessagerOutputProject::UpdateStatusBarText(std::string const & the_text)
{
	emit SignalUpdateStatusBarText(current_messager_id, the_text);
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
