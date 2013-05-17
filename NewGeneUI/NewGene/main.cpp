#include "newgenemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include "..\..\NewGeneBackEnd\test.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "newgeneapplication.h"

int main( int argc, char * argv[] )
{

	NewGeneApplication a( argc, argv );
	NewGeneMainWindow w;
	theMainWindow = &w;

	QTimer::singleShot( 0, &w, SLOT( doInitialize() ) );

	w.show();
	return a.exec();

}
