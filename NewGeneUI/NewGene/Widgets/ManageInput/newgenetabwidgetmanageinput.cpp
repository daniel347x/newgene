#include "newgenetabwidgetmanageinput.h"

#include <QTabBar>

NewGeneTabWidgetManageInput::NewGeneTabWidgetManageInput( QWidget * parent ) :
	QTabWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET) ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
}

void NewGeneTabWidgetManageInput::NewGeneUIInitialize()
{
	QTabBar * pTB = tabBar();

	if ( pTB )
	{
		pTB->setDrawBase( false );
	}
}
