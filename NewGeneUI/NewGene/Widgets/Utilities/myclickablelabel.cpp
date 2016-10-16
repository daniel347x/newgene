#include "myclickablelabel.h"

MyClickableLabel::MyClickableLabel(QWidget* parent) :
	QLabel(parent)
{
	setCursor(Qt::PointingHandCursor);
}

MyClickableLabel::~MyClickableLabel()
{

}

void MyClickableLabel::mousePressEvent(QMouseEvent* event)
{
	emit clicked(text());
}
