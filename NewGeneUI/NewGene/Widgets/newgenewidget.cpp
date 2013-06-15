
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"

NewGeneMainWindow * NewGeneWidget::theMainWindow = nullptr;

NewGeneWidget::NewGeneWidget( QWidget * self_ )
	: self( self_ )
	, inp(nullptr)
	, outp(nullptr)
{
}

void NewGeneWidget::PrepareInputWidget()
{
	if (self == nullptr)
	{
		return;
	}
	self->connect(&projectManagerUI(), SIGNAL(UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIInputProject *)), self, SLOT(UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE, UIInputProject *)));
}

void NewGeneWidget::PrepareOutputWidget()
{
	if (self == nullptr)
	{
		return;
	}
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
}

void NewGeneWidget::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	outp = project;
}
