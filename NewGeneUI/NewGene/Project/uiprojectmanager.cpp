#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"


std::unique_ptr<UIProjectManager> UIProjectManager::projectManager_;

UIProjectManager::UIProjectManager(NewGeneMainWindow *parent) :
	UIManager(parent)
{
}

UIProjectManager & UIProjectManager::projectManager( NewGeneMainWindow * parent )
{
	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if ( projectManager_ == NULL )
	{
		if (parent == NULL)
		{
			boost::format msg( "Project manager's main window not set." );

			throw NewGeneException() << newgene_error_description( msg.str() );
		}

		projectManager_.reset(new UIProjectManager( parent ));

		if ( projectManager_ )
		{
			projectManager_ -> which            = MANAGER_PROJECT;
			projectManager_ -> which_descriptor = "UIProjectManager";

			// instantiate other managers
			statusManager(parent);
			documentManager(parent);
			settingsManager(parent);
			modelManager(parent);
		}
	}

	if ( projectManager_ == NULL )
	{
		boost::format msg( "Project manager not instantiated." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	if ( projectManager_ -> parent() == NULL )
	{
		boost::format msg( "Project manager's main window not set." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *(projectManager_.get());
}

void UIProjectManager::LoadDefaultProject()
{
	//		globalSettings.reset(settingsManager().loadGlobalSettings());
	//		model.reset(modelManager().loadDefaultModel());
}
