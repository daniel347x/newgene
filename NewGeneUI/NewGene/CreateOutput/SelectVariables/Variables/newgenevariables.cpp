#include "newgenevariables.h"
#include "ui_newgenevariables.h"

NewGeneVariables::NewGeneVariables( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( this ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariables )
{
	ui->setupUi( this );
}

NewGeneVariables::~NewGeneVariables()
{
	delete ui;
}

void NewGeneVariables::changeEvent( QEvent * e )
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
