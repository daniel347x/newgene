#include "uiuidatamanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"

UIUIDataManager::UIUIDataManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
