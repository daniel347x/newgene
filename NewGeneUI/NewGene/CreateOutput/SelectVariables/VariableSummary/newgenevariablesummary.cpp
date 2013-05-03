#include "newgenevariablesummary.h"
#include "ui_newgenevariablesummary.h"

#include <QListView>
#include <QStringList>

NewGeneVariableSummary::NewGeneVariableSummary(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariableSummary)
{
    ui->setupUi(this);
}

NewGeneVariableSummary::~NewGeneVariableSummary()
{
    delete ui;
}

void NewGeneVariableSummary::changeEvent(QEvent *e)
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
