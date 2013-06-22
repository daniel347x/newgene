#include "newgeneapplication.h"
#include "./Infrastructure/UIData/uiwidgetdatarefresh.h"


Q_DECLARE_METATYPE(STD_STRING);

Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA);
Q_DECLARE_METATYPE(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX);


NewGeneApplication::NewGeneApplication( int argc, char * argv[] ) :
	QApplication( argc, argv )
{
}

bool NewGeneApplication::notify( QObject * receiver, QEvent * e )
{

	bool returnVal = false;


	qRegisterMetaType<STD_STRING>();

	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA>();
	qRegisterMetaType<WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX>();


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
