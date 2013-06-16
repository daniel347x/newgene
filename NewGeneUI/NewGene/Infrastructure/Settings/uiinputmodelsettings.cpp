#include "uiinputmodelsettings.h"
#include <QMessageBox>

void UIInputModelSettings::SignalMessageBox(STD_STRING msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();
}

void UIInputModelSettings::UpdateConnections()
{

}
