#ifndef UILOGGINGMANAGER_H
#define UILOGGINGMANAGER_H

#include "uimanager.h"

class NewGeneMainWindow;

class UILoggingManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UILoggingManager( QObject * parent = 0 );

		static UILoggingManager & getLoggingManager();

	signals:

	public slots:

	protected:

		bool ObtainLogfilePath();

	private:

		static std::unique_ptr<UILoggingManager> loggingManager_;

		boost::filesystem::path loggingPath;

};

#endif // UILOGGINGMANAGER_H
