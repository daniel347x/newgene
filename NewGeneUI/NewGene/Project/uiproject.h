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
		std::unique_ptr<UIModel> model_;

	private:
		UIModelManager & modelManager();
		UISettingsManager & settingsManager();
		UIDocumentManager & documentManager();
		UIStatusManager & statusManager();

		std::unique_ptr<NewGeneMainWindow> parent_;
		std::unique_ptr<UIProjectSettings> settings_;
};

#endif // UIPROJECT_H
