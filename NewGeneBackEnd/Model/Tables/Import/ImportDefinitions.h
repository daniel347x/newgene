#ifndef IMPORTDEFINITION_H
#define IMPORTDEFINITION_H

#include "Import.h"
#include <vector>

ImportDefinition Development__CreateImportDefinition_Maoz();

class ImportDefinitions
{

	public:

		typedef std::vector<ImportDefinition> ImportDefinitionVector;

		static ImportDefinition CreateImportDefinition(std::string const & vg_code);

		static ImportDefinitionVector definitions;

};

#endif
