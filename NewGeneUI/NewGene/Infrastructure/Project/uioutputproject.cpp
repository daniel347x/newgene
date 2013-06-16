#include "uioutputproject.h"

void UIOutputProject::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}
