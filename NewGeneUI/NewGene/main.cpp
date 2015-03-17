#ifdef QT_DEBUG
#	ifndef Q_MOC_RUN
#		include <boost/date_time/posix_time/posix_time.hpp>
#		include <boost/thread/thread.hpp>
#	endif
#endif
#include "Widgets/newgenemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QWebSettings>

#include "Infrastructure/Model/uimodelmanager.h"
#include "Infrastructure/Settings/uisettingsmanager.h"
#include "Infrastructure/Documents/uidocumentmanager.h"
#include "Infrastructure/Status/uistatusmanager.h"
#include "Infrastructure/Logging/uiloggingmanager.h"
#include "newgeneapplication.h"

int main( int argc, char * argv[] )
{

#	ifdef QT_DEBUG
        // For debugging startup issues
		//boost::this_thread::sleep(boost::posix_time::seconds(10));
#	endif

    int retVal {};

    NewGeneApplication a( argc, argv );

    {

        NewGeneMainWindow w;
        theMainWindow = &w;

        QTimer::singleShot( 500, theMainWindow, SLOT( show() ) );
        QTimer::singleShot( 1000, theMainWindow, SLOT( doInitialize() ) );

        retVal = a.exec();

    }

    QWebSettings::clearMemoryCaches();

    return retVal;

}
