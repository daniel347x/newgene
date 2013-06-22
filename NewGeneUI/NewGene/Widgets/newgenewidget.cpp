
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
	if (IsInputProjectWidget())
	{
		if (!inp)
		{
			statusManagerUI().PostStatus( "Input project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		inp->AddWidgetToUUIDMap(this, uuid);
	}
	if (IsOutputProjectWidget())
	{
		if (!outp)
		{
			statusManagerUI().PostStatus( "Output project is unavailable.", UIStatusManager::IMPORTANCE_HIGH, true );
			return;
		}
		outp->AddWidgetToUUIDMap(this, uuid);
	}
}

NewGeneWidget::~NewGeneWidget()
{
	if (IsInputProjectWidget())
	{
		inp->RemoveWidgetFromUUIDMap(uuid);
	}
	if (IsOutputProjectWidget())
	{
		outp->RemoveWidgetFromUUIDMap(uuid);
	}
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
	inp = project;
	self->connect(self, SIGNAL(RefreshWidget(DATA_WIDGETS)), inp->getConnector(), SLOT(RefreshWidget(DATA_WIDGETS)));
	self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
}

void NewGeneWidget::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	outp = project;
	self->connect(self, SIGNAL(RefreshWidget(DATA_WIDGETS)), outp->getConnector(), SLOT(RefreshWidget(DATA_WIDGETS)));
	self->connect(project, SIGNAL(RefreshAllWidgets()), self, SLOT(RefreshAllWidgets()));
}

bool NewGeneWidget::IsInputProjectWidget() const
{
	return widget_nature == WIDGET_NATURE_INPUT_WIDGET;
}

bool NewGeneWidget::IsOutputProjectWidget() const
{
	return widget_nature == WIDGET_NATURE_OUTPUT_WIDGET;
}
