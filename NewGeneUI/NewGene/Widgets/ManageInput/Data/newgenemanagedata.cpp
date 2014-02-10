#include "newgenemanagedata.h"
#include "ui_newgenemanagedata.h"

NewGeneManageData::NewGeneManageData( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageData )
{
	ui->setupUi( this );
}

NewGeneManageData::~NewGeneManageData()
{
	delete ui;
}

void NewGeneManageData::changeEvent( QEvent * e )
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
