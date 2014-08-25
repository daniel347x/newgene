#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

#include "newgenetabwidget.h"
#include "uioutputproject.h"

NewGeneCreateOutput::NewGeneCreateOutput( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneCreateOutput )
{

	ui->setupUi( this );

	NewGeneTabWidget * pTWoutput = findChild<NewGeneTabWidget *>( "tabWidgetOutput" );

	if ( pTWoutput )
	{
		pTWoutput->NewGeneUIInitialize();
	}

	PrepareOutputWidget();

    ui->LabelCreateOutput->setTextFormat(Qt::TextFormat::RichText);

}

NewGeneCreateOutput::~NewGeneCreateOutput()
{
	delete ui;
}

void NewGeneCreateOutput::changeEvent( QEvent * e )
{
	QWidget::changeEvent( e );

	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;

		default:
			break;
	}
}

void NewGeneCreateOutput::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	QString newLabel { "Create Output Dataset" };
	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT && project != nullptr)
	{
		newLabel += " - ";
		newLabel += project->backend().projectSettings().GetSettingsPath().string().c_str();
		ui->LabelCreateOutput->setText(newLabel);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		ui->LabelCreateOutput->setText(newLabel);
	}

}
