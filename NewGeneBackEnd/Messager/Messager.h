#ifndef MESSAGER_H
#define MESSAGER_H

#include <vector>
#include <set>
#include <memory>
#include "../../NewGeneBackEnd/globals.h"
#include "../UIData/DataWidgets.h"
#include "../UIAction/ActionChanges.h"

#ifndef Q_MOC_RUN
#endif

enum MESSAGER_MESSAGE_ENUM
{
	  MESSAGER_MESSAGE__FIRST

	, MESSAGER_MESSAGE__GENERAL_MESSAGE
	, MESSAGER_MESSAGE__GENERAL_WARNING
	, MESSAGER_MESSAGE__GENERAL_ERROR
	, MESSAGER_MESSAGE__GENERAL_ERROR_CATASTROPHIC
	, MESSAGER_MESSAGE__UNKNOWN_MESSAGE
	, MESSAGER_MESSAGE__UNKNOWN_WARNING
	, MESSAGER_MESSAGE__UNKNOWN_ERROR
	, MESSAGER_MESSAGE__UNKNOWN_ERROR_CATASTROPHIC
	, MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST
	, MESSAGER_MESSAGE__FILE_INVALID_FORMAT
	, MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT
	, MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE
	, MESSAGER_MESSAGE__SETTING_NOT_FOUND
	, MESSAGER_MESSAGE__SETTING_NOT_CREATED
	, MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE
	, MESSAGER_MESSAGE__PROJECT_PRESENT
	, MESSAGER_MESSAGE__PROJECT_IS_NULL
	, MESSAGER_MESSAGE__INPUT_MODELS_DO_NOT_MATCH
	, MESSAGER_MESSAGE__OUTPUT_MODELS_DO_NOT_MATCH
	, MESSAGER_MESSAGE__INPUT_MODEL_NOT_LOADED
	, MESSAGER_MESSAGE__OUTPUT_MODEL_NOT_LOADED
	, MESSAGER_MESSAGE__INPUT_MODEL_DATABASE_CANNOT_BE_CREATED
	, MESSAGER_MESSAGE__OUTPUT_MODEL_DATABASE_CANNOT_BE_CREATED

	, MESSAGER_MESSAGE__LAST
};

enum MESSAGER_MESSAGE_CATEGORY_ENUM
{
	  MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE = 0x00000001
	, MESSAGER_MESSAGE_CATEGORY__LOG_MESSAGE = 0x00000002
	, MESSAGER_MESSAGE_CATEGORY__WARNING = 0x00000004
	, MESSAGER_MESSAGE_CATEGORY__ERROR = 0x00000008
	, MESSAGER_MESSAGE_CATEGORY__ERROR_CATASTROPHIC = 0x00000010
};

class MessagerMessage
{

	public:

		virtual ~MessagerMessage() {}

		MessagerMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::int32_t const TheMessageCategory, std::string const & TheMessageText)
			: _message(TheMessage)
			, _message_category(TheMessageCategory)
			, _message_text(TheMessageText)
		{

		}

	public:

		MESSAGER_MESSAGE_ENUM _message;
		std::int32_t _message_category;
		std::string _message_text;

		friend class Messager;

};

class MessagerStatusMessage : public MessagerMessage
{

	public:

		MessagerStatusMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
		{

		}

};

class MessagerWarningMessage : public MessagerMessage
{

	public:

		MessagerWarningMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__WARNING | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
		{

		}

};

class MessagerErrorMessage : public MessagerMessage
{

	public:

		MessagerErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__ERROR | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
		{

		}

};

class MessagerCatastrophicErrorMessage : public MessagerMessage
{

	public:

		MessagerCatastrophicErrorMessage(MESSAGER_MESSAGE_ENUM const TheMessage, std::string const & TheMessageText)
			: MessagerMessage(TheMessage, MESSAGER_MESSAGE_CATEGORY__ERROR_CATASTROPHIC | MESSAGER_MESSAGE_CATEGORY__STATUS_MESSAGE, TheMessageText)
		{

		}

};

class Messager
{

	public:

		typedef std::vector<std::unique_ptr<MessagerMessage> > MessagesVector;

		void AppendMessage(MessagerMessage * message);

		bool HasStatus();
		bool RequiresLogging();
		bool IsWarning();
		bool IsError();
		bool IsErrorCatastrophic();

		// ***************************************************************************************************************//
		// The following functions are all overridden in [Input|Output]-specific Messager class that derives from this class
		// ***************************************************************************************************************//

        virtual void EmitChangeMessage(DataChangeMessage &) {}

		virtual void ShowMessageBox(std::string) {}
		virtual bool ShowQuestionMessageBox(std::string, std::string) { return false; } // title, question text
        virtual void StartProgressBar(std::int64_t const, std::int64_t const) {}
		virtual void EndProgressBar() {}
		virtual void UpdateProgressBarValue(std::int64_t const) {}
		virtual void UpdateStatusBarText(std::string const &) {}
		virtual void UpdateStatusBarText(std::string const &, void *);
		virtual void AppendKadStatusText(std::string const &) {}
		virtual void AppendKadStatusText(std::string const &, void *);
		virtual void SetPerformanceLabel(std::string const &) {}

        // Output
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET &) {}
        virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB &) {}

        // Input
        virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET &) {}
		virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET &) {}

	protected:

		MessagesVector _messages;

	protected:

		Messager() {} // Must only instantiate UIMessager from UI layer

};

#endif
