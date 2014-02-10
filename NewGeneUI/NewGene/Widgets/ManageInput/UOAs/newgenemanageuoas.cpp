#include "newgenemanageuoas.h"
#include "ui_newgenemanageuoas.h"

NewGeneManageUOAs::NewGeneManageUOAs( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageUOAs )
{
	ui->setupUi( this );
}

NewGeneManageUOAs::~NewGeneManageUOAs()
{
	delete ui;
}

void NewGeneManageUOAs::changeEvent( QEvent * e )
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
