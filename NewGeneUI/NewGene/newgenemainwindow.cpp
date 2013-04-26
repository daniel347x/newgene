#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"

NewGeneMainWindow::NewGeneMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewGeneMainWindow)
{

    ui->setupUi(this);

    NewGeneTabWidget * pTWmain = findChild<NewGeneTabWidget*>("tabWidgetMain");
    if (pTWmain)
    {
        pTWmain->NewGeneInitialize();
    }

    NewGeneTabWidget * pTWoutput = findChild<NewGeneTabWidget*>("tabWidgetOutput");
    if (pTWoutput)
    {
        pTWoutput->NewGeneInitialize();
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
