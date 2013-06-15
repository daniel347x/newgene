#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"

NewGeneVariableGroupsScrollArea::NewGeneVariableGroupsScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( this ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableGroupsScrollArea )
{
	PrepareOutputWidget();
	ui->setupUi( this );
}

NewGeneVariableGroupsScrollArea::~NewGeneVariableGroupsScrollArea()
{
	delete ui;
}

void NewGeneVariableGroupsScrollArea::changeEvent( QEvent * e )
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

void NewGeneVariableGroupsScrollArea::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(TestSignal()), outp->GetModelConnector(), SLOT(TestSlot()));
	QTimer::singleShot( 0, this, SLOT(TestSlot()) );
}

void NewGeneVariableGroupsScrollArea::TestSlot()
{
	emit TestSignal();
}
