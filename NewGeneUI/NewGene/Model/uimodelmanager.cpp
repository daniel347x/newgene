#include "uimodelmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

std::unique_ptr<UIModelManager> UIModelManager::_modelManager;

UIModelManager::UIModelManager( QObject * parent ) :
	UIManager( parent )
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}}

UIModelManager & UIModelManager::getModelManager()
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if ( _modelManager == NULL )
	{
		_modelManager.reset( new UIModelManager( NULL ) );

		if ( _modelManager )
		{
			_modelManager->which = MANAGER_MODEL;
			_modelManager->which_descriptor = "UIModelManager";
		}
	}

	if ( _modelManager == NULL )
	{
		boost::format msg( "Model manager not instantiated." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *_modelManager;

}

UIModel * UIModelManager::loadDefaultModel()
{
	// Settings manager constructor obtains location of settings file
	// ... and then calls backend settings manager to load settings file
	// Then, in this function,
	// request flag regarding whether to automatically load most recent input/output document;
	// if not, request flag regarding whether to automatically load a default input/output document shipped with NewGene;
	// if not, return NULL; otherwise, request corresponding setting for the path of the appropriate input/output document
	// and call the backend model manager to load the input/output main document.
	// On backend, only basic validation of the main documents takes place, and a mapping from document to GUID is created.
	// This pair of GUIDs is returned and stored in the UIModel and control returns to this application;
	// a signal is sent to the Status Manager (this class must be created - in conjunction with a backend class which really
	// maintains the status) which reports "loading input and output file...";
	// also each UI manager, starting with the model manager (see following comments!)
	// must kick off a background thread for various operations, including this one (the loading of the input/output).
	// Note that the ONLY POSSIBLE WAY TO IMPLEMENT THIS
	// is for the **Qt** (UI) manager to kick off a Qt background thread,
	// which will internally call a backend thread function,
	// and when the backend function returns to the Qt background thread,
	// this Qt thread will call the Qt function which wraps a windowing OS system
	// function that adds a message to the message queue that the main UI loop waits on,
	// and the main thread will wake up with a Qt Event to handle,
	// so a QEvent handler needs to be built into UIModel class to handle
	// "model loaded" or other messages FROM the backend system, translating
	// the QEvent into a ModelChange message and sending a signal with the proper message.
	return NULL;
}
