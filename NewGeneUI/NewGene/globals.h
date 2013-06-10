#ifndef GLOBALS_H
#define GLOBALS_H

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "uitriggermanager.h"
#include "uithreadmanager.h"
#include <QMessageBox>
#include "newgenefilenames.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Messager/uimessager.h"

class NewGeneMainWindow;

extern NewGeneMainWindow * theMainWindow;

UIProjectManager & projectManagerUI();
UISettingsManager & settingsManagerUI();
UILoggingManager & loggingManagerUI();
UIModelManager & modelManagerUI();
UIDocumentManager & documentManagerUI();
UIStatusManager & statusManagerUI();
UITriggerManager & triggerManagerUI();
UIThreadManager & threadManagerUI();

#endif // GLOBALS_H
