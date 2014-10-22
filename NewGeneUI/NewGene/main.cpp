#include "Widgets/newgenemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <memory>

#include "Infrastructure/Model/uimodelmanager.h"
#include "Infrastructure/Settings/uisettingsmanager.h"
#include "Infrastructure/Documents/uidocumentmanager.h"
#include "Infrastructure/Status/uistatusmanager.h"
#include "Infrastructure/Logging/uiloggingmanager.h"
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
        // For debugging startup issues
		//boost::this_thread::sleep(boost::posix_time::seconds(10));
#	endif

	NewGeneApplication a( argc, argv );

    std::unique_ptr<QSplashScreen> splash {new QSplashScreen {QPixmap(":/earth.bits.png"), Qt::WindowStaysOnTopHint }};
    splash->show();

    NewGeneMainWindow w;
	theMainWindow = &w;

	QTimer::singleShot( 0, &w, SLOT( doInitialize() ) );

	w.show();
	return a.exec();

}
