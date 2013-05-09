/********************************************************************************
** Form generated from reading UI file 'kadcolumnselectionbox.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KADCOLUMNSELECTIONBOX_H
#define UI_KADCOLUMNSELECTIONBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_KAdColumnSelectionBox
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;

    void setupUi(QFrame *KAdColumnSelectionBox)
    {
        if (KAdColumnSelectionBox->objectName().isEmpty())
            KAdColumnSelectionBox->setObjectName(QStringLiteral("KAdColumnSelectionBox"));
        KAdColumnSelectionBox->resize(400, 300);
        KAdColumnSelectionBox->setFrameShape(QFrame::StyledPanel);
        KAdColumnSelectionBox->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(KAdColumnSelectionBox);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(KAdColumnSelectionBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);


        retranslateUi(KAdColumnSelectionBox);

        QMetaObject::connectSlotsByName(KAdColumnSelectionBox);
    } // setupUi

    void retranslateUi(QFrame *KAdColumnSelectionBox)
    {
        KAdColumnSelectionBox->setWindowTitle(QApplication::translate("KAdColumnSelectionBox", "Frame", 0));
        label->setText(QApplication::translate("KAdColumnSelectionBox", "K-ad primary column selections", 0));
    } // retranslateUi

};

namespace Ui {
    class KAdColumnSelectionBox: public Ui_KAdColumnSelectionBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KADCOLUMNSELECTIONBOX_H
