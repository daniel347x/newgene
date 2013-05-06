#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

class QWidget;
class NewGeneMainWindow;
class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UIModel;

class NewGeneWidget
{
public:
    explicit NewGeneWidget(QWidget * self_ = 0);

    NewGeneMainWindow & mainWindow();
    UIModel & model();
    UIModelManager & modelManager(NewGeneMainWindow * parent = NULL);
    UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);
    UIDocumentManager & documentManager(NewGeneMainWindow * parent = NULL);
    UIStatusManager & statusManager(NewGeneMainWindow * parent = NULL);

private:
    QWidget * self;

};

#endif // NEWGENEWIDGET_H
