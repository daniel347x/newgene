#include "uiinputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"
#include <QMessageBox>

void UIInputModel::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}

bool UIInputModel::is_model_equivalent(UIMessager & messager, UIInputModel * model_)
{
	if (!model_)
	{
		return false;
	}

	if (this == model_ && &backend() == &model_->backend())
	{
		boost::filesystem::path this_path = backend().getPathToDatabaseFile();
		boost::filesystem::path that_path = model_->backend().getPathToDatabaseFile();
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
			boost::format msg("Error during input model equivalence determination in UIInputModel evaluating model database file pathnames (%1% vs. %2%): %3%");
			msg % this_path.string() % that_path.string() % ex.what();
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
			return false;
		}
	}

	return true;
}

void UIInputModel::UpdateConnections()
{
}
