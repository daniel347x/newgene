#ifndef GLOBALS_BACKEND_H
#define GLOBALS_BACKEND_H

#include "Project/projectmanager.h"
#include "Model/modelmanager.h"
#include "Settings/settingsmanager.h"
#include "Documents/documentmanager.h"
#include "Status/statusmanager.h"
#include "Logging/loggingmanager.h"
#include "Utilities\NewGeneException.h"
#include <memory>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include "Messager/Messager.h"

ProjectManager & projectManager();
SettingsManager & settingsManager();
LoggingManager & loggingManager();
ModelManager & modelManager();
DocumentManager & documentManager();
StatusManager & statusManager();

#endif
