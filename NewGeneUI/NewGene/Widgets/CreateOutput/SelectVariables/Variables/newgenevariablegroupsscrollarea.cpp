#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"

NewGeneVariableGroupsScrollArea::NewGeneVariableGroupsScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( this ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableGroupsScrollArea )
{
	ui->setupUi( this );
	PrepareInputWidget();
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

void NewGeneVariableGroupsScrollArea::UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		boost::format msg( "Made it to scroll area slot!" );
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
	}
}
