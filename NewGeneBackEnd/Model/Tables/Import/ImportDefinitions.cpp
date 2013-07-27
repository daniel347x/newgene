#include "ImportDefinitions.h"
#include "..\TableInstances\VariableGroupData.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

ImportDefinitions::ImportDefinitionVector ImportDefinitions::definitions;

ImportDefinition ImportDefinitions::CreateImportDefinition(std::string const & vg_code)
{
	
	std::string table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);
	
	ImportDefinition new_definition;
	if (boost::iequals(vg_code, "VG_CTY_MID_MAOZ"))
	{
		new_definition = Development__CreateImportDefinition_Maoz();
		definitions.push_back(new_definition);
	}

	return new_definition;

}

ImportDefinition Development__CreateImportDefinition_Maoz()
{
	return ImportDefinition();
}
