#ifndef MODELCHANGE_H
#define MODELCHANGE_H

#include <QObject>
#include "modelchangeindicator.h"

class ModelChange : public ModelChangeIndicator
{
    Q_OBJECT
public:
    explicit ModelChange(QObject *parent = 0);

signals:

public slots:

};

#endif // MODELCHANGE_H
