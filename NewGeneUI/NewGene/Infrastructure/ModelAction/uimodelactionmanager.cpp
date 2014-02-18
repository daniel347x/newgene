#include "uimodelactionmanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"

UIModelActionManager::UIModelActionManager( QObject * parent, UIMessager & messager )
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
