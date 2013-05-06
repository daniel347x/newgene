#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include "globals.h"
#include <QObject>
#include "uimodel.h"
#include "uisettingsmanager.h"
#include "uistatusmanager.h"

class NewGeneMainWindow;

class UIModelManager : public QObject
{
    Q_OBJECT
public:
    explicit UIModelManager(QObject *parent = 0);

    static UIModelManager * getModelManager(NewGeneMainWindow * parent = NULL);

    UIModel * loadDefaultModel();

signals:

public slots:

private:
    static UIModelManager * modelManager;
};

#endif // UIMODELMANAGER_H
