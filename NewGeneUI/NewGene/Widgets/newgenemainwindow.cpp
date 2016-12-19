#ifndef Q_MOC_RUN
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>
#endif
#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"
#include <QLabel>
#include <QMessageBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
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
#include <QFileDialog>
#include <QCloseEvent>
#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"
#include "newgeneselectvariables.h"
#include "ui_newgeneselectvariables.h"
#include "newgenevariables.h"
#include "ui_newgenevariables.h"
#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"
#include "newgenevariablesummary.h"
#include "ui_newgenevariablesummary.h"
#include "kadcolumnselectionbox.h"
#include "ui_kadcolumnselectionbox.h"
#include "newgenevariablesummaryscrollarea.h"
#include "ui_newgenevariablesummaryscrollarea.h"
#include "splash.h"
#include "uiinputmodel.h"
#include "uioutputmodel.h"
#include "vacuumdialog.h"
#include "kadwidgetsscrollarea.h"
#include <QLabel>
#include <QPixmap>
#include <QMovie>
#include <Qbitmap>
#include <QMessageBox>

NewGeneMainWindow::NewGeneMainWindow(QWidget * parent) :
	QMainWindow(parent),
	NewGeneWidget(WidgetCreationInfo(this,
	                                 WIDGET_NATURE_GENERAL)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneMainWindow),
	messager(parent),
	theSplash(nullptr),
	newInputDataset(false),
	newOutputDataset(false)
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

		ui->setupUi(this);

		QLabel * labelLoadingSpinner { findChild<QLabel *>("labelLoadingSpinner") };

		if (labelLoadingSpinner)
		{
			QMovie * movieLoadingSpinner = new QMovie(":/bluespinner.gif");
			labelLoadingSpinner->setMask((new QPixmap(":/bluespinner.gif"))->mask());
			labelLoadingSpinner->setAttribute(Qt::WA_TranslucentBackground);
			labelLoadingSpinner->setMovie(movieLoadingSpinner);
			movieLoadingSpinner->start();
		}

		NewGeneTabWidget * pTWmain = findChild<NewGeneTabWidget *>("tabWidgetMain");

		if (pTWmain)
		{
			pTWmain->NewGeneUIInitialize();
		}

	}
	catch (boost::exception & e)
	{

		if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
		{
			boost::format msg(error_desc->c_str());
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
		}
		else
		{
			std::string the_error = boost::diagnostic_information(e);
			boost::format msg("Error: %1%");
			msg % the_error.c_str();
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
		}

		QCoreApplication::exit(-1);

	}
	catch (std::exception & e)
	{

		boost::format msg("Exception thrown: %1%");
		msg % e.what();
		QCoreApplication::exit(-1);

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

void NewGeneMainWindow::changeEvent(QEvent * e)
{
	QMainWindow::changeEvent(e);

	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}
}

void NewGeneMainWindow::Run()
{

	if (theSplash)
	{
		bool opened_as_about_box = theSplash->opened_as_about_box;

		show();
		theSplash->done(0);
		theSplash = nullptr; // It deletes itself due to the WA_DeleteOnClose window attribute

		if (!opened_as_about_box)
		{
			UIMessager messager;

			// Load global settings in main thread
			settingsManagerUI().globalSettings().InitializeEventLoop(&settingsManagerUI().globalSettings());
			settingsManagerUI().globalSettings().WriteSettingsToFile(messager); // Write any defaults back to disk, along with values just read from disk

			PrepareInputWidget();
			PrepareOutputWidget();

			PrepareGlobalConnections();

			projectManagerUI().LoadOpenProjects(this, this);
		}
	}
}

void NewGeneMainWindow::doInitialize()
{

	QTimer::singleShot(500, this, SLOT(displaySplashOpening()));

}

void NewGeneMainWindow::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		SetTitle();
		ui->actionClose_Current_Output_Dataset->setEnabled(true);
		ui->actionDisplay_output_dataset_path->setEnabled(true);
		ui->actionSave_Output_Dataset_As->setEnabled(true);
		ui->actionVacuum_output_database->setEnabled(true);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		SetTitle();
		ui->actionClose_Current_Output_Dataset->setEnabled(false);
		ui->actionDisplay_output_dataset_path->setEnabled(false);
		ui->actionSave_Output_Dataset_As->setEnabled(false);
		ui->actionVacuum_output_database->setEnabled(false);
	}

}

void NewGeneMainWindow::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		SetTitle();
		ui->actionClose_Current_Input_Dataset->setEnabled(true);
		ui->actionDisplay_input_dataset_path->setEnabled(true);
		ui->actionSave_Input_Dataset_As->setEnabled(true);
		ui->actionVacuum_input_database->setEnabled(true);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		SetTitle();
		ui->actionClose_Current_Input_Dataset->setEnabled(false);
		ui->actionDisplay_input_dataset_path->setEnabled(false);
		ui->actionSave_Input_Dataset_As->setEnabled(false);
		ui->actionVacuum_input_database->setEnabled(false);
	}

}

void NewGeneMainWindow::showLoading(bool const loading)
{
	try
	{
		KadWidgetsScrollArea * scrollArea { findChild<KadWidgetsScrollArea *>("scrollAreaWidgetContents") };
		if (scrollArea == nullptr)
		{
			return;
		}
		scrollArea->ShowLoading(loading);
	}
	catch (std::bad_cast &)
	{
	}
}

void NewGeneMainWindow::SignalMessageBox(STD_STRING msg)
{

	QMessageBox msgBox;
	msgBox.setText(msg.c_str());
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
	QString the_file = QFileDialog::getOpenFileName(this, "Choose input dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene input settings file (*.xml)");

	if (the_file.size())
	{
		if (boost::filesystem::exists(the_file.toStdString()) && !boost::filesystem::is_directory(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager,
			        file_path.parent_path()));

			emit SignalOpenInputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionClose_Current_Input_Dataset_triggered()
{

	if (inp == nullptr)
	{
		return;
	}

	emit SignalCloseCurrentInputDataset();

}

void NewGeneMainWindow::on_actionOpen_Output_Dataset_triggered()
{

	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
	QString the_file = QFileDialog::getOpenFileName(this, "Choose output dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene output settings file (*.xml)");

	if (the_file.size())
	{
		if (boost::filesystem::exists(the_file.toStdString()) && !boost::filesystem::is_directory(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager,
			        file_path.parent_path()));

			emit SignalOpenOutputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionClose_Current_Output_Dataset_triggered()
{

	if (outp == nullptr)
	{
		return;
	}

	emit SignalCloseCurrentOutputDataset();

}

void NewGeneMainWindow::closeEvent(QCloseEvent * event)
{

	{
		std::lock_guard<std::recursive_mutex> guard(OutputModel::OutputGenerator::is_generating_output_mutex);

		if (OutputModel::OutputGenerator::is_generating_output)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString("Exit?"), QString("A k-ad output set is being generated.  Are you sure you wish to exit?"),
			                              QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

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
			reply = QMessageBox::question(nullptr, QString("Exit?"), QString("An import is taking place.  Are you sure you wish to exit?"),
			                              QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

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
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the new input dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene input settings file (*.xml)");

	if (the_file.size())
	{
		if (!boost::filesystem::exists(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager,
			        file_path.parent_path()));

			newInputDataset = true; // kluge; see header
			emit SignalOpenInputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionNew_Output_Dataset_triggered()
{

	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the new output dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene output settings file (*.xml)");

	if (the_file.size())
	{
		if (!boost::filesystem::exists(the_file.toStdString()))
		{
			boost::filesystem::path file_path(the_file.toStdString());
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager,
			        file_path.parent_path()));

			newOutputDataset = true; // kluge; see header
			emit SignalOpenOutputDataset(the_file.toStdString(), this);
		}
	}

}

void NewGeneMainWindow::on_actionSave_Input_Dataset_As_triggered()
{

	if (inp == nullptr)
	{
		QMessageBox::information(nullptr, "NewGene", "No input project is currently open.");
		return;
	}

	UIMessager messager;
	OpenInputFilePath::instance folder_path = OpenInputFilePath::get(messager);
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the copied input dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene input settings file (*.xml)", nullptr, QFileDialog::DontConfirmOverwrite);

	if (the_file.size())
	{
		boost::filesystem::path file_path(the_file.toStdString());
		settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager,
		        file_path.parent_path()));
		emit SignalSaveCurrentInputDatasetAs(the_file.toStdString(), this);
	}

}

void NewGeneMainWindow::on_actionSave_Output_Dataset_As_triggered()
{

	if (outp == nullptr)
	{
		QMessageBox::information(nullptr, "NewGene", "No output project is currently open.");
		return;
	}

	UIMessager messager;
	OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager);
	QString the_file = QFileDialog::getSaveFileName(this, "Select a name and location for the copied output dataset", folder_path ? folder_path->getPath().string().c_str() : "",
	                   "NewGene output settings file (*.xml)", nullptr, QFileDialog::DontConfirmOverwrite);

	if (the_file.size())
	{
		boost::filesystem::path file_path(the_file.toStdString());
		settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager,
		        file_path.parent_path()));
		emit SignalSaveCurrentOutputDatasetAs(the_file.toStdString(), this);
	}

}

void NewGeneMainWindow::on_actionDisplay_input_dataset_path_triggered()
{
	if (inp == nullptr)
	{
		QMessageBox::information(nullptr, "NewGene", "No input project is currently open.");
		return;
	}

	std::string input = inp->backend().projectSettings().GetSettingsPath().string();

	if (!input.empty())
	{
		// From http://stackoverflow.com/a/17512615/368896
		QDialog dialog(this);
		dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
		QFormLayout form(&dialog);
		form.addRow(new QLabel("Input dataset path:"));
		QLineEdit * lineEdit = new QLineEdit(&dialog);
		setLineEditWidth(lineEdit);
		lineEdit->setText(input.c_str());
		lineEdit->setReadOnly(true);
		lineEdit->setMinimumWidth(600);
		form.addRow(lineEdit);
		QDialogButtonBox buttonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
		form.addRow(&buttonBox);
		QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
		{
			dialog.close();
		});
		dialog.exec();
	}
}

void NewGeneMainWindow::on_actionDisplay_output_dataset_path_triggered()
{
	if (outp == nullptr)
	{
		QMessageBox::information(nullptr, "NewGene", "No output project is currently open.");
		return;
	}

	std::string output = outp->backend().projectSettings().GetSettingsPath().string();

	if (!output.empty())
	{
		// From http://stackoverflow.com/a/17512615/368896
		QDialog dialog(this);
		dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
		QFormLayout form(&dialog);
		form.addRow(new QLabel("Output dataset path:"));
		QLineEdit * lineEdit = new QLineEdit(&dialog);
		setLineEditWidth(lineEdit);
		lineEdit->setText(output.c_str());
		lineEdit->setReadOnly(true);
		lineEdit->setMinimumWidth(600);
		form.addRow(lineEdit);
		QDialogButtonBox buttonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
		form.addRow(&buttonBox);
		QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
		{
			dialog.close();
		});
		dialog.exec();
	}
}

void NewGeneMainWindow::SetTitle()
{

	boost::filesystem::path output_path;
	boost::filesystem::path input_path;

	if (outp)
	{
		output_path = outp->backend().projectSettings().GetSettingsPath();
	}

	if (inp)
	{
		input_path = inp->backend().projectSettings().GetSettingsPath();
	}

	std::string output {output_path.string()};
	std::string input {input_path.string()};

	std::string title {"NewGene"};
	std::string status {};

	if (!input.empty())
	{
		title += " [Input: ";
		title += input_path.filename().string();
		title += "]";
	}

	if (!output.empty())
	{
		title += " [Output: ";
		title += output_path.filename().string();
		title += "]";
	}

	if (input.empty() && !output.empty())
	{
		status = "Please open or create an input dataset";
	}
	else if (!input.empty() && output.empty())
	{
		status = "Please open or create an output dataset";
	}
	else if (input.empty() && output.empty())
	{
		status = "Please open or create an input and output dataset";
	}

	this->setWindowTitle(title.c_str());

	ui->statusBar->showMessage(status.c_str());

}

void NewGeneMainWindow::PrepareGlobalConnections()
{
	{
		QWidget * source =
			ui->CreateOutputPane->ui->widgetSelectVariablesPane->ui->CreateOutputDataset_VariablesSplitter_VariableSelections->ui->scrollAreaWidgetContents->ui->toolbox->newgeneToolBox;
		QWidget * target = ui->CreateOutputPane->ui->widgetSelectVariablesPane->ui->CreateOutputDataset_VariablesSplitter_VariableSummary->ui->scrollAreaWidgetContents;
		connect(source, SIGNAL(DoTabChange(WidgetInstanceIdentifier)), target, SLOT(DoTabChange(WidgetInstanceIdentifier)));
	}

	{
		QWidget * source =
			ui->CreateOutputPane->ui->widgetSelectVariablesPane->ui->CreateOutputDataset_VariablesSplitter_VariableSelections->ui->scrollAreaWidgetContents->ui->toolbox->newgeneToolBox;
		QWidget * target = ui->CreateOutputPane->ui->widgetSelectVariablesPane->ui->frameKAdSelectionArea->ui->scrollAreaWidgetContents;
		connect(source, SIGNAL(DoTabChange(WidgetInstanceIdentifier)), target, SLOT(DoTabChange(WidgetInstanceIdentifier)));
	}
}

void NewGeneMainWindow::doDisable()
{
	ui->centralWidget->setEnabled(false);
	ui->menuBar->setEnabled(false);
	ui->statusBar->setEnabled(false);
}

void NewGeneMainWindow::doEnable()
{
	ui->centralWidget->setEnabled(true);
	ui->menuBar->setEnabled(true);
	ui->statusBar->setEnabled(true);
	activateWindow();
}

void NewGeneMainWindow::on_actionAbout_NewGene_triggered()
{
	QTimer::singleShot(50, this, SLOT(displaySplashAbout()));
}

void NewGeneMainWindow::displaySplashOpening()
{
	displaySplash(false);
}

void NewGeneMainWindow::displaySplashAbout()
{
	displaySplash(true);
}

void NewGeneMainWindow::displaySplash(bool const opened_as_about_box)
{
	theSplash = new Splash{this, this, opened_as_about_box};

	Splash * view = theSplash;

	Qt::WindowFlags flags = view->windowFlags();
	flags |= Qt::WindowStaysOnTopHint;
	flags |= Qt::SplashScreen;
	flags &= ~Qt::WindowContextHelpButtonHint;
	view->installEventFilter(view);
	view->setWindowFlags(flags);
	view->show();
	//view->activateWindow();
}

void NewGeneMainWindow::on_actionVacuum_input_database_triggered()
{
	VacuumDialog dlg(this, true);
	dlg.exec();
}

void NewGeneMainWindow::on_actionVacuum_output_database_triggered()
{
	VacuumDialog dlg(this, false);
	dlg.exec();
}
