#include "newgeneselectvariables.h"
#include "ui_newgeneselectvariables.h"

NewGeneSelectVariables::NewGeneSelectVariables(QWidget *parent) :
    QWidget(parent),
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
