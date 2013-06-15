#include "uiinputmodelsettings.h"
#include <QMessageBox>

void UIInputModelSettings::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
