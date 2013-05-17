#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"


std::unique_ptr<UIProjectManager> UIProjectManager::_projectManager;

UIProjectManager::UIProjectManager( QObject * parent ) :
	UIManager( parent )
{
}

UIProjectManager & UIProjectManager::getProjectManager()
{
	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if ( _projectManager == NULL )
	{

		_projectManager.reset( new UIProjectManager( NULL ) );

		if ( _projectManager )
		{
			_projectManager -> which            = MANAGER_PROJECT;
			_projectManager -> which_descriptor = "UIProjectManager";
		}
	}

	if ( _projectManager == NULL )
	{
		boost::format msg( "Project manager not instantiated." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *( _projectManager.get() );
}

UIProject * UIProjectManager::LoadDefaultProject( NewGeneMainWindow * )
{
	return NULL;
}
