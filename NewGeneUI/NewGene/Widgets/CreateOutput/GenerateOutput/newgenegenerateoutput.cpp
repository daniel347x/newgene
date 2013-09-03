#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

#include "globals.h"
#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"
#include <QFileDialog>

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
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)));
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
	UIMessager messager;
	OutputProjectPathToKadOutputFile * setting_path_to_kad_output = nullptr;
	std::unique_ptr<BackendProjectOutputSetting> & path_to_kad_output = projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().GetSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE);
	bool bad = false;
	try
	{
		setting_path_to_kad_output = dynamic_cast<OutputProjectPathToKadOutputFile*>(path_to_kad_output.get());
	}
	catch (std::bad_cast &)
	{
		bad = true;
	}

	if (!bad)
	{
		if (setting_path_to_kad_output)
		{
			QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
			if (editControl)
			{
				editControl->setText(QString(setting_path_to_kad_output->ToString().c_str()));
			}
		}
	}
}

void NewGeneGenerateOutput::RefreshAllWidgets()
{
	WidgetDataItemRequest_GENERATE_OUTPUT_TAB request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneGenerateOutput::on_pushButton_clicked()
{
	UIMessager messager;
	QString the_file = QFileDialog::getSaveFileName(this, "Choose output file");
	if (the_file.size())
	{
		projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().UpdateSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE, OutputProjectPathToKadOutputFile(messager, the_file.toStdString()));
		QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
		if (editControl)
		{
			editControl->setText(the_file);
		}
	}
}

void NewGeneGenerateOutput::on_lineEditFilePathToKadOutput_lostFocus()
{
	UIMessager messager;
	QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
	if (editControl)
	{
		QString the_path = editControl->text();
		projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().UpdateSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE, OutputProjectPathToKadOutputFile(messager, the_path.toStdString()));
	}
}
