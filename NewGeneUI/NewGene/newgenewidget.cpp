
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
