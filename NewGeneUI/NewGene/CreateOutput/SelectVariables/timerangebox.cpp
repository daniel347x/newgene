#include "timerangebox.h"
#include "ui_timerangebox.h"

TimeRangeBox::TimeRangeBox(QWidget *parent) :
    QFrame(parent),
    NewGeneWidget(this), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
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
