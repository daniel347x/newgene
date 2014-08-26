#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

#include "newgenetabwidget.h"
#include "uioutputproject.h"

QString NewGeneCreateOutput::titleBarBaseText { "Create Output Dataset" };

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
    QString newLabel { "<div align: center;><span style=\"font-size: 18px; font-weight: normal;\">" };

    newLabel += "<table><tbody><tr><td valign: center;>";
    newLabel += titleBarBaseText;
    newLabel + "</td></tr></tbody></table>";

    newLabel += "</span></div>";
    ui->LabelCreateOutput->setText(newLabel);

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

    QString newLabel { "<div align: center;><span style=\"font-size: 18px; font-weight: normal;\">" };

    newLabel += "<table><tbody><tr><td valign: center;>";
    newLabel += titleBarBaseText;
    newLabel + "</td></tr></tbody></table>";

    if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT && project != nullptr)
	{
        newLabel += "</span><span style=\"font-size: 12px; font-weight: normal;\">";

        newLabel += "<table><tbody><tr><td valign: center;>";
        newLabel += "  - "; // spacer
        newLabel += project->backend().projectSettings().GetSettingsPath().string().c_str();
        newLabel + "</td></tr></tbody></table>";

        newLabel += "</span></div>";
		ui->LabelCreateOutput->setText(newLabel);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
        newLabel += "</span></div>";
		ui->LabelCreateOutput->setText(newLabel);
	}

}
