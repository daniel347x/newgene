#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "globals.h"
#include <QObject>
#include <QString>

class NewGeneMainWindow;
class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UILoggingManager;
class NewGeneWidget;

class UIManager : public QObject
{

	Q_OBJECT

public:

	enum WHICH_MANAGER
	{
		  MANAGER_DOCUMENTS
		, MANAGER_SETTINGS
		, MANAGER_STATUS
		, MANAGER_MODEL
		, MANAGER_LOGGING
		, MANAGER_PROJECT
	};

	explicit UIManager(QObject *parent = 0);
	NewGeneMainWindow & getMainWindow();

signals:

public slots:

protected:
	WHICH_MANAGER which;
	QString which_descriptor;

protected:

	static UIModelManager & modelManager();
	static UISettingsManager & settingsManager();
	static UIDocumentManager & documentManager();
	static UIStatusManager & statusManager();
	static UILoggingManager & loggingManager();

	friend class NewGeneWidget;

};

#endif // UIMANAGER_H
