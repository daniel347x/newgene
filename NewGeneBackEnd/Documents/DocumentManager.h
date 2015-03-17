#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "../Manager.h"

class DocumentManager : public Manager<DocumentManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS>
{
	public:
		DocumentManager(Messager & messager_) : Manager<DocumentManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS>(messager_) {}
};

#endif
