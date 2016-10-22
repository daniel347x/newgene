#include "kadwidgetsscrollarea.h"

#include "kadspinbox.h"
#include <QLayout>
#include <QFont>
#include <QGraphicsColorizeEffect>
#include <QLabel>
#include <QSize>
#include <QPalette>
#include <QPainter>
#include <QSpacerItem>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

#include "../Variables/newgenevariablestoolboxwrapper.h"
#include "../Variables/newgenevariablestoolbox.h"

KadWidgetsScrollArea::KadWidgetsScrollArea(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, KAD_SPIN_CONTROLS_AREA,
									 true))   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{

	QBoxLayout * layout = new QBoxLayout(QBoxLayout::BottomToTop, this);
	layout->addSpacing(20);
	QBoxLayout * layoutInner = new QBoxLayout(QBoxLayout::LeftToRight, this);
	layoutInner->setMargin(0);
	layoutInner->addStretch();
	layoutInner->addStretch();
	QFrame * frameInner = new QFrame();
	frameInner->setLayout(layoutInner);
	layout->addWidget(frameInner);
	setLayout(layout);

	loading = false;

	PrepareOutputWidget();
}

void KadWidgetsScrollArea::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{

		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA)));

		// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA)));

		// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET)));

		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{

		cached_active_vg = WidgetInstanceIdentifier{};
		project->UnregisterInterestInChanges(this);
		Empty();

	}
}

void KadWidgetsScrollArea::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE, false, "");
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
			WidgetInstanceIdentifiers active_dmus(widget_data.active_dmus.cbegin(), widget_data.active_dmus.cend());
			AddKadSpinWidget(identifier, active_dmus);
		}

		if (identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, cached_active_vg))
		{
			DoTabChange(identifier); // pick up any metadata changes?
			DoVariableSelectionChange();
		}
	});

	EmptyTextCheck();

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
	//cached_active_vg = WidgetInstanceIdentifier{}; // No - this is sometimes called after the variable selection widget has loaded, but before this widget has begun loading - we need to save the cached value
	DoTabChange(cached_active_vg);

	QLayoutItem * child;

	QLayout * spinnerLayout = layout()->itemAt(1)->widget()->layout();

	while ((child = spinnerLayout->takeAt(0)) != 0)
	{
		delete child->widget();
		delete child;
	}

	try
	{
		QBoxLayout * boxLayout = dynamic_cast<QBoxLayout *>(spinnerLayout);

		if (boxLayout)
		{
			boxLayout->addStretch();
			boxLayout->addStretch();
		}
	}
	catch (std::bad_cast &)
	{
	}

	EmptyTextCheck();
	DoVariableSelectionChange();
}

void KadWidgetsScrollArea::HandleChanges(DataChangeMessage const & change_message)
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
			case DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{
								// no-op
							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{

									// See comments in the 'DoTabChange' function
									if (false)
									{
										WidgetInstanceIdentifier vg_to_remove(change.parent_identifier);
										if (vg_to_remove.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, cached_active_vg))
										{
											DoTabChange(WidgetInstanceIdentifier());
										}
									}
								}

								DoVariableSelectionChange();

							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// leave this line of code here so you know how to obtain the vg that was updated
								// in case you ever do need it
								WidgetInstanceIdentifier vg = change.parent_identifier;

								DoVariableSelectionChange();
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// Will be handled, instead, by a 'refresh all widgets' command,
								// although full infrastructure is in place to support, send, and receive here this more granular message
							}
							break;

						default:
							{

							}
							break;

					}
				}
				break;

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE:
				{
					switch (change.change_intention)
					{
						case DATA_CHANGE_INTENTION__ADD:
							{
								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{
									/* From the server arrives a list of DMU INSTANCES (USA, Russia, etc.), not DMUs (COUNTRY, etc.) -
									this is NOT what the function AddKadSpinWidget expects, so send empty for the second argument.
									HOWEVER, this line is only reached when the user has added a NEW DMU so the list is empty anyways;
									note that when this function is called, the new DMU is HIDDEN, which is as it should be,
									because it should only be shown when there are variables selected corresponding to VGs
									corresponding to UOAs corresponding to this new DMU, and there can't be when the DMU was just created,
									so the new DMU's Kad spin box should certainly always be hidden in this scenario */
									AddKadSpinWidget(change.parent_identifier, WidgetInstanceIdentifiers());
								}
							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{

									WidgetInstanceIdentifier uoa_to_remove(change.parent_identifier);

									QLayout * spinnerLayout = layout()->itemAt(1)->widget()->layout();

									int current_number = spinnerLayout->count();
									bool found = false;
									QWidget * widgetToRemove = nullptr;
									QLayoutItem * layoutItemToRemove = nullptr;
									int i = 0;

									for (i = 0; i < current_number; ++i)
									{
										QLayoutItem * testLayoutItem = spinnerLayout->itemAt(i);
										QWidget * testWidget(testLayoutItem->widget());

										try
										{
											KadSpinBox * testSpinBox = dynamic_cast<KadSpinBox *>(testWidget);

											if (testSpinBox && testSpinBox->data_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, uoa_to_remove))
											{
												widgetToRemove = testSpinBox;
												layoutItemToRemove = testLayoutItem;
												found = true;
												break;
											}
										}
										catch (std::bad_cast &)
										{
											// guess not
										}

									}

									if (found && widgetToRemove != nullptr)
									{
										spinnerLayout->takeAt(i);
										delete widgetToRemove;
										delete layoutItemToRemove;
										widgetToRemove = nullptr;
										layoutItemToRemove = nullptr;

										EmptyTextCheck();

										Resequence();

										DoVariableSelectionChange();
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

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION:
				{
					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
							{
								// This is the OUTPUT model changing.
								// "Add" means to simply add an item that is CHECKED (previously unchecked) -
								// NOT to add a new variable.  That would be input model change type.
								DoVariableSelectionChange();
							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// Should never receive this.
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								DoVariableSelectionChange();
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

void KadWidgetsScrollArea::AddKadSpinWidget(WidgetInstanceIdentifier const & identifier, WidgetInstanceIdentifiers const & active_dmus)
{

	// Only called when an actual DMU is being ADDED to the input dataset!!
	// (or on full refresh)

	// Remove the stretch at the end
	QLayoutItem * stretcher {};

	QLayout * spinnerLayout = layout()->itemAt(1)->widget()->layout();

	if (spinnerLayout->count() && (stretcher = spinnerLayout->takeAt(spinnerLayout->count() - 1)) != 0)
	{
		try
		{
			QSpacerItem * spacer { dynamic_cast<QSpacerItem *>(stretcher) };

			if (spacer)
			{
				delete stretcher->widget();
				delete stretcher;
			}
		}
		catch (std::bad_cast &)
		{

		}
	}

	WidgetInstanceIdentifier new_identifier(identifier);
	KadSpinBox * newSpinBox = new KadSpinBox(this, new_identifier, outp);
	newSpinBox->setFixedHeight(20);
	newSpinBox->setFixedWidth(200);
	QFont currFont = newSpinBox->font();
	currFont.setPixelSize(11);
	newSpinBox->setFont(currFont);
	std::string stylesheet(" QSpinBox { color: #333333; font-weight: bold; } ");
	newSpinBox->setStyleSheet(stylesheet.c_str());
	std::string prefixText(" #");
	prefixText += *identifier.longhand;
	prefixText += " cols:  ";
	newSpinBox->setPrefix(prefixText.c_str());
	bool not_me = true;
	std::for_each(active_dmus.cbegin(), active_dmus.cend(), [&](WidgetInstanceIdentifier const & the_dmu)
	{
		if (identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu))
		{
			not_me = false;
		}
	});

	// DMUs for ALL UOAs corresponding to ALL variable groups EXIST,
	// but only those corresponding to UOAs corresponding to variable groups with SELECTED variables
	// are VISIBLE.
	if (not_me)
	{
		newSpinBox->doSetVisible(false);
	}
	else
	{
		newSpinBox->doSetVisible(true);
	}

	spinnerLayout->addWidget(newSpinBox);

	try
	{
		QBoxLayout * boxLayout = dynamic_cast<QBoxLayout *>(spinnerLayout);

		if (boxLayout)
		{
			boxLayout->addStretch();
		}
	}
	catch (std::bad_cast &)
	{
	}

	EmptyTextCheck();

	Resequence();
}

void KadWidgetsScrollArea::ShowLoading(bool const loading_)
{
	loading = loading_;
	EmptyTextCheck();
}

void KadWidgetsScrollArea::EmptyTextCheck()
{

	QLabel * emptySpinsLabel { findChild<QLabel *>("emptyKadsLabel") };
	QLabel * noOutputProjectLabel { findChild<QLabel *>("noOutputProjectLabel") };
	QFrame * frameLoadingDataset { findChild<QFrame *>("frameLoadingDataset") };

	if (loading)
	{
		if (emptySpinsLabel)
		{
			emptySpinsLabel->setVisible(false);
		}

		if (noOutputProjectLabel)
		{
			noOutputProjectLabel->setVisible(false);
		}

		if (frameLoadingDataset)
		{
			frameLoadingDataset->setVisible(true);
		}

		return;
	}

	if (frameLoadingDataset)
	{
		frameLoadingDataset->setVisible(false);
	}

	QLayout * spinnerLayout = layout()->itemAt(1)->widget()->layout();

	int current_number = spinnerLayout->count();
	bool any_spincontrols_visible = false;
	int i = 0;

	for (i = 0; i < current_number; ++i)
	{
		QLayoutItem * testLayoutItem = spinnerLayout->itemAt(i);
		QWidget * testWidget(testLayoutItem->widget());

		try
		{
			KadSpinBox * testSpinBox = dynamic_cast<KadSpinBox *>(testWidget);

			if (testSpinBox && testSpinBox->visible)
			{
				any_spincontrols_visible = true;
				break;
			}
		}
		catch (std::bad_cast &)
		{
			// this will catch throws for the "stretch" layout items, but who cares - it's infrequent
		}

	}

	bool noOutputProjectOpen = (outp == nullptr ? true : false);

	if (!noOutputProjectOpen)
	{
		if (emptySpinsLabel)
		{
			if (!any_spincontrols_visible)
			{
				emptySpinsLabel->setVisible(true);
			}
			else
			{
				emptySpinsLabel->setVisible(false);
			}
		}

		if (noOutputProjectLabel)
		{
			noOutputProjectLabel->setVisible(false);
		}
	}
	else
	{
		if (emptySpinsLabel)
		{
			emptySpinsLabel->setVisible(false);
		}

		if (noOutputProjectLabel)
		{
			noOutputProjectLabel->setVisible(true);
		}
	}

}

void KadWidgetsScrollArea::resizeEvent(QResizeEvent *)
{
	QLabel * emptySpinsLabel { findChild<QLabel *>("emptyKadsLabel") };

	if (emptySpinsLabel)
	{
		QSize mySize { size() };
		QSize labelSize { emptySpinsLabel->size() };
		emptySpinsLabel->move(mySize.width() / 2 - labelSize.width() / 2, mySize.height() / 3 - labelSize.height() / 2);
	}

	QLabel * noOutputProjectLabel { findChild<QLabel *>("noOutputProjectLabel") };

	if (noOutputProjectLabel)
	{
		QSize mySize { size() };
		QSize labelSize { noOutputProjectLabel->size() };
		noOutputProjectLabel->move(mySize.width() / 2 - labelSize.width() / 2, mySize.height() / 3 - labelSize.height() / 2);
	}

	QFrame * frameLoadingDataset { findChild<QFrame *>("frameLoadingDataset") };

	if (frameLoadingDataset)
	{
		QSize mySize { size() };
		QSize frameSize { frameLoadingDataset->size() };
		frameLoadingDataset->move(mySize.width() / 2 - frameSize.width() / 2, mySize.height() / 2 - frameSize.height() / 2);
	}

	QLabel * vgWarningLabel { findChild<QLabel *>("labelVariableGroupWarning") };

	if (vgWarningLabel)
	{
		QSize mySize { size() };
		QSize labelSize { vgWarningLabel->size() };
		vgWarningLabel->resize(mySize.width(), labelSize.height());
		vgWarningLabel->move(0, mySize.height() - labelSize.height());
	}

	EmptyTextCheck();
}

void KadWidgetsScrollArea::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void KadWidgetsScrollArea::Resequence()
{
	NewGeneVariablesToolboxWrapper * toolbox = this->parentWidget()->parentWidget()->parentWidget()->parentWidget()->findChild<NewGeneVariablesToolboxWrapper *>("toolbox");

	if (toolbox)
	{
		WidgetInstanceIdentifiers orderedDmus = toolbox->getDmuSequence();
		bool misordered = false;

		QLayout * spinnerLayout = layout()->itemAt(1)->widget()->layout();

		{
			int order = 0;

			for (auto orderedDmu : orderedDmus)
			{
				int current_number = layout()->count();

				for (int i = 0; i < current_number; ++i)
				{
					QLayoutItem * testLayoutItem = spinnerLayout->itemAt(i);
					QWidget * testWidget(testLayoutItem->widget());

					try
					{
						KadSpinBox * testSpinBox = dynamic_cast<KadSpinBox *>(testWidget);

						if (testSpinBox)
						{
							if (orderedDmu.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, testSpinBox->data_instance))
							{
								if (order != i)
								{
									misordered = true;
									break;
								}
							}
						}
					}
					catch (std::bad_cast &)
					{
						// this will catch throws for the "stretch" layout items, but who cares - it's infrequent
					}
				}

				if (misordered)
				{
					break;
				}

				++order;
			}
		}

		if (misordered)
		{
			// Remove the stretch at the end
			QLayoutItem * stretcher {};

			if (spinnerLayout->count() && (stretcher = spinnerLayout->takeAt(spinnerLayout->count() - 1)) != 0)
			{
				try
				{
					QSpacerItem * spacer { dynamic_cast<QSpacerItem *>(stretcher) };

					if (spacer)
					{
						delete stretcher->widget();
						delete stretcher;
					}
				}
				catch (std::bad_cast &)
				{

				}
			}

			std::vector<KadSpinBox *> cache;

			for (auto orderedDmu : orderedDmus)
			{
				int current_number = spinnerLayout->count();

				for (int i = 0; i < current_number; ++i)
				{
					QLayoutItem * testLayoutItem = spinnerLayout->itemAt(i);
					QWidget * testWidget(testLayoutItem->widget());

					try
					{
						KadSpinBox * testSpinBox = dynamic_cast<KadSpinBox *>(testWidget);

						if (testSpinBox)
						{
							if (orderedDmu.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, testSpinBox->data_instance))
							{
								cache.push_back(testSpinBox);
								spinnerLayout->removeWidget(testSpinBox);
								break;
							}
						}
					}
					catch (std::bad_cast &)
					{
						// this will catch throws for the "stretch" layout items, but who cares - it's infrequent
					}
				}
			}


			for (auto cachedWidget : cache)
			{
				spinnerLayout->addWidget(cachedWidget);
			}

			try
			{
				QBoxLayout * boxLayout = dynamic_cast<QBoxLayout *>(spinnerLayout);

				if (boxLayout)
				{
					boxLayout->addStretch();
				}
			}
			catch (std::bad_cast &)
			{
			}
		}
	}
}

QString KadWidgetsScrollArea::getFullWarningTextSingleVG(bool newline, WidgetInstanceIdentifier vg)
{
	QString vgWarningText;

	// See comments in the 'DoTabChange' function
	if (false)
	{
		if (!cached_active_vg.IsEmpty() && cached_active_vg.longhand && cached_active_vg.notes.notes1 && !cached_active_vg.notes.notes1->empty())
		{
			vgWarningText += "<b><FONT COLOR='#ff0000'>";
			vgWarningText += "&nbsp;Warning for <FONT COLOR='#1f3eba'>\"";
			vgWarningText += cached_active_vg.longhand->c_str();
			vgWarningText += "\":</b>";
			if (newline)
			{
				vgWarningText += "<br><br>";
			}
			else
			{
				vgWarningText += "&nbsp;&nbsp;";
			}
			vgWarningText += "<FONT COLOR='#000000'>";
			vgWarningText += cached_active_vg.notes.notes1->c_str();
		}
		else
		{
			vgWarningText += "You may set a warning message to appear here!\n\nTo do so, go to the 'Input dataset' -> 'Manage Variable Groups' tab.";
		}
	}

	if (true)
	{
		if (!vg.IsEmpty() && vg.longhand && vg.notes.notes1 && !vg.notes.notes1->empty())
		{
			vgWarningText += "<b><FONT COLOR='#ff0000'>";
			vgWarningText += "&nbsp;Warning for <FONT COLOR='#1f3eba'>\"";
			vgWarningText += vg.longhand->c_str();
			vgWarningText += "\":</b>";
			if (newline)
			{
				vgWarningText += "<br><br>";
			}
			else
			{
				vgWarningText += "&nbsp;&nbsp;";
			}
			vgWarningText += "<FONT COLOR='#000000'>";
			vgWarningText += vg.notes.notes1->c_str();
		}
	}

	return vgWarningText;
}

QString KadWidgetsScrollArea::getFullWarningTextAllVGs()
{
	QString vgWarningTextDefault = "You may set a warning message to appear here for selected variable groups!\n\nTo do so, go to the 'Input dataset' -> 'Manage Variable Groups' tab.";

	std::set<WidgetInstanceIdentifier> activeVGs;

	if (this->outp)
	{
		UIOutputProject * ui_output_project = this->outp;
		OutputModel & backendOutputModel = ui_output_project->model().backend();
		InputModel & backendInputModel = backendOutputModel.getInputModel();
		activeVGs = backendOutputModel.t_variables_selected_identifiers.GetActiveVGs(&backendOutputModel, &backendInputModel);
	}

	if (activeVGs.empty())
	{
		return vgWarningTextDefault;
	}

	if (activeVGs.size() == 1)
	{
		for (auto & vg : activeVGs)
		{
			QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
			if (vgWarningText.size() == 0)
			{
				return vgWarningTextDefault;
			}
			return vgWarningText;
		}
	}

	// More than 1 VG - how many actually have a warning?
	int howManyHaveWarnings {0};
	for (auto & vg : activeVGs)
	{
		QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
		if (vgWarningText.size() > 0)
		{
			++howManyHaveWarnings;
		}
	}

	if (howManyHaveWarnings == 0)
	{
		return vgWarningTextDefault;
	}

	if (howManyHaveWarnings == 1)
	{
		for (auto & vg : activeVGs)
		{
			QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
			if (vgWarningText.size() > 0)
			{
				return vgWarningText;
			}
		}
	}

	// More than 1 VG has a warning
	QString allWarnings;
	int numberAdded {0};
	for (auto & vg : activeVGs)
	{
		QString vgWarningText = this->getFullWarningTextSingleVG(true, vg);
		if (vgWarningText.size() > 0)
		{
			if (numberAdded > 0)
			{
				allWarnings += "<br><br>";
			}
			++numberAdded;
			allWarnings += vgWarningText;
		}
	}

	if (allWarnings.size() == 0)
	{
		return vgWarningTextDefault;
	}

	return allWarnings;
}

void KadWidgetsScrollArea::DoTabChange(WidgetInstanceIdentifier data)
{
	// The following code was in place for a brief time when the variable group warning
	// that appears in the blue box corresponded to the one and only variable group
	// currently EXPANDED in the left pane (the Select Variables pane).
	// Leave the code in place for future reference and in case we ever return to this GUI behavior.
	if (false)
	{
		cached_active_vg = data;
		QLabel * vgWarningLabel { findChild<QLabel *>("labelVariableGroupWarning") };
		if (vgWarningLabel)
		{
			if (!data.IsEmpty() && data.longhand && data.notes.notes1 && !data.notes.notes1->empty())
			{
				//QString vgWarningText = getFullWarningText(false);
				//vgWarningLabel->setText(vgWarningText);
			}
			else
			{
				vgWarningLabel->setText(QString());
			}
		}
	}
}

void KadWidgetsScrollArea::DoVariableSelectionChange()
{
	QLabel * vgWarningLabel { findChild<QLabel *>("labelVariableGroupWarning") };
	if (vgWarningLabel == nullptr)
	{
		return;
	}

	std::set<WidgetInstanceIdentifier> activeVGs;

	if (this->outp)
	{
		UIOutputProject * ui_output_project = this->outp;
		OutputModel & backendOutputModel = ui_output_project->model().backend();
		InputModel & backendInputModel = backendOutputModel.getInputModel();
		activeVGs = backendOutputModel.t_variables_selected_identifiers.GetActiveVGs(&backendOutputModel, &backendInputModel);
	}

	if (activeVGs.empty())
	{
		vgWarningLabel->setText(QString());
		return;
	}

	if (activeVGs.size() == 1)
	{
		for (auto & vg : activeVGs)
		{
			QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
			vgWarningLabel->setText(vgWarningText);
		}
		return;
	}

	// More than 1 VG - how many actually have a warning?
	int howManyHaveWarnings {0};
	for (auto & vg : activeVGs)
	{
		QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
		if (vgWarningText.size() > 0)
		{
			++howManyHaveWarnings;
		}
	}

	if (howManyHaveWarnings == 0)
	{
		vgWarningLabel->setText(QString());
		return;
	}

	if (howManyHaveWarnings == 1)
	{
		for (auto & vg : activeVGs)
		{
			QString vgWarningText = this->getFullWarningTextSingleVG(false, vg);
			if (vgWarningText.size() > 0)
			{
				vgWarningLabel->setText(vgWarningText);
			}
		}
		return;
	}

	// More than 1 VG has a warning
	QString vgWarningText = "<b><FONT COLOR='#ff0000'>Warnings for selected variable groups!  Click here</b>";
	vgWarningLabel->setText(vgWarningText);
}
