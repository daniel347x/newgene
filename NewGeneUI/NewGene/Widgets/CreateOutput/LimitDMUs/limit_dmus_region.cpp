#include "limit_dmus_region.h"
#include "ui_limit_dmus_region.h"

limit_dmus_region::limit_dmus_region(QWidget *parent) :
	QWidget(parent),
    NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, LIMIT_DMUS_TAB) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
    ui( new Ui::limit_dmus_region )
{

    ui->setupUi( this );

    PrepareOutputWidget();

}

limit_dmus_region::~limit_dmus_region()
{
	delete ui;
}

void limit_dmus_region::changeEvent( QEvent * e )
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

void limit_dmus_region::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
    NewGeneWidget::UpdateOutputConnections(connection_type, project);
    if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
    {
        Empty();
    }
}

void limit_dmus_region::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
    NewGeneWidget::UpdateInputConnections(connection_type, project);
}
