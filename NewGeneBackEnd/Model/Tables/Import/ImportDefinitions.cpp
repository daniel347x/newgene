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
	ImportDefinition import_definition__maoz;

	import_definition__maoz.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	import_definition__maoz.input_file = "L:\\daniel347x\\__DanExtras\\EuGene\\InputDat\\dyadmid602.csv";
	import_definition__maoz.first_row_is_header_row = true;
	import_definition__maoz.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "DISNO"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STATEA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_STRING_FIXED, "NAMEA"));
	input_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STATEB"));
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

	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "DISNO", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STATEA", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STATEB", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "STRTDAY", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "YEAR", true));
	output_schema_vector.push_back(SchemaEntry(FIELD_TYPE_INT32, "ENDDAY", true));
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
	std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>();
	time_range_mapping->field_mapping_type = FieldMapping::FIELD_MAPPING_TYPE__TIME_RANGE;
	time_range_mapping->time_range_type = TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY_MONTH_YEAR;
	FieldTypeEntries input_file_fields;
	FieldTypeEntries output_table_fields;
	FieldTypeEntry input_time_field__Day = std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTDAY"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__Month = std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTMNTH"), FIELD_TYPE_INT32);
	FieldTypeEntry input_time_field__Year = std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTYR"), FIELD_TYPE_INT32);
	input_file_fields.push_back(input_time_field__Day);
	input_file_fields.push_back(input_time_field__Month);
	input_file_fields.push_back(input_time_field__Year);
	FieldTypeEntry output_time_field__Year = std::make_pair(NameOrIndex(NameOrIndex::NAME, "YEAR"), FIELD_TYPE_INT32);
	output_table_fields.push_back(output_time_field__Year);
	time_range_mapping->input_file_fields = input_file_fields;
	time_range_mapping->output_table_fields = output_table_fields;
	mappings.push_back(time_range_mapping);

	// Default (field-to-field) mappings
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEA"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEA"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEB"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STATEB"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTDAY"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "STRTDAY"), FIELD_TYPE_INT32)));
	mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAY"), FIELD_TYPE_INT32), std::make_pair(NameOrIndex(NameOrIndex::NAME, "ENDDAY"), FIELD_TYPE_INT32)));
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

	return import_definition__maoz;
}
