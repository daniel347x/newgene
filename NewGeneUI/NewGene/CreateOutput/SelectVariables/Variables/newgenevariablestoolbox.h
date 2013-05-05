#ifndef NEWGENEVARIABLESTOOLBOX_H
#define NEWGENEVARIABLESTOOLBOX_H

#include <QToolBox>
#include "..\..\..\newgenewidget.h"
#include "newgenevariablegroup.h"

class NewGeneVariablesToolbox : public QToolBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
    Q_OBJECT
public:
    explicit NewGeneVariablesToolbox(QWidget *parent = 0);

signals:

public slots:

private:
    NewGeneVariableGroup * groups;

};

#endif // NEWGENEVARIABLESTOOLBOX_H
