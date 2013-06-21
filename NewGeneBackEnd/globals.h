#ifndef GLOBALS_BACKEND_H
#define GLOBALS_BACKEND_H

#include "Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Messager/Messager.h"

typedef std::string UUID;

class ProjectManager;
class SettingsManager;
class LoggingManager;
class ModelManager;
class DocumentManager;
class StatusManager;
class TriggerManager;
class ThreadManager;
class UIDataManager;
class UIActionManager;
class ModelActionManager;

ProjectManager & projectManager();
SettingsManager & settingsManager();
LoggingManager & loggingManager();
ModelManager & modelManager();
DocumentManager & documentManager();
StatusManager & statusManager();
TriggerManager & triggerManager();
ThreadManager & threadManager();
UIDataManager & uidataManager();
UIActionManager & uiactionManager();
ModelActionManager & modelactionManager();

#endif
