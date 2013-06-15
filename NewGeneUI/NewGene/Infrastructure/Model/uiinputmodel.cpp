#include "uiinputmodel.h"
#include <QMessageBox>

void UIInputModel::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
