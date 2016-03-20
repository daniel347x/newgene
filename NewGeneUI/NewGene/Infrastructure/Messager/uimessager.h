#ifndef UIMESSAGER_H
#define UIMESSAGER_H

#include "globals.h"
#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include "../../NewGeneBackEnd/Messager/Messager.h"

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

		enum Mode
		{
			NORMAL
			, KAD_GENERATION
		};

		UIMessager(QObject * parent = 0);
		~UIMessager();

		void setMode(Mode const mode_) { mode = mode_; }
		Mode getMode() const { return mode; }
		void displayStatusMessages();

		void InitializeSingleShot();
		void FinalizeSingleShot();

		void EmitChangeMessage(DataChangeMessage & widgetData);

		virtual void UpdateStatusBarText(std::string const &, void *);
		virtual void AppendKadStatusText(std::string const &, void *);

		virtual void EmitInputProjectChangeMessage(DataChangeMessage &) {}
		virtual void EmitOutputProjectChangeMessage(DataChangeMessage &) {}

		void ShowMessageBox(std::string, bool block = false);

		virtual boost::filesystem::path GetSystemDependentPath(MESSAGER_PATH_ENUM const &);

	signals:

		void PostStatus(STD_STRING, int, bool);
		void DisplayMessageBox(STD_STRING);
		void SignalStartProgressBar(int, STD_INT64 const, STD_INT64 const);
		void SignalEndProgressBar(int);
		void SignalUpdateProgressBarValue(int, STD_INT64 const);
		void SignalUpdateStatusBarText(int, STD_STRING const);
		void SignalAppendKadStatusText(int, STD_STRING const);
		void SignalSetPerformanceLabel(int, STD_STRING const);
		void SignalSetRunStatus(int, RUN_STATUS_ENUM const);

	public slots:

	public:

		bool do_not_handle_messages_on_destruction;
		static bool ManagersInitialized;

	protected:

		Mode mode;
		bool singleShotActive;
		int current_messager_id;
		static int next_messager_id;

	private:

		UIMessager(UIMessager const &) {}

};

class UIMessagerInputProject : public UIMessager
{

	public:

		UIMessagerInputProject(QObject * parent = 0);

		void set(UIInputProject * inp_);

		UIInputProject * get()
		{
			return inp;
		}

		void ShowMessageBox(std::string, bool block = false);
		bool ShowQuestionMessageBox(std::string, std::string); // title, question text
		virtual void StartProgressBar(std::int64_t const min_value, std::int64_t const max_value);
		virtual void EndProgressBar();
		virtual void UpdateProgressBarValue(std::int64_t const);
		virtual void UpdateStatusBarText(std::string const & status_bar_text, void *);
		virtual void pauseLists();

		void EmitInputProjectChangeMessage(DataChangeMessage & changes);

		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET & widgetData);
		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET & widgetData);
		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET & widgetData);
		void EmitSignalUpdateVGImportProgressBar(int, int, int, int);
		void EmitSignalUpdateDMUImportProgressBar(int, int, int, int);

	protected:

		UIInputProject * inp;

};

class UIMessagerOutputProject : public UIMessager
{

	public:

		UIMessagerOutputProject(QObject * parent = 0);

		void set(UIOutputProject * outp_);

		UIOutputProject * get()
		{
			return outp;
		}

		void ShowMessageBox(std::string, bool block = false);
		bool ShowQuestionMessageBox(std::string, std::string); // title, question text
		int  ShowOptionMessageBox(std::string, std::string, std::vector<WidgetInstanceIdentifier>); // title, question, option list

		void SetRunStatus(RUN_STATUS_ENUM const &);
		virtual void pauseLists();

		virtual void StartProgressBar(std::int64_t const min_value, std::int64_t const max_value);
		virtual void EndProgressBar();
		virtual void UpdateProgressBarValue(std::int64_t const);
		virtual void UpdateStatusBarText(std::string const & status_bar_text, void *);
		virtual void AppendKadStatusText(std::string const & kad_status_text, void *);
		virtual void SetPerformanceLabel(std::string const & kad_status_text);

		void EmitOutputProjectChangeMessage(DataChangeMessage & changes);

		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB & widgetData);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB & widgetData);

	protected:

		UIOutputProject * outp;

};

#endif // UIMESSAGER_H
