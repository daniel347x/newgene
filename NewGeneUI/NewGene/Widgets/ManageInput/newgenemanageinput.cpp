#include "newgenemanageinput.h"
#include "ui_newgenemanageinput.h"

#include "newgenetabwidgetmanageinput.h"

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

    if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
    {
        ui->LabelManageInput->text = "Manage Input Dataset - ";
    }

    if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
    {
        ui->LabelManageInput->text = "Manage Input Dataset";
    }

}
