#include "uiuiactionmanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"

UIUIActionManager::UIUIActionManager( QObject * parent, UIMessager & messager )
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
