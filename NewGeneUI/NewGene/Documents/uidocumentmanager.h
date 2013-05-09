#ifndef UIDOCUMENTMANAGER_H
#define UIDOCUMENTMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;



class UIDocumentManager : public UIManager
{
	Q_OBJECT
public:
	explicit UIDocumentManager(QObject *parent = 0);

	static UIDocumentManager & getDocumentManager();

signals:

public slots:

private:
	static std::unique_ptr<UIDocumentManager> documentManager;

};

#endif // UIDOCUMENTMANAGER_H
