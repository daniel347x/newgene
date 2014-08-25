#include "newgenemanageinput.h"
#include "ui_newgenemanageinput.h"

#include "newgenetabwidgetmanageinput.h"
#include "uiinputproject.h"

QString NewGeneManageInput::titleBarBaseText { "<span style=\"font-size: 18px; font-weight: normal;\">Manage Input Dataset</span>" };

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
    ui->LabelManageInput->setText(titleBarBaseText);

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

    QString newLabel { titleBarBaseText };
    if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT && project != nullptr)
	{
        newLabel += "<span style=\"font-size: 12px; font-weight: normal; vertical-align: super;\"> - ";
		newLabel += project->backend().projectSettings().GetSettingsPath().string().c_str();
        newLabel += "</span>";
		ui->LabelManageInput->setText(newLabel);
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		ui->LabelManageInput->setText(newLabel);
	}

}
