#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

#include "newgenetabwidget.h"
#include "uioutputproject.h"

QString NewGeneCreateOutput::titleBarBaseText { "<span style=\"font-size: 18px; font-weight: normal;\">Create Output Dataset</span>" };

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
	ui->LabelCreateOutput->setText(titleBarBaseText);

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

	QString newLabel { titleBarBaseText };
	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT && project != nullptr)
	{
        newLabel += "<span style=\"font-size: 12px; font-weight: normal; vertical-align: super;\"> - ";
		newLabel += project->backend().projectSettings().GetSettingsPath().string().c_str();
		newLabel += "</span>";
        ui->LabelCreateOutput->setText(newLabel);
    }

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		ui->LabelCreateOutput->setText(newLabel);
	}

}
