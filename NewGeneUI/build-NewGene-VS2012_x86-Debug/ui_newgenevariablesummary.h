/********************************************************************************
** Form generated from reading UI file 'newgenevariablesummary.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLESUMMARY_H
#define UI_NEWGENEVARIABLESUMMARY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "newgenevariablesummaryscrollarea.h"

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariableSummary
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QScrollArea *scrollArea;
    NewGeneVariableSummaryScrollArea *scrollAreaWidgetContents;

    void setupUi(QWidget *NewGeneVariableSummary)
    {
        if (NewGeneVariableSummary->objectName().isEmpty())
            NewGeneVariableSummary->setObjectName(QStringLiteral("NewGeneVariableSummary"));
        NewGeneVariableSummary->resize(543, 427);
        verticalLayout = new QVBoxLayout(NewGeneVariableSummary);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(NewGeneVariableSummary);
        label->setObjectName(QStringLiteral("label"));
        label->setAutoFillBackground(false);
        label->setStyleSheet(QLatin1String("QLabel\n"
"{\n"
"	background-color : #b1b1ff;\n"
"	font-weight: bold;\n"
"	font-size: 16px;\n"
"}"));
        label->setAlignment(Qt::AlignCenter);
        label->setTextInteractionFlags(Qt::NoTextInteraction);

        verticalLayout->addWidget(label);

        scrollArea = new QScrollArea(NewGeneVariableSummary);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new NewGeneVariableSummaryScrollArea();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 541, 406));
        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);


        retranslateUi(NewGeneVariableSummary);

        QMetaObject::connectSlotsByName(NewGeneVariableSummary);
    } // setupUi

    void retranslateUi(QWidget *NewGeneVariableSummary)
    {
        NewGeneVariableSummary->setWindowTitle(QApplication::translate("NewGeneVariableSummary", "Form", 0));
        label->setText(QApplication::translate("NewGeneVariableSummary", "Variable Summary", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariableSummary: public Ui_NewGeneVariableSummary {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLESUMMARY_H
