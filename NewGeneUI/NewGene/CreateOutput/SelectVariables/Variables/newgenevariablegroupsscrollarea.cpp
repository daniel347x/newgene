#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"

NewGeneVariableGroupsScrollArea::NewGeneVariableGroupsScrollArea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariableGroupsScrollArea)
{
    ui->setupUi(this);
}

NewGeneVariableGroupsScrollArea::~NewGeneVariableGroupsScrollArea()
{
    delete ui;
}

void NewGeneVariableGroupsScrollArea::changeEvent(QEvent *e)
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
