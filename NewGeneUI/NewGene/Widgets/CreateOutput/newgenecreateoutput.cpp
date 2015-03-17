#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

#include "newgenetabwidget.h"
#include "uioutputproject.h"

QString NewGeneCreateOutput::titleBarBaseText { "Output Dataset" };

NewGeneCreateOutput::NewGeneCreateOutput(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this,
									 WIDGET_NATURE_OUTPUT_WIDGET)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneCreateOutput)
{

	ui->setupUi(this);

	NewGeneTabWidget * pTWoutput = findChild<NewGeneTabWidget *>("tabWidgetOutput");

	if (pTWoutput)
	{
		pTWoutput->NewGeneUIInitialize();
	}

	PrepareOutputWidget();

}

NewGeneCreateOutput::~NewGeneCreateOutput()
{
	delete ui;
}

void NewGeneCreateOutput::changeEvent(QEvent * e)
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

void NewGeneCreateOutput::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT && project != nullptr)
	{
	}

}

void NewGeneCreateOutput::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT && project != nullptr)
	{
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
	}

}

void NewGeneCreateOutput::on_tabWidgetOutput_currentChanged(int index)
{
	setFocus();
}
