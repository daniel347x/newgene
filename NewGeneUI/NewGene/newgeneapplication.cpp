#include "newgeneapplication.h"

NewGeneApplication::NewGeneApplication( int argc, char * argv[] ) :
	QApplication( argc, argv )
{
	qRegisterMetaType<STD_STRING>("STD_STRING");
	qRegisterMetaType<STD_INT64>("STD_INT64");

	//qRegisterMetaType<QVector<int>>();

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
	qRegisterMetaType<WidgetDataItemRequest_DATETIME_WIDGET>("WidgetDataItemRequest_DATETIME_WIDGET");
	qRegisterMetaType<WidgetDataItemRequest_GENERATE_OUTPUT_TAB>("WidgetDataItemRequest_GENERATE_OUTPUT_TAB");

	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA>("WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_TOOLBOX>("WidgetDataItem_VARIABLE_GROUPS_TOOLBOX");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>("WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>("WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA");
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>("WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE");
	qRegisterMetaType<WidgetDataItem_KAD_SPIN_CONTROLS_AREA>("WidgetDataItem_KAD_SPIN_CONTROLS_AREA");
	qRegisterMetaType<WidgetDataItem_KAD_SPIN_CONTROL_WIDGET>("WidgetDataItem_KAD_SPIN_CONTROL_WIDGET");
	qRegisterMetaType<WidgetDataItem_DATETIME_WIDGET>("WidgetDataItem_DATETIME_WIDGET");
	qRegisterMetaType<WidgetDataItem_GENERATE_OUTPUT_TAB>("WidgetDataItem_GENERATE_OUTPUT_TAB");

	qRegisterMetaType<WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>("WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE>("WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE>("WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE");
	qRegisterMetaType<WidgetActionItemRequest_ACTION_GENERATE_OUTPUT>("WidgetActionItemRequest_ACTION_GENERATE_OUTPUT");
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
			boost::format msg( "Unknown exception thrown" );
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
