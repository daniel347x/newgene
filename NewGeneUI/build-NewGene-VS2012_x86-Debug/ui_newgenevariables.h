/********************************************************************************
** Form generated from reading UI file 'newgenevariables.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEVARIABLES_H
#define UI_NEWGENEVARIABLES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <newgenevariablegroupsscrollarea.h>

QT_BEGIN_NAMESPACE

class Ui_NewGeneVariables
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QScrollArea *scrollArea;
    NewGeneVariableGroupsScrollArea *scrollAreaWidgetContents;

    void setupUi(QWidget *NewGeneVariables)
    {
        if (NewGeneVariables->objectName().isEmpty())
            NewGeneVariables->setObjectName(QStringLiteral("NewGeneVariables"));
        NewGeneVariables->resize(400, 300);
        verticalLayout = new QVBoxLayout(NewGeneVariables);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(NewGeneVariables);
        label->setObjectName(QStringLiteral("label"));
        label->setAutoFillBackground(false);
        label->setStyleSheet(QLatin1String("QLabel\n"
"{\n"
"	background-color : #0cff7f;\n"
"	font-weight: bold;\n"
"	font-size: 16px;\n"
"}"));
        label->setAlignment(Qt::AlignCenter);
        label->setTextInteractionFlags(Qt::NoTextInteraction);

        verticalLayout->addWidget(label);

        scrollArea = new QScrollArea(NewGeneVariables);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new NewGeneVariableGroupsScrollArea();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 400, 281));
        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);


        retranslateUi(NewGeneVariables);

        QMetaObject::connectSlotsByName(NewGeneVariables);
    } // setupUi

    void retranslateUi(QWidget *NewGeneVariables)
    {
        NewGeneVariables->setWindowTitle(QApplication::translate("NewGeneVariables", "Form", 0));
        label->setText(QApplication::translate("NewGeneVariables", "Variables", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneVariables: public Ui_NewGeneVariables {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEVARIABLES_H
