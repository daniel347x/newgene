#include "uidocumentmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

UIDocumentManager::UIDocumentManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
