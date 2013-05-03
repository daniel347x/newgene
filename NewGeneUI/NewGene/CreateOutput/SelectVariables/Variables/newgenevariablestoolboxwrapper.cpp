#include "newgenevariablestoolboxwrapper.h"

NewGeneVariablesToolboxWrapper::NewGeneVariablesToolboxWrapper(QWidget *parent) :
    QWidget(parent)
{
    gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(1, 0, 0, 0);
    newgeneToolBox = new NewGeneVariablesToolbox(this);
    gridLayout->addWidget(newgeneToolBox);
    setLayout(gridLayout);
}

NewGeneVariablesToolboxWrapper::~NewGeneVariablesToolboxWrapper()
{
}
