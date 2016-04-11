#ifndef VACUUMDIALOG_H
#define VACUUMDIALOG_H

#include <QDialog>

namespace Ui
{
	class VacuumDialog;
}

class VacuumDialog : public QDialog
{
		Q_OBJECT

	public:
		explicit VacuumDialog(QWidget * parent = 0, bool const inputmodel = true);
		~VacuumDialog();

	private slots:
		void on_pushButtonVacuum_clicked();
		void setVacuuming(bool const vacuuming = true);
		void noDatabase();

	private:
		void reject();

	private:
		bool const inputModel;
		bool isVacuuming;

	private:
		Ui::VacuumDialog * ui;
};

#endif // VACUUMDIALOG_H
