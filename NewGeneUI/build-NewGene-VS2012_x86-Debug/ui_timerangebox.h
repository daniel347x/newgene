/********************************************************************************
** Form generated from reading UI file 'timerangebox.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TIMERANGEBOX_H
#define UI_TIMERANGEBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_TimeRangeBox
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;

    void setupUi(QFrame *TimeRangeBox)
    {
        if (TimeRangeBox->objectName().isEmpty())
            TimeRangeBox->setObjectName(QStringLiteral("TimeRangeBox"));
        TimeRangeBox->resize(400, 300);
        TimeRangeBox->setFrameShape(QFrame::StyledPanel);
        TimeRangeBox->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(TimeRangeBox);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(TimeRangeBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);


        retranslateUi(TimeRangeBox);

        QMetaObject::connectSlotsByName(TimeRangeBox);
    } // setupUi

    void retranslateUi(QFrame *TimeRangeBox)
    {
        TimeRangeBox->setWindowTitle(QApplication::translate("TimeRangeBox", "Frame", 0));
        label->setText(QApplication::translate("TimeRangeBox", "Time Range", 0));
    } // retranslateUi

};

namespace Ui {
    class TimeRangeBox: public Ui_TimeRangeBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TIMERANGEBOX_H
