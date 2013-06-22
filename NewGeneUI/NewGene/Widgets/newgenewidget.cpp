
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"
#include "uistatusmanager.h"
#include "../../../NewGeneBackEnd/Utilities/UUID.h"

NewGeneMainWindow * NewGeneWidget::theMainWindow = nullptr;

NewGeneWidget::NewGeneWidget( WIDGET_NATURE const widget_nature_, QWidget * self_ )
	: self( self_ )
	, inp(nullptr)
	, outp(nullptr)
	, widget_type(WIDGET_TYPE_NONE)
	, uuid(newUUID())
	, widget_nature(widget_nature_)
{
}

NewGeneWidget::~NewGeneWidget()
{
}

void NewGeneWidget::PrepareInputWidget(DATA_WIDGETS widget_type_)
{
	if (self == nullptr)
	{
		return;
	}
	widget_type = widget_type_;
	self->connect(&projectManagerUI(), SIGNAL(UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIInputProject *)), self, SLOT(UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIInputProject *)));
}

void NewGeneWidget::PrepareOutputWidget(DATA_WIDGETS widget_type_)
{
	if (self == nullptr)
	{
		return;
	}
	widget_type = widget_type_;
	self->connect(&projectManagerUI(), SIGNAL(UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)), self, SLOT(UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIOutputProject *)));
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
//	QWidget * parent_ = self;
//	QWidget * parentTest = parent_;
//	while (parentTest)
//	{
//		parent_ = parentTest;
//		parentTest = parent_->parentWidget();
//	}
	//QWidget * pMainWindow = QApplication::activeWindow();
//	QWidget * pMainWindow = self->window();
//	pMainWindow = pMainWindow->window();
//	NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>( pMainWindow );
//	//NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>( parent_ );
	//return *pNewGeneMainWindow;
	return *theMainWindow; // FIX THIS!!!
}

void NewGeneWidget::UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	if (!IsInputProjectWidget())
	{
		boost::format msg("Attempting to connect an input project to a widget that has not registered as an input widget.");
		statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
		return;
	}

	if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project && project == inp)
		{
			inp->RemoveWidgetFromUUIDMap(uuid);
		}
		inp = nullptr;

		// TODO: release connections here
	}

	else if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		inp = project;

		if (!inp)
		{
			statusManagerUI().PostStatus( "Input project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		try
		{
			inp->AddWidgetToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid);
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Unable to obtain proper widget during construction: %1%");
			msg % bc.what();
			statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}

		self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
	}

}

void NewGeneWidget::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	if (!IsOutputProjectWidget())
	{
		boost::format msg("Attempting to connect an output project to a widget that has not registered as an output widget.");
		statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
		return;
	}

	if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (project && project == outp)
		{
			outp->RemoveWidgetFromUUIDMap(uuid);
		}
		outp = nullptr;

		// TODO: release connections here
	}

	else if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp = project;

		if (!outp)
		{
			statusManagerUI().PostStatus( "Output project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		try
		{
			outp->AddWidgetToUUIDMap(dynamic_cast<NewGeneWidget*>(self), uuid);
		}
		catch (std::bad_cast & bc)
		{
			boost::format msg("Unable to obtain proper widget during construction: %1%");
			msg % bc.what();
			statusManagerUI().PostStatus( msg.str().c_str(), UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}

		self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
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
