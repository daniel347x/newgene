#include "uiinputmodel.h"
#include <QMessageBox>

void UIInputModel::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}
