#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

NewGeneGenerateOutput::NewGeneGenerateOutput(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, GENERATE_OUTPUT_TAB,
									 true)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneGenerateOutput)
{

	ui->setupUi(this);
	PrepareOutputWidget();
	connect(this, SIGNAL(SelectAndSetKadOutputPath()), ui->optionsBox, SLOT(SelectAndSetKadOutputPath()));
	connect(this, SIGNAL(EditingFinishedKadOutputPath()), ui->optionsBox, SLOT(EditingFinishedKadOutputPath()));

	setGenerateOutputPushbuttonClass("");
	ui->pushButton_cancel->setEnabled(false);
	ui->pushButtonGenerateOutput->setEnabled(false);

	this->setFocus();

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
		connect(this, SIGNAL(GenerateOutputSignal(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)), outp->getConnector(),
				SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT)));
		ui->pushButtonGenerateOutput->setEnabled(true);
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
		ui->pushButton_cancel->setEnabled(false);
		ui->pushButtonGenerateOutput->setEnabled(false);
	}

}

void NewGeneGenerateOutput::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
}

void NewGeneGenerateOutput::on_pushButtonGenerateOutput_clicked()
{

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem__GenerateOutput()))));
	WidgetActionItemRequest_ACTION_GENERATE_OUTPUT action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, actionItems);
	emit GenerateOutputSignal(action_request);

}

void NewGeneGenerateOutput::WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB)
{

	// Everything is delegated to the child OutputBox's implementation of this function

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
	QPlainTextEdit * edit_pane = findChild<QPlainTextEdit *>("plainTextEdit_KadStatus");

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
	QLabel * label_ = findChild<QLabel *>("labelOngoingPerformance");

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

	emit SelectAndSetKadOutputPath();

}

void NewGeneGenerateOutput::on_lineEditFilePathToKadOutput_editingFinished()
{

	emit EditingFinishedKadOutputPath();

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

void NewGeneGenerateOutput::ReceiveSignalSetRunStatus(int, RUN_STATUS_ENUM const runStatus)
{

	switch (runStatus)
	{
		case RUN_STATUS__RUNNING:
			{
				ui->pushButton_cancel->setEnabled(true);
				ui->pushButtonGenerateOutput->setEnabled(false);
				ui->pushButtonGenerateOutput->setText("Generating...");
				setGenerateOutputPushbuttonClass("active");
			}
			break;

		case RUN_STATUS__NOT_RUNNING:
			{
				ui->pushButton_cancel->setEnabled(false);
				ui->pushButtonGenerateOutput->setEnabled(true);
				ui->pushButtonGenerateOutput->setText("Generate Output");
				setGenerateOutputPushbuttonClass("");
			}
			break;

		default:
			break;
	}

}

void NewGeneGenerateOutput::setGenerateOutputPushbuttonClass(std::string const & theClass)
{

	ui->pushButtonGenerateOutput->setProperty("class", theClass.c_str());
	ui->pushButtonGenerateOutput->style()->unpolish(ui->pushButtonGenerateOutput);
	ui->pushButtonGenerateOutput->style()->polish(ui->pushButtonGenerateOutput);
	ui->pushButtonGenerateOutput->update();

}
