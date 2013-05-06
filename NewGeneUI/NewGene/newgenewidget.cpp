
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"

NewGeneWidget::NewGeneWidget(QWidget * self_) :
    self(self_)
{
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
    QWidget * pMainWindow = self->window();
    NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>(pMainWindow);
    return *pNewGeneMainWindow;
}

UIModel & NewGeneWidget::model()
{
    UIModel * pNewGeneModel = mainWindow().model.get();
    if (!pNewGeneModel)
    {
        boost::format msg("Model not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return *pNewGeneModel;
}

UIModelManager &NewGeneWidget::modelManager()
{
    UIModelManager * pModelManager = UIModelManager::getModelManager();
    if (!pModelManager)
    {
        boost::format msg("Model manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return *pModelManager;
}

UISettingsManager &NewGeneWidget::settingsManager()
{
    UISettingsManager * pSettingsManager = UISettingsManager::getSettingsManager();
    if (!pSettingsManager)
    {
        boost::format msg("Settings manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return *pSettingsManager;
}
