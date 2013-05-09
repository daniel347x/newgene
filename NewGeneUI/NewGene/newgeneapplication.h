#ifndef NEWGENEAPPLICATION_H
#define NEWGENEAPPLICATION_H

#include <QApplication>
#include "globals.h"

class NewGeneApplication : public QApplication
{
		Q_OBJECT
	public:
		explicit NewGeneApplication(int argc, char *argv[]);

	bool notify ( QObject * receiver, QEvent * e );

	signals:

	public slots:

};

#endif // NEWGENEAPPLICATION_H
