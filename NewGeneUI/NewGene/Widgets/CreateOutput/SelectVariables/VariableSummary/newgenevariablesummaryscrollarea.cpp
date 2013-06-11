#include "newgenevariablesummaryscrollarea.h"
#include "ui_newgenevariablesummaryscrollarea.h"

NewGeneVariableSummaryScrollArea::NewGeneVariableSummaryScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( this ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableSummaryScrollArea )
{
	ui->setupUi( this );

	groups = new NewGeneVariableSummaryGroup( this );
	groups->setTitle( "Country Variables" );
	layout()->addWidget( groups );

	NewGeneVariableSummaryGroup * tmpGrp = new NewGeneVariableSummaryGroup( this );
	tmpGrp->setTitle( "MID Detail Variables" );
	layout()->addWidget( tmpGrp );
}

NewGeneVariableSummaryScrollArea::~NewGeneVariableSummaryScrollArea()
{
	delete ui;
}

void NewGeneVariableSummaryScrollArea::changeEvent( QEvent * e )
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
