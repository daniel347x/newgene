#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

#include "newgenetabwidget.h"

NewGeneCreateOutput::NewGeneCreateOutput(QWidget *parent) :
    QWidget(parent),
    NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
    ui(new Ui::NewGeneCreateOutput)
{

    ui->setupUi(this);

    NewGeneTabWidget * pTWoutput = findChild<NewGeneTabWidget*>("tabWidgetOutput");
    if (pTWoutput)
    {
        pTWoutput->NewGeneUIInitialize();
    }

}

NewGeneCreateOutput::~NewGeneCreateOutput()
{
    delete ui;
}

void NewGeneCreateOutput::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
