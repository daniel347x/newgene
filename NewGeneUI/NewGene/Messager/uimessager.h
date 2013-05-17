#ifndef UIMESSAGER_H
#define UIMESSAGER_H

#include "globals.h"
#include <QObject>

class UIMessager : public QObject, public Messager
{
        Q_OBJECT
    public:
        explicit UIMessager(QObject *parent = 0);

    signals:

    public slots:

};

#endif // UIMESSAGER_H
