#include "kadwidgetsscrollarea.h"

#include "kadspinbox.h"
#include <QLayout>
#include <QFont>
#include <QGraphicsColorizeEffect>

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
	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)));

		// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)));

		// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)));
	}
	else if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}

void KadWidgetsScrollArea::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void KadWidgetsScrollArea::WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA widget_data)
{

	Empty();

	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			WidgetInstanceIdentifier new_identifier(identifier);
			QSpinBox * newSpinBox = new KadSpinBox(this, new_identifier, outp);
			newSpinBox->setFixedHeight(50);
			newSpinBox->setFixedWidth(400);
			//QGraphicsColorizeEffect * backColor = new QGraphicsColorizeEffect();
			//QColor color_("yellow");
			//backColor->setColor(color_);
			//newSpinBox->setGraphicsEffect(backColor); // takes ownership
			QFont currFont = newSpinBox->font();
			currFont.setPixelSize(24);
			newSpinBox->setFont(currFont);
			//std::string stylesheet(" QSpinBox { color: #d006bc; font-weight: bold; } ");
			std::string stylesheet(" QSpinBox { color: #960488; font-weight: bold; } ");
			newSpinBox->setStyleSheet(stylesheet.c_str());
			if (identifier.longhand)
			{
				std::string prefixText(" #");
				prefixText += *identifier.longhand;
				prefixText += " columns:  ";
				newSpinBox->setPrefix(prefixText.c_str());
			}
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

void KadWidgetsScrollArea::Empty()
{
	QLayoutItem *child;
	while ((child = layout()->takeAt(0)) != 0)
	{
		delete child;
	}
}
