#include "newgenevariablesummary.h"
#include "ui_newgenevariablesummary.h"

#include <QListView>
#include <QStringList>

NewGeneVariableSummary::NewGeneVariableSummary( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_SUMMARY) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableSummary )
{

	ui->setupUi( this );

	PrepareOutputWidget();

}

NewGeneVariableSummary::~NewGeneVariableSummary()
{
	delete ui;
}

void NewGeneVariableSummary::changeEvent( QEvent * e )
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

void NewGeneVariableSummary::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}
