#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include <QObject>
#include "uimodel.h"

class UIModelManager : public QObject
{
    Q_OBJECT
public:
    explicit UIModelManager(QObject *parent = 0);

    static UIModelManager * getModelManager();

    UIModel * loadDefaultModel();

signals:

public slots:

private:
    static UIModelManager * modelManager;
};

#endif // UIMODELMANAGER_H
