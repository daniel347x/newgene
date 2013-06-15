#include "uiinputproject.h"

void UIInputProject::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
