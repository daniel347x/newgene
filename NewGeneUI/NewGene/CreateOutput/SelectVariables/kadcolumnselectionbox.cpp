#include "kadcolumnselectionbox.h"
#include "ui_kadcolumnselectionbox.h"

KAdColumnSelectionBox::KAdColumnSelectionBox(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::KAdColumnSelectionBox)
{
    ui->setupUi(this);
}

KAdColumnSelectionBox::~KAdColumnSelectionBox()
{
    delete ui;
}

void KAdColumnSelectionBox::changeEvent(QEvent *e)
{
    QFrame::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
