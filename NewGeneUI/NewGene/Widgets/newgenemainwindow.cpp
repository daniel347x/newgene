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
#include "uiallglobalsettings_list.h"
#ifndef Q_MOC_RUN
#	include <boost/thread/thread.hpp>
#	include <boost/filesystem.hpp>
#endif
#include <QFileDialog>

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

	projectManagerUI().EndAllLoops();

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

	projectManagerUI().LoadOpenProjects(this, this);

}

void NewGeneMainWindow::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}

void NewGeneMainWindow::ReceiveSignalStartProgressBar(int progress_bar_id, STD_INT64 const min_value, STD_INT64 const max_value)
{
	status_bar_progress_bars[progress_bar_id] = std::unique_ptr<QProgressBar>(new QProgressBar(this));
	main_pane_progress_bars[progress_bar_id] = std::unique_ptr<QProgressBar>(new QProgressBar(this));

	if (this->statusBar())
	{
		status_bar_progress_bars[progress_bar_id]->setMinimum(min_value);
		status_bar_progress_bars[progress_bar_id]->setMaximum(max_value);
		status_bar_progress_bars[progress_bar_id]->setValue(min_value);
		this->statusBar()->addPermanentWidget(status_bar_progress_bars[progress_bar_id].get());
	}
}

void NewGeneMainWindow::ReceiveSignalStopProgressBar(int progress_bar_id)
{
	if (this->statusBar())
	{
		if (status_bar_progress_bars.find(progress_bar_id) != status_bar_progress_bars.cend())
		{
			this->statusBar()->removeWidget(status_bar_progress_bars[progress_bar_id].get());
			status_bar_progress_bars.erase(progress_bar_id);
		}
	}

	main_pane_progress_bars.erase(progress_bar_id);
}

void NewGeneMainWindow::ReceiveSignalUpdateProgressBarValue(int progress_bar_id, STD_INT64 const new_value)
{
	if (this->statusBar())
	{
		if (status_bar_progress_bars.find(progress_bar_id) != status_bar_progress_bars.cend())
		{
			status_bar_progress_bars[progress_bar_id]->setValue(new_value);
		}
	}
}

void NewGeneMainWindow::ReceiveSignalUpdateStatusBarText(int progress_bar_id, STD_STRING const status_bar_message)
{
	if (this->statusBar())
	{
		this->statusBar()->showMessage(QString(status_bar_message.c_str()));
	}
}

void NewGeneMainWindow::on_actionClose_Current_Input_Dataset_triggered()
{
	emit SignalCloseCurrentInputDataset();
}

void NewGeneMainWindow::on_actionOpen_Input_Dataset_triggered()
{
	UIMessager messager;
	OpenInputFilePath::instance folder_path = OpenInputFilePath::get(messager);
	QString the_file = QFileDialog::getOpenFileName(this, "Choose input dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene input settings file (*.newgene.in.xml)");
	if (the_file.size())
	{
		if (boost::filesystem::exists(the_file.toStdString()) && !boost::filesystem::is_directory(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager, file_path.parent_path()));
		}
	}
}

void NewGeneMainWindow::on_actionOpen_Output_Dataset_triggered()
{
	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
	QString the_file = QFileDialog::getOpenFileName(this, "Choose output dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene output settings file (*.newgene.out.xml)");
	if (the_file.size())
	{
		if (boost::filesystem::exists(the_file.toStdString()) && !boost::filesystem::is_directory(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager, file_path.parent_path()));
		}
	}
}

void NewGeneMainWindow::on_actionClose_Current_Output_Dataset_triggered()
{
	emit SignalCloseCurrentOutputDataset();
}
