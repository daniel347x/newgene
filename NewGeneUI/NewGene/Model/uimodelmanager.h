#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;
class UIModel;

class UIModelManager : public UIManager
{
    Q_OBJECT
public:
    explicit UIModelManager(NewGeneMainWindow *parent = 0);

    static UIModelManager * getModelManager(NewGeneMainWindow * parent = NULL);

    UIModel * loadDefaultModel();

signals:

public slots:

private:
    static UIModelManager * modelManager;
};

#endif // UIMODELMANAGER_H
