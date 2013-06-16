#include "uiinputproject.h"

void UIInputProject::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}
