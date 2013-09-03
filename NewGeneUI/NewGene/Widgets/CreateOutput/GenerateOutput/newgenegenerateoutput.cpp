#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

NewGeneGenerateOutput::NewGeneGenerateOutput(QWidget *parent) :
	QWidget(parent),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, GENERATE_OUTPUT_TAB, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneGenerateOutput)
{

	ui->setupUi(this);

	PrepareOutputWidget();

}

NewGeneGenerateOutput::~NewGeneGenerateOutput()
{
	delete ui;
}

void NewGeneGenerateOutput::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		NewGeneWidget::UpdateOutputConnections(connection_type, project);
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB)));
		connect(this, SIGNAL(GenerateOutputSignal(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)));
	}

}

void NewGeneGenerateOutput::on_pushButtonGenerateOutput_clicked()
{

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__GenerateOutput()))));
	WidgetActionItemRequest_ACTION_GENERATE_OUTPUT action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, actionItems);
	emit GenerateOutputSignal(action_request);

}

void NewGeneGenerateOutput::WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB widget_data)
{

}

void NewGeneGenerateOutput::RefreshAllWidgets()
{
	WidgetDataItemRequest_GENERATE_OUTPUT_TAB request(value(), WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}
