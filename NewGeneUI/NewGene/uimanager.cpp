#include "uimanager.h"
#include "newgenemainwindow.h"

UIManager::UIManager(NewGeneMainWindow *parent) :
    QObject(static_cast<QObject*>(parent))
{
}

NewGeneMainWindow &UIManager::mainWindow()
{
    QObject * myParent = parent();
    try
    {
        NewGeneMainWindow * myWindow = dynamic_cast<NewGeneMainWindow *>(myParent);
    }
    catch (std::bad_cast)
    {
        boost::format msg("Main window not valid in %1%");
        msg % which_descriptor.toStdString();
        throw NewGeneException() << newgene_error_description(msg.str());
    }
}
