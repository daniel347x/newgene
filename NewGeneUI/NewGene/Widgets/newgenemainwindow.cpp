#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"
#include <QMessageBox>
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
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
#include <QCloseEvent>

NewGeneMainWindow::NewGeneMainWindow( QWidget * parent ) :
	QMainWindow( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_GENERAL) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneMainWindow ),
	messager(parent)
{

	NewGeneWidget::theMainWindow = this;

	connect(&messager, SIGNAL(DisplayMessageBox(STD_STRING)), this, SLOT(SignalMessageBox(STD_STRING)));

	try
	{

		// Instantiate Managers in main thread
		UIStatusManager::getManager(&messager);
		UIDocumentManager::getManager(&messager);
		UILoggingManager::getManager(&messager);
		UISettingsManager::getManager(&messager);
		UIModelManager::getManager(&messager);
		UIProjectManager::getManager(&messager);
		UIThreadManager::getManager(&messager);
		UITriggerManager::getManager(&messager);
		UIUIDataManager::getManager(&messager);
		UIUIActionManager::getManager(&messager);
		UIModelActionManager::getManager(&messager);

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
			std::string the_error = boost::diagnostic_information(e);
			boost::format msg("Error: %1%");
			msg % the_error.c_str();
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

	if (outp)
	{
		outp->UnregisterInterestInChanges(this);
	}
	if (inp)
	{
		inp->UnregisterInterestInChanges(this);
	}

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

	PrepareInputWidget();
	PrepareOutputWidget();

	projectManagerUI().LoadOpenProjects(this, this);

}

void NewGeneMainWindow::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		// Let the pane itself handle this.
		//ui->CreateOutputPane->LabelCreateOutput->text = "Create Output Dataset - ";
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		// Let the pane itself handle this.
		//ui->CreateOutputPane->LabelCreateOutput->text = "Create Output Dataset";
	}

}

void NewGeneMainWindow::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		// Let the pane itself handle this.
		//ui->ManageInputPane->LabelManageInput->text = "Manage Input Dataset - ";
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		// Let the pane itself handle this.
		//ui->ManageInputPane->LabelManageInput->text = "Manage Input Dataset";
	}

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

	status_bar_progress_bars[progress_bar_id]->hide();
	main_pane_progress_bars[progress_bar_id]->hide();

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
			if (new_value == 0)
			{
				status_bar_progress_bars[progress_bar_id]->hide();
			}
			else
			{
				status_bar_progress_bars[progress_bar_id]->show();
			}
		}
	}

}

void NewGeneMainWindow::ReceiveSignalUpdateStatusBarText(int /* progress_bar_id */, STD_STRING const status_bar_message)
{

	if (this->statusBar())
	{
		this->statusBar()->showMessage(QString(status_bar_message.c_str()));
	}

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

			emit SignalOpenInputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionClose_Current_Input_Dataset_triggered()
{

	emit SignalCloseCurrentInputDataset();

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

			emit SignalOpenOutputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionClose_Current_Output_Dataset_triggered()
{

	emit SignalCloseCurrentOutputDataset();

}

void NewGeneMainWindow::closeEvent(QCloseEvent *event)
{

	{
		std::lock_guard<std::recursive_mutex> guard(OutputModel::OutputGenerator::is_generating_output_mutex);
		if (OutputModel::OutputGenerator::is_generating_output)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString("Exit?"), QString("A K-ad output set is being generated.  Are you sure you wish to exit?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::Yes)
			{
				// No lock - not necessary for a boolean checked multiple times by back end and that will not cause an error if it is messed up in extraordinarily rare circumstances
				OutputModel::OutputGenerator::cancelled = true;
			}
			else
			{
				event->ignore();
				return;
			}
		}
	}

	{
		std::lock_guard<std::recursive_mutex> guard(Importer::is_performing_import_mutex);
		if (Importer::is_performing_import)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString("Exit?"), QString("An import is taking place.  Are you sure you wish to exit?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::Yes)
			{
				// No lock - not necessary for a boolean sequenced properly
				Importer:: is_performing_import = true;
			}
			else
			{
				event->ignore();
				return;
			}
		}
	}

	event->accept();

}

void NewGeneMainWindow::on_actionNew_Input_Dataset_triggered()
{

	UIMessager messager;
	OpenInputFilePath::instance folder_path = OpenInputFilePath::get(messager);
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the new input dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene input settings file (*.newgene.in.xml)");
	if (the_file.size())
	{
		if (!boost::filesystem::exists(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager, file_path.parent_path()));

			emit SignalOpenInputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionNew_Output_Dataset_triggered()
{

	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the new output dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene output settings file (*.newgene.out.xml)");
	if (the_file.size())
	{
		if (!boost::filesystem::exists(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager, file_path.parent_path()));

			emit SignalOpenOutputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionSave_Input_Dataset_As_triggered()
{

	UIMessager messager;
	OpenInputFilePath::instance folder_path = OpenInputFilePath::get(messager);
    QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the copied input dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene input settings file (*.newgene.in.xml)", nullptr, QFileDialog::DontConfirmOverwrite);
	if (the_file.size())
	{
		boost::filesystem::path file_path(the_file.toStdString());
		settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager, file_path.parent_path()));
		emit SignalSaveCurrentInputDatasetAs(the_file.toStdString(), this);
	}

}

void NewGeneMainWindow::on_actionSave_Output_Dataset_As_triggered()
{

	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
    QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the copied output dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene output settings file (*.newgene.out.xml)", nullptr, QFileDialog::DontConfirmOverwrite);
	if (the_file.size())
	{
		boost::filesystem::path file_path(the_file.toStdString());
		settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager, file_path.parent_path()));
		emit SignalSaveCurrentOutputDatasetAs(the_file.toStdString(), this);
	}

}

void NewGeneMainWindow::SetInputDatasetText()
{
	ui->ManageInputPane->SetInputDatasetText();
}

void NewGeneMainWindow::SetOutputDatasetText()
{
	ui->CreateOutputPane->SetOutputDatasetText();
}
