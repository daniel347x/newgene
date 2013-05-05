#include "newgenetabwidget.h"

#include <QTabBar>

NewGeneTabWidget::NewGeneTabWidget(QWidget *parent) :
    QTabWidget(parent),
    NewGeneWidget(this) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
}

void NewGeneTabWidget::NewGeneInitialize()
{
    QTabBar * pTB = tabBar();
    if (pTB)
    {
        pTB->setDrawBase(false);
    }
}
