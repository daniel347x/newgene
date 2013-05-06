#ifndef UIDOCUMENTMANAGER_H
#define UIDOCUMENTMANAGER_H

#include "globals.h"
#include <QObject>

class UIDocumentManager : public QObject
{
    Q_OBJECT
public:
    explicit UIDocumentManager(QObject *parent = 0);

    static UIDocumentManager * getDocumentManager();

signals:

public slots:

private:
    static UIDocumentManager * documentManager;

};

#endif // UIDOCUMENTMANAGER_H
