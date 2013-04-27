#include "newgenevariablesummarygroupbox.h"
#include "ui_newgenevariablesummarygroupbox.h"

int NewGeneVariableSummaryGroupBox::index_ = 0;

NewGeneVariableSummaryGroupBox::NewGeneVariableSummaryGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::NewGeneVariableSummaryGroupBox)
{

    ui->setupUi(this);
    myIndex_ = index_;
    ++index_;

}

NewGeneVariableSummaryGroupBox::~NewGeneVariableSummaryGroupBox()
{
    delete ui;
}

void NewGeneVariableSummaryGroupBox::changeEvent(QEvent *e)
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
