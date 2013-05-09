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
class UIProjectSettings;
class UILoggingManager;
class UIProjectManager;

class UIProject : public QObject
{
		Q_OBJECT
	public:
		explicit UIProject(NewGeneMainWindow *parent = 0);

		UIModel * model();
		UIProjectSettings * settings();

	signals:

	public slots:

	protected:
		UIModel * model_; // owned by this UIProject
		UIProjectSettings * projectSettings_; // owned by this UIProject

	private:
		UIModelManager & modelManager();
		UISettingsManager & settingsManager();
		UIDocumentManager & documentManager();
		UIStatusManager & statusManager();
		UILoggingManager & loggingManager();
		UIProjectManager & projectManager();
};

#endif // UIPROJECT_H
