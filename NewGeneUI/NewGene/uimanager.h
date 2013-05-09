#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>
#include <QString>

class NewGeneMainWindow;

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

};

#endif // UIMANAGER_H
