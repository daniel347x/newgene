#include "uidocumentmanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

UIDocumentManager::UIDocumentManager(QObject * parent, UIMessager & messager)
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}
