#include "newgenetabwidget.h"

#include <QTabBar>

NewGeneTabWidget::NewGeneTabWidget( QWidget * parent ) :
	QTabWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET) ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
}

void NewGeneTabWidget::NewGeneUIInitialize()
{
	QTabBar * pTB = tabBar();

	if ( pTB )
	{
        //pTB->setDrawBase( false );
	}
}

void NewGeneTabWidget::ReceiveSignalSetRunStatus(int, RUN_STATUS_ENUM const runStatus)
{

    switch (runStatus)
    {
        case RUN_STATUS__RUNNING:
            {
                setTabText(2, QString(" Running..."));
            }
            break;
        case RUN_STATUS__NOT_RUNNING:
            {
                setTabText(2, QString(" Prepare run"));
            }
            break;
        default:
            {
                setTabText(2, QString(" Prepare run"));
            }
            break;
    }

}
