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
		, MANAGER_PROJECT
	};

	explicit UIManager(NewGeneMainWindow *parent = 0);
	NewGeneMainWindow & getMainWindow();

signals:

public slots:

protected:
	WHICH_MANAGER which;
	QString which_descriptor;

protected:

	static UIModelManager & modelManager(NewGeneMainWindow * parent = NULL);
	static UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);
	static UIDocumentManager & documentManager(NewGeneMainWindow * parent = NULL);
	static UIStatusManager & statusManager(NewGeneMainWindow * parent = NULL);

};

#endif // UIMANAGER_H
