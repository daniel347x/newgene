#include "newgenevariablesummary.h"
#include "ui_newgenevariablesummary.h"

#include "newgenevariablesummarygroupbox.h"
#include <QListView>
#include <QStringList>

NewGeneVariableSummary::NewGeneVariableSummary(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewGeneVariableSummary)
{

    ui->setupUi(this);

    NewGeneVariableSummaryHolder * pHolder1 = findChild<NewGeneVariableSummaryHolder*>("widgetHolder1");
    NewGeneVariableSummaryHolder * pHolder2 = findChild<NewGeneVariableSummaryHolder*>("widgetHolder2");
    NewGeneVariableSummaryHolder * pHolder3 = findChild<NewGeneVariableSummaryHolder*>("widgetHolder3");
    NewGeneVariableSummaryHolder * pHolder4 = findChild<NewGeneVariableSummaryHolder*>("widgetHolder4");
    NewGeneVariableSummaryHolder * pHolder5 = findChild<NewGeneVariableSummaryHolder*>("widgetHolder5");

    if (pHolder1)
    {
        NewGeneVariableSummaryGroupBox * pGroupBox = pHolder1->findChild<NewGeneVariableSummaryGroupBox*>("groupBox");
        if (pGroupBox)
        {
            pGroupBox->setTitle("Country");
            QListView * pListView = pGroupBox->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("Country name");
                stringList_.append("Country abbreviation");
                stringList_.append("Capabilities (CINC score)");
                model1.setStringList(stringList_);
                pListView->setModel(&model1);
            }
        }
    }

    if (pHolder2)
    {
        NewGeneVariableSummaryGroupBox * pGroupBox = pHolder2->findChild<NewGeneVariableSummaryGroupBox*>("groupBox");
        if (pGroupBox)
        {
            pGroupBox->setTitle("Geography");
            QListView * pListView = pGroupBox->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("Geography variable #1");
                stringList_.append("Geography variable #2");
                model2.setStringList(stringList_);
                pListView->setModel(&model2);
            }
        }
    }

    if (pHolder3)
    {
        NewGeneVariableSummaryGroupBox * pGroupBox = pHolder3->findChild<NewGeneVariableSummaryGroupBox*>("groupBox");
        if (pGroupBox)
        {
            pGroupBox->setTitle("Polity");
            QListView * pListView = pGroupBox->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("Polity variable #1");
                stringList_.append("Polity variable #2");
                stringList_.append("Polity variable #3");
                model3.setStringList(stringList_);
                pListView->setModel(&model3);
            }
        }
    }

    if (pHolder4)
    {
        NewGeneVariableSummaryGroupBox * pGroupBox = pHolder4->findChild<NewGeneVariableSummaryGroupBox*>("groupBox");
        if (pGroupBox)
        {
            pGroupBox->setTitle("MID");
            QListView * pListView = pGroupBox->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("MID Name");
                stringList_.append("Hostility Level");
                model4.setStringList(stringList_);
                pListView->setModel(&model4);
            }
        }
    }

    if (pHolder5)
    {
        NewGeneVariableSummaryGroupBox * pGroupBox = pHolder5->findChild<NewGeneVariableSummaryGroupBox*>("groupBox");
        if (pGroupBox)
        {
            pGroupBox->setTitle("MID Detail");
            QListView * pListView = pGroupBox->findChild<QListView*>("listView");
            if (pListView)
            {
                QStringList stringList_;
                stringList_.append("Start Date");
                stringList_.append("End Date");
                stringList_.append("Side A (boolean)");
                stringList_.append("Hostility Level");
                model5.setStringList(stringList_);
                pListView->setModel(&model5);
            }
        }
    }

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
