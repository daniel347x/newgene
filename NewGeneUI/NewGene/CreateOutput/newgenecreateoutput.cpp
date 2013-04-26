#include "newgenecreateoutput.h"
#include "ui_newgenecreateoutput.h"

NewGeneCreateOutput::NewGeneCreateOutput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneCreateOutput)
{
    ui->setupUi(this);
}

NewGeneCreateOutput::~NewGeneCreateOutput()
{
    delete ui;
}

void NewGeneCreateOutput::changeEvent(QEvent *e)
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
