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
#include <memory>

#include "Infrastructure/Model/uimodelmanager.h"
#include "Infrastructure/Settings/uisettingsmanager.h"
#include "Infrastructure/Documents/uidocumentmanager.h"
#include "Infrastructure/Status/uistatusmanager.h"
#include "Infrastructure/Logging/uiloggingmanager.h"
#include "newgeneapplication.h"
#include "splashwindow.h"

int main( int argc, char * argv[] )
{

#	ifdef QT_DEBUG
        // For debugging startup issues
		//boost::this_thread::sleep(boost::posix_time::seconds(10));
#	endif

	NewGeneApplication a( argc, argv );

    NewGeneMainWindow w;
    theMainWindow = &w;

    std::unique_ptr<SplashWindow> view { new SplashWindow{} };
    QQmlEngine * engine = view->engine();
    engine->rootContext()->setContextProperty("view", view.get());
    view->setSource(QUrl{"qrc:///splash.qml"});
    Qt::WindowFlags flags = view->windowFlags();
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::SplashScreen;
    flags &= ~Qt::WindowContextHelpButtonHint;
    view->installEventFilter(view.get());
    view->setWindowFlags(flags);
    view->show();
    view->activateWindow();

	return a.exec();

}
