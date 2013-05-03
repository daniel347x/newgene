#include "timerangebox.h"
#include "ui_timerangebox.h"

TimeRangeBox::TimeRangeBox(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TimeRangeBox)
{
    ui->setupUi(this);
}

TimeRangeBox::~TimeRangeBox()
{
    delete ui;
}

void TimeRangeBox::changeEvent(QEvent *e)
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
