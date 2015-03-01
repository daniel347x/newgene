#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

#include "globals.h"
#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"
#include <QFileDialog>
#include <QPlainTextEdit>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

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

void NewGeneGenerateOutput::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB)));
		connect(this, SIGNAL(GenerateOutputSignal(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}

}

void NewGeneGenerateOutput::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
}

void NewGeneGenerateOutput::on_pushButtonGenerateOutput_clicked()
{

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__GenerateOutput()))));
	WidgetActionItemRequest_ACTION_GENERATE_OUTPUT action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, actionItems);
	emit GenerateOutputSignal(action_request);

}

void NewGeneGenerateOutput::WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	std::unique_ptr<Setting> path_to_kad_output = projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().GetSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE);

	OutputProjectPathToKadOutputFile * setting_path_to_kad_output = nullptr;
	bool bad = false;
	try
	{
		setting_path_to_kad_output = dynamic_cast<OutputProjectPathToKadOutputFile*>(path_to_kad_output.get());
	}
	catch (std::bad_cast &)
	{
		bad = true;
	}

	if (setting_path_to_kad_output == nullptr)
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

void NewGeneGenerateOutput::ReceiveSignalAppendKadStatusText(int /* progress_bar_id */, STD_STRING const kad_status_update)
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

void NewGeneGenerateOutput::ReceiveSignalSetPerformanceLabel(int /* progress_bar_id */, STD_STRING const performance_measure_text)
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

void NewGeneGenerateOutput::on_pushButtonChooseLocation_clicked()
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

    std::unique_ptr<Setting> path_to_kad_output = projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().GetSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE);

    QString current_file {};
    if (path_to_kad_output) current_file = path_to_kad_output->ToString().c_str();


    QString selectedFilter {"Comma-separated values (*.csv)"};
	QString the_file = QFileDialog::getSaveFileName(this, "Choose output file", current_file, QString { selectedFilter }, &selectedFilter, QFileDialog::DontUseNativeDialog | QFileDialog::DontConfirmOverwrite);

    if (the_file.size())
	{
        if (! the_file.endsWith(".csv", Qt::CaseInsensitive))
        {
            the_file += ".csv";
        }
		projectManagerUI().getActiveUIOutputProject()->projectSettings().getBackendSettings().UpdateSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE, OutputProjectPathToKadOutputFile(messager, the_file.toStdString()));
		QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
		if (editControl)
		{
			editControl->setText(the_file);
		}
	}

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
        if (! the_path.endsWith(".csv", Qt::CaseInsensitive))
        {
            the_path += ".csv";
        }
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
				// No lock - not necessary for a boolean whose read/write is guaranteed to be in proper sequence
				OutputModel::OutputGenerator::cancelled = true;
			}
		}
	}
}
