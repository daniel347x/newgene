#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"

NewGeneMainWindow::NewGeneMainWindow(QWidget *parent) :
    QMainWindow(parent),
    NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
    ui(new Ui::NewGeneMainWindow)
{

    ui->setupUi(this);

    NewGeneTabWidget * pTWmain = findChild<NewGeneTabWidget*>("tabWidgetMain");
    if (pTWmain)
    {
        pTWmain->NewGeneInitialize();
    }

}

NewGeneMainWindow::~NewGeneMainWindow()
{
    delete ui;
}

void NewGeneMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
