
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"
#include "uistatusmanager.h"
#include "../../../NewGeneBackEnd/Utilities/UUID.h"
#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

NewGeneMainWindow * NewGeneWidget::theMainWindow = nullptr;

NewGeneWidget::NewGeneWidget( WidgetCreationInfo const & creation_info )
	: self( creation_info.self )
	, widget_nature(creation_info.widget_nature)
	, uuid(newUUID())
	, uuid_parent(creation_info.uuid_parent)
	, inp(nullptr)
	, outp(nullptr)
	, widget_type(creation_info.widget_type)
	, data_instance(creation_info.data_instance)
	, top_level(creation_info.top_level)
{
}

NewGeneWidget::~NewGeneWidget()
{
	if (outp)
	{
		outp->UnregisterInterestInChanges(this);
	}
	if (inp)
	{
		inp->UnregisterInterestInChanges(this);
	}

	if (inp)
	{
		inp->RemoveWidgetFromUUIDMap(uuid);
		if (data_instance.uuid)
		{
			inp->RemoveWidgetDataItemFromUUIDMap(uuid_parent, *data_instance.uuid);
		}
	}
	if (outp)
	{
		outp->RemoveWidgetFromUUIDMap(uuid);
		if (data_instance.uuid)
		{
			outp->RemoveWidgetDataItemFromUUIDMap(uuid_parent, *data_instance.uuid);
		}
		UIInputProject * _inp = outp->getUIInputProject();
		if (_inp)
		{
			_inp->UnregisterInterestInChanges(this);
			_inp->RemoveWidgetFromUUIDMap(uuid);
			if (data_instance.uuid)
			{
				_inp->RemoveWidgetDataItemFromUUIDMap(uuid_parent, *data_instance.uuid);
			}
		}
	}
}

void NewGeneWidget::PrepareInputWidget(bool const also_link_output)
{
	if (self == nullptr)
	{
		return;
	}
	self->connect((QObject*)&projectManagerUI(), SIGNAL(UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIInputProject *)), self, SLOT(UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIInputProject *)));
	if (also_link_output)
	{
		self->connect((QObject*)&projectManagerUI(), SIGNAL(UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)), self, SLOT(UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)));
	}
}

void NewGeneWidget::PrepareOutputWidget()
{
	if (self == nullptr)
	{
		return;
	}
	self->connect((QObject*)&projectManagerUI(), SIGNAL(UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)), self, SLOT(UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)));
	self->connect((QObject*)&projectManagerUI(), SIGNAL(UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIInputProject *)), self, SLOT(UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE, UIInputProject *)));
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
	return *theMainWindow; // FIX THIS!!!
}

// Be sure to call this only in the context of the UI thread
void NewGeneWidget::ShowMessageBox(std::string msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}

void NewGeneWidget::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	if (IsOutputProjectWidget())
	{
		if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
		{
			Empty();
			return;
		}
	}

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project && project == inp)
		{
			inp->RemoveWidgetFromUUIDMap(uuid);
			if (data_instance.uuid)
			{
				inp->RemoveWidgetDataItemFromUUIDMap(uuid_parent, *data_instance.uuid);
			}
		}
		inp = nullptr;

		// TODO: release connections here
	}

	else if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		inp = project;

		if (!inp)
		{
			statusManagerUI().PostStatus( "Input project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		try
		{
			inp->AddWidgetUUIDToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid);
			if (data_instance.uuid)
			{
				inp->AddWidgetDataItemUUIDToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid_parent, *data_instance.uuid);
			}
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Unable to obtain proper widget during construction: %1%");
			msg % bc.what();
			statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}

		if (IsInputProjectWidget() && IsTopLevel())
		{
			self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
		}
	}

}

void NewGeneWidget::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (project && project == outp)
		{
			outp->RemoveWidgetFromUUIDMap(uuid);
			if (data_instance.uuid)
			{
				outp->RemoveWidgetDataItemFromUUIDMap(uuid_parent, *data_instance.uuid);
			}
		}
		outp = nullptr;

		// TODO: release connections here
	}

	else if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp = project;

		if (!outp)
		{
			statusManagerUI().PostStatus( "Output project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		try
		{
			outp->AddWidgetUUIDToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid);
			if (data_instance.uuid)
			{
				outp->AddWidgetDataItemUUIDToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid_parent, *data_instance.uuid);
			}
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Unable to obtain proper widget during construction: %1%");
			msg % bc.what();
			statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}

		if (IsOutputProjectWidget() && IsTopLevel())
		{
			self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
		}
	}


}

bool NewGeneWidget::IsInputProjectWidget() const
{
	return widget_nature == WIDGET_NATURE_INPUT_WIDGET;
}

bool NewGeneWidget::IsOutputProjectWidget() const
{
	return widget_nature == WIDGET_NATURE_OUTPUT_WIDGET;
}
