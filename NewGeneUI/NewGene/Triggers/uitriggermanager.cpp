#include "uitriggermanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "globals.h"

UITriggerManager::UITriggerManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}

void UITriggerManager::TriggerActiveInputProject(NewGeneMainWindow* newGeneMainWindow/* to be filled in */)
{
	projectManagerUI().TriggerActiveInputProject(newGeneMainWindow);
}

void UITriggerManager::TriggerActiveOutputProject(NewGeneMainWindow* newGeneMainWindow/* to be filled in */)
{
	projectManagerUI().TriggerActiveOutputProject(newGeneMainWindow);
}

void UITriggerManager::ConnectTrigger(QWidget * w)
{
	projectManagerUI().ConnectTrigger(w);
}
