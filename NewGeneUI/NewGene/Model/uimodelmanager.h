#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include <QObject>

class UIModelManager : public QObject
{
    Q_OBJECT
public:
    explicit UIModelManager(QObject *parent = 0);

    UIModelManager * getModelManager();

signals:

public slots:

private:
    static UIModelManager * modelManager;
};

#endif // UIMODELMANAGER_H
