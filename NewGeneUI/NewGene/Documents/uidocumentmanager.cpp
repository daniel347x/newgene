#include "uidocumentmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

std::unique_ptr<UIDocumentManager> UIDocumentManager::_documentManager;

UIDocumentManager::UIDocumentManager( QObject * parent ) :
	UIManager( parent )
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}

UIDocumentManager & UIDocumentManager::getDocumentManager()
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if ( _documentManager == NULL )
	{
		_documentManager.reset( new UIDocumentManager( NULL ) );

		if ( _documentManager )
		{
			_documentManager->which = MANAGER_DOCUMENTS;
			_documentManager->which_descriptor = "UIDocumentManager";
		}
	}

	if ( documentManager == NULL )
	{
		boost::format msg( "Document manager not instantiated." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *_documentManager;

}
