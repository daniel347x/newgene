#include "uioutputmodel.h"
#include <QMessageBox>

void UIOutputModel::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}
