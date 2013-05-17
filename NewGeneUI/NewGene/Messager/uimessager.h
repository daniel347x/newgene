#ifndef UIMESSAGER_H
#define UIMESSAGER_H

#include <QObject>
#include "..\..\NewGeneBackEnd\Messager\Messager.h"

class UISettingsManager;
class UIProjectManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;

class UIMessager : public QObject, public Messager
{
		Q_OBJECT
	public:
		explicit UIMessager(QObject *parent = 0);

		void displayStatusMessages();

		static UIProjectManager & projectManager();
		static UISettingsManager & settingsManager();
		static UILoggingManager & loggingManager();
		static UIModelManager & modelManager();
		static UIDocumentManager & documentManager();
		static UIStatusManager & statusManager();

	signals:

	public slots:

};

#endif // UIMESSAGER_H
