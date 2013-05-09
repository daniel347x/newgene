#include "uidocumentmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

std::unique_ptr<UIDocumentManager> UIDocumentManager::documentManager;

UIDocumentManager::UIDocumentManager(NewGeneMainWindow *parent) :
	UIManager(parent)
{
}

UIDocumentManager & UIDocumentManager::getDocumentManager(NewGeneMainWindow * parent)
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if (documentManager == NULL)
	{
		documentManager.reset(new UIDocumentManager(parent));
		if (documentManager)
		{
			documentManager->which = MANAGER_DOCUMENTS;
			documentManager->which_descriptor = "UIDocumentManager";
		}
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
	return *documentManager;

}
