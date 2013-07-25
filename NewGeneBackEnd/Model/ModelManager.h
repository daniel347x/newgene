#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "..\Manager.h"
#include "Tables\TableManager.h"

class ModelManager : public Manager<ModelManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL>
{

	public:

		bool ImportRawData(Messager & messager, Model_basemost * model_, TABLE_TYPES const table_type, boost::filesystem::path const & path_to_raw_data);

};

#endif
