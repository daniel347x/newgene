#ifndef LOGGINGMANAGER_H
#define LOGGINGMANAGER_H

#include "../Manager.h"

class LoggingManager : public Manager<LoggingManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING>
{
public:
	LoggingManager(Messager & messager_) : Manager<LoggingManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING>(messager_) {}
};

#endif
