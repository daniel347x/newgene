#ifndef UISTATUSMANAGER_H
#define UISTATUSMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;

class UIStatusManager : public UIManager
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

	explicit UIStatusManager(QObject *parent = 0);

	static UIStatusManager & getStatusManager();

	void LogStatus(QString const & status_, IMPORTANCE const importance_level = IMPORTANCE_STANDARD);
	void PostStatus(QString const & status_, IMPORTANCE const importance_level = IMPORTANCE_STANDARD, bool const forbidWritingToLog = false);

signals:

public slots:

private:
	static std::unique_ptr<UIStatusManager> status_;

};

#endif // UISTATUSMANAGER_H
