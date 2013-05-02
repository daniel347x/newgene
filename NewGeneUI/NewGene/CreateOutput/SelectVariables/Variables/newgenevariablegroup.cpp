#include "newgenevariablegroup.h"
#include "ui_newgenevariablegroup.h"

NewGeneVariableGroup::NewGeneVariableGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariableGroup)
{
    ui->setupUi(this);
}

NewGeneVariableGroup::~NewGeneVariableGroup()
{
    delete ui;
}

void NewGeneVariableGroup::changeEvent(QEvent *e)
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
