#include "uioutputmodel.h"
#include <QMessageBox>

void UIOutputModel::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
