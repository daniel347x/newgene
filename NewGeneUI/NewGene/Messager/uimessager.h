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
		~UIMessager();

		void displayStatusMessages();

		static UIProjectManager & projectManager();
		static UISettingsManager & settingsManager();
		static UILoggingManager & loggingManager();
		static UIModelManager & modelManager();
		static UIDocumentManager & documentManager();
		static UIStatusManager & statusManager();

	signals:

	public slots:

	public:

		bool do_not_handle_messages_on_desctruction;

};

#endif // UIMESSAGER_H
