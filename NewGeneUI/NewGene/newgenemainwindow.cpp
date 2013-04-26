#include "newgenemainwindow.h"
#include "ui_newgenemainwindow.h"

NewGeneMainWindow::NewGeneMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewGeneMainWindow)
{
    ui->setupUi(this);
    NewGeneTabWidget * pTW = findChild<NewGeneTabWidget*>("tabWidgetMain");
    if (pTW)
    {
        pTW->NewGeneInitialize();
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
