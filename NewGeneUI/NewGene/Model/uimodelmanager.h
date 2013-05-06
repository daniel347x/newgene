#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include <QObject>
#include "uimodel.h"
#include "..\Settings\uisettingsmanager.h"

class UIModelManager : public QObject
{
    Q_OBJECT
public:
    explicit UIModelManager(QObject *parent = 0);

    static UIModelManager * getModelManager();

    UIModel * loadDefaultModel();

signals:

public slots:

protected:
    UISettingsManager & getSettingsManager();

private:
    static UIModelManager * modelManager;
};

#endif // UIMODELMANAGER_H
