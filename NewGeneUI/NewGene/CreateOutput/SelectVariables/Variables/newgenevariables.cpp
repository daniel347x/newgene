#include "newgenevariables.h"
#include "ui_newgenevariables.h"

#include <QToolBox>
#include <QListView>

NewGeneVariables::NewGeneVariables(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariables)
{

    ui->setupUi(this);

    QToolBox * pTB = findChild<QToolBox*>("toolBox");
    if (pTB)
    {
        QWidget * pPage = pTB->findChild<QWidget*>("page");
        if (pPage)
        {
            QListView * pListView = pPage->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("Country name");
                stringList_.append("Country abbreviation");
                stringList_.append("Major Power Status");
                stringList_.append("Home Region");
                stringList_.append("Capabilities (CINC score)");
                model_.setStringList(stringList_);
                pListView->setModel(&model_);
            }
        }
    }

}

NewGeneVariables::~NewGeneVariables()
{
    delete ui;
}

void NewGeneVariables::changeEvent(QEvent *e)
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
