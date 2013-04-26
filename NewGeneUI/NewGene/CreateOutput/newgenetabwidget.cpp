#include "newgenetabwidget.h"

#include <QTabBar>

NewGeneTabWidget::NewGeneTabWidget(QWidget *parent) :
    QTabWidget(parent)
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
