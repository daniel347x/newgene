#include "newgenevariablesummaryholder.h"
#include "ui_newgenevariablesummaryholder.h"

NewGeneVariableSummaryHolder::NewGeneVariableSummaryHolder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariableSummaryHolder)
{
    ui->setupUi(this);
}

NewGeneVariableSummaryHolder::~NewGeneVariableSummaryHolder()
{
    delete ui;
}

void NewGeneVariableSummaryHolder::changeEvent(QEvent *e)
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
