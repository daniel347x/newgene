#include "optionsbox.h"
#include "ui_optionsbox.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QPlainTextEdit>

#include "globals.h"
#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"
#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

OptionsBox::OptionsBox( QWidget * parent ) :
	QFrame( parent ),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, TIMERANGE_REGION_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::OptionsBox )
{

	ui->setupUi( this );

	QLineEdit * lineEdit = this->findChild<QLineEdit*>("randomSamplingHowManyRows");
	if (lineEdit)
	{
		QIntValidator * val = new QIntValidator(0,2147483647, this);
		if (val)
		{
			lineEdit->setValidator(val);
		}
	}

	PrepareOutputWidget();

	this->hide();

}

OptionsBox::~OptionsBox()
{
	delete ui;
}

void OptionsBox::changeEvent( QEvent * e )
{
	QFrame::changeEvent( e );

	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;

		default:
			break;
	}
}

void OptionsBox::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		this->show();
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET)));
		connect(this, SIGNAL(UpdateDoRandomSampling(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)));
		connect(this, SIGNAL(UpdateRandomSamplingCount(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)));
		connect(this, SIGNAL(UpdateConsolidateRows(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE)));
		connect(this, SIGNAL(UpdateDisplayAbsoluteTimeColumns(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		this->hide();
		Empty();
	}

}

void OptionsBox::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
}

void OptionsBox::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_TIMERANGE_REGION_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void OptionsBox::WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET widget_data)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	bool do_random_sampling = widget_data.do_random_sampling;
	std::int64_t random_sampling_count_per_stage = widget_data.random_sampling_count_per_stage;
	bool consolidate_rows = widget_data.consolidate_rows;
	bool display_absolute_time_columns = widget_data.display_absolute_time_columns;

	{
		QCheckBox * checkBox = this->findChild<QCheckBox*>("doRandomSampling");
		if (checkBox)
		{
			bool is_checked = checkBox->isChecked();
			if (is_checked != do_random_sampling)
			{
				checkBox->setChecked(do_random_sampling);
			}
		}
	}

	{
		QLineEdit * lineEdit = this->findChild<QLineEdit*>("randomSamplingHowManyRows");
		if (lineEdit)
		{
			QString the_string = lineEdit->text();
			std::int64_t the_number = 1;
			if (the_string.size() > 0)
			{
				the_number = boost::lexical_cast<std::int64_t>(the_string.toStdString());
			}
			if (the_number != random_sampling_count_per_stage)
			{
				lineEdit->setText(QString((boost::lexical_cast<std::string>(random_sampling_count_per_stage).c_str())));
			}
		}
	}

	{
		QCheckBox * checkBox = this->findChild<QCheckBox*>("mergeIdenticalRows");
		if (checkBox)
		{
			bool is_checked = checkBox->isChecked();
			if (is_checked != consolidate_rows)
			{
				checkBox->setChecked(consolidate_rows);
			}
		}
	}

	{
		QCheckBox * checkBox = this->findChild<QCheckBox*>("displayAbsoluteTimeColumns");
		if (checkBox)
		{
			bool is_checked = checkBox->isChecked();
			if (is_checked != display_absolute_time_columns)
			{
				checkBox->setChecked(display_absolute_time_columns);
			}
		}
	}

    {
        setFilenameInSelectionEditBox();
    }

}

void OptionsBox::on_doRandomSampling_stateChanged(int)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	QCheckBox * checkBox = this->findChild<QCheckBox*>("doRandomSampling");
	if (checkBox)
	{
		bool is_checked = checkBox->isChecked();
		InstanceActionItems actionItems;
		actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Checkbox(is_checked)))));
		WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
		emit UpdateDoRandomSampling(action_request);
	}

}

void OptionsBox::on_randomSamplingHowManyRows_textChanged(const QString &)
{

	QLineEdit * lineEdit = this->findChild<QLineEdit*>("randomSamplingHowManyRows");
	if (lineEdit)
	{
		QString the_string = lineEdit->text();
		std::int64_t the_number = 1;
		if (the_string.size() > 0)
		{
			the_number = boost::lexical_cast<std::int64_t>(the_string.toStdString());
		}
		InstanceActionItems actionItems;
		actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Int64(the_number)))));
		WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
		emit UpdateRandomSamplingCount(action_request);
	}

}

void OptionsBox::on_mergeIdenticalRows_stateChanged(int arg1)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	QCheckBox * checkBox = this->findChild<QCheckBox*>("mergeIdenticalRows");
	if (checkBox)
	{
		bool is_checked = checkBox->isChecked();
		InstanceActionItems actionItems;
		actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Checkbox(is_checked)))));
		WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
		emit UpdateConsolidateRows(action_request);
	}

}

void OptionsBox::on_displayAbsoluteTimeColumns_stateChanged(int arg1)
{

    UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
    if (project == nullptr)
    {
        return;
    }

    QCheckBox * checkBox = this->findChild<QCheckBox*>("displayAbsoluteTimeColumns");
    if (checkBox)
    {
        bool is_checked = checkBox->isChecked();
        InstanceActionItems actionItems;
        actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Checkbox(is_checked)))));
        WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
        emit UpdateDisplayAbsoluteTimeColumns(action_request);
    }

}

void OptionsBox::SelectAndSetKadOutputPath()
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
        setFilenameInSelectionEditBox();
    }

}

void OptionsBox::setFilenameInSelectionEditBox()
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
    QLineEdit * editControl = this->findChild<QLineEdit*>("lineEditFilePathToKadOutput");
    if (editControl)
    {
        editControl->setText(current_file);
    }
}

void OptionsBox::EditingFinishedKadOutputPath()
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
