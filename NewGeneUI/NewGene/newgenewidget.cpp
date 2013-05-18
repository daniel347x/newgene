
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"

NewGeneWidget::NewGeneWidget( QWidget * self_ ) :
	self( self_ )
{
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
	QWidget * pMainWindow = self->window();
	NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>( pMainWindow );
	return *pNewGeneMainWindow;
}
