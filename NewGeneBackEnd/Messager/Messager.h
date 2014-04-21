#ifndef MESSAGER_H
#define MESSAGER_H

#include <vector>
#include <set>
#include <memory>
#include "../../NewGeneBackEnd/globals.h"
#include "../UIData/DataWidgets.h"
#include "../UIAction/ActionChanges.h"
#ifndef Q_MOC_RUN
#	include <boost/multiprecision/number.hpp>
#	include <boost/multiprecision/cpp_int.hpp>
#	include <boost/multiprecision/cpp_dec_float.hpp>
#	include <boost/pool/pool_alloc.hpp>
#endif 

struct newgene_cpp_int_tag 
{};

struct newgene_cpp_int_random_tag
{};

typedef boost::multiprecision::number<boost::multiprecision::cpp_int_backend<0, 0, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, boost::fast_pool_allocator<boost::multiprecision::limb_type, boost::default_user_allocator_malloc_free, newgene_cpp_int_tag, boost::details::pool::null_mutex>>> newgene_cpp_int;
typedef boost::multiprecision::number<boost::multiprecision::cpp_int_backend<0, 0, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, boost::fast_pool_allocator<boost::multiprecision::limb_type, boost::default_user_allocator_malloc_free, newgene_cpp_int_random_tag, boost::details::pool::null_mutex>>> newgene_random_cpp_int;

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

		virtual void ShowMessageBox(std::string, bool block = true) {}
		virtual bool ShowQuestionMessageBox(std::string, std::string) { return false; } // title, question text
		virtual int  ShowOptionMessageBox(std::string, std::string, std::vector<WidgetInstanceIdentifier>) { return 0; } // title, question, option list
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
		virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET &) {}
		virtual void EmitSignalUpdateVGImportProgressBar(int, int, int, int) {}
		virtual void EmitSignalUpdateDMUImportProgressBar(int, int, int, int) {}

	protected:

		MessagesVector _messages;

	protected:

		Messager()
		{
		}

};

// constexpr not available in current version of Visual Studio 2013, so use #define
#define cpp_float_digits 100

class ProgressBarMeter
{

	public:

		// constexpr not available in current version of Visual Studio 2013
		//static constexpr int cpp_float_digits = 10000;

		ProgressBarMeter(Messager & messager_, std::string const & progress_message, std::int64_t const max_value, bool const doNotStart = false)
			: messager(messager_)
			, msg{ progress_message }
			, progress_bar_max_value{ max_value }
			, update_every_how_often{0}
			, cpp_int_mode(false)
		{

			if (!doNotStart)
			{
				messager.StartProgressBar(0, progress_bar_max_value);
				messager.UpdateProgressBarValue(0);
				update_every_how_often = progress_bar_max_value / 100;
				if (update_every_how_often == 0) ++update_every_how_often;
			}

		}

		ProgressBarMeter(Messager & messager_, std::string const & progress_message, newgene_cpp_int const & max_value)
			: messager(messager_)
			, msg{ progress_message }
			, progress_bar_max_value{ 0 }
			, update_every_how_often{ 0 }
			, cpp_int_mode(true)
			, progress_bar_max_huge_value{ max_value.convert_to<boost::multiprecision::cpp_dec_float_100>() }
			, ratio_1000_const{ static_cast<boost::multiprecision::cpp_dec_float<0>>(0.0L) }
		{

			// set to 1000

			BOOST_ASSERT_MSG(sizeof(boost::multiprecision::cpp_dec_float_100) < 20000, "boost::multiprecision::cpp_dec_float<10000> is too large a data type to use!");

			messager.StartProgressBar(0, 100);
			messager.UpdateProgressBarValue(0);
			update_every_how_often = max_value.convert_to<std::int64_t>() / 100; // If we are using the progress bar in this way, it's because the user chose "output full samples" in which case the number of total rows will need to fit in a 64-bit integer
			if (update_every_how_often == 0) ++update_every_how_often;

		}

		~ProgressBarMeter()
		{

			messager.UpdateProgressBarValue(100);
			messager.SetPerformanceLabel("");

		}

		void StartProgressBar(std::int64_t const max_value)
		{

			// Used in case of delayed start
			progress_bar_max_value = max_value;

			// set to 1000

			messager.StartProgressBar(0, 100);
			messager.UpdateProgressBarValue(0);

		}

		void UpdateProgressBarValue(std::int64_t const current_value)
		{

			if (current_value % update_every_how_often == 0)
			{
				messager.UpdateProgressBarValue(current_value);
				messager.SetPerformanceLabel((msg % current_value % progress_bar_max_value).str().c_str());
			}

		}

		void UpdateProgressBarValue(newgene_cpp_int const & current_value)
		{

			// This version of the function is only called by the routine that actually generates output rows.
			// It's OK to do this calculation every time in the loop,
			// because we know that this loop is only going to be called once per branch,
			// *not* once per calculation of an output row.

			boost::multiprecision::cpp_dec_float_100 ratio(current_value);
			ratio /= progress_bar_max_huge_value;
			ratio *= 100.0L;
			int current_progress_bar_value = ratio.convert_to<int>();
			messager.UpdateProgressBarValue(current_progress_bar_value);
			messager.SetPerformanceLabel((msg % boost::lexical_cast<std::string>(current_value).c_str() % boost::lexical_cast<std::string>(progress_bar_max_value).c_str()).str().c_str());

		}

		std::int64_t update_every_how_often;

	protected:

		Messager & messager;
		boost::format msg;
		std::int64_t progress_bar_max_value;

		bool cpp_int_mode;
		boost::multiprecision::cpp_dec_float_100 progress_bar_max_huge_value;
		boost::multiprecision::cpp_dec_float_100 ratio_1000_const;

};

#endif
