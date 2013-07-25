#include "Import.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include <fstream>

TimeRangeFieldMapping::TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_)
	: time_range_type(time_range_type_)
{
}

ImportDefinition::ImportDefinition()
{
}

ImportDefinition::ImportDefinition(ImportDefinition const & rhs)
	: mappings(rhs.mappings)
	, input_file(rhs.input_file)
	, input_or_output(rhs.input_or_output)
	, table_name(rhs.table_name)
	, first_row_is_header_row(rhs.first_row_is_header_row)
	, input_schema(rhs.input_schema)
	, output_schema(rhs.output_schema)
{
}

Importer::Importer(ImportDefinition const & import_definition_)
	: import_definition(import_definition_)
{
}

void Importer::InitializeFields()
{

	for (int n=0; n<block_size; ++n)
	{

		DataFields fields;

		std::for_each(import_definition.input_schema.schema.cbegin(), import_definition.input_schema.schema.cend(), [&fields](SchemaEntry const & column)
		{
			FIELD_TYPE field_type = column.field_type;
			std::string field_name = column.field_name;
			switch (field_type)
			{
			case FIELD_TYPE_INT32:
				{
					std::shared_ptr<Field<FIELD_TYPE_INT32>> field = std::make_shared<Field<FIELD_TYPE_INT32>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_INT64:
				{
					std::shared_ptr<Field<FIELD_TYPE_INT64>> field = std::make_shared<Field<FIELD_TYPE_INT64>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_UINT32:
				{
					std::shared_ptr<Field<FIELD_TYPE_UINT32>> field = std::make_shared<Field<FIELD_TYPE_UINT32>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_UINT64:
				{
					std::shared_ptr<Field<FIELD_TYPE_UINT64>> field = std::make_shared<Field<FIELD_TYPE_UINT64>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_STRING_FIXED:
				{
					std::shared_ptr<Field<FIELD_TYPE_STRING_FIXED>> field = std::make_shared<Field<FIELD_TYPE_STRING_FIXED>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_STRING_VAR:
				{
					std::shared_ptr<Field<FIELD_TYPE_STRING_VAR>> field = std::make_shared<Field<FIELD_TYPE_STRING_VAR>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_TIMESTAMP:
				{
					std::shared_ptr<Field<FIELD_TYPE_TIMESTAMP>> field = std::make_shared<Field<FIELD_TYPE_TIMESTAMP>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_UUID:
				{
					std::shared_ptr<Field<FIELD_TYPE_UUID>> field = std::make_shared<Field<FIELD_TYPE_UUID>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_UUID_FOREIGN:
				{
					std::shared_ptr<Field<FIELD_TYPE_UUID_FOREIGN>> field = std::make_shared<Field<FIELD_TYPE_UUID_FOREIGN>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_STRING_CODE:
				{
					std::shared_ptr<Field<FIELD_TYPE_STRING_CODE>> field = std::make_shared<Field<FIELD_TYPE_STRING_CODE>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_STRING_LONGHAND:
				{
					std::shared_ptr<Field<FIELD_TYPE_STRING_LONGHAND>> field = std::make_shared<Field<FIELD_TYPE_STRING_LONGHAND>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_TIME_RANGE:
				{
					std::shared_ptr<Field<FIELD_TYPE_TIME_RANGE>> field = std::make_shared<Field<FIELD_TYPE_TIME_RANGE>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_NOTES_1:
				{
					std::shared_ptr<Field<FIELD_TYPE_NOTES_1>> field = std::make_shared<Field<FIELD_TYPE_NOTES_1>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_NOTES_2:
				{
					std::shared_ptr<Field<FIELD_TYPE_NOTES_2>> field = std::make_shared<Field<FIELD_TYPE_NOTES_2>>(field_name);
					fields.push_back(field);
				}
				break;
			case FIELD_TYPE_NOTES_3:
				{
					std::shared_ptr<Field<FIELD_TYPE_NOTES_3>> field = std::make_shared<Field<FIELD_TYPE_NOTES_3>>(field_name);
					fields.push_back(field);
				}
				break;
			}
		});

		block.push_back(fields);

	}

}

bool Importer::ValidateMapping()
{
	
	std::vector<std::pair<int, bool>> indices_required_are_matched;

	// loop through input schema pushing back required indices
	int current_index = 0;
	std::for_each(import_definition.input_schema.schema.cbegin(), import_definition.input_schema.schema.cend(), [this, &indices_required_are_matched, &current_index](SchemaEntry const & schema_entry)
	{
		if (schema_entry.required)
		{
			indices_required_are_matched.push_back(std::make_pair(current_index, false)); // start off with false, indicating NOT MATCHED yet
		}
		++current_index;
	});

	std::for_each(import_definition.mappings.cbegin(), import_definition.mappings.cend(), [this, &indices_required_are_matched](std::shared_ptr<FieldMapping> const & field_mapping)
	{

		if (!field_mapping)
		{
			// TODO: Import problem warning
			return; // from lambda
		}

		if (!field_mapping->Validate())
		{
			// TODO: Import problem warning
			return; // from lambda
		}

		std::for_each(field_mapping->input_file_fields.cbegin(), field_mapping->input_file_fields.cend(), [this](FieldTypeEntry const & input_entry)
		{
			if (input_entry.first.name_or_index == NameOrIndex::NAME)
			{
				// Make certain the name exists in the input schema
			}
			else
			{
				// Make certain the index exists in the input schema
			}
		});

		std::for_each(field_mapping->output_table_fields.cbegin(), field_mapping->output_table_fields.cend(), [this](FieldTypeEntry const & output_entry)
		{
			if (output_entry.first.name_or_index == NameOrIndex::NAME)
			{
				// Make certain the name exists in the output table
			}
			else
			{
				// Make certain the index exists in the output table
			}
		});

		switch (field_mapping->field_mapping_type)
		{

			case FieldMapping::FIELD_MAPPING_TYPE__ONE_TO_ONE:
				{
					try
					{
						OneToOneFieldMapping const & the_mapping = dynamic_cast<OneToOneFieldMapping const &>(*field_mapping);
						FieldTypeEntry const & output_entry = field_mapping->output_table_fields[0];

						// search the output schema to see what numeric index this corresponds to, and set it in indices_required_are_matched
						int current_index = 0;
						std::for_each(import_definition.output_schema.schema.cbegin(), import_definition.output_schema.schema.cend(), [this, &output_entry, &indices_required_are_matched, &current_index](SchemaEntry const & output_schema_entry)
						{
							bool match = false;
							if (output_entry.first.name_or_index == NameOrIndex::NAME)
							{
								if (boost::iequals(output_entry.first.name, output_schema_entry.field_name))
								{
									match = true;
								}
							}
							else
							{
								if (output_entry.first.index == current_index)
								{
									match = true;
								}
							}
							if (match)
							{
								std::for_each(indices_required_are_matched.begin(), indices_required_are_matched.end(), [&current_index](std::pair<int, bool> & the_pair)
								{
									if (the_pair.first == current_index)
									{
										the_pair.second = true;
									}
								});
							}
							++current_index;
						});
					}
					catch (std::bad_cast &)
					{
						// TODO: Import problem warning
						return; // from lambda
					}
				}
				break;

			case FieldMapping::FIELD_MAPPING_TYPE__TIME_RANGE:
				{
					try
					{
						TimeRangeFieldMapping const & the_mapping = dynamic_cast<TimeRangeFieldMapping const &>(*field_mapping);
					}
					catch (std::bad_cast &)
					{
						// TODO: Import problem warning
						return; // from lambda
					}
				}
				break;

		}
	});

	bool something_did_not_match = false;
	std::for_each(indices_required_are_matched.begin(), indices_required_are_matched.end(), [&something_did_not_match](std::pair<int, bool> & the_pair)
	{
		if (the_pair.second == false)
		{
			something_did_not_match = true;
		}
	});

	if (something_did_not_match)
	{
		// Not all required table entries have matched!
		// Error!
		return false;
	}

	return true;

}

bool Importer::DoImport()
{

	// validate
	if (!ValidateMapping())
	{
		// TODO: Error logging
		return false;
	}

	// TODO: try/finally or exit block
	// open file
	std::fstream data_file;
	data_file.open(import_definition.input_file.c_str(), std::ios::in);
	if (data_file.is_open())
	{

		char line[MAX_LINE_SIZE];
		bool read_line = true;

		// skip first row if necessary
		if (import_definition.first_row_is_header_row && data_file.good())
		{
			// TODO: Validate column headers
			read_line = data_file.getline(line, MAX_LINE_SIZE - 1);
		}

		int current_lines_read = 0;
		while (data_file.getline(line, MAX_LINE_SIZE - 1) && data_file.good())
		{

			++current_lines_read;

			char * current_line_ptr = line;
			std::for_each(import_definition.input_schema.schema.cbegin(), import_definition.input_schema.schema.cend(), [this, &current_line_ptr](SchemaEntry const & column)
			{
			});

			// loop through mappings
			std::for_each(import_definition.mappings.cbegin(), import_definition.mappings.cend(), [this](std::shared_ptr<FieldMapping> const & field_mapping)
			{
				if (!field_mapping)
				{
					// TODO: Import problem warning
					return; // from lambda
				}

				if (!field_mapping->Validate())
				{
					// TODO: Import problem warning
					return; // from lambda
				}

				switch (field_mapping->field_mapping_type)
				{

					case FieldMapping::FIELD_MAPPING_TYPE__ONE_TO_ONE:
						{
							try
							{

								OneToOneFieldMapping const & the_mapping = dynamic_cast<OneToOneFieldMapping const &>(*field_mapping);
								FieldTypeEntry const & output_entry = field_mapping->output_table_fields[0];

								// perform the mapping here

							
							}
							catch (std::bad_cast &)
							{
								// TODO: Import problem warning
								return; // from lambda
							}
						}
						break;

					case FieldMapping::FIELD_MAPPING_TYPE__TIME_RANGE:
						{
							try
							{

								TimeRangeFieldMapping const & the_mapping = dynamic_cast<TimeRangeFieldMapping const &>(*field_mapping);

								// perform the mapping here

							}
							catch (std::bad_cast &)
							{
								// TODO: Import problem warning
								return; // from lambda
							}
						}
						break;

				}
			});

			if (current_lines_read == block_size)
			{
				// Write rows to database here
				current_lines_read = 0;
			}
		}

		if (current_lines_read > 0)
		{
			// Write rows to database here
		}

		data_file.close();
	}


}
