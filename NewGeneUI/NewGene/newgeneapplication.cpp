#include "newgeneapplication.h"

NewGeneApplication::NewGeneApplication( int argc, char * argv[] ) :
	QApplication( argc, argv )
{
	qRegisterMetaType<STD_STRING>();

	qRegisterMetaType<QVector<int>>();

	qRegisterMetaType<WidgetChangeMessage>();
	qRegisterMetaType<WidgetChangeMessages>();

	qRegisterMetaType<WidgetInstanceIdentifier>();

	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA>();
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX>();
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>();

	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA>();
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUPS_TOOLBOX>();
	qRegisterMetaType<WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>();

	qRegisterMetaType<WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>();
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
