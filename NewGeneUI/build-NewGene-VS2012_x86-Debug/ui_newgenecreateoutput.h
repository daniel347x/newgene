/********************************************************************************
** Form generated from reading UI file 'newgenecreateoutput.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENECREATEOUTPUT_H
#define UI_NEWGENECREATEOUTPUT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <newgeneselectvariables.h>
#include <newgenetabwidget.h>

QT_BEGIN_NAMESPACE

class Ui_NewGeneCreateOutput
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    NewGeneTabWidget *tabWidgetOutput;
    QWidget *tabSelectVariables;
    QGridLayout *gridLayout_2;
    NewGeneSelectVariables *widget;
    QWidget *tabColumnOrder;
    QGridLayout *gridLayout_4;
    QWidget *widget_2;
    QWidget *tabOutputDestination;
    QGridLayout *gridLayout_5;
    QWidget *widget_3;

    void setupUi(QWidget *NewGeneCreateOutput)
    {
        if (NewGeneCreateOutput->objectName().isEmpty())
            NewGeneCreateOutput->setObjectName(QStringLiteral("NewGeneCreateOutput"));
        NewGeneCreateOutput->resize(926, 572);
        NewGeneCreateOutput->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(NewGeneCreateOutput);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(NewGeneCreateOutput);
        label->setObjectName(QStringLiteral("label"));
        label->setStyleSheet(QLatin1String("QLabel\n"
"{\n"
"	background-color : #fde0d3;\n"
"	font-weight: bold;\n"
"	font-size: 20px;\n"
"}"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        tabWidgetOutput = new NewGeneTabWidget(NewGeneCreateOutput);
        tabWidgetOutput->setObjectName(QStringLiteral("tabWidgetOutput"));
        tabWidgetOutput->setStyleSheet(QLatin1String("\n"
"QWidget {\n"
"border-bottom: 9px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #9394ea,\n"
"                                 stop: 0.1 #4d4eff,\n"
"                                 stop: 0.15 #5354dd,\n"
"                                 stop: 1.0 #ff9372\n"
"                                 );\n"
"}\n"
"\n"
"QTabBar::tab:selected {\n"
"border-left: 0px solid #1c1515;\n"
"padding-left: 10px;\n"
"border-right: 0px solid #1c1515;\n"
"padding-right: 10px;\n"
"border-bottom: 0px;\n"
"border-top: 0px;\n"
"margin-bottom: 2px;\n"
"border-bottom-left-radius: 0px;\n"
"border-bottom-right-radius: 0px;\n"
"padding-top: 1px;\n"
"padding-bottom: 2px;\n"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #f9dac7,\n"
"                                 /*stop: 0.3 #ffbaa5,*/\n"
"                                 stop: 1.0 #ff9372\n"
"                                 /*stop: 1 #ff450c*/\n"
"                                 );"
                        "\n"
" }\n"
"\n"
"QTabBar::tab:!selected {\n"
"border-left: 1px solid #1c1515;\n"
"padding-left: 10px;\n"
"border-right: 1px solid #1c1515;\n"
"padding-right: 10px;\n"
"border-bottom: 1px solid #7e7fff;\n"
"border-top: 0px solid #a0a0a0;\n"
"margin-bottom: 9px;\n"
"border-bottom-left-radius: 0px;\n"
"border-bottom-right-radius: 0px;\n"
"padding-top: 2px;\n"
"padding-bottom: 2px;\n"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #cbcbff,\n"
"                                 stop: 0.3 #cbcbff,\n"
"                                 stop: 0.6 #b1b1ff,\n"
"                                 stop: 1.0 #9394ea);\n"
"}"));
        tabWidgetOutput->setTabPosition(QTabWidget::South);
        tabWidgetOutput->setDocumentMode(true);
        tabSelectVariables = new QWidget();
        tabSelectVariables->setObjectName(QStringLiteral("tabSelectVariables"));
        tabSelectVariables->setStyleSheet(QLatin1String("QWidget {\n"
"/* border-bottom: 3px solid #f9dac7; */\n"
"border-bottom: 5px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #ededed,\n"
"                                 stop: 1.0 #f9dac7\n"
"                                 );\n"
"}"));
        gridLayout_2 = new QGridLayout(tabSelectVariables);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 3);
        widget = new NewGeneSelectVariables(tabSelectVariables);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setStyleSheet(QLatin1String("QWidget {\n"
"border-bottom: 0px;;\n"
"}"));

        gridLayout_2->addWidget(widget, 0, 0, 1, 1);

        tabWidgetOutput->addTab(tabSelectVariables, QString());
        tabColumnOrder = new QWidget();
        tabColumnOrder->setObjectName(QStringLiteral("tabColumnOrder"));
        tabColumnOrder->setStyleSheet(QLatin1String("QWidget {\n"
"/* border-bottom: 3px solid #f9dac7; */\n"
"border-bottom: 5px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #ededed,\n"
"                                 stop: 1.0 #f9dac7\n"
"                                 );\n"
"}"));
        gridLayout_4 = new QGridLayout(tabColumnOrder);
        gridLayout_4->setSpacing(0);
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        widget_2 = new QWidget(tabColumnOrder);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        widget_2->setStyleSheet(QLatin1String("QWidget {\n"
"border-bottom: 0px;\n"
"}"));

        gridLayout_4->addWidget(widget_2, 0, 0, 1, 1);

        tabWidgetOutput->addTab(tabColumnOrder, QString());
        tabOutputDestination = new QWidget();
        tabOutputDestination->setObjectName(QStringLiteral("tabOutputDestination"));
        tabOutputDestination->setStyleSheet(QLatin1String("QWidget {\n"
"/* border-bottom: 3px solid #f9dac7; */\n"
"border-bottom: 5px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #ededed,\n"
"                                 stop: 1.0 #f9dac7\n"
"                                 );\n"
"}"));
        gridLayout_5 = new QGridLayout(tabOutputDestination);
        gridLayout_5->setSpacing(0);
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        widget_3 = new QWidget(tabOutputDestination);
        widget_3->setObjectName(QStringLiteral("widget_3"));
        widget_3->setStyleSheet(QLatin1String("QWidget {\n"
"border-bottom: 0px;\n"
"}"));

        gridLayout_5->addWidget(widget_3, 0, 0, 1, 1);

        tabWidgetOutput->addTab(tabOutputDestination, QString());

        verticalLayout->addWidget(tabWidgetOutput);


        retranslateUi(NewGeneCreateOutput);

        tabWidgetOutput->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(NewGeneCreateOutput);
    } // setupUi

    void retranslateUi(QWidget *NewGeneCreateOutput)
    {
        NewGeneCreateOutput->setWindowTitle(QApplication::translate("NewGeneCreateOutput", "Form", 0));
        label->setText(QApplication::translate("NewGeneCreateOutput", "Create Output Dataset", 0));
        tabWidgetOutput->setTabText(tabWidgetOutput->indexOf(tabSelectVariables), QApplication::translate("NewGeneCreateOutput", " Select variables ", 0));
        tabWidgetOutput->setTabText(tabWidgetOutput->indexOf(tabColumnOrder), QApplication::translate("NewGeneCreateOutput", " Set column order ", 0));
        tabWidgetOutput->setTabText(tabWidgetOutput->indexOf(tabOutputDestination), QApplication::translate("NewGeneCreateOutput", " Choose output destination ", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneCreateOutput: public Ui_NewGeneCreateOutput {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENECREATEOUTPUT_H
