#include "uimanager.h"
#include "newgenemainwindow.h"

UIManager::UIManager(NewGeneMainWindow *parent) :
    QObject(static_cast<QObject*>(parent))
{
}
