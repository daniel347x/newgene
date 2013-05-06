#include "uidocumentmanager.h"

UIDocumentManager * UIDocumentManager::documentManager = NULL;

UIDocumentManager::UIDocumentManager(QObject *parent) :
    QObject(parent)
{
}

UIDocumentManager * UIDocumentManager::getDocumentManager()
{
    if (documentManager == NULL)
    {
        documentManager = new UIDocumentManager;
    }
    if (documentManager == NULL)
    {
        boost::format msg("Document manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return documentManager;
}
