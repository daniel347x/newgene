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

#ifdef QT_DEBUG
#	ifndef Q_MOC_RUN
#		include <boost/date_time/posix_time/posix_time.hpp>
#		include <boost/thread/thread.hpp>
#	endif
#endif

int main( int argc, char * argv[] )
{

#	ifdef QT_DEBUG
		//boost::this_thread::sleep(boost::posix_time::seconds(20));
#	endif

	NewGeneApplication a( argc, argv );
	NewGeneMainWindow w;
	theMainWindow = &w;

	QTimer::singleShot( 0, &w, SLOT( doInitialize() ) );

	w.show();
	return a.exec();

}
