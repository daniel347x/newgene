#include "newgenevariables.h"
#include "ui_newgenevariables.h"

NewGeneVariables::NewGeneVariables(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariables)
{
    ui->setupUi(this);
}

NewGeneVariables::~NewGeneVariables()
{
    delete ui;
}

void NewGeneVariables::changeEvent(QEvent *e)
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
