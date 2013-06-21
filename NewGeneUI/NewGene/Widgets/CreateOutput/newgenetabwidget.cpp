#include "newgenetabwidget.h"

#include <QTabBar>

NewGeneTabWidget::NewGeneTabWidget( QWidget * parent ) :
	QTabWidget( parent ),
	NewGeneWidget( NewGeneWidget::WIDGET_NATURE_OUTPUT_WIDGET, this ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
}

void NewGeneTabWidget::NewGeneUIInitialize()
{
	QTabBar * pTB = tabBar();

	if ( pTB )
	{
		pTB->setDrawBase( false );
	}
}
