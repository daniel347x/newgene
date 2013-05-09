/********************************************************************************
** Form generated from reading UI file 'newgenevariablegroupsscrollarea.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLEGROUPSSCROLLAREA_H
#define UI_NEWGENEVARIABLEGROUPSSCROLLAREA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include <newgenevariablestoolboxwrapper.h>

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariableGroupsScrollArea
{
public:
    QGridLayout *gridLayout;
    NewGeneVariablesToolboxWrapper *widget;

    void setupUi(QWidget *NewGeneVariableGroupsScrollArea)
    {
        if (NewGeneVariableGroupsScrollArea->objectName().isEmpty())
            NewGeneVariableGroupsScrollArea->setObjectName(QStringLiteral("NewGeneVariableGroupsScrollArea"));
        NewGeneVariableGroupsScrollArea->resize(400, 300);
        gridLayout = new QGridLayout(NewGeneVariableGroupsScrollArea);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        widget = new NewGeneVariablesToolboxWrapper(NewGeneVariableGroupsScrollArea);
        widget->setObjectName(QStringLiteral("widget"));

        gridLayout->addWidget(widget, 0, 0, 1, 1);


        retranslateUi(NewGeneVariableGroupsScrollArea);

        QMetaObject::connectSlotsByName(NewGeneVariableGroupsScrollArea);
    } // setupUi

    void retranslateUi(QWidget *NewGeneVariableGroupsScrollArea)
    {
        NewGeneVariableGroupsScrollArea->setWindowTitle(QApplication::translate("NewGeneVariableGroupsScrollArea", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariableGroupsScrollArea: public Ui_NewGeneVariableGroupsScrollArea {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLEGROUPSSCROLLAREA_H
