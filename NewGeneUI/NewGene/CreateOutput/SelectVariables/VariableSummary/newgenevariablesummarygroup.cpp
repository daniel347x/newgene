#include "newgenevariablesummarygroup.h"
#include "ui_newgenevariablesummarygroup.h"

NewGeneVariableSummaryGroup::NewGeneVariableSummaryGroup(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::NewGeneVariableSummaryGroup)
{
    ui->setupUi(this);
}

NewGeneVariableSummaryGroup::~NewGeneVariableSummaryGroup()
{
    delete ui;
}

void NewGeneVariableSummaryGroup::changeEvent(QEvent *e)
{
    QGroupBox::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
