#include "newgenevariablestoolboxwrapper.h"

NewGeneVariablesToolboxWrapper::NewGeneVariablesToolboxWrapper(QWidget *parent) :
    QWidget(parent)
{
    gridLayout = new QGridLayout;
    newgeneToolBox = new NewGeneVariablesToolbox(this);
    gridLayout->addWidget(newgeneToolBox);
    setLayout(gridLayout);
}

NewGeneVariablesToolboxWrapper::~NewGeneVariablesToolboxWrapper()
{
}
