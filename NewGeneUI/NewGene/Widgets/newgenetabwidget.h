#ifndef NEWGENETABWIDGET_H
#define NEWGENETABWIDGET_H

#include <QTabWidget>
#include "../newgenewidget.h"

class NewGeneTabWidget : public QTabWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT
	public:
		explicit NewGeneTabWidget( QWidget * parent = 0 );

	signals:

	public slots:

	public:
		void NewGeneUIInitialize();

};

#endif // NEWGENETABWIDGET_H
