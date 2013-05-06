#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uimodel.h"
#include "uimodelmanager.h"
class QWidget;
class NewGeneMainWindow;

class NewGeneWidget
{
public:
    explicit NewGeneWidget(QWidget * self_ = 0);

protected:
    NewGeneMainWindow & mainWindow();
    UIModel & model();
    UIModelManager & modelManager();
    UISettingsManager & settingsManager();
    UIDocumentManager & documentManager();

private:
    QWidget * self;

};

#endif // NEWGENEWIDGET_H
