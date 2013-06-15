#include "uimodelactionmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIModelActionManager::UIModelActionManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
