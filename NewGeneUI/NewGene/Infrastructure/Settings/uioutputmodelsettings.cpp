#include "uioutputmodelsettings.h"
#include <QMessageBox>

void UIOutputModelSettings::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}

void UIOutputModelSettings::UpdateConnections()
{

}
