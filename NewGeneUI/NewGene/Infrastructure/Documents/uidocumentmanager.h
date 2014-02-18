#ifndef UIDOCUMENTMANAGER_H
#define UIDOCUMENTMANAGER_H

#include "globals.h"
#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/Documents/DocumentManager.h"

class NewGeneMainWindow;

class UIDocumentManager : public QObject, public UIManager<UIDocumentManager, DocumentManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS>
{
		Q_OBJECT
	public:
		explicit UIDocumentManager( QObject * parent, UIMessager & messager );

	signals:

	public slots:

};

#endif // UIDOCUMENTMANAGER_H
