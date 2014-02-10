#include "newgenemanagedmus.h"
#include "ui_newgenemanagedmus.h"

NewGeneManageDMUs::NewGeneManageDMUs( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageDMUs )
{
	ui->setupUi( this );
}

NewGeneManageDMUs::~NewGeneManageDMUs()
{
	delete ui;
}

void NewGeneManageDMUs::changeEvent( QEvent * e )
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
