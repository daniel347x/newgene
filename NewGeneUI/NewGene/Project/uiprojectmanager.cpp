#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"


std::unique_ptr<UIProjectManager> UIProjectManager::projectManager_;

UIProjectManager::UIProjectManager(QObject *parent) :
    UIManager(parent)
{
}

UIProjectManager & UIProjectManager::projectManager()
{
    // *****************************************************************
    // TODO: Create std::map<> from parent to manager, and retrieve that
    // ... this will support multiple main windows in the future.
    // *****************************************************************

    if ( projectManager_ == NULL )
    {

        projectManager_.reset(new UIProjectManager(NULL));

        if ( projectManager_ )
        {
            projectManager_ -> which            = MANAGER_PROJECT;
            projectManager_ -> which_descriptor = "UIProjectManager";
        }
    }

    if ( projectManager_ == NULL )
    {
        boost::format msg( "Project manager not instantiated." );

        throw NewGeneException() << newgene_error_description( msg.str() );
    }

    return *(projectManager_.get());
}

UIProject * UIProjectManager::LoadDefaultProject(NewGeneMainWindow * parent)
{
    //		model.reset(modelManager().loadDefaultModel());
    return NULL;
}
