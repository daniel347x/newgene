#include "uimessager.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "newgenemainwindow.h"
#include "newgenegenerateoutput.h"
#include "newgenetabwidget.h"

#include <vector>

#include <QMetaObject>

bool UIMessager::ManagersInitialized = false;
int UIMessager::next_messager_id = 1;

UIMessager::UIMessager(QObject * parent) :
	QObject(parent)
	, do_not_handle_messages_on_destruction(false)
	, mode(NORMAL)
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

boost::filesystem::path UIMessager::GetSystemDependentPath(MESSAGER_PATH_ENUM const & pathSpec)
{
	boost::filesystem::path thePath {};
	switch (pathSpec)
	{
		case MESSAGER_PATH_ENUM__IMPORT_LOG:
			thePath = settingsManagerUI().ObtainGlobalPath(QStandardPaths::DocumentsLocation, "", NewGeneFileNames::importLogFileName);
			break;
		default:
			break;
	}
	return thePath;
}

void UIMessager::ShowMessageBox(std::string msg, bool block)
{
	if (!block)
	{
		emit DisplayMessageBox(msg);
	}
}

void UIMessager::displayStatusMessages()
{

	std::vector<std::string> msgs;
	std::pair<std::set<std::string>::iterator, bool> insert_result;

	for (MessagesVector::const_iterator _m = _messages.cbegin(); _m != _messages.cend(); ++_m)
	{
		if (_m->get()->_message_category & MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE)
		{
			msgs.push_back(_m->get()->_message_text);
		}
	}

	std::string msg;
	bool first = true;

	for (std::vector<std::string>::const_iterator _s = msgs.cbegin(); _s != msgs.cend(); ++_s)
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

void UIMessager::UpdateStatusBarText(std::string const & the_text, void * generator)
{
	Messager::UpdateStatusBarText(the_text, generator);
}

void UIMessager::AppendKadStatusText(std::string const & kad_status_text, void * generator)
{
	Messager::AppendKadStatusText(kad_status_text, generator);
}

UIMessagerInputProject::UIMessagerInputProject(QObject * parent)
	: UIMessager(parent)
	, inp(nullptr)
{
}

void UIMessagerInputProject::set(UIInputProject * inp_)
{
	inp = inp_;

	if (ManagersInitialized)
	{
		if (get())
		{
			connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));

			if (get()->mainWindowObject)
			{
				try
				{
					NewGeneMainWindow * mainWindow = dynamic_cast<NewGeneMainWindow *>(get()->mainWindowObject);

					if (mainWindow)
					{
						connect(this, SIGNAL(SignalStartProgressBar(int, STD_INT64 const, STD_INT64 const)), mainWindow, SLOT(ReceiveSignalStartProgressBar(int, STD_INT64 const, STD_INT64 const)));
						connect(this, SIGNAL(SignalEndProgressBar(int)), mainWindow, SLOT(ReceiveSignalStopProgressBar(int)));
						connect(this, SIGNAL(SignalUpdateProgressBarValue(int, STD_INT64 const)), mainWindow, SLOT(ReceiveSignalUpdateProgressBarValue(int, STD_INT64 const)));
						connect(this, SIGNAL(SignalUpdateStatusBarText(int, STD_STRING const)), mainWindow, SLOT(ReceiveSignalUpdateStatusBarText(int, STD_STRING const)));
					}
				}
				catch (std::bad_cast &)
				{
				}
			}
		}
	}
}

UIMessagerOutputProject::UIMessagerOutputProject(QObject * parent)
	: UIMessager(parent)
	, outp(nullptr)
{
}

void UIMessagerOutputProject::set(UIOutputProject * outp_)
{
	outp = outp_;

	if (ManagersInitialized)
	{
		if (get())
		{
			connect(this, SIGNAL(DisplayMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));

			if (get()->mainWindowObject)
			{
				try
				{
					NewGeneMainWindow * mainWindow = dynamic_cast<NewGeneMainWindow *>(get()->mainWindowObject);

					if (mainWindow)
					{
						connect(this, SIGNAL(SignalStartProgressBar(int, STD_INT64 const, STD_INT64 const)), mainWindow, SLOT(ReceiveSignalStartProgressBar(int, STD_INT64 const, STD_INT64 const)));
						connect(this, SIGNAL(SignalEndProgressBar(int)), mainWindow, SLOT(ReceiveSignalStopProgressBar(int)));
						connect(this, SIGNAL(SignalUpdateProgressBarValue(int, STD_INT64 const)), mainWindow, SLOT(ReceiveSignalUpdateProgressBarValue(int, STD_INT64 const)));
						connect(this, SIGNAL(SignalUpdateStatusBarText(int, STD_STRING const)), mainWindow, SLOT(ReceiveSignalUpdateStatusBarText(int, STD_STRING const)));
						NewGeneGenerateOutput * outputPane = mainWindow->findChild<NewGeneGenerateOutput *>("widgetOutputPane");

						if (outputPane)
						{
							connect(this, SIGNAL(SignalAppendKadStatusText(int, STD_STRING const)), outputPane, SLOT(ReceiveSignalAppendKadStatusText(int, STD_STRING const)));
							connect(this, SIGNAL(SignalSetPerformanceLabel(int, STD_STRING const)), outputPane, SLOT(ReceiveSignalSetPerformanceLabel(int, STD_STRING const)));
							connect(this, SIGNAL(SignalSetRunStatus(int, RUN_STATUS_ENUM const)), outputPane, SLOT(ReceiveSignalSetRunStatus(int, RUN_STATUS_ENUM const)));
						}

						NewGeneTabWidget * tabWidget = mainWindow->findChild<NewGeneTabWidget *>("tabWidgetOutput");

						if (tabWidget)
						{
							connect(this, SIGNAL(SignalSetRunStatus(int, RUN_STATUS_ENUM const)), tabWidget, SLOT(ReceiveSignalSetRunStatus(int, RUN_STATUS_ENUM const)));
						}
					}
				}
				catch (std::bad_cast &)
				{
				}
			}
		}
	}
}

void UIMessagerInputProject::ShowMessageBox(std::string msg, bool block)
{
	if (block)
	{
		QMetaObject::invokeMethod(get(), "SignalMessageBox", Qt::BlockingQueuedConnection, Q_ARG(STD_STRING, msg));
	}
	else
	{
		UIMessager::ShowMessageBox(msg, block);
	}
}

bool UIMessagerInputProject::ShowQuestionMessageBox(std::string msg_title, std::string msg_text)
{
	bool yes = false;
	QMetaObject::invokeMethod(get(), "QuestionMessageBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, yes), Q_ARG(STD_STRING, msg_title), Q_ARG(STD_STRING, msg_text));
	return yes;
}

void UIMessagerInputProject::StartProgressBar(std::int64_t const min_value, std::int64_t const max_value)
{
	emit SignalStartProgressBar(current_messager_id, (STD_INT64)min_value, (STD_INT64)max_value);
}

void UIMessagerInputProject::EndProgressBar()
{
	emit SignalEndProgressBar(current_messager_id);
}

void UIMessagerInputProject::UpdateProgressBarValue(std::int64_t const the_value)
{
	emit SignalUpdateProgressBarValue(current_messager_id, (STD_INT64)the_value);
}

void UIMessagerInputProject::UpdateStatusBarText(std::string const & the_text, void *)
{
	emit SignalUpdateStatusBarText(current_messager_id, the_text);
}

void UIMessagerInputProject::EmitInputProjectChangeMessage(DataChangeMessage & changes)
{
	get()->getQueueManager()->HandleChanges(changes);
}

void UIMessagerOutputProject::ShowMessageBox(std::string msg, bool block)
{
	if (block)
	{
		QMetaObject::invokeMethod(get(), "SignalMessageBox", Qt::BlockingQueuedConnection, Q_ARG(STD_STRING, msg));
	}
	else
	{
		UIMessager::ShowMessageBox(msg, block);
	}
}

bool UIMessagerOutputProject::ShowQuestionMessageBox(std::string msg_title, std::string msg_text)
{
	bool yes = false;
	QMetaObject::invokeMethod(get(), "QuestionMessageBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, yes), Q_ARG(STD_STRING, msg_title), Q_ARG(STD_STRING, msg_text));
	return yes;
}

int UIMessagerOutputProject::ShowOptionMessageBox(std::string msg_title, std::string msg_question, std::vector<WidgetInstanceIdentifier> option_list)
{
	int selection = -1;
	QMetaObject::invokeMethod(get(), "OptionMessageBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, selection), Q_ARG(STD_STRING, msg_title), Q_ARG(STD_STRING, msg_question),
							  Q_ARG(STD_VECTOR_WIDGETINSTANCEIDENTIFIER, option_list));
	return selection;
}

void UIMessagerOutputProject::SetRunStatus(RUN_STATUS_ENUM const & runStatus)
{
	emit SignalSetRunStatus(current_messager_id, runStatus);
}

void UIMessagerOutputProject::StartProgressBar(std::int64_t const min_value, std::int64_t const max_value)
{
	emit SignalStartProgressBar(current_messager_id, (STD_INT64)min_value, (STD_INT64)max_value);
}

void UIMessagerOutputProject::EndProgressBar()
{
	emit SignalEndProgressBar(current_messager_id);
}

void UIMessagerOutputProject::UpdateProgressBarValue(std::int64_t const the_value)
{
	emit SignalUpdateProgressBarValue(current_messager_id, (STD_INT64)the_value);
}

void UIMessagerOutputProject::UpdateStatusBarText(std::string const & the_text, void * generator)
{
	Messager::UpdateStatusBarText(the_text, generator);
	emit SignalUpdateStatusBarText(current_messager_id, the_text);
}

void UIMessagerOutputProject::AppendKadStatusText(std::string const & the_text, void * generator)
{
	Messager::AppendKadStatusText(the_text, generator);
	emit SignalAppendKadStatusText(current_messager_id, the_text);
}

void UIMessagerOutputProject::SetPerformanceLabel(std::string const & the_text)
{
	emit SignalSetPerformanceLabel(current_messager_id, the_text);
}

void UIMessagerOutputProject::EmitOutputProjectChangeMessage(DataChangeMessage & changes)
{
	get()->getQueueManager()->HandleChanges(changes);
}


// Ouput project-related widgets

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

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET & widgetData)
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

void UIMessagerOutputProject::EmitOutputWidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB & widgetData)
{
	get()->getQueueManager()->EmitOutputWidgetDataRefresh(widgetData);
}


// Input project-related widgets

void UIMessagerInputProject::EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET & widgetData)
{
	get()->getQueueManager()->EmitInputWidgetDataRefresh(widgetData);
}

void UIMessagerInputProject::EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET & widgetData)
{
	get()->getQueueManager()->EmitInputWidgetDataRefresh(widgetData);
}

void UIMessagerInputProject::EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET & widgetData)
{
	get()->getQueueManager()->EmitInputWidgetDataRefresh(widgetData);
}

void UIMessagerInputProject::EmitSignalUpdateVGImportProgressBar(int mode_, int min_, int max_, int val_)
{
	get()->getQueueManager()->SignalUpdateVGImportProgressBar(mode_, min_, max_, val_);
}

void UIMessagerInputProject::EmitSignalUpdateDMUImportProgressBar(int mode_, int min_, int max_, int val_)
{
	get()->getQueueManager()->SignalUpdateDMUImportProgressBar(mode_, min_, max_, val_);
}
