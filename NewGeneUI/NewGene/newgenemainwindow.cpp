#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"
#include <QMessageBox>
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uistatusmanager.h"
#include "uidocumentmanager.h"
#include "uisettingsmanager.h"
#include "uimodelmanager.h"
#include "uimodel.h"
#include "uiproject.h"

NewGeneMainWindow::NewGeneMainWindow(QWidget *parent) :
	QMainWindow(parent),
	NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneMainWindow)
{

	try
	{

		ui->setupUi(this);

		NewGeneTabWidget * pTWmain = findChild<NewGeneTabWidget*>("tabWidgetMain");
		if (pTWmain)
		{
			pTWmain->NewGeneInitialize();
		}

		// Empty calls to create manager instances
//		statusManager(this);
//		documentManager(this);
//		settingsManager(this);
//		modelManager(this);

//		globalSettings.reset(settingsManager().loadGlobalSettings());
//		model.reset(modelManager().loadDefaultModel());

	}
	catch (boost::exception & e)
	{
		if( std::string const * error_desc = boost::get_error_info<newgene_error_description>(e) )
		{
			boost::format msg(error_desc->c_str());
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
		}
		else
		{
			boost::format msg("Unknown exception thrown");
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
	delete ui;
}

void NewGeneMainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
