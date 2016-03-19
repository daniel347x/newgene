#include "newgenevariablestoolbox.h"
#include "../../../newgenemainwindow.h"
#include "../../../../Infrastructure/Project/uiprojectmanager.h"

#include <QLayout>
#include <QPalette>
#include <QAbstractButton>
#include <string>

NewGeneVariablesToolbox::NewGeneVariablesToolbox(QWidget * parent) :
	QToolBox(parent),
	NewGeneWidget(WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_TOOLBOX,
									 true))   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
	layout()->setSpacing(4);
	setStyleSheet("QToolBox::Tab:selected {font-weight: bold;}");

	PrepareOutputWidget();
}

void NewGeneVariablesToolbox::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)));

		// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)));

		// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), this,
				SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));

		// Handle clicks on different variable group tabs
		connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChange(int)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}

void NewGeneVariablesToolbox::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (inp)
		{
			inp->UnregisterInterestInChanges(this);
		}
	}
}

void NewGeneVariablesToolbox::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}

	WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX widget_data)
{

	WidgetInstanceIdentifier currentVG;
	NewGeneVariableGroup * vg { dynamic_cast<NewGeneVariableGroup *>(currentWidget()) };
	if (vg)
	{
		currentVG = vg->data_instance;
	}

	Empty();

	int currentIndex = 0;
	int index = 0;
	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](std::pair<WidgetInstanceIdentifier, std::string> const & identifier_)
	{
		WidgetInstanceIdentifier const & identifier {identifier_.first};
		std::string const & uoa_text {identifier_.second};
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			WidgetInstanceIdentifier new_identifier(identifier);
			NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup(this, this, new_identifier, outp);
			std::string displayText {identifier.longhand->c_str()};
			displayText += " (";
			displayText += uoa_text;
			displayText += ")";
			addItem(tmpGrp, displayText.c_str());

			if (!currentVG.IsEmpty() && new_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, currentVG))
			{
				currentIndex = index;
			}

			++index;
		}
	});

	setCurrentIndex(currentIndex);

}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget_data)
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

void NewGeneVariablesToolbox::Empty()
{
	int nItems = count();

	for (int n = 0; n < nItems; ++n)
	{
		QWidget * child = widget(n);
		removeItem(n);

		if (child)
		{
			delete child;
			child = nullptr;
		}

		--n;
		--nItems;
	}
}

void NewGeneVariablesToolbox::HandleChanges(DataChangeMessage const & change_message)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();

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
								//if (change.parent_identifier.code && change.parent_identifier.uuid && change.parent_identifier.longhand)
								//{
								//	WidgetInstanceIdentifier new_identifier = change.parent_identifier;
								//	NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup(this, this, new_identifier, outp);
								//	addItem(tmpGrp, new_identifier.longhand->c_str());
								//}

								RefreshAllWidgets(); // this triggers a resort by loading everything in the pane again
							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{

									WidgetInstanceIdentifier vg_to_remove(change.parent_identifier);

									int nItems = count();

									for (int n = 0; n < nItems; ++n)
									{
										QWidget * testWidget = widget(n);

										try
										{
											NewGeneVariableGroup * testVG = dynamic_cast<NewGeneVariableGroup *>(testWidget);

											if (testVG && testVG->data_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg_to_remove))
											{

												removeItem(n);
												delete testVG;
												testVG = nullptr;

												break;

											}
										}
										catch (std::bad_cast &)
										{
											// guess not
										}

									}

								}

							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// Should never receive this.
							}

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								RefreshAllWidgets();
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

QListView * NewGeneVariablesToolbox::GetListView(int const index)
{
	NewGeneVariableGroup * vg = static_cast<NewGeneVariableGroup *>(widget(index));
	return vg->GetListView();
}

void NewGeneVariablesToolbox::SetBarColor(bool active, std::string const & name)
{
	int nItems = count();
	int index { -1};

	for (int n = 0; n < nItems; ++n)
	{
		QWidget * testWidget = widget(n);
		std::string testName {testWidget->objectName().toStdString()};

		if (testName == name)
		{
			index = n;
			break;
		}
	}

	if (index >= 0)
	{
		int i {};

		foreach (QAbstractButton * button, findChildren<QAbstractButton *>())
		{
			// make sure only toolbox button palettes are modified
			if (button->metaObject()->className() == QString("QToolBoxButton"))
			{
				if (i == index)
				{
					// found correct button
					QPalette p = button->palette();

					if (active)
					{
						p.setColor(QPalette::Button, "#AFD4F6");
					}
					else
					{
						p.setColor(QPalette::Button, "#ECECEC");
					}

					button->setPalette(p);
					break;
				}

				i++;
			}
		}
	}
}

void NewGeneVariablesToolbox::tabChange(int index)
{
	if (index != -1)
	{
		NewGeneVariableGroup * vg { dynamic_cast<NewGeneVariableGroup *>(widget(index)) };

		if (vg)
		{
			emit DoTabChange(vg->data_instance);
		}
	}
}
