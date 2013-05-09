#ifndef UIDOCUMENTMANAGER_H
#define UIDOCUMENTMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;



class UIDocumentManager : public UIManager
{
	Q_OBJECT
public:
	explicit UIDocumentManager(NewGeneMainWindow *parent = 0);

	static UIDocumentManager & getDocumentManager(NewGeneMainWindow * parent = NULL);

signals:

public slots:

private:
	static std::unique_ptr<UIDocumentManager> documentManager;

};

#endif // UIDOCUMENTMANAGER_H
