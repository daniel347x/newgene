#include "newgenemanageinput.h"
#include "ui_newgenemanageinput.h"

#include "newgenetabwidgetmanageinput.h"
#include "uiinputproject.h"

QString NewGeneManageInput::titleBarBaseText { "Manage Input Dataset" };

NewGeneManageInput::NewGeneManageInput( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageInput )
{

	ui->setupUi( this );

	NewGeneTabWidgetManageInput * pTWinput = findChild<NewGeneTabWidgetManageInput *>( "tabWidgetInput" );

	if ( pTWinput )
	{
		pTWinput->NewGeneUIInitialize();
	}

	PrepareInputWidget();

    ui->LabelManageInput->setTextFormat(Qt::TextFormat::RichText);
    QString newLabel { "<div align: center;><span style=\"font-size: 18px; font-weight: normal;\">" };

    newLabel += "<table><tbody><tr><td valign: center;>";
    newLabel += titleBarBaseText;
    newLabel + "</td></tr></tbody></table>";

    newLabel += "</span></div>";
    ui->LabelManageInput->setText(newLabel);

}

NewGeneManageInput::~NewGeneManageInput()
{
	delete ui;
}

void NewGeneManageInput::changeEvent( QEvent * e )
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

void NewGeneManageInput::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

    QString newLabel { "<div align: center;><span style=\"font-size: 18px; font-weight: normal;\">" };

    newLabel += "<table><tbody><tr><td valign: center;>";
    newLabel += titleBarBaseText;
    newLabel + "</td></tr></tbody></table>";

    if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT && project != nullptr)
	{
        newLabel += "</span><span style=\"font-size: 12px; font-weight: normal;\">";

        newLabel += "<table><tbody><tr><td valign: center;>";
        newLabel += "  - "; // spacer
        newLabel += project->backend().projectSettings().GetSettingsPath().string().c_str();
        newLabel + "</td></tr></tbody></table>";

        newLabel += "</span></div>";
        ui->LabelManageInput->setText(newLabel);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
        newLabel += "</span></div>";
        ui->LabelManageInput->setText(newLabel);
	}

}
