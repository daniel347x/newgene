#include "newgenevariablesummarygroup.h"
#include "ui_newgenevariablesummarygroup.h"

NewGeneVariableSummaryGroup::NewGeneVariableSummaryGroup(QWidget *parent) :
    QGroupBox(parent),
    NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
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
