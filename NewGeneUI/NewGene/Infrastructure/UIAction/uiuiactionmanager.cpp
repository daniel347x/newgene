#include "uiuiactionmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIUIActionManager::UIUIActionManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
