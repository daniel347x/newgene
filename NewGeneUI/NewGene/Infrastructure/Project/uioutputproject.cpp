#include "uioutputproject.h"
#include "uiinputmodel.h"

void UIOutputProject::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
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

}

void UIOutputProject::DoRefreshAllWidgets()
{
	emit RefreshAllWidgets();
}
