/********************************************************************************
** Form generated from reading UI file 'newgenevariablesummaryscrollarea.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLESUMMARYSCROLLAREA_H
#define UI_NEWGENEVARIABLESUMMARYSCROLLAREA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariableSummaryScrollArea
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *NewGeneVariableSummaryScrollArea)
    {
        if (NewGeneVariableSummaryScrollArea->objectName().isEmpty())
            NewGeneVariableSummaryScrollArea->setObjectName(QStringLiteral("NewGeneVariableSummaryScrollArea"));
        NewGeneVariableSummaryScrollArea->resize(400, 300);
        verticalLayout = new QVBoxLayout(NewGeneVariableSummaryScrollArea);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

        retranslateUi(NewGeneVariableSummaryScrollArea);

        QMetaObject::connectSlotsByName(NewGeneVariableSummaryScrollArea);
    } // setupUi

    void retranslateUi(QWidget *NewGeneVariableSummaryScrollArea)
    {
        NewGeneVariableSummaryScrollArea->setWindowTitle(QApplication::translate("NewGeneVariableSummaryScrollArea", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariableSummaryScrollArea: public Ui_NewGeneVariableSummaryScrollArea {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLESUMMARYSCROLLAREA_H
