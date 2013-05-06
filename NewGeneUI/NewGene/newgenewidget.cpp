
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"

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

UIModelManager &NewGeneWidget::modelManager(NewGeneMainWindow * parent)
{
    return *UIModelManager::getModelManager(parent);
}

UISettingsManager &NewGeneWidget::settingsManager(NewGeneMainWindow * parent)
{
    return *UISettingsManager::getSettingsManager(parent);
}

UIDocumentManager &NewGeneWidget::documentManager(NewGeneMainWindow * parent)
{
    return *UIDocumentManager::getDocumentManager(parent);
}

UIStatusManager &NewGeneWidget::statusManager(NewGeneMainWindow * parent)
{
    return *UIStatusManager::getStatusManager(parent);
}
