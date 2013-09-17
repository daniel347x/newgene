#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

#include "globals.h"
#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"
#include <QFileDialog>
#include <QPlainTextEdit>

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
	else if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
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

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

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
	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_GENERATE_OUTPUT_TAB request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneGenerateOutput::ReceiveSignalAppendKadStatusText(int progress_bar_id, STD_STRING const kad_status_update)
{
	QPlainTextEdit * edit_pane = findChild<QPlainTextEdit *>( "plainTextEdit_KadStatus" );
	if (edit_pane)
	{
		if (kad_status_update.length())
		{
			edit_pane->appendPlainText(kad_status_update.c_str());
		}
		else
		{
			edit_pane->clear();
		}
	}
}

void NewGeneGenerateOutput::ReceiveSignalSetPerformanceLabel(int progress_bar_id, STD_STRING const performance_measure_text)
{
	QLabel * label_ = findChild<QLabel *>( "labelOngoingPerformance" );
	if (label_)
	{
		QString html_prefix("<span style=\"font-size:12pt; font-weight:600; color:#aa0000;\">");
		QString html_postfix("</span>");
		QString html_text = html_prefix;
		html_text += QString(performance_measure_text.c_str());
		html_text += html_postfix;
		label_->setText(html_text);
	}
}

void NewGeneGenerateOutput::on_pushButton_clicked()
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

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
}

void NewGeneGenerateOutput::on_lineEditFilePathToKadOutput_editingFinished()
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
	if (editControl)
	{
		QString the_path = editControl->text();
		projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().UpdateSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE, OutputProjectPathToKadOutputFile(messager, the_path.toStdString()));
	}

}

void NewGeneGenerateOutput::on_pushButton_cancel_clicked()
{
	{
		std::lock_guard<std::recursive_mutex> guard(OutputModel::OutputGenerator::is_generating_output_mutex);
		if (OutputModel::OutputGenerator::is_generating_output)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString("Cancel?"), QString("Are you sure you wish to cancel?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::Yes)
			{
				// No lock - not necessary for a boolean checked multiple times by back end and that will not cause an error if it is messed up in extraordinarily rare circumstances
				OutputModel::OutputGenerator::cancelled = true;
			}
		}
	}
}
