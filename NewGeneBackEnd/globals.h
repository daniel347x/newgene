#ifndef GLOBALS_BACKEND_H
#define GLOBALS_BACKEND_H

#include "Project/projectmanager.h"
#include "Model/modelmanager.h"
#include "Settings/settingsmanager.h"
#include "Documents/documentmanager.h"
#include "Status/statusmanager.h"
#include "Logging/loggingmanager.h"
#include "Triggers/triggermanager.h"
#include "Threads/threadmanager.h"
#include "Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Messager/Messager.h"

class ProjectManager;
class SettingsManager;
class LoggingManager;
class ModelManager;
class DocumentManager;
class StatusManager;

ProjectManager & projectManager();
SettingsManager & settingsManager();
LoggingManager & loggingManager();
ModelManager & modelManager();
DocumentManager & documentManager();
StatusManager & statusManager();
TriggerManager & triggerManager();
ThreadManager & threadManager();

#endif
