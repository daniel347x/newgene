/********************************************************************************
** Form generated from reading UI file 'newgenevariablegroup.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLEGROUP_H
#define UI_NEWGENEVARIABLEGROUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariableGroup
{
public:
    QGridLayout *gridLayout;
    QLabel *label;

    void setupUi(QWidget *NewGeneVariableGroup)
    {
        if (NewGeneVariableGroup->objectName().isEmpty())
            NewGeneVariableGroup->setObjectName(QStringLiteral("NewGeneVariableGroup"));
        NewGeneVariableGroup->resize(400, 300);
        gridLayout = new QGridLayout(NewGeneVariableGroup);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(NewGeneVariableGroup);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(NewGeneVariableGroup);

        QMetaObject::connectSlotsByName(NewGeneVariableGroup);
    } // setupUi

    void retranslateUi(QWidget *NewGeneVariableGroup)
    {
        NewGeneVariableGroup->setWindowTitle(QApplication::translate("NewGeneVariableGroup", "Form", 0));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariableGroup: public Ui_NewGeneVariableGroup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLEGROUP_H
