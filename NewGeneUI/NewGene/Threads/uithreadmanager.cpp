#include "uithreadmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIThreadManager::UIThreadManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
