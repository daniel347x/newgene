#ifndef KADCOLUMNSELECTIONBOX_H
#define KADCOLUMNSELECTIONBOX_H

#include <QFrame>
#include "../../newgenewidget.h"

namespace Ui
{
	class KAdColumnSelectionBox;
}

class KAdColumnSelectionBox : public QFrame, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit KAdColumnSelectionBox(QWidget * parent = 0);
		~KAdColumnSelectionBox();

	protected:

		void changeEvent(QEvent * e);

	private:

		Ui::KAdColumnSelectionBox * ui;

	public:

		friend class NewGeneMainWindow;

	signals:

	public slots:

		void popupWarning(QString const &);

};

#endif // KADCOLUMNSELECTIONBOX_H
