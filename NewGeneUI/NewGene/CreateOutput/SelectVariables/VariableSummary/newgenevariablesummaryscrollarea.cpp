#include "newgenevariablesummaryscrollarea.h"
#include "ui_newgenevariablesummaryscrollarea.h"

NewGeneVariableSummaryScrollArea::NewGeneVariableSummaryScrollArea(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::NewGeneVariableSummaryScrollArea)
{
    ui->setupUi(this);
}

NewGeneVariableSummaryScrollArea::~NewGeneVariableSummaryScrollArea()
{
    delete ui;
}

void NewGeneVariableSummaryScrollArea::changeEvent(QEvent *e)
{
    QScrollArea::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
