#include "vacuumdialog.h"
#include "ui_vacuumdialog.h"
#include <QPixmap>
#include <QMovie>
#include <Qbitmap>
#include <future>
#include "uiinputproject.h"
#include "uioutputproject.h"
#include "globals.h"
#include "uiprojectmanager.h"

VacuumDialog::VacuumDialog(QWidget * parent, bool const inputmodel) :
	QDialog(parent),
	ui(new Ui::VacuumDialog),
	inputModel(inputmodel),
	isVacuuming(false)
{
	ui->setupUi(this);

	ui->labelVacuuming->hide();
	ui->labelDoneVacuuming->hide();
	ui->labelVacuumSpinner->hide();

	ui->labelVacuumSpinner->setMask((new QPixmap(":/bluespinner.gif"))->mask());
	QMovie * movieVacuumSpinner = new QMovie(":/bluespinner.gif");
	ui->labelVacuumSpinner->setMovie(movieVacuumSpinner);
	movieVacuumSpinner->start();
}

VacuumDialog::~VacuumDialog()
{
	delete ui;
}

void VacuumDialog::setVacuuming(bool const vacuuming)
{
	if (vacuuming)
	{
		isVacuuming = true;
		ui->labelVacuuming->show();
		ui->labelDoneVacuuming->hide();
		ui->labelVacuumSpinner->show();
		this->setEnabled(false);
	}
	else
	{
		isVacuuming = false;
		ui->labelVacuuming->hide();
		ui->labelDoneVacuuming->show();
		ui->labelVacuumSpinner->hide();
		this->setEnabled(true);
	}
}

void VacuumDialog::on_pushButtonVacuum_clicked()
{

	setVacuuming();

	try
	{
		std::async(std::launch::async, [ =, this]()
		{
			if (inputModel)
			{
				UIInputProject * active_input_project = projectManagerUI().getActiveUIInputProject();

				if (active_input_project != nullptr)
				{
					// Copy the database
					active_input_project->backend().model().VacuumDatabase(true);
				}
				else
				{
					QMessageBox::information(this, QString("No database"), QString("No input database is open."));
				}
			}
			else
			{
				UIOutputProject * active_output_project = projectManagerUI().getActiveUIOutputProject();

				if (active_output_project != nullptr)
				{
					// Copy the database
					active_output_project->backend().model().VacuumDatabase(true);
				}
				else
				{
					QMessageBox::information(this, QString("No database"), QString("No output database is open."));
				}
			}

			QMetaObject::invokeMethod(this, "setVacuuming", Qt::QueuedConnection, Q_ARG(bool, false));
		});
	}
	catch (std::runtime_error const & z)
	{
		QMessageBox::information(this, QString("Error"), QString("Error running vacuum thread: ") + z.what());
		setVacuuming(false);
	}

}

void VacuumDialog::reject()
{
	if (!isVacuuming)
	{
		QDialog::reject();
	}
}
