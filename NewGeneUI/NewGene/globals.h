#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
typedef std::string STD_STRING;

#include "../../NewGeneBackEnd/globals.h"
#include <QMessageBox>
#include "newgenefilenames.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Infrastructure/Messager/uimessager.h"

class NewGeneMainWindow;

class UIProjectManager;
class UISettingsManager;
class UILoggingManager;
class UIModelManager;
class UIDocumentManager;
class UIStatusManager;
class UITriggerManager;
class UIThreadManager;
class UIUIDataManager;
class UIUIActionManager;
class UIModelActionManager;

extern NewGeneMainWindow * theMainWindow;

UIProjectManager & projectManagerUI();
UISettingsManager & settingsManagerUI();
UILoggingManager & loggingManagerUI();
UIModelManager & modelManagerUI();
UIDocumentManager & documentManagerUI();
UIStatusManager & statusManagerUI();
UITriggerManager & triggerManagerUI();
UIThreadManager & threadManagerUI();
UIUIDataManager & uidataManagerUI();
UIUIActionManager & uiactionManagerUI();
UIModelActionManager & modelactionManagerUI();

#endif // GLOBALS_H
