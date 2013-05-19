#ifndef UISTATUSMANAGER_H
#define UISTATUSMANAGER_H

#include "globals.h"
#include "uimanager.h"
#include "..\..\..\NewGeneBackEnd\Status\StatusManager.h"

class NewGeneMainWindow;

class UIStatusManager : public QObject, public UIManager<UIStatusManager, StatusManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS>
{
		Q_OBJECT

	public:

		enum IMPORTANCE
		{
			  IMPORTANCE_DEBUG
			, IMPORTANCE_STANDARD
			, IMPORTANCE_HIGH
			, IMPORTANCE_CRITICAL
		};

		explicit UIStatusManager( QObject * parent = 0 );

		void LogStatus( QString const & _statusManager, IMPORTANCE const importance_level = IMPORTANCE_STANDARD );
		void PostStatus( QString const & _statusManager, IMPORTANCE const importance_level = IMPORTANCE_STANDARD, bool const forbidWritingToLog = false );

	signals:

	public slots:

	private:

};

#endif // UISTATUSMANAGER_H
