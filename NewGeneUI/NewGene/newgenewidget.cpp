
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"

NewGeneMainWindow * NewGeneWidget::theMainWindow = nullptr;

NewGeneWidget::NewGeneWidget( QWidget * self_ ) :
	self( self_ )
{
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
