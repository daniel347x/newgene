#include "uioutputproject.h"

void UIOutputProject::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
