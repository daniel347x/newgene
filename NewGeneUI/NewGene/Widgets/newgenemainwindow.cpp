#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"
#include <QMessageBox>
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uistatusmanager.h"
#include "uidocumentmanager.h"
#include "uisettingsmanager.h"
#include "uimodelmanager.h"
#include "uiloggingmanager.h"
#include "uithreadmanager.h"
#include "uitriggermanager.h"
#include "uiuidatamanager.h"
#include "uiuiactionmanager.h"
#include "uimodelactionmanager.h"
#include "uimodel.h"
#include "uiprojectmanager.h"
#include "uiinputproject.h"
#include "uioutputproject.h"
#ifndef Q_MOC_RUN
#	include <boost/thread/thread.hpp>
#endif

NewGeneMainWindow::NewGeneMainWindow( QWidget * parent ) :
	QMainWindow( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_GENERAL) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneMainWindow )
{

	NewGeneWidget::theMainWindow = this;

	try
	{

		// Instantiate Managers in main thread
		UIStatusManager::getManager();
		UIDocumentManager::getManager();
		UILoggingManager::getManager();
		UISettingsManager::getManager();
		UIModelManager::getManager();
		UIProjectManager::getManager();
		UIThreadManager::getManager();
		UITriggerManager::getManager();
		UIUIDataManager::getManager();
		UIUIActionManager::getManager();
		UIModelActionManager::getManager();

		UIMessager::ManagersInitialized = true;

		ui->setupUi( this );

		NewGeneTabWidget * pTWmain = findChild<NewGeneTabWidget *>( "tabWidgetMain" );

		if ( pTWmain )
		{
			pTWmain->NewGeneUIInitialize();
		}

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

		QCoreApplication::exit( -1 );

	}
	catch ( std::exception & e )
	{

		boost::format msg( "Exception thrown: %1%" );
		msg % e.what();
		QCoreApplication::exit( -1 );

	}

}

NewGeneMainWindow::~NewGeneMainWindow()
{

	// Manage global settings in main thread
	settingsManagerUI().globalSettings().EndLoopAndBackgroundPool();

	NewGeneWidget::theMainWindow = nullptr;
	delete ui;
}

void NewGeneMainWindow::changeEvent( QEvent * e )
{
	QMainWindow::changeEvent( e );

	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;

		default:
			break;
	}
}

void NewGeneMainWindow::doInitialize()
{

	UIMessager messager;

	// Load global settings in main thread
	settingsManagerUI().globalSettings().InitializeEventLoop(&settingsManagerUI().globalSettings());
	settingsManagerUI().globalSettings().WriteSettingsToFile(messager); // Write any defaults back to disk, along with values just read from disk

	projectManagerUI().LoadOpenProjects(this);

}

void NewGeneMainWindow::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}
