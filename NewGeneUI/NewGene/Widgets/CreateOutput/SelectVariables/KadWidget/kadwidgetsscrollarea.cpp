#include "kadwidgetsscrollarea.h"

#include "kadspinbox.h"
#include <QLayout>

KadWidgetsScrollArea::KadWidgetsScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, KAD_SPIN_CONTROLS_AREA, true) ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{

    QLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    this->setLayout(layout);

    PrepareOutputWidget();

}

void KadWidgetsScrollArea::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)));

	// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)));

	// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)));
}

void KadWidgetsScrollArea::RefreshAllWidgets()
{
	WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void KadWidgetsScrollArea::WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA widget_data)
{

	QLayoutItem *child;
	while ((child = layout()->takeAt(0)) != 0)
	{
		delete child;
	}

	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			WidgetInstanceIdentifier new_identifier(identifier);
			QWidget * newSpinBox = new KadSpinBox(this, new_identifier, outp);
			//newSpinBox->setTitle(identifier.longhand->c_str());
			layout()->addWidget(newSpinBox);
		}
	});

}

void KadWidgetsScrollArea::WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET widget_data)
{
	if (widget_data.identifier && widget_data.identifier->uuid)
	{
		NewGeneWidget * child = outp->FindWidgetFromDataItem(uuid, *widget_data.identifier->uuid);
		if (child)
		{
			child->WidgetDataRefreshReceive(widget_data);
		}
	}
}
