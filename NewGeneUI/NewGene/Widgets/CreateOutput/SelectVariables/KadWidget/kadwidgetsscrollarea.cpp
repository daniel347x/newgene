#include "kadwidgetsscrollarea.h"

#include "kadspinbox.h"
#include <QLayout>

KadWidgetsScrollArea::KadWidgetsScrollArea(QWidget *parent) :
    QWidget(parent)
{

    QLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    this->setLayout(layout);

    QWidget * newSpinBox = new KadSpinBox(this);
    layout->addWidget(newSpinBox);

}
