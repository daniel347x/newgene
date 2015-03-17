#include "uitriggermanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include "globals.h"

UITriggerManager::UITriggerManager(QObject * parent, UIMessager & messager)
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
