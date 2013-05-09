/********************************************************************************
** Form generated from reading UI file 'newgenevariablesummarygroup.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLESUMMARYGROUP_H
#define UI_NEWGENEVARIABLESUMMARYGROUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariableSummaryGroup
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;

    void setupUi(QGroupBox *NewGeneVariableSummaryGroup)
    {
        if (NewGeneVariableSummaryGroup->objectName().isEmpty())
            NewGeneVariableSummaryGroup->setObjectName(QStringLiteral("NewGeneVariableSummaryGroup"));
        NewGeneVariableSummaryGroup->resize(400, 300);
        verticalLayout = new QVBoxLayout(NewGeneVariableSummaryGroup);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(NewGeneVariableSummaryGroup);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);


        retranslateUi(NewGeneVariableSummaryGroup);

        QMetaObject::connectSlotsByName(NewGeneVariableSummaryGroup);
    } // setupUi

    void retranslateUi(QGroupBox *NewGeneVariableSummaryGroup)
    {
        NewGeneVariableSummaryGroup->setWindowTitle(QApplication::translate("NewGeneVariableSummaryGroup", "GroupBox", 0));
        NewGeneVariableSummaryGroup->setTitle(QApplication::translate("NewGeneVariableSummaryGroup", "GroupBox", 0));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariableSummaryGroup: public Ui_NewGeneVariableSummaryGroup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLESUMMARYGROUP_H
