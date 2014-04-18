#include "newgeneapplication.h"

QEvent::Type QEVENT_NONE = QEvent::None;
QEvent::Type QEVENT_PROMPT_FOR_VG_REFRESH = QEvent::None;
QEvent::Type QEVENT_CLICK_VG_REFRESH = QEvent::None;
QEvent::Type QEVENT_PROMPT_FOR_DMU_REFRESH = QEvent::None;
QEvent::Type QEVENT_CLICK_DMU_REFRESH = QEvent::None;

NewGeneApplication::NewGeneApplication( int argc, char * argv[] ) :
	QApplication( argc, argv )
{

	qRegisterMetaType<STD_STRING>("STD_STRING");
	qRegisterMetaType<STD_INT64>("STD_INT64");
	qRegisterMetaType<STD_INT64>("STD_VECTOR_STRING");
	qRegisterMetaType<STD_INT64>("STD_VECTOR_WIDGETIDENTIFIER");

	qRegisterMetaType<WidgetChangeMessage>("WidgetChangeMessage");
	qRegisterMetaType<WidgetChangeMessages>("WidgetChangeMessages");

	qRegisterMetaType<WidgetInstanceIdentifier>("WidgetInstanceIdentifier");
	qRegisterMetaType<WidgetInstanceIdentifier_Bool_Pair>("WidgetInstanceIdentifier_Bool_Pair");

	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA>("WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX>("WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX");
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>("WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>("WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>("WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA>("WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA");
	qRegisterMetaType<WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET>("WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_TIMERANGE_REGION_WIDGET>("WidgetDataItemRequest_TIMERANGE_REGION_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_DATETIME_WIDGET>("WidgetDataItemRequest_DATETIME_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_GENERATE_OUTPUT_TAB>("WidgetDataItemRequest_GENERATE_OUTPUT_TAB");
	qRegisterMetaType<WidgetDataItemRequest_MANAGE_DMUS_WIDGET>("WidgetDataItemRequest_MANAGE_DMUS_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_MANAGE_UOAS_WIDGET>("WidgetDataItemRequest_MANAGE_UOAS_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_MANAGE_VGS_WIDGET>("WidgetDataItemRequest_MANAGE_VGS_WIDGET");

	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA>("WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_TOOLBOX>("WidgetDataItem_VARIABLE_GROUPS_TOOLBOX");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>("WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>("WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>("WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItem_KAD_SPIN_CONTROLS_AREA>("WidgetDataItem_KAD_SPIN_CONTROLS_AREA");
	qRegisterMetaType<WidgetDataItem_KAD_SPIN_CONTROL_WIDGET>("WidgetDataItem_KAD_SPIN_CONTROL_WIDGET");
	qRegisterMetaType<WidgetDataItem_TIMERANGE_REGION_WIDGET>("WidgetDataItem_TIMERANGE_REGION_WIDGET");
	qRegisterMetaType<WidgetDataItem_DATETIME_WIDGET>("WidgetDataItem_DATETIME_WIDGET");
	qRegisterMetaType<WidgetDataItem_GENERATE_OUTPUT_TAB>("WidgetDataItem_GENERATE_OUTPUT_TAB");
	qRegisterMetaType<WidgetDataItem_MANAGE_DMUS_WIDGET>("WidgetDataItem_MANAGE_DMUS_WIDGET");
	qRegisterMetaType<WidgetDataItem_MANAGE_UOAS_WIDGET>("WidgetDataItem_MANAGE_UOAS_WIDGET");
	qRegisterMetaType<WidgetDataItem_MANAGE_VGS_WIDGET>("WidgetDataItem_MANAGE_VGS_WIDGET");

	qRegisterMetaType<WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>("WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE>("WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE>("WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>("WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE>("WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE>("WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_GENERATE_OUTPUT>("WidgetActionItemRequest_ACTION_GENERATE_OUTPUT");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_ADD_DMU>("WidgetActionItemRequest_ACTION_ADD_DMU");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DELETE_DMU>("WidgetActionItemRequest_ACTION_DELETE_DMU");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS>("WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS>("WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE>("WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_ADD_UOA>("WidgetActionItemRequest_ACTION_ADD_UOA");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DELETE_UOA>("WidgetActionItemRequest_ACTION_DELETE_UOA");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_CREATE_VG>("WidgetActionItemRequest_ACTION_CREATE_VG");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DELETE_VG>("WidgetActionItemRequest_ACTION_DELETE_VG");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_REFRESH_VG>("WidgetActionItemRequest_ACTION_REFRESH_VG");

	QEVENT_NONE                            = static_cast<QEvent::Type>(QEvent::registerEventType(QEVENT_NONE_HINT));
	QEVENT_PROMPT_FOR_VG_REFRESH           = static_cast<QEvent::Type>(QEvent::registerEventType(QEVENT_PROMPT_FOR_VG_REFRESH_HINT));
	QEVENT_CLICK_VG_REFRESH                = static_cast<QEvent::Type>(QEvent::registerEventType(QEVENT_CLICK_VG_REFRESH_HINT));
	QEVENT_PROMPT_FOR_DMU_REFRESH          = static_cast<QEvent::Type>(QEvent::registerEventType(QEVENT_PROMPT_FOR_DMU_REFRESH_HINT));
	QEVENT_CLICK_DMU_REFRESH               = static_cast<QEvent::Type>(QEvent::registerEventType(QEVENT_CLICK_DMU_REFRESH_HINT));

}

bool NewGeneApplication::notify( QObject * receiver, QEvent * e )
{

	bool returnVal = false;

	try
	{
		returnVal = QApplication::notify( receiver, e );
	}
	catch ( boost::exception & e )
	{
		if ( std::string const * error_desc = boost::get_error_info<newgene_error_description>( e ) )
		{
			boost::format msg( error_desc->c_str() );
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
		}
		else
		{
			std::string the_error = boost::diagnostic_information(e);
			boost::format msg("Error: %1%");
			msg % the_error.c_str();
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
		}

		QApplication::exit( -1 );
		return false;
	}
	catch ( std::exception & e )
	{
		boost::format msg( "Exception thrown: %1%" );
		msg % e.what();
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		QApplication::exit( -1 );
		return false;
	}

	return true;

}

void NewGeneApplication::showErrorBox(std::string const theMsg)
{
	boost::format msg( theMsg );
	QMessageBox msgBox;
	msgBox.setText( msg.str().c_str() );
	msgBox.exec();
}
