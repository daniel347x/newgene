#ifndef UIMESSAGER_H
#define UIMESSAGER_H

#include "globals.h"
#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include "..\..\NewGeneBackEnd\Messager\Messager.h"

class UISettingsManager;
class UIProjectManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;

class UIInputProject;
class UIOutputProject;

class UIMessagerStatusMessage : public MessagerStatusMessage
{

	public:

		UIMessagerStatusMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerStatusMessage(TheMessage, TheMessageText)
		{

		}

};

class UIMessagerWarningMessage : public MessagerWarningMessage
{

	public:

		UIMessagerWarningMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerWarningMessage(TheMessage, TheMessageText)
		{

		}

};

class UIMessagerErrorMessage : public QObject, public MessagerErrorMessage
{

		Q_OBJECT

	public:

		UIMessagerErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerErrorMessage(TheMessage, TheMessageText)
		{
			QObject::connect(this, SIGNAL(sendErrorMessageToBeDisplayed(std::string const)), QCoreApplication::instance(), SLOT(showErrorBox(std::string const)));
			emit sendErrorMessageToBeDisplayed(TheMessageText);
		}

	signals:

		void sendErrorMessageToBeDisplayed(std::string const msg);

};

class UIMessager : public QObject, public Messager
{
		Q_OBJECT
	public:
		UIMessager(QObject *parent = 0);
		~UIMessager();

		void displayStatusMessages();

		void InitializeSingleShot();
		void FinalizeSingleShot();

		void EmitChangeMessage(DataChangeMessage & widgetData);

		virtual void EmitInputProjectChangeMessage(DataChangeMessage & changes) {}
		virtual void EmitOutputProjectChangeMessage(DataChangeMessage & changes) {}

	signals:
		void PostStatus(STD_STRING, int, bool);
		void DisplayMessageBox(STD_STRING);
		void QuestionMessageBox(STD_STRING);

	public slots:

	public:

		bool do_not_handle_messages_on_destruction;
		static bool ManagersInitialized;

	protected:

		bool singleShotActive;

	private:

		UIMessager(UIMessager const &) {}

};

class UIMessagerInputProject : public UIMessager
{
	public:

		UIMessagerInputProject(UIInputProject * inp_, QObject * parent = 0);

		UIInputProject * get()
		{
			return inp;
		}

		void ShowMessageBox(std::string);
		bool ShowQuestionMessageBox(std::string, std::string); // title, question text
		virtual void StartProgressBar(std::int64_t const min_value, std::int64_t const max_value);
		virtual void EndProgressBar();
		virtual void UpdateStatusBarValue(std::int64_t const);
		virtual void UpdateStatusBarText(std::string const & status_bar_text);

		void EmitInputProjectChangeMessage(DataChangeMessage & changes);

	protected:

		UIInputProject * inp;
};

class UIMessagerOutputProject : public UIMessager
{
	public:
		UIMessagerOutputProject(UIOutputProject * outp_, QObject * parent = 0);

		UIOutputProject * get()
		{
			return outp;
		}

		void ShowMessageBox(std::string);
		bool ShowQuestionMessageBox(std::string, std::string); // title, question text
		virtual void StartProgressBar(std::int64_t const min_value, std::int64_t const max_value);
		virtual void EndProgressBar();
		virtual void UpdateStatusBarValue(std::int64_t const);
		virtual void UpdateStatusBarText(std::string const & status_bar_text);

		void EmitOutputProjectChangeMessage(DataChangeMessage & changes);

		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB & widgetData);

	protected:

		UIOutputProject * outp;
};

#endif // UIMESSAGER_H
