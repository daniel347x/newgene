/********************************************************************************
** Form generated from reading UI file 'newgeneselectvariables.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENESELECTVARIABLES_H
#define UI_NEWGENESELECTVARIABLES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include <newgenevariables.h>
#include <newgenevariablesummary.h>
#include "kadcolumnselectionbox.h"
#include "timerangebox.h"

QT_BEGIN_NAMESPACE

class Ui_NewGeneSelectVariables
{
public:
    QGridLayout *gridLayout;
    QSplitter *splitter;
    NewGeneVariables *CreateOutputDataset_VariablesSplitter_VariableSelections;
    NewGeneVariableSummary *CreateOutputDataset_VariablesSplitter_VariableSummary;
    TimeRangeBox *frame;
    KAdColumnSelectionBox *frame_2;

    void setupUi(QWidget *NewGeneSelectVariables)
    {
        if (NewGeneSelectVariables->objectName().isEmpty())
            NewGeneSelectVariables->setObjectName(QStringLiteral("NewGeneSelectVariables"));
        NewGeneSelectVariables->resize(663, 395);
        gridLayout = new QGridLayout(NewGeneSelectVariables);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 20, 0, 0);
        splitter = new QSplitter(NewGeneSelectVariables);
        splitter->setObjectName(QStringLiteral("splitter"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setStyleSheet(QLatin1String("/* from http://stackoverflow.com/a/6833364/368896 */\n"
"QSplitter::handle {\n"
"    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, \n"
"stop:0 rgba(255, 255, 255, 0), \n"
"stop:0.407273 rgba(200, 200, 200, 255), \n"
"stop:0.4825 rgba(101, 104, 113, 235), \n"
"stop:0.6 rgba(255, 255, 255, 0));\n"
"    /* image: url(:/images/splitter.png); */\n"
"     }\n"
"\n"
""));
        splitter->setOrientation(Qt::Horizontal);
        CreateOutputDataset_VariablesSplitter_VariableSelections = new NewGeneVariables(splitter);
        CreateOutputDataset_VariablesSplitter_VariableSelections->setObjectName(QStringLiteral("CreateOutputDataset_VariablesSplitter_VariableSelections"));
        splitter->addWidget(CreateOutputDataset_VariablesSplitter_VariableSelections);
        CreateOutputDataset_VariablesSplitter_VariableSummary = new NewGeneVariableSummary(splitter);
        CreateOutputDataset_VariablesSplitter_VariableSummary->setObjectName(QStringLiteral("CreateOutputDataset_VariablesSplitter_VariableSummary"));
        splitter->addWidget(CreateOutputDataset_VariablesSplitter_VariableSummary);

        gridLayout->addWidget(splitter, 2, 0, 1, 1);

        frame = new TimeRangeBox(NewGeneSelectVariables);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(frame, 1, 0, 1, 1);

        frame_2 = new KAdColumnSelectionBox(NewGeneSelectVariables);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(frame_2, 0, 0, 1, 1);


        retranslateUi(NewGeneSelectVariables);

        QMetaObject::connectSlotsByName(NewGeneSelectVariables);
    } // setupUi

    void retranslateUi(QWidget *NewGeneSelectVariables)
    {
        NewGeneSelectVariables->setWindowTitle(QApplication::translate("NewGeneSelectVariables", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneSelectVariables: public Ui_NewGeneSelectVariables {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENESELECTVARIABLES_H
