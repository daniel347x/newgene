#include "newgenevariablestoolbox.h"
#include "ui_newgenevariablestoolbox.h"

NewGeneVariablesToolbox::NewGeneVariablesToolbox(QWidget *parent) :
    QToolBox(parent),
    ui(new Ui::NewGeneVariablesToolbox)
{
    ui->setupUi(this);

    removeItem(0);


}

NewGeneVariablesToolbox::~NewGeneVariablesToolbox()
{
    delete ui;
}

void NewGeneVariablesToolbox::changeEvent(QEvent *e)
{
    QToolBox::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
