#ifndef NEWGENEAPPLICATION_H
#define NEWGENEAPPLICATION_H

#include <QApplication>
#include "globals.h"
#include "Widgets/newgenewidget.h"
#include "q_declare_metatype.h"

class NewGeneApplication : public QApplication
{
		Q_OBJECT
	public:
		explicit NewGeneApplication(int argc, char * argv[]);

		bool notify(QObject * receiver, QEvent * e);

	signals:

	public slots:
		void showErrorBox(std::string const msg);

};

#endif // NEWGENEAPPLICATION_H
