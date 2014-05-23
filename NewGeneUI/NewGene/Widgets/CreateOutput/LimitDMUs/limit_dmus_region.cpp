#include "limit_dmus_region.h"
#include "ui_limit_dmus_region.h"

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

#include <QStandardItemModel>

limit_dmus_region::limit_dmus_region(QWidget *parent) :
	QWidget(parent),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, LIMIT_DMUS_TAB, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
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

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_LIMIT_DMUS_TAB)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}

void limit_dmus_region::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
			//project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (inp)
		{
			//inp->UnregisterInterestInChanges(this);
		}
	}
}

void limit_dmus_region::RefreshAllWidgets()
{

	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_LIMIT_DMUS_TAB request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);

}

void limit_dmus_region::WidgetDataRefreshReceive(WidgetDataItem_LIMIT_DMUS_TAB widget_data)
{

	Empty();

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listView_limit_dmus_top_pane)
	{
		boost::format msg("Invalid top pane list view in Limit DMUs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_top_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listView_limit_dmus_top_pane);

	std::vector<dmu_category_limit_members_info_tuple> & dmu_category_limit_members_info = widget_data.dmu_category_limit_members_info;

	int index = 0;
	for (auto & dmu_category_tuple : dmu_category_limit_members_info)
	{
		WidgetInstanceIdentifier const & dmu_category = std::get<0>(dmu_category_tuple);
		bool const is_limited = std::get<1>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_all = std::get<2>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_not_limited = std::get<3>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_limited = std::get<4>(dmu_category_tuple);
		if (dmu_category.code && !dmu_category.code->empty())
		{

			QStandardItem * item = new QStandardItem();
			std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			//v.setValue(dmu_and_members);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	};

	model->sort(0);

	ui->listView_limit_dmus_top_pane->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	//EmptyDmuMembersPane();

	//connect( ui->listView_dmus->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveDMUSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void limit_dmus_region::Empty()
{

	//    QLayoutItem *child;
//    while ((child = layout()->takeAt(0)) != 0)
//    {
//        delete child->widget();
//        delete child;
//    }

}


void limit_dmus_region::HandleChanges(DataChangeMessage const & change_message)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
	{
		switch (change.change_type)
		{
			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE:
				{
					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{

//                                if (change.parent_identifier.uuid && change.parent_identifier.code && change.parent_identifier.longhand)
//                                {
//                                    WidgetInstanceIdentifier new_identifier(change.parent_identifier);
//                                    NewGeneVariableSummaryGroup * tmpGrp = new NewGeneVariableSummaryGroup( this, new_identifier, outp );
//                                    tmpGrp->setTitle(new_identifier.longhand->c_str());
//                                    layout()->addWidget(tmpGrp);
//                                }

							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{

//                                    WidgetInstanceIdentifier vg_to_remove(change.parent_identifier);

//                                    int current_number = layout()->count();
//                                    bool found = false;
//                                    QWidget * widgetToRemove = nullptr;
//                                    QLayoutItem * layoutItemToRemove = nullptr;
//                                    int i = 0;
//                                    for (i=0; i<current_number; ++i)
//                                    {
//                                        QLayoutItem * testLayoutItem = layout()->itemAt(i);
//                                        QWidget * testWidget(testLayoutItem->widget());
//                                        try
//                                        {
//                                            NewGeneVariableSummaryGroup * testVG = dynamic_cast<NewGeneVariableSummaryGroup*>(testWidget);
//                                            if (testVG->data_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg_to_remove))
//                                            {
//                                                widgetToRemove = testVG;
//                                                layoutItemToRemove = testLayoutItem;
//                                                found = true;
//                                                break;
//                                            }
//                                        }
//                                        catch (std::bad_cast &)
//                                        {
//                                            // guess not
//                                        }

//                                    }

//                                    if (found && widgetToRemove != nullptr)
//                                    {
//                                        layout()->takeAt(i);
//                                        delete widgetToRemove;
//                                        delete layoutItemToRemove;
//                                        widgetToRemove = nullptr;
//                                        layoutItemToRemove = nullptr;
//                                    }

								}

							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// Should never receive this.
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// Ditto above.
							}
							break;

						default:
							{
							}
							break;

					}
				}
				break;
			default:
				{
				}
				break;
		}
	});

}
