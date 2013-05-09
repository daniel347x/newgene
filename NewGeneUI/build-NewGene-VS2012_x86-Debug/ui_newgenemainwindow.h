/********************************************************************************
** Form generated from reading UI file 'newgenemainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWGENEMAINWINDOW_H
#define UI_NEWGENEMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <newgenecreateoutput.h>
#include <newgenetabwidget.h>

QT_BEGIN_NAMESPACE

class Ui_NewGeneMainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    NewGeneTabWidget *tabWidgetMain;
    QWidget *tabHome;
    QGridLayout *gridLayout;
    QWidget *tabOutput;
    QVBoxLayout *verticalLayoutOutput;
    NewGeneCreateOutput *widget;
    QWidget *tabInput;
    QGridLayout *gridLayout_3;
    QMenuBar *menuBar;
    QMenu *menuInput;
    QMenu *menuOutput;
    QMenu *menuView;
    QMenu *menuOptions;
    QMenu *menuHelp;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *NewGeneMainWindow)
    {
        if (NewGeneMainWindow->objectName().isEmpty())
            NewGeneMainWindow->setObjectName(QStringLiteral("NewGeneMainWindow"));
        NewGeneMainWindow->resize(1000, 664);
        NewGeneMainWindow->setLayoutDirection(Qt::LeftToRight);
        NewGeneMainWindow->setDocumentMode(true);
        centralWidget = new QWidget(NewGeneMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidgetMain = new NewGeneTabWidget(centralWidget);
        tabWidgetMain->setObjectName(QStringLiteral("tabWidgetMain"));
        QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidgetMain->sizePolicy().hasHeightForWidth());
        tabWidgetMain->setSizePolicy(sizePolicy);
        tabWidgetMain->setStyleSheet(QLatin1String("QTabWidget::tab-bar {\n"
"border: 0px;\n"
"}\n"
"\n"
"QTabBar::tab:selected {\n"
"border-left: 1px solid #1c1515;\n"
"padding-left: 10px;\n"
"border-right: 1px solid #1c1515;\n"
"padding-right: 10px;\n"
"border-bottom: 1px solid #7e7fff;\n"
"border-top: 0px solid #a0a0a0;\n"
"margin-bottom: 0px;\n"
"border-bottom-left-radius: 8px;\n"
"border-bottom-right-radius: 8px;\n"
"padding-top: 6px;\n"
"padding-bottom: 6px;\n"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #ff450c,\n"
"                                 stop: 0.05 #f98a74,\n"
"                                 stop: 0.2 #fde0d3,\n"
"                                 stop: 0.9 #f9dac7,\n"
"                                 stop: 1 #f98a74);\n"
" }\n"
"\n"
"QTabBar::tab:!selected {\n"
"border-left: 1px solid #1c1515;\n"
"padding-left: 10px;\n"
"border-right: 1px solid #1c1515;\n"
"padding-right: 10px;\n"
"border-bottom: 1px solid #7e7fff;\n"
"border-top: 0px solid #a0a0a0;\n"
"margin-bottom: 4px;\n"
"border"
                        "-bottom-left-radius: 8px;\n"
"border-bottom-right-radius: 8px;\n"
"padding-top: 1px;\n"
"padding-bottom: 1px;\n"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 /* #5354dd, */ #b1b1ff,\n"
"                                 stop: 0.3 /* #b1b1ff, */ #c7c7c7\n"
"                                 stop: 0.6 #c7c7c7,\n"
"                                 stop: 1.0 #a1a1a1);\n"
"}\n"
""));
        tabWidgetMain->setTabPosition(QTabWidget::South);
        tabWidgetMain->setTabShape(QTabWidget::Triangular);
        tabWidgetMain->setElideMode(Qt::ElideNone);
        tabWidgetMain->setDocumentMode(true);
        tabHome = new QWidget();
        tabHome->setObjectName(QStringLiteral("tabHome"));
        tabHome->setStyleSheet(QLatin1String("QWidget {\n"
"border-bottom: 9px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #9394ea,\n"
"                                 stop: 0.1 #4d4eff,\n"
"                                 stop: 0.15 #5354dd,\n"
"                                 stop: 1.0 #ff9372\n"
"                                 );\n"
"}\n"
""));
        gridLayout = new QGridLayout(tabHome);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        tabWidgetMain->addTab(tabHome, QString());
        tabOutput = new QWidget();
        tabOutput->setObjectName(QStringLiteral("tabOutput"));
        tabOutput->setStyleSheet(QStringLiteral(""));
        verticalLayoutOutput = new QVBoxLayout(tabOutput);
        verticalLayoutOutput->setSpacing(6);
        verticalLayoutOutput->setContentsMargins(0, 0, 0, 0);
        verticalLayoutOutput->setObjectName(QStringLiteral("verticalLayoutOutput"));
        widget = new NewGeneCreateOutput(tabOutput);
        widget->setObjectName(QStringLiteral("widget"));

        verticalLayoutOutput->addWidget(widget);

        tabWidgetMain->addTab(tabOutput, QString());
        tabInput = new QWidget();
        tabInput->setObjectName(QStringLiteral("tabInput"));
        tabInput->setStyleSheet(QLatin1String("QWidget {\n"
"border-bottom: 9px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
"                                 stop: 0 #9394ea,\n"
"                                 stop: 0.1 #4d4eff,\n"
"                                 stop: 0.15 #5354dd,\n"
"                                 stop: 1.0 #ff9372\n"
"                                 );\n"
"}\n"
""));
        gridLayout_3 = new QGridLayout(tabInput);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        tabWidgetMain->addTab(tabInput, QString());

        verticalLayout->addWidget(tabWidgetMain);

        NewGeneMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(NewGeneMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1000, 21));
        menuInput = new QMenu(menuBar);
        menuInput->setObjectName(QStringLiteral("menuInput"));
        menuOutput = new QMenu(menuBar);
        menuOutput->setObjectName(QStringLiteral("menuOutput"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QStringLiteral("menuOptions"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        NewGeneMainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(NewGeneMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        NewGeneMainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuInput->menuAction());
        menuBar->addAction(menuOutput->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menuHelp->menuAction());

        retranslateUi(NewGeneMainWindow);

        tabWidgetMain->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(NewGeneMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *NewGeneMainWindow)
    {
        NewGeneMainWindow->setWindowTitle(QApplication::translate("NewGeneMainWindow", "NewGene - K-adic data management for the social sciences", 0));
        tabWidgetMain->setTabText(tabWidgetMain->indexOf(tabHome), QApplication::translate("NewGeneMainWindow", " Home ", 0));
        tabWidgetMain->setTabText(tabWidgetMain->indexOf(tabOutput), QApplication::translate("NewGeneMainWindow", " Create output dataset ", 0));
        tabWidgetMain->setTabText(tabWidgetMain->indexOf(tabInput), QApplication::translate("NewGeneMainWindow", " Manage input dataset ", 0));
        menuInput->setTitle(QApplication::translate("NewGeneMainWindow", "Input", 0));
        menuOutput->setTitle(QApplication::translate("NewGeneMainWindow", "Output", 0));
        menuView->setTitle(QApplication::translate("NewGeneMainWindow", "View", 0));
        menuOptions->setTitle(QApplication::translate("NewGeneMainWindow", "Options", 0));
        menuHelp->setTitle(QApplication::translate("NewGeneMainWindow", "Help", 0));
    } // retranslateUi

};

namespace Ui {
    class NewGeneMainWindow: public Ui_NewGeneMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWGENEMAINWINDOW_H
