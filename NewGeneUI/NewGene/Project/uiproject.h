#ifndef UIPROJECT_H
#define UIPROJECT_H

#include <QObject>
#ifndef Q_MOC_RUN
#	include <memory>
#endif

class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UIProjectManager;
class UIModel;
class NewGeneMainWindow;

class UIProject : public QObject
{
		Q_OBJECT
	public:
		explicit UIProject(QObject *parent = 0);

		UIModel * model();
		UIModelManager & modelManager(NewGeneMainWindow * parent = NULL);
		UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);
		UIDocumentManager & documentManager(NewGeneMainWindow * parent = NULL);
		UIStatusManager & statusManager(NewGeneMainWindow * parent = NULL);

	signals:

	public slots:

	protected:
		std::unique_ptr<UIModel> model_;

};

#endif // UIPROJECT_H
