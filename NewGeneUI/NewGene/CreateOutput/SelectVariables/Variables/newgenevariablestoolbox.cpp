#include "newgenevariablestoolbox.h"

NewGeneVariablesToolbox::NewGeneVariablesToolbox(QWidget *parent) :
    QToolBox(parent)
{
    groups = new NewGeneVariableGroup(this);
    addItem(groups, "Country Variables");

    NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup(this);
    addItem(tmpGrp, "MID Detail Variables");
}
