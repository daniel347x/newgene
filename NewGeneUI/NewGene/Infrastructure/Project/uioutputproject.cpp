#include "uioutputproject.h"
#include "uiinputmodel.h"
#include "../../Widgets/newgenewidget.h"
#include "newgenemainwindow.h"
#include "newgenegenerateoutput.h"

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
	, output_pane(nullptr)
	, _inp(inp)
	{
		messager.set(this);
	}

void UIOutputProject::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
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
