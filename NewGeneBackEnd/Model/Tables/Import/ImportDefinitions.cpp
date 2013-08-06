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
	else if (boost::iequals(vg_code, "VG_CTY"))
	{
		new_definition = Development__CreateImportDefinition_Cty();
		definitions.push_back(new_definition);
	}
	else if (boost::iequals(vg_code, "VG_MID"))
	{
		new_definition = Development__CreateImportDefinition_Mid();
		definitions.push_back(new_definition);
	}
	else if (boost::iequals(vg_code, "VG_MID_NAMES"))
	{
		new_definition = Development__CreateImportDefinition_MidNames();
		definitions.push_back(new_definition);
	}

	return new_definition;

}

ImportDefinition Development__CreateImportDefinition_Maoz()
{
	ImportDefinition import_definition__maoz;

	import_definition__maoz.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	import_definition__maoz.input_file = "L:\\daniel347x\\__DanExtras\\EuGene\\InputDat\\dyadmid602.csv";
	import_definition__maoz.first_row_is_header_row = true;
	import_definition__maoz.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	input_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "DISNO"));
	input_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "STATEA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "NAMEA"));
	input_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "STATEB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "NAMEB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTDAY"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTMNTH"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTYR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "YEAR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAY"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTH"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEAR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "OUTCOME"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SETTLMNT"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATLEV"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MAXDUR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MINDUR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHACT"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOST"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "RECIP"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NOINIT"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NOTARG"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STDAYA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STMNTHA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STYEARA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAYA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTHA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEARA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SIDEAA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVSTATA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVTYPEA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATALEVA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHMCAA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOSTA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ORIGNATA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STDAYB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STMNTHB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STYEARB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAYB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTHB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEARB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SIDEAB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVSTATE"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVTYPEB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATALEVB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHMCAB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOSTB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ORIGNATB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ROLEA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ROLEB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "WAR"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Durindx"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Duration"));

	output_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "DISNO", true));
	output_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "STATEA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "NAMEA", true));
	output_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "STATEB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "NAMEB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "YEAR"));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTDAY", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTMNTH", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTYR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_START", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_END", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAY", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTH", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEAR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "OUTCOME", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SETTLMNT", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATLEV", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MAXDUR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MINDUR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHACT", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOST", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "RECIP", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NOINIT", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NOTARG", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STDAYA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STMNTHA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STYEARA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAYA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTHA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEARA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SIDEAA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVSTATA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVTYPEA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATALEVA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHMCAA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOSTA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ORIGNATA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STDAYB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STMNTHB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STYEARB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAYB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDMNTHB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDYEARB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "SIDEAB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVSTATE", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "REVTYPEB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FATALEVB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIGHMCAB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HIHOSTB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ORIGNATB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ROLEA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ROLEB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "WAR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Durindx", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Duration", true));

	ImportDefinition::ImportMappings mappings;
	// Time-range mapping
	std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__YEAR);
	FieldTypeEntries input_file_fields;
	FieldTypeEntries output_table_fields;
	FieldTypeEntry input_time_field__Year = std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32);
	input_file_fields.push_back(input_time_field__Year);
	FieldTypeEntry output_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
	FieldTypeEntry output_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
	output_table_fields.push_back(output_time_field__YearStart);
	output_table_fields.push_back(output_time_field__YearEnd);
	time_range_mapping->input_file_fields = input_file_fields;
	time_range_mapping->output_table_fields = output_table_fields;
	mappings.push_back(time_range_mapping);

	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "DISNO"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "DISNO"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEA"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEA"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NAMEA"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "NAMEA"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEB"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEB"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NAMEB"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "NAMEB"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTDAY"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTDAY"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTMNTH"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTMNTH"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTYR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTYR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAY"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAY"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTH"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTH"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEAR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEAR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "OUTCOME"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "OUTCOME"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "SETTLMNT"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "SETTLMNT"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATLEV"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATLEV"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "MAXDUR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MAXDUR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "MINDUR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MINDUR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHACT"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHACT"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOST"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOST"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "RECIP"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "RECIP"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NOINIT"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "NOINIT"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NOTARG"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "NOTARG"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STDAYA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STDAYA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STMNTHA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STMNTHA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STYEARA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STYEARA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAYA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAYA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTHA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTHA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEARA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEARA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "SIDEAA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "SIDEAA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVSTATA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVSTATA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVTYPEA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVTYPEA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATALEVA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATALEVA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHMCAA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHMCAA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOSTA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOSTA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ORIGNATA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ORIGNATA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STDAYB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STDAYB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STMNTHB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STMNTHB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STYEARB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STYEARB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAYB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAYB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTHB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDMNTHB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEARB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDYEARB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "SIDEAB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "SIDEAB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVSTATE"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVSTATE"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVTYPEB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "REVTYPEB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATALEVB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "FATALEVB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHMCAB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIGHMCAB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOSTB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "HIHOSTB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ORIGNATB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ORIGNATB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ROLEA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ROLEA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ROLEB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ROLEB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "WAR"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "WAR"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Durindx"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "Durindx"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Duration"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "Duration"), FIELD_TYPE_INT32)));

	schema_input.schema = input_schema_vector;
	schema_output.schema = output_schema_vector;

	import_definition__maoz.input_schema = schema_input;
	import_definition__maoz.output_schema = schema_output;

	import_definition__maoz.mappings = mappings;

	return import_definition__maoz;
}

ImportDefinition Development__CreateImportDefinition_Cty()
{
	ImportDefinition import_definition__cty;

	import_definition__cty.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	import_definition__cty.input_file = "L:\\daniel347x\\__DanExtras\\EuGene\\InputDat\\states2008.1.csv";
	import_definition__cty.first_row_is_header_row = true;
	import_definition__cty.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED | ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "StateAbb"));
	input_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "CCode"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "StateNme"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StYear"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StMon"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StDay"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndYear"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndMonth"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndDay"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "Version"));

	output_schema_vector.push_back(SchemaEntry("CTY", FIELD_TYPE_STRING_FIXED, "CCode", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "StateAbb", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "StateName", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_START", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_END", true));

	ImportDefinition::ImportMappings mappings;
	// Time-range mapping
	std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__RANGE__FROM__YR_MNTH_DAY);
	FieldTypeEntries input_file_fields;
	FieldTypeEntries output_table_fields;
	FieldTypeEntry input_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StYear"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__MonthStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StMon"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StDay"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndYear"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__MonthEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndMonth"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndDay"), FIELD_TYPE_INT32);
	input_file_fields.push_back(input_time_field__DayStart);
	input_file_fields.push_back(input_time_field__MonthStart);
	input_file_fields.push_back(input_time_field__YearStart);
	input_file_fields.push_back(input_time_field__DayEnd);
	input_file_fields.push_back(input_time_field__MonthEnd);
	input_file_fields.push_back(input_time_field__YearEnd);
	FieldTypeEntry output_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
	FieldTypeEntry output_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
	output_table_fields.push_back(output_time_field__DayStart);
	output_table_fields.push_back(output_time_field__DayEnd);
	time_range_mapping->input_file_fields = input_file_fields;
	time_range_mapping->output_table_fields = output_table_fields;
	mappings.push_back(time_range_mapping);

	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "StateAbb"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "StateAbb"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "CCode"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "CCode"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "StateNme"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "StateName"), FIELD_TYPE_STRING_FIXED)));

	schema_input.schema = input_schema_vector;
	schema_output.schema = output_schema_vector;

	import_definition__cty.input_schema = schema_input;
	import_definition__cty.output_schema = schema_output;

	import_definition__cty.mappings = mappings;

	return import_definition__cty;
}

ImportDefinition Development__CreateImportDefinition_Mid()
{
	ImportDefinition import_definition__cty;

	import_definition__cty.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	import_definition__cty.input_file = "L:\\daniel347x\\__DanExtras\\EuGene\\InputDat\\MIDA_3.10.csv";
	import_definition__cty.first_row_is_header_row = true;
	import_definition__cty.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	input_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "DispNum"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StDay"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StMon"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "StYear"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndDay"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndMon"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "EndYear"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Outcome"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Settle"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Fatality"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "FatalPre"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MaxDur"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MinDur"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HiAct"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "HostLev"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Recip"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NumA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "NumB"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Link1"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Link2"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Link3"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "Ongo2001"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "Version"));

	output_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "MID", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "MIDName", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "MIDOnset", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDOutcome", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDSettle", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDFatal", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDFatalPre", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDMaxDur", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDMinDur", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDHiAct", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDHost", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDRecip", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDNumA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "MIDNumB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_START", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT64, "DATETIME_ROW_END", true));

	ImportDefinition::ImportMappings mappings;

	// Time-range mapping
	std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__RANGE__FROM__YR_MNTH_DAY);
	FieldTypeEntries input_file_fields;
	FieldTypeEntries output_table_fields;
	FieldTypeEntry input_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StDay"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__MonthStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StMon"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StYear"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndDay"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__MonthEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndMon"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "EndYear"), FIELD_TYPE_INT32);
	input_file_fields.push_back(input_time_field__DayStart);
	input_file_fields.push_back(input_time_field__MonthStart);
	input_file_fields.push_back(input_time_field__YearStart);
	input_file_fields.push_back(input_time_field__DayEnd);
	input_file_fields.push_back(input_time_field__MonthEnd);
	input_file_fields.push_back(input_time_field__YearEnd);
	FieldTypeEntry output_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
	FieldTypeEntry output_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
	output_table_fields.push_back(output_time_field__DayStart);
	output_table_fields.push_back(output_time_field__DayEnd);
	time_range_mapping->input_file_fields = input_file_fields;
	time_range_mapping->output_table_fields = output_table_fields;
	mappings.push_back(time_range_mapping);

	// Time-range mapping
	std::shared_ptr<TimeRangeFieldMapping> time_range_mapping_onset = std::make_shared<TimeRangeFieldMapping>(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__FROM__YR_MNTH_DAY);
	input_file_fields.clear();
	output_table_fields.clear();
	FieldTypeEntry input_time_field__Day = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StDay"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__Month = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StMon"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__Year = std::make_pair(NameOrIndex(NameOrIndex::NAME, "StYear"), FIELD_TYPE_INT32);
	input_file_fields.push_back(input_time_field__Day);
	input_file_fields.push_back(input_time_field__Month);
	input_file_fields.push_back(input_time_field__Year);
	FieldTypeEntry output_time_field__Day = std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDOnset"), FIELD_TYPE_INT64);
	output_table_fields.push_back(output_time_field__Day);
	time_range_mapping_onset->input_file_fields = input_file_fields;
	time_range_mapping_onset->output_table_fields = output_table_fields;
	mappings.push_back(time_range_mapping_onset);

	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "DispNum"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MID"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<HardCodedFieldMapping>(std::make_shared<Field<FIELD_TYPE_STRING_FIXED>>("MIDName", FieldValue<FIELD_TYPE_STRING_FIXED>("") ), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDName"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Outcome"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDOutcome"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Settle"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDSettle"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Fatality"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDFatal"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "FatalPre"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDFatalPre"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "MaxDur"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDMaxDur"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "MinDur"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDMinDur"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HiAct"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDHiAct"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "HostLev"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDHost"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "Recip"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDRecip"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NumA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDNumA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "NumB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDNumB"), FIELD_TYPE_INT32)));

	schema_input.schema = input_schema_vector;
	schema_output.schema = output_schema_vector;

	import_definition__cty.input_schema = schema_input;
	import_definition__cty.output_schema = schema_output;

	import_definition__cty.mappings = mappings;

	return import_definition__cty;
}

ImportDefinition Development__CreateImportDefinition_MidNames()
{
	ImportDefinition import_definition__mid;

	import_definition__mid.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	import_definition__mid.input_file = "L:\\daniel347x\\__DanExtras\\EuGene\\InputDat\\MIDC_210.TXT";
	import_definition__mid.first_row_is_header_row = true;
	import_definition__mid.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED | ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	input_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "ID Number"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "Name"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "Version"));

	output_schema_vector.push_back(SchemaEntry("MID", FIELD_TYPE_STRING_FIXED, "MIDNameMID", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "MIDNameName", true));

	ImportDefinition::ImportMappings mappings;

	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ID Number"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDNameMID"), FIELD_TYPE_STRING_FIXED)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDNameName"), FIELD_TYPE_STRING_FIXED), std::make_pair(NameOrIndex(NameOrIndex::NAME, "MIDNameName"), FIELD_TYPE_STRING_FIXED)));

	schema_input.schema = input_schema_vector;
	schema_output.schema = output_schema_vector;

	import_definition__mid.input_schema = schema_input;
	import_definition__mid.output_schema = schema_output;

	import_definition__mid.mappings = mappings;

	return import_definition__mid;
}
