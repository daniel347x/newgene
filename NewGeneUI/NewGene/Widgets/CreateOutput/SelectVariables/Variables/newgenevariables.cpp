#include "newgenevariables.h"
#include "ui_newgenevariables.h"
#include "newgenevariablestoolboxwrapper.h"

NewGeneVariables::NewGeneVariables(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this,
									 WIDGET_NATURE_OUTPUT_WIDGET)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneVariables)
{
	ui->setupUi(this);
}

NewGeneVariables::~NewGeneVariables()
{
	delete ui;
}

void NewGeneVariables::changeEvent(QEvent * e)
{
	QWidget::changeEvent(e);

	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}
}

void NewGeneVariables::on_checkBoxHideBars_stateChanged(int checkedState)
{
	NewGeneVariablesToolboxWrapper * toolbox = this->findChild<NewGeneVariablesToolboxWrapper *>("toolbox");

	if (toolbox)
	{
		bool checked {static_cast<Qt::CheckState>(checkedState) == Qt::Checked};
		toolbox->showInactiveVariableGroups(!checked);
	}
}
