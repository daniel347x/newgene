#include "uidocumentmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIDocumentManager * UIDocumentManager::documentManager = NULL;

UIDocumentManager::UIDocumentManager(QObject *parent) :
    QObject(parent)
{
}

UIDocumentManager * UIDocumentManager::getDocumentManager(NewGeneMainWindow * parent)
{

    // *****************************************************************
    // TODO: Create std::map<> from parent to manager, and retrieve that
    // ... this will support multiple main windows in the future.
    // *****************************************************************

    if (documentManager == NULL)
    {
        documentManager = new UIDocumentManager(parent);
    }
    if (documentManager == NULL)
    {
        boost::format msg("Document manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    if (documentManager->parent() == NULL)
    {
        boost::format msg("Document manager's main window not set.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return documentManager;

}
