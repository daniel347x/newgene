#include "uiuidatamanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"

UIUIDataManager::UIUIDataManager( QObject * parent, UIMessager & messager )
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
