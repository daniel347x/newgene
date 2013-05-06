#ifndef UIDOCUMENTMANAGER_H
#define UIDOCUMENTMANAGER_H

#include "globals.h"
#include "newgenemainwindow.h"
#include <QObject>

class NewGeneMainWindow;

class UIDocumentManager : public QObject
{
    Q_OBJECT
public:
    explicit UIDocumentManager(QObject *parent = 0);

    static UIDocumentManager * getDocumentManager(NewGeneMainWindow * parent = NULL);

signals:

public slots:

private:
    static UIDocumentManager * documentManager;

};

#endif // UIDOCUMENTMANAGER_H
