#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"

NewGeneVariableGroupsScrollArea::NewGeneVariableGroupsScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( this ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableGroupsScrollArea )
{
	PrepareOutputWidget(VARIABLE_GROUPS_SCROLL_AREA);
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

void NewGeneVariableGroupsScrollArea::UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
	//connect(this, SIGNAL(TestSignal()), inp->getConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), inp->GetModelConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), inp->GetModelSettingsConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), inp->GetProjectSettingsConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), settingsManagerUI().globalSettings().getConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), projectManagerUI().getConnector(), SLOT(TestSlot()));
}

void NewGeneVariableGroupsScrollArea::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)));
	//connect(this, SIGNAL(TestSignal()), outp->getConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), outp->GetModelConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), outp->GetModelSettingsConnector(), SLOT(TestSlot()));
	//connect(this, SIGNAL(TestSignal()), outp->GetProjectSettingsConnector(), SLOT(TestSlot()));
	//QTimer::singleShot( 0, this, SLOT(TestSlot()) );
}

void NewGeneVariableGroupsScrollArea::TestSlot()
{
	//emit TestSignal();
}

void NewGeneVariableGroupsScrollArea::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA widget_data)
{
//	boost::format msg( "Made it BACK to the scroll area widget with refresh data!! Here's the data: %1%" );
//	msg % widget_data.n;
//	QMessageBox msgBox;
//	msgBox.setText( msg.str().c_str() );
//	msgBox.exec();


}

void NewGeneVariableGroupsScrollArea::RefreshAllWidgets()
{
	emit RefreshWidget(VARIABLE_GROUPS_SCROLL_AREA);
}
