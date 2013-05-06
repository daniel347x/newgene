#include "uistatusmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIStatusManager * UIStatusManager::status_ = NULL;

UIStatusManager::UIStatusManager(QObject *parent) :
    QObject(parent)
{
}

UIStatusManager * UIStatusManager::getStatusManager()
{
    if (status_ == NULL)
    {
        status_ = new UIStatusManager;
    }
    if (status_ == NULL)
    {
        boost::format msg("Status manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return status_;
}
