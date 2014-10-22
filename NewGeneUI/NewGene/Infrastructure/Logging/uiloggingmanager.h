#ifndef UILOGGINGMANAGER_H
#define UILOGGINGMANAGER_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/Logging/LoggingManager.h"

class NewGeneMainWindow;

class UILoggingManager : public QObject, public UIManager<UILoggingManager, LoggingManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING>
{
		Q_OBJECT
	public:
		explicit UILoggingManager( QObject * parent, UIMessager & messager );

	signals:

	public slots:

	protected:

		bool ObtainLogfilePath();

	private:

		boost::filesystem::path loggingPath;
		QString current_error;

};

#endif // UILOGGINGMANAGER_H
