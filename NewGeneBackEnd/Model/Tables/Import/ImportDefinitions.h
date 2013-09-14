#ifndef IMPORTDEFINITION_H
#define IMPORTDEFINITION_H

#include "Import.h"
#include <vector>

ImportDefinition Development__CreateImportDefinition_Maoz();
ImportDefinition Development__CreateImportDefinition_COW();
ImportDefinition Development__CreateImportDefinition_Cty();
ImportDefinition Development__CreateImportDefinition_MidNames();
ImportDefinition Development__CreateImportDefinition_Mid();
ImportDefinition Development__CreateImportDefinition_CINC();

class ImportDefinitions
{

	public:

		typedef std::vector<ImportDefinition> ImportDefinitionVector;

		static ImportDefinition CreateImportDefinition(std::string const & vg_code);

		static ImportDefinitionVector definitions;

};

#endif
