#include "newgenevariablestoolbox.h"

#include <QLayout>

NewGeneVariablesToolbox::NewGeneVariablesToolbox(QWidget *parent) :
    QToolBox(parent)
{
    layout()->setSpacing(1);

    groups = new NewGeneVariableGroup(this);
    addItem(groups, "Country Variables");

    NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup(this);
    addItem(tmpGrp, "MID Detail Variables");
}
