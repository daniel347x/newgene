#include "timerangebox.h"
#include "ui_timerangebox.h"

#include <QCheckBox>
#include <QLineEdit>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

TimeRangeBox::TimeRangeBox( QWidget * parent ) :
	QFrame( parent ),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, TIMERANGE_REGION_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::TimeRangeBox )
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

TimeRangeBox::~TimeRangeBox()
{
	delete ui;
}

void TimeRangeBox::changeEvent( QEvent * e )
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

void TimeRangeBox::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		this->show();
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET)));
		connect(this, SIGNAL(UpdateDoRandomSampling(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE)));
		connect(this, SIGNAL(UpdateKadSamplerCount(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE)));
		connect(this, SIGNAL(UpdateConsolidateRows(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE)));
		connect(this, SIGNAL(UpdateDisplayAbsoluteTimeColumns(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		this->hide();
		Empty();
	}

}

void TimeRangeBox::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
}

void TimeRangeBox::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_TIMERANGE_REGION_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void TimeRangeBox::WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET widget_data)
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

}

void TimeRangeBox::on_doRandomSampling_stateChanged(int)
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

void TimeRangeBox::on_randomSamplingHowManyRows_textChanged(const QString &)
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
		emit UpdateKadSamplerCount(action_request);
	}

}

void TimeRangeBox::on_mergeIdenticalRows_stateChanged(int arg1)
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

void TimeRangeBox::on_displayAbsoluteTimeColumns_stateChanged(int arg1)
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
