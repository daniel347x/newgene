#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uimanager.h"

class NewGeneMainWindow;

class UIProjectManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UIProjectManager(NewGeneMainWindow *parent = 0);

	signals:

	public slots:

};

#endif // UIPROJECTMANAGER_H
