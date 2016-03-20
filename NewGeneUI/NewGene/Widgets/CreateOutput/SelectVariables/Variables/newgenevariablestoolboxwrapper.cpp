#include "newgenevariablestoolboxwrapper.h"

NewGeneVariablesToolboxWrapper::NewGeneVariablesToolboxWrapper(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this,
									 WIDGET_NATURE_OUTPUT_WIDGET))   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
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

void NewGeneVariablesToolboxWrapper::showInactiveVariableGroups(bool const visible)
{
	if (newgeneToolBox)
	{
		newgeneToolBox->showInactiveVariableGroups(visible);
	}
}
