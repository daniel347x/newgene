#include "uioutputmodelsettings.h"
#include <QMessageBox>

void UIOutputModelSettings::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
