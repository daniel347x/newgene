#include "newgeneselectvariables.h"
#include "ui_newgeneselectvariables.h"

NewGeneSelectVariables::NewGeneSelectVariables(QWidget *parent) :
    QWidget(parent),
    NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
    ui(new Ui::NewGeneSelectVariables)
{
    ui->setupUi(this);
}

NewGeneSelectVariables::~NewGeneSelectVariables()
{
    delete ui;
}

void NewGeneSelectVariables::changeEvent(QEvent *e)
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
