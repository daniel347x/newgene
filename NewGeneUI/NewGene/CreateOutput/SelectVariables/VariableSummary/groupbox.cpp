#include "groupbox.h"
#include "ui_groupbox.h"

GroupBox::GroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::GroupBox)
{
    ui->setupUi(this);
}

GroupBox::~GroupBox()
{
    delete ui;
}

void GroupBox::changeEvent(QEvent *e)
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
