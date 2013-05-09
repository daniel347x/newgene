#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"

UIProjectManager * UIProjectManager::projectManager_ = NULL;

UIProjectManager::UIProjectManager(NewGeneMainWindow *parent) :
	UIManager(parent)
{
}

UIProjectManager * UIProjectManager::projectManager( NewGeneMainWindow * parent )
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

		projectManager_ = new UIProjectManager( parent );

		if ( projectManager_ )
		{
			projectManager_ -> which            = MANAGER_PROJECT;
			projectManager_ -> which_descriptor = "UIProjectManager";

			// instantiate other managers
			projectManager_->modelManager(parent);
			projectManager_->settingsManager(parent);
			projectManager_->documentManager(parent);
			projectManager_->statusManager(parent);
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

	return projectManager_;
}

UIModelManager &UIProjectManager::modelManager(NewGeneMainWindow * parent)
{
	return *UIModelManager::getModelManager(parent);
}

UISettingsManager &UIProjectManager::settingsManager(NewGeneMainWindow * parent)
{
	return *UISettingsManager::getSettingsManager(parent);
}

UIDocumentManager &UIProjectManager::documentManager(NewGeneMainWindow * parent)
{
	return *UIDocumentManager::getDocumentManager(parent);
}

UIStatusManager &UIProjectManager::statusManager(NewGeneMainWindow * parent)
{
	return *UIStatusManager::getStatusManager(parent);
}

void UIProjectManager::LoadDefaultProject()
{
	//		globalSettings.reset(settingsManager().loadGlobalSettings());
	//		model.reset(modelManager().loadDefaultModel());
}
