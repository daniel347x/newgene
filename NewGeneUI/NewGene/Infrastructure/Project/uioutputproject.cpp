#include "uioutputproject.h"
#include "uiinputmodel.h"
#include "../../Widgets/newgenewidget.h"
#include "newgenemainwindow.h"
#include "newgenegenerateoutput.h"
#include "../../Widgets/Utilities/dialoghelper.h"

#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QListView>

UIOutputProject::UIOutputProject(std::shared_ptr<UIOutputProjectSettings> const & project_settings,
								 std::shared_ptr<UIOutputModelSettings> const & model_settings,
								 std::shared_ptr<UIOutputModel> const & model,
								 QObject * mainWindowObject_,
								 QObject * parent,
								 UIMessagerOutputProject & messager_,
								 UIInputProject * inp)
	: QObject(parent)
	, UIProject(project_settings, model_settings, model, parent, messager_)
	, mainWindowObject(mainWindowObject_)
	, messager(messager_)
	, number_timerange_widgets_created(0)
	, _inp(inp)
{
	messager.set(this);
	is_input_project = false;
}

void UIOutputProject::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText(msg.c_str());
	msgBox.exec();
}

bool UIOutputProject::QuestionMessageBox(STD_STRING msg_title, STD_STRING msg_text)
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(nullptr, QString(msg_title.c_str()), QString(msg_text.c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

	if (reply == QMessageBox::Yes)
	{
		return true;
	}

	return false;
}

int UIOutputProject::OptionMessageBox(STD_STRING msg_title, STD_STRING msg_question, STD_VECTOR_WIDGETINSTANCEIDENTIFIER option_list)
{

	QDialog dialog;
	dialog.setWindowTitle(msg_title.c_str());
	dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
	QFormLayout form(&dialog);

	QWidget VgConstructionWidget;
	QVBoxLayout formOverall;
	QWidget VgConstructionPanes;
	QHBoxLayout formConstructionPane;
	QListView * listpane = nullptr;
	DialogHelper::AddTopLevelVariableGroupChooserBlock(dialog, form, VgConstructionWidget, formOverall, VgConstructionPanes, formConstructionPane, listpane, msg_question, option_list);

	if (!listpane)
	{
		boost::format msg("Unable to create \"Choose top-level variable group\" dialog.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return -1;
	}

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	WidgetInstanceIdentifier vg_to_use;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		std::string errorMsg;

		bool valid = true;

		if (valid)
		{

			// retrieve the VG to use as the primary, top-level VG
			QStandardItemModel * listpaneModel = static_cast<QStandardItemModel *>(listpane->model());

			if (listpaneModel == nullptr)
			{
				boost::format msg("Invalid list view items in \"Select VG to use as Primary\" popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QItemSelectionModel * listpane_selectionModel = listpane->selectionModel();

			if (listpane_selectionModel == nullptr)
			{
				boost::format msg("Invalid selection in Select Variable Group popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndexList selectedIndexes = listpane_selectionModel->selectedIndexes();

			if (selectedIndexes.empty())
			{
				// No selection
				return false;
			}

			if (selectedIndexes.size() > 1)
			{
				boost::format msg("Simultaneous selections not allowed.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndex selectedIndex = selectedIndexes[0];

			if (!selectedIndex.isValid())
			{
				boost::format msg("A variable group must be selected.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QVariant vg_variant = listpaneModel->item(selectedIndex.row())->data();
			vg_to_use = vg_variant.value<WidgetInstanceIdentifier>();

			dialog.accept();
		}

	});

	if (dialog.exec() != QDialog::Accepted)
	{
		return -1;
	}

	auto const found = std::find_if(option_list.cbegin(), option_list.cend(), [&](WidgetInstanceIdentifier const & test)
	{
		return test.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, vg_to_use);
	});

	if (found == option_list.cend())
	{
		boost::format msg("Selected variable group cannot be found.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return -1;
	}

	return found - option_list.cbegin();

}

// Note: The following function, and the preceding function, are very similar
// and COULD be templatized, but there are various steps required for the template functionality
// to work with Qt - specifically, the function 'AddTopLevelVariableGroupChooserBlock'
// in the above function (and equivalently in the following function)
// would need to correspond to a template argument, in turn requiring both a typedef
// and registering the type in the Qt system to work with Qt;
// it would be a function receiving all 9 arguments (because the dialog elements
// must remain in scope after that function returns) which is just fine,
// but a tad irritating to track through a function with 9 arguments passed
// as a template argument.
// Additionally, the equality check that occurs at the end would need to be
// a template function argument because a simple requirement that it have operator==
// is insufficient because WidgetInstanceIdentifier cannot have a comparison operator
// that simple due to the need for the first argument.
// Again, adding an additional template parameter is just fine,
// but the number of small and slightly irritating details just isn't worth the time
// for only a second instance of the following function,
// especially given that any future, third uses of the function might require yet more changes.
int UIOutputProject::StringOptionMessageBox(STD_STRING msg_title, STD_STRING msg_question, STD_VECTOR_STRING option_list)
{

	QDialog dialog;
	dialog.setWindowTitle(msg_title.c_str());
	dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
	QFormLayout form(&dialog);

	QWidget VgConstructionWidget;
	QVBoxLayout formOverall;
	QWidget VgConstructionPanes;
	QHBoxLayout formConstructionPane;
	QListView * listpane = nullptr;
	DialogHelper::AddStringChooserBlock(dialog, form, VgConstructionWidget, formOverall, VgConstructionPanes, formConstructionPane, listpane, msg_question, option_list);

	if (!listpane)
	{
		boost::format msg("Unable to create multiple choice dialog.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return -1;
	}

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string string_result;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		std::string errorMsg;

		bool valid = true;

		if (valid)
		{

			QStandardItemModel * listpaneModel = static_cast<QStandardItemModel *>(listpane->model());

			if (listpaneModel == nullptr)
			{
				boost::format msg("Invalid list view items in multiple selection popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QItemSelectionModel * listpane_selectionModel = listpane->selectionModel();

			if (listpane_selectionModel == nullptr)
			{
				boost::format msg("Invalid selection in multiple selection popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndexList selectedIndexes = listpane_selectionModel->selectedIndexes();

			if (selectedIndexes.empty())
			{
				// No selection
				return false;
			}

			if (selectedIndexes.size() > 1)
			{
				boost::format msg("Simultaneous selections not allowed.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndex selectedIndex = selectedIndexes[0];

			if (!selectedIndex.isValid())
			{
				boost::format msg("An option must be selected.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QVariant vg_variant = listpaneModel->item(selectedIndex.row())->data();
			string_result = vg_variant.value<std::string>();

			dialog.accept();
		}

	});

	if (dialog.exec() != QDialog::Accepted)
	{
		return -1;
	}

	auto const found = std::find_if(option_list.cbegin(), option_list.cend(), [&](std::string const & test)
	{
		return string_result == test;
	});

	if (found == option_list.cend())
	{
		boost::format msg("Selected option cannot be found.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return -1;
	}

	return found - option_list.cbegin();

}

bool UIOutputProject::is_model_equivalent(UIMessager & messager, UIOutputModel * model_)
{
	if (!model_)
	{
		return false;
	}

	InputModel & this_input_model = model().backend().getInputModel();
	InputModel & that_input_model = model_->backend().getInputModel();

	if (&this_input_model != &that_input_model)
	{
		// corresponding backend input models are not the same
		return false;
	}

	{
		boost::filesystem::path this_path = this_input_model.getPathToDatabaseFile();
		boost::filesystem::path that_path = that_input_model.getPathToDatabaseFile();

		try
		{
			bool this_exists = boost::filesystem::exists(this_path);
			bool that_exists = boost::filesystem::exists(that_path);

			if (this_exists != that_exists)
			{
				return false;
			}

			if (!this_exists && !that_exists)
			{
				return true;
			}

			boost::filesystem::path this_path_canonical = boost::filesystem::canonical(this_path);
			boost::filesystem::path that_path_canonical = boost::filesystem::canonical(that_path);

			if (!boost::filesystem::equivalent(this_path_canonical, that_path_canonical))
			{
				return false;
			}
		}
		catch (boost::filesystem::filesystem_error & ex)
		{
			boost::format msg("Error during output model equivalence determination in UIOutputProject evaluating input model database file pathnames (%1% vs. %2%): %3%");
			msg % this_path.string() % that_path.string() % ex.what();
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
			return false;
		}
	}

	if (&model() == model_ && &model().backend() == &model_->backend())
	{
		boost::filesystem::path this_path = model().backend().getPathToDatabaseFile();
		boost::filesystem::path that_path = model_->backend().getPathToDatabaseFile();

		try
		{
			if (this_path == boost::filesystem::path() || that_path == boost::filesystem::path())
			{
				return false;
			}

			boost::filesystem::path this_path_canonical = boost::filesystem::canonical(this_path);
			boost::filesystem::path that_path_canonical = boost::filesystem::canonical(that_path);

			if (!boost::filesystem::equivalent(this_path_canonical, that_path_canonical))
			{
				return false;
			}
		}
		catch (boost::filesystem::filesystem_error & ex)
		{
			boost::format msg("Error during output model equivalence determination in UIOutputProject evaluating input model database file pathnames (%1% vs. %2%): %3%");
			msg % this_path.string() % that_path.string() % ex.what();
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
			return false;
		}
	}

	return true;
}

void UIOutputProject::UpdateConnections()
{
	connect(getConnector(), SIGNAL(DataChangeMessageSignal(WidgetChangeMessages)), this, SLOT(DataChangeMessageSlot(WidgetChangeMessages)));
}

void UIOutputProject::DoRefreshAllWidgets()
{
	emit RefreshAllWidgets();
}

// Called in UI thread
void UIOutputProject::DataChangeMessageSlot(WidgetChangeMessages widget_change_messages)
{
	DisplayChanges(widget_change_messages);
}

void UIOutputProject::PassChangeMessageToWidget(NewGeneWidget * widget, DataChangeMessage const & change_message)
{
	widget->HandleChanges(change_message);
}

void UIOutputProject::PauseLists()
{
	NewGeneMainWindow * mainWindow = dynamic_cast<NewGeneMainWindow *>(mainWindowObject);
	if (mainWindow)
	{
		foreach (QListView * listPane, mainWindow->findChildren<QListView *>())
		{
			if (listPane->metaObject()->className() == QString("QListView"))
			{
				listPane->setUpdatesEnabled(false);
				QTimer::singleShot(2000, this, SLOT(UnpauseList(listPane)));
			}
		}
	}
}

void UIOutputProject::UnpauseList(QListView * listPane)
{
	// In the worst case - a race condition in which the user clicks on a heavy-hitting activity
	// a few seconds AFTER a previous heavy-hitting activity paused the lists -
	// the "disable list updates" triggered on the second heavy-hitting activity
	// will be undone by the "enable list updates" single-shot that expires from the first heavy-hitting activity.
	// This isn't great because the list view might hang things refreshing itself over and over,
	// but it's not a crash and it's pretty rare that will happen, so stick with this approach for now.
	listPane->setUpdatesEnabled(true);
	listPane->update();
}

void UIOutputProject::DoDeselectAllVariables()
{
	emit deselectAllVariables();
}
