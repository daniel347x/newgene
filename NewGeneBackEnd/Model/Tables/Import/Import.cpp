#include "Import.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/date_time/local_time/local_time.hpp>
#	include <boost/tokenizer.hpp>
#endif
#include <fstream>
#include <cstdint>
#include "../Table.h"
#include "../../OutputModel.h"

void TimeRangeFieldMapping::ConvertStringToDate(int & year, int & month, int & day, std::string const & the_string)
{

	boost::tokenizer<> tok(the_string);
	int count = 0;

	for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it)
	{
		switch (count)
		{
			case 0:
				{
					std::string test = *it;
					year = boost::lexical_cast<int>(*it);
				}
				break;

			case 1:
				{
					std::string test = *it;
					std::string the_month = *it;

					if (boost::iequals(*it, "jan"))
					{
						month = 1;
					}
					else if (boost::iequals(*it, "feb"))
					{
						month = 2;
					}
					else if (boost::iequals(*it, "mar"))
					{
						month = 3;
					}
					else if (boost::iequals(*it, "apr"))
					{
						month = 4;
					}
					else if (boost::iequals(*it, "may"))
					{
						month = 5;
					}
					else if (boost::iequals(*it, "jun"))
					{
						month = 6;
					}
					else if (boost::iequals(*it, "jul"))
					{
						month = 7;
					}
					else if (boost::iequals(*it, "aug"))
					{
						month = 8;
					}
					else if (boost::iequals(*it, "sep"))
					{
						month = 9;
					}
					else if (boost::iequals(*it, "oct"))
					{
						month = 10;
					}
					else if (boost::iequals(*it, "nov"))
					{
						month = 11;
					}
					else if (boost::iequals(*it, "dec"))
					{
						month = 12;
					}
				}
				break;

			case 2:
				{
					std::string test = *it;
					day = boost::lexical_cast<int>(*it);
				}
				break;
		}

		++count;
	}

}

void TimeRangeFieldMapping::PerformMapping(DataFields const & input_data_fields, DataFields const & output_data_fields)
{
	switch (time_range_type)
	{
		case TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__STRING_RANGE:
			{

				std::shared_ptr<BaseField> const the_input_field_datetime_start = RetrieveDataField(input_file_fields[0], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_datetime_end = RetrieveDataField(input_file_fields[1], input_data_fields);
				std::shared_ptr<BaseField> the_output_field_datetime_start = RetrieveDataField(output_table_fields[0], output_data_fields);
				std::shared_ptr<BaseField> the_output_field_datetime_end = RetrieveDataField(output_table_fields[1], output_data_fields);

				if (!the_input_field_datetime_start || !the_input_field_datetime_end || !the_output_field_datetime_start || !the_output_field_datetime_end)
				{
					// Todo: log warning
					return;
				}

				Field<FIELD_TYPE_STRING_FIXED> const & the_input_field_start_string = static_cast<Field<FIELD_TYPE_STRING_FIXED> const &>(*the_input_field_datetime_start);
				Field<FIELD_TYPE_STRING_FIXED> const & the_input_field_end_string = static_cast<Field<FIELD_TYPE_STRING_FIXED> const &>(*the_input_field_datetime_end);
				Field<FIELD_TYPE_INT64> & the_output_field_start_int = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_datetime_start);
				Field<FIELD_TYPE_INT64> & the_output_field_end_int = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_datetime_end);

				boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));

				int year = 0, month = 0, day = 0;
				ConvertStringToDate(year, month, day, the_input_field_start_string.GetValueReference());
				boost::posix_time::ptime time_t_epoch__rowdatestart(boost::gregorian::date(year, month, day));

				year = 0; month = 0; day = 0;
				ConvertStringToDate(year, month, day, the_input_field_end_string.GetValueReference());
				boost::posix_time::ptime time_t_epoch__rowdateend(boost::gregorian::date(year, month, day));

				boost::posix_time::time_duration diff_start_from_1970 = time_t_epoch__rowdatestart - time_t_epoch__1970;
				boost::posix_time::time_duration diff_end_from_1970 = time_t_epoch__rowdateend - time_t_epoch__1970;

				the_output_field_start_int.SetValue(diff_start_from_1970.total_milliseconds());
				the_output_field_end_int.SetValue(diff_end_from_1970.total_milliseconds());

			}
			break;

		case TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__YEAR:
			{

				std::shared_ptr<BaseField> const the_input_field = RetrieveDataField(input_file_fields[0], input_data_fields);
				std::shared_ptr<BaseField> the_output_field_year_start = RetrieveDataField(output_table_fields[0], output_data_fields);
				std::shared_ptr<BaseField> the_output_field_year_end = RetrieveDataField(output_table_fields[1], output_data_fields);

				if (!the_input_field || !the_output_field_year_start || !the_output_field_year_end)
				{
					// Todo: log warning
					return;
				}

				Field<FIELD_TYPE_INT32> const & the_input_field_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field);
				Field<FIELD_TYPE_INT64> & the_output_field_year_start_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_year_start);
				Field<FIELD_TYPE_INT64> & the_output_field_year_end_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_year_end);

				// convert year to ms since jan 1, 1970 00:00:00.000
				boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));
				boost::posix_time::ptime time_t_epoch__rowdatestart(boost::gregorian::date(the_input_field_int32.GetValueReference(), 1, 1));
				boost::posix_time::ptime time_t_epoch__rowdateend(boost::gregorian::date(the_input_field_int32.GetValueReference() + 1, 1, 1));

				boost::posix_time::time_duration diff_start_from_1970 = time_t_epoch__rowdatestart - time_t_epoch__1970;
				boost::posix_time::time_duration diff_end_from_1970 = time_t_epoch__rowdateend - time_t_epoch__1970;

				the_output_field_year_start_int64.SetValue(diff_start_from_1970.total_milliseconds());
				the_output_field_year_end_int64.SetValue(diff_end_from_1970.total_milliseconds());

			}
			break;

		case TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__FROM__YR_MNTH_DAY:
			{

				std::shared_ptr<BaseField> const the_input_field_day = RetrieveDataField(input_file_fields[0], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_month = RetrieveDataField(input_file_fields[1], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_year = RetrieveDataField(input_file_fields[2], input_data_fields);
				std::shared_ptr<BaseField> the_output_field_day_start = RetrieveDataField(output_table_fields[0], output_data_fields);

				if (!the_input_field_day || !the_input_field_month || !the_input_field_year || !the_output_field_day_start)
				{
					// Todo: log warning
					return;
				}

				Field<FIELD_TYPE_INT32> const & the_input_field_day_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_day);
				Field<FIELD_TYPE_INT32> const & the_input_field_month_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_month);
				Field<FIELD_TYPE_INT32> const & the_input_field_year_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_year);
				Field<FIELD_TYPE_INT64> & the_output_field_day_start_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_day_start);

				// convert year to ms since jan 1, 1970 00:00:00.000
				boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));

				boost::gregorian::date row_start_date(the_input_field_year_int32.GetValueReference(),
													  the_input_field_month_int32.GetValueReference() > 0 ? the_input_field_month_int32.GetValueReference() : 12,
													  the_input_field_day_int32.GetValueReference() > 0 ? the_input_field_day_int32.GetValueReference() : 1);

				boost::posix_time::ptime time_t_epoch__rowdatestart(row_start_date);

				boost::posix_time::time_duration diff_start_from_1970 = time_t_epoch__rowdatestart - time_t_epoch__1970;

				the_output_field_day_start_int64.SetValue(diff_start_from_1970.total_milliseconds());

			}
			break;

		case TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__RANGE__FROM__YR_MNTH_DAY:
			{

				std::shared_ptr<BaseField> const the_input_field_day_start = RetrieveDataField(input_file_fields[0], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_month_start = RetrieveDataField(input_file_fields[1], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_year_start = RetrieveDataField(input_file_fields[2], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_day_end = RetrieveDataField(input_file_fields[3], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_month_end = RetrieveDataField(input_file_fields[4], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_year_end = RetrieveDataField(input_file_fields[5], input_data_fields);
				std::shared_ptr<BaseField> the_output_field_day_start = RetrieveDataField(output_table_fields[0], output_data_fields);
				std::shared_ptr<BaseField> the_output_field_day_end = RetrieveDataField(output_table_fields[1], output_data_fields);

				if (!the_input_field_day_start || !the_input_field_month_start || !the_input_field_year_start || !the_input_field_day_end || !the_input_field_month_end
					|| !the_input_field_year_end || !the_output_field_day_start || !the_output_field_day_end)
				{
					// Todo: log warning
					return;
				}

				Field<FIELD_TYPE_INT32> const & the_input_field_day_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_day_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_month_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_month_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_year_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_year_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_day_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_day_end);
				Field<FIELD_TYPE_INT32> const & the_input_field_month_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_month_end);
				Field<FIELD_TYPE_INT32> const & the_input_field_year_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_year_end);
				Field<FIELD_TYPE_INT64> & the_output_field_day_start_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_day_start);
				Field<FIELD_TYPE_INT64> & the_output_field_day_end_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_day_end);

				// convert year to ms since jan 1, 1970 00:00:00.000
				boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));


				int year_start = the_input_field_year_start_int32.GetValueReference();
				int year_end = the_input_field_year_end_int32.GetValueReference();
				int month_start = the_input_field_month_start_int32.GetValueReference();
				int month_end = the_input_field_month_end_int32.GetValueReference();
				int day_start = the_input_field_day_start_int32.GetValueReference();
				int day_end = the_input_field_day_end_int32.GetValueReference();

				bool start_month_valid = month_start > 0;
				bool end_month_valid = month_end > 0;
				bool start_day_valid = day_start > 0;
				bool end_day_valid = day_end > 0;

				if (!start_day_valid && !start_month_valid)
				{
					month_start = 1;
					day_start = 1;
				}
				else if (start_day_valid && !start_month_valid)
				{
					month_start = 1;
				}
				else if (!start_day_valid && start_month_valid)
				{
					day_start = 1;
				}

				// add a day to the end? (since days start at the very end of the previous day - to give the specified day a full 24 hours)
				bool end_handled = false;

				bool add_month = false; // See comment below

				if (!end_day_valid && !end_month_valid)
				{
					year_end += 1;
					month_end = 1;
					day_end = 1;

					// do not add a day to the end; we've already handled it
					end_handled = true;
				}
				else if (end_day_valid && !end_month_valid)
				{
					month_end = 12;
				}
				else if (!end_day_valid && end_month_valid)
				{
					// trick: start at beginning of the known month, and then add a month
					day_end = 1;
					add_month = true;
					end_handled = true; // The end is handled by setting add_month
				}

				boost::gregorian::date row_start_date(year_start, month_start, day_start);
				boost::posix_time::ptime time_t_epoch__rowdatestart(row_start_date);

				boost::gregorian::date row_end_date(year_end, month_end, day_end);

				if (add_month)
				{
					row_end_date += boost::gregorian::months(1);
				}

				if (!end_handled)
				{
					row_end_date += boost::gregorian::days(1);
				}

				boost::posix_time::ptime time_t_epoch__rowdateend(row_end_date);

				boost::posix_time::time_duration diff_start_from_1970 = time_t_epoch__rowdatestart - time_t_epoch__1970;
				boost::posix_time::time_duration diff_end_from_1970 = time_t_epoch__rowdateend - time_t_epoch__1970;

				the_output_field_day_start_int64.SetValue(diff_start_from_1970.total_milliseconds());
				the_output_field_day_end_int64.SetValue(diff_end_from_1970.total_milliseconds());

			}
			break;

		case TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__YEAR__RANGE__FROM__YR_MNTH_DAY:
			{

				std::shared_ptr<BaseField> const the_input_field_day_start = RetrieveDataField(input_file_fields[0], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_month_start = RetrieveDataField(input_file_fields[1], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_year_start = RetrieveDataField(input_file_fields[2], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_day_end = RetrieveDataField(input_file_fields[3], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_month_end = RetrieveDataField(input_file_fields[4], input_data_fields);
				std::shared_ptr<BaseField> const the_input_field_year_end = RetrieveDataField(input_file_fields[5], input_data_fields);
				std::shared_ptr<BaseField> the_output_field_year_start = RetrieveDataField(output_table_fields[0], output_data_fields);
				std::shared_ptr<BaseField> the_output_field_year_end = RetrieveDataField(output_table_fields[1], output_data_fields);

				if (!the_input_field_day_start || !the_input_field_month_start || !the_input_field_year_start || !the_input_field_day_end || !the_input_field_month_end
					|| !the_input_field_year_end || !the_output_field_year_start || !the_output_field_year_end)
				{
					// Todo: log warning
					return;
				}

				Field<FIELD_TYPE_INT32> const & the_input_field_day_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_day_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_month_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_month_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_year_start_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_year_start);
				Field<FIELD_TYPE_INT32> const & the_input_field_day_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_day_end);
				Field<FIELD_TYPE_INT32> const & the_input_field_month_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_month_end);
				Field<FIELD_TYPE_INT32> const & the_input_field_year_end_int32 = static_cast<Field<FIELD_TYPE_INT32> const &>(*the_input_field_year_end);
				Field<FIELD_TYPE_INT64> & the_output_field_year_start_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_year_start);
				Field<FIELD_TYPE_INT64> & the_output_field_year_end_int64 = static_cast<Field<FIELD_TYPE_INT64> &>(*the_output_field_year_end);

				// convert year to ms since jan 1, 1970 00:00:00.000
				boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));
				boost::gregorian::date row_start_date(the_input_field_year_start_int32.GetValueReference(), 1, 1);
				boost::posix_time::ptime time_t_epoch__rowdatestart(row_start_date);
				boost::gregorian::date row_end_date(the_input_field_year_end_int32.GetValueReference() + 1, 1, 1);
				boost::posix_time::ptime time_t_epoch__rowdateend(row_end_date);

				boost::posix_time::time_duration diff_start_from_1970 = time_t_epoch__rowdatestart - time_t_epoch__1970;
				boost::posix_time::time_duration diff_end_from_1970 = time_t_epoch__rowdateend - time_t_epoch__1970;

				the_output_field_year_start_int64.SetValue(diff_start_from_1970.total_milliseconds());
				the_output_field_year_end_int64.SetValue(diff_end_from_1970.total_milliseconds());

			}
			break;
	}
}

ImportDefinition::ImportDefinition()
{
}

ImportDefinition::ImportDefinition(ImportDefinition const & rhs)
	: mappings(rhs.mappings)
	, input_file(rhs.input_file)
	, first_row_is_header_row(rhs.first_row_is_header_row)
	, second_row_is_data_type_row(rhs.second_row_is_data_type_row)
	, input_schema(rhs.input_schema)
	, output_schema(rhs.output_schema)
	, format_qualifiers(rhs.format_qualifiers)
	, import_type(rhs.import_type)
{
}

bool ImportDefinition::IsEmpty()
{
	if (mappings.size() == 0)
	{
		return true;
	}

	return false;
}

Importer::Importer(ImportDefinition const & import_definition_, Model_basemost * model_, Table_basemost * table_, Mode const mode_, WidgetInstanceIdentifier const & identifier_,
				   TableImportCallbackFn table_write_callback_)
	: import_definition(import_definition_)
	, table_write_callback(table_write_callback_)
	, model(model_)
	, table(table_)
	, identifier(identifier_)
	, mode(mode_)
{
}

void Importer::InitializeFields()
{

	for (int n = 0; n < block_size; ++n)
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
						std::string testname1 = field->GetName();
						fields.push_back(field);
						std::shared_ptr<BaseField> test = fields[fields.size() - 1];
						std::string testname = test->GetName();
						BaseField * testcase = test.get();
						std::string testname3 = testcase->GetName();
					}
					break;

				case FIELD_TYPE_STRING_VAR:
					{
						std::shared_ptr<Field<FIELD_TYPE_STRING_VAR>> field = std::make_shared<Field<FIELD_TYPE_STRING_VAR>>(field_name);
						fields.push_back(field);
					}
					break;

				case FIELD_TYPE_FLOAT:
					{
						std::shared_ptr<Field<FIELD_TYPE_FLOAT>> field = std::make_shared<Field<FIELD_TYPE_FLOAT>>(field_name);
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

		input_block.push_back(fields);

	}

	for (int n = 0; n < block_size; ++n)
	{

		DataFields fields;

		std::for_each(import_definition.output_schema.schema.cbegin(), import_definition.output_schema.schema.cend(), [&fields](SchemaEntry const & column)
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

				case FIELD_TYPE_FLOAT:
					{
						std::shared_ptr<Field<FIELD_TYPE_FLOAT>> field = std::make_shared<Field<FIELD_TYPE_FLOAT>>(field_name);
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

		output_block.push_back(fields);

	}

}

bool Importer::ValidateMapping()
{

	std::vector<std::pair<int, bool>> indices_required_are_matched;

	// loop through input schema pushing back required indices
	int current_index = 0;
	std::for_each(import_definition.input_schema.schema.cbegin(),
				  import_definition.input_schema.schema.cend(), [this, &indices_required_are_matched, &current_index](SchemaEntry const & schema_entry)
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
			return;
		}

		if (!field_mapping->Validate())
		{
			// TODO: Import problem warning
			return;
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
						std::for_each(import_definition.output_schema.schema.cbegin(),
									  import_definition.output_schema.schema.cend(), [this, &output_entry, &indices_required_are_matched, &current_index](SchemaEntry const & output_schema_entry)
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
						return;
					}
				}
				break;

			case FieldMapping::FIELD_MAPPING_TYPE__HARD_CODED:
				{
					try
					{
						HardCodedFieldMapping const & the_mapping = dynamic_cast<HardCodedFieldMapping const &>(*field_mapping);
						FieldTypeEntry const & output_entry = field_mapping->output_table_fields[0];

						// search the output schema to see what numeric index this corresponds to, and set it in indices_required_are_matched
						int current_index = 0;
						std::for_each(import_definition.output_schema.schema.cbegin(),
									  import_definition.output_schema.schema.cend(), [this, &output_entry, &indices_required_are_matched, &current_index](SchemaEntry const & output_schema_entry)
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
						return;
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
						return;
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

void Importer::RetrieveStringField(char *& current_line_ptr, char *& parsed_line_ptr, bool & stop)
{
	bool is_quoted_field = false;
	char * starting_parsed_pointer = parsed_line_ptr;

	if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_SINGLEQUOTED)
	{
		is_quoted_field = true;

		if (*current_line_ptr != '\'')
		{
			is_quoted_field = false;
		}
		else
		{
			++current_line_ptr;
		}
	}
	else if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED)
	{
		is_quoted_field = true;

		if (*current_line_ptr != '"')
		{
			is_quoted_field = false;
		}
		else
		{
			++current_line_ptr;
		}
	}

	if (*current_line_ptr == '\0')
	{
		// Todo: warning
		stop = true;
		return;
	}

	bool escapemode = false;
	bool done = false;

	while (!done)
	{
		if (*current_line_ptr == '\0')
		{
			// Whitespace on right
			if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__TAB_DELIMITED)
			{
				while (parsed_line_ptr > starting_parsed_pointer)
				{
					--parsed_line_ptr;

					if (*parsed_line_ptr == ' ')
					{
						*parsed_line_ptr = '\0';
					}
					else
					{
						++parsed_line_ptr;
						break;
					}
				}
			}
			else
			{
				while (parsed_line_ptr > starting_parsed_pointer)
				{
					--parsed_line_ptr;

					if (*parsed_line_ptr == ' ' || *parsed_line_ptr == '\t')
					{
						*parsed_line_ptr = '\0';
					}
					else
					{
						++parsed_line_ptr;
						break;
					}
				}
			}

			done = true;
			continue;
		}

		if (escapemode)
		{
			*(parsed_line_ptr) = *current_line_ptr;
			++parsed_line_ptr;
			*parsed_line_ptr = '\0';
			escapemode = false;
		}
		else
		{
			if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__BACKSLASH_ESCAPE_CHAR)
			{
				if (*current_line_ptr == '\\')
				{
					escapemode = true;
					++current_line_ptr;
					continue;
				}
			}

			if (is_quoted_field && import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_SINGLEQUOTED)
			{
				if (*current_line_ptr == '\'')
				{
					done = true;
				}
			}
			else if (is_quoted_field && import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED)
			{
				if (*current_line_ptr == '"')
				{
					done = true;
				}
			}
			else
			{
				if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__TAB_DELIMITED)
				{
					if (*current_line_ptr == '\t')
					{
						done = true;
					}
				}
				else
				{
					if (*current_line_ptr == ',')
					{
						done = true;
					}
				}
			}

			if (!done)
			{
				*(parsed_line_ptr) = *current_line_ptr;
				++parsed_line_ptr;
				*parsed_line_ptr = '\0';
			}
		}

		++current_line_ptr;

		if (done)
		{
			// Whitespace on right
			if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__TAB_DELIMITED)
			{
				while (parsed_line_ptr > starting_parsed_pointer)
				{
					--parsed_line_ptr;

					if (*parsed_line_ptr == ' ')
					{
						*parsed_line_ptr = '\0';
					}
					else
					{
						++parsed_line_ptr;
						break;
					}
				}
			}
			else
			{
				while (parsed_line_ptr > starting_parsed_pointer)
				{
					--parsed_line_ptr;

					if (*parsed_line_ptr == ' ' || *parsed_line_ptr == '\t')
					{
						*parsed_line_ptr = '\0';
					}
					else
					{
						++parsed_line_ptr;
						break;
					}
				}
			}

			++parsed_line_ptr;
			*parsed_line_ptr = '\0';
		}
	}

	parsed_line_ptr = starting_parsed_pointer;
}

std::shared_ptr<BaseField> RetrieveDataField(FieldTypeEntry const & field_type_entry, DataFields const & data_fields)
{
	std::shared_ptr<BaseField> the_field;

	if (field_type_entry.first.name_or_index == NameOrIndex::NAME)
	{
		std::string const & input_field_name = field_type_entry.first.name;
		std::for_each(data_fields.cbegin(), data_fields.cend(), [&input_field_name, &the_field](std::shared_ptr<BaseField> const & input_field)
		{
			if (input_field && boost::iequals(input_field->GetName(), input_field_name))
			{
				the_field = input_field;
			}
		});
	}
	else
	{
		if (field_type_entry.first.index < (int)data_fields.size())
		{
			the_field = data_fields[field_type_entry.first.index];
		}
	}

	return the_field;
}

void Importer::ReadFieldFromFile(char *& current_line_ptr, int & current_lines_read, int const & current_column_index, char *& parsed_line_ptr, bool & stop,
								 SchemaEntry const & column)
{

	// use sscanf to read in the type, based on switch on "column"'s type type-traits... read it into input_block[current_lines_read]
	std::shared_ptr<BaseField> field_entry = input_block[current_lines_read][current_column_index];

	if (!field_entry)
	{
		// Todo: warning log here
		stop = true;
		return;
	}

	BaseField & theField = *field_entry;

	EatWhitespace(current_line_ptr);

	if (*current_line_ptr == '\0')
	{
		// Todo: warning
		stop = true;
		return;
	}

	ReadOneDataField(column, theField, current_line_ptr, parsed_line_ptr, stop);

	if (stop)
	{
		return;
	}

	EatWhitespace(current_line_ptr);
	EatSeparator(current_line_ptr);

}

void Importer::SkipFieldInFile(char *& current_line_ptr, char *& parsed_line_ptr, bool & stop)
{

	EatWhitespace(current_line_ptr);

	if (*current_line_ptr == '\0')
	{
		// Todo: warning
		stop = true;
		return;
	}

	// read it, and do nothing with it
	RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

	if (stop)
	{
		return;
	}

	EatWhitespace(current_line_ptr);
	EatSeparator(current_line_ptr);

}

int Importer::ReadBlockFromFile(std::fstream & data_file, char * line, char * parsedline)
{
	int current_lines_read = 0;

	while (data_file.getline(line, MAX_LINE_SIZE - 1) && data_file.good())
	{

		char * current_line_ptr = line;
		char * parsed_line_ptr = parsedline;
		*parsed_line_ptr = '\0';
		bool stop = false;

		int nCols = import_definition.input_schema.validcols.size();
		int nValColIdx = 0;

		for (int ncol = 0; ncol < nCols; ++ncol)
		{
			if (stop)
			{
				break;
			}

			if (import_definition.input_schema.validcols[ncol])
			{
				ReadFieldFromFile(current_line_ptr, current_lines_read, nValColIdx, parsed_line_ptr, stop, import_definition.input_schema.schema[nValColIdx]);
				++nValColIdx;
			}
			else
			{
				SkipFieldInFile(current_line_ptr, parsed_line_ptr, stop);
			}
		}

		if (stop)
		{
			return -1;
		}

		// loop through mappings to generate output
		DataFields & input_data_fields = input_block[current_lines_read];
		DataFields & output_data_fields = output_block[current_lines_read];
		stop = false;
		std::for_each(import_definition.mappings.begin(), import_definition.mappings.end(), [this, &input_data_fields, &output_data_fields, &stop](
						  std::shared_ptr<FieldMapping> & field_mapping)
		{
			if (!field_mapping)
			{
				// TODO: Import problem warning
				stop = true;
				return;
			}

			if (!field_mapping->Validate())
			{
				// TODO: Import problem warning
				stop = true;
				return;
			}

			switch (field_mapping->field_mapping_type)
			{

				case FieldMapping::FIELD_MAPPING_TYPE__ONE_TO_ONE:
					{
						try
						{

							OneToOneFieldMapping const & the_mapping = dynamic_cast<OneToOneFieldMapping const &>(*field_mapping);
							FieldTypeEntry const & input_entry = field_mapping->input_file_fields[0];
							FieldTypeEntry const & output_entry = field_mapping->output_table_fields[0];

							if (input_entry.second != output_entry.second)
							{
								// Todo: log warning
								stop = true;
								return;
							}

							// perform the mapping here

							std::shared_ptr<BaseField> the_input_field = RetrieveDataField(input_entry, input_data_fields);
							std::shared_ptr<BaseField> the_output_field = RetrieveDataField(output_entry, output_data_fields);;

							if (the_input_field && the_output_field)
							{
								// map them
								try
								{
									switch (the_input_field->GetType())
									{
										case FIELD_TYPE_INT32:
											{
												Field<FIELD_TYPE_INT32> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_INT32>&>(*the_input_field);
												Field<FIELD_TYPE_INT32> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_INT32>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_INT64:
											{
												Field<FIELD_TYPE_INT64> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_INT64>&>(*the_input_field);
												Field<FIELD_TYPE_INT64> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_INT64>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UINT32:
											{
												Field<FIELD_TYPE_UINT32> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UINT32>&>(*the_input_field);
												Field<FIELD_TYPE_UINT32> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UINT32>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UINT64:
											{
												Field<FIELD_TYPE_UINT64> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UINT64>&>(*the_input_field);
												Field<FIELD_TYPE_UINT64> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UINT64>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_FIXED:
											{
												Field<FIELD_TYPE_STRING_FIXED> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_FIXED>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_FIXED> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_FIXED>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_VAR:
											{
												Field<FIELD_TYPE_STRING_VAR> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_VAR>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_VAR> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_VAR>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_FLOAT:
											{
												Field<FIELD_TYPE_FLOAT> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_FLOAT>&>(*the_input_field);
												Field<FIELD_TYPE_FLOAT> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_FLOAT>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_TIMESTAMP:
											{
												Field<FIELD_TYPE_TIMESTAMP> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_TIMESTAMP>&>(*the_input_field);
												Field<FIELD_TYPE_TIMESTAMP> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_TIMESTAMP>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UUID:
											{
												Field<FIELD_TYPE_UUID> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UUID>&>(*the_input_field);
												Field<FIELD_TYPE_UUID> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UUID>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UUID_FOREIGN:
											{
												Field<FIELD_TYPE_UUID_FOREIGN> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UUID_FOREIGN>&>(*the_input_field);
												Field<FIELD_TYPE_UUID_FOREIGN> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UUID_FOREIGN>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_CODE:
											{
												Field<FIELD_TYPE_STRING_CODE> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_CODE>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_CODE> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_CODE>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_LONGHAND:
											{
												Field<FIELD_TYPE_STRING_LONGHAND> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_LONGHAND>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_LONGHAND> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_LONGHAND>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_TIME_RANGE:
											{
												Field<FIELD_TYPE_TIME_RANGE> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_TIME_RANGE>&>(*the_input_field);
												Field<FIELD_TYPE_TIME_RANGE> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_TIME_RANGE>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_1:
											{
												Field<FIELD_TYPE_NOTES_1> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_1>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_1> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_1>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_2:
											{
												Field<FIELD_TYPE_NOTES_2> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_2>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_2> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_2>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_3:
											{
												Field<FIELD_TYPE_NOTES_3> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_3>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_3> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_3>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;
									}
								}
								catch (std::bad_cast &)
								{
									// Todo: log warning
									stop = true;
									return;
								}
							}

						}
						catch (std::bad_cast &)
						{
							// TODO: Import problem warning
							stop = true;
							return;
						}
					}
					break;

				case FieldMapping::FIELD_MAPPING_TYPE__HARD_CODED:
					{
						try
						{

							HardCodedFieldMapping const & the_mapping = dynamic_cast<HardCodedFieldMapping const &>(*field_mapping);
							FieldTypeEntry const & output_entry = field_mapping->output_table_fields[0];

							// perform the mapping here

							std::shared_ptr<BaseField> the_input_field = the_mapping.data;
							std::shared_ptr<BaseField> the_output_field = RetrieveDataField(output_entry, output_data_fields);

							if (the_input_field && the_output_field)
							{

								if (the_input_field->GetType() != the_output_field->GetType())
								{
									// Todo: log warning
									stop = true;
									return;
								}

								// map them
								try
								{
									switch (the_input_field->GetType())
									{
										case FIELD_TYPE_INT32:
											{
												Field<FIELD_TYPE_INT32> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_INT32>&>(*the_input_field);
												Field<FIELD_TYPE_INT32> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_INT32>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_INT64:
											{
												Field<FIELD_TYPE_INT64> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_INT64>&>(*the_input_field);
												Field<FIELD_TYPE_INT64> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_INT64>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UINT32:
											{
												Field<FIELD_TYPE_UINT32> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UINT32>&>(*the_input_field);
												Field<FIELD_TYPE_UINT32> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UINT32>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UINT64:
											{
												Field<FIELD_TYPE_UINT64> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UINT64>&>(*the_input_field);
												Field<FIELD_TYPE_UINT64> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UINT64>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_FIXED:
											{
												Field<FIELD_TYPE_STRING_FIXED> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_FIXED>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_FIXED> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_FIXED>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_VAR:
											{
												Field<FIELD_TYPE_STRING_VAR> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_VAR>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_VAR> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_VAR>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_TIMESTAMP:
											{
												Field<FIELD_TYPE_TIMESTAMP> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_TIMESTAMP>&>(*the_input_field);
												Field<FIELD_TYPE_TIMESTAMP> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_TIMESTAMP>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UUID:
											{
												Field<FIELD_TYPE_UUID> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UUID>&>(*the_input_field);
												Field<FIELD_TYPE_UUID> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UUID>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_UUID_FOREIGN:
											{
												Field<FIELD_TYPE_UUID_FOREIGN> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_UUID_FOREIGN>&>(*the_input_field);
												Field<FIELD_TYPE_UUID_FOREIGN> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_UUID_FOREIGN>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_CODE:
											{
												Field<FIELD_TYPE_STRING_CODE> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_CODE>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_CODE> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_CODE>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_STRING_LONGHAND:
											{
												Field<FIELD_TYPE_STRING_LONGHAND> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_STRING_LONGHAND>&>(*the_input_field);
												Field<FIELD_TYPE_STRING_LONGHAND> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_STRING_LONGHAND>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_TIME_RANGE:
											{
												Field<FIELD_TYPE_TIME_RANGE> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_TIME_RANGE>&>(*the_input_field);
												Field<FIELD_TYPE_TIME_RANGE> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_TIME_RANGE>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_1:
											{
												Field<FIELD_TYPE_NOTES_1> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_1>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_1> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_1>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_2:
											{
												Field<FIELD_TYPE_NOTES_2> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_2>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_2> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_2>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;

										case FIELD_TYPE_NOTES_3:
											{
												Field<FIELD_TYPE_NOTES_3> & data_entry_input = dynamic_cast<Field<FIELD_TYPE_NOTES_3>&>(*the_input_field);
												Field<FIELD_TYPE_NOTES_3> & data_entry_output = dynamic_cast<Field<FIELD_TYPE_NOTES_3>&>(*the_output_field);
												data_entry_output.SetValue(data_entry_input.GetValueReference());
											}
											break;
									}
								}
								catch (std::bad_cast &)
								{
									// Todo: log warning
									stop = true;
									return;
								}
							}

						}
						catch (std::bad_cast &)
						{
							// TODO: Import problem warning
							stop = true;
							return;
						}
					}
					break;

				case FieldMapping::FIELD_MAPPING_TYPE__TIME_RANGE:
					{
						try
						{

							TimeRangeFieldMapping & the_mapping = dynamic_cast<TimeRangeFieldMapping &>(*field_mapping);
							the_mapping.PerformMapping(input_data_fields, output_data_fields);

						}
						catch (std::bad_cast &)
						{
							// TODO: Import problem warning
							stop = true;
							return;
						}
					}
					break;

			}
		});

		if (stop)
		{
			return -1;
		}

		++current_lines_read;

		if (current_lines_read == block_size)
		{
			return current_lines_read;
		}
	}

	return current_lines_read;
}

bool Importer::DoImport()
{

	InputModel * inputModel = nullptr;
	OutputModel * outputModel = nullptr;

	if (table->table_model_type == Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
	{
		try
		{
			inputModel = dynamic_cast<InputModel *>(model);
		}
		catch (std::bad_cast &)
		{
			return false;
		}

		if (!table->ImportStart(inputModel->getDb(), identifier, import_definition, nullptr, inputModel))
		{
			// Todo: log warning
			return false;
		}
	}

	if (table->table_model_type == Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{
		try
		{
			outputModel = dynamic_cast<OutputModel *>(model);
		}
		catch (std::bad_cast &)
		{
			return false;
		}

		inputModel = &outputModel->getInputModel();

		if (!table->ImportStart(outputModel->getDb(), identifier, import_definition, outputModel, inputModel))
		{
			// Todo: log warning
			return false;
		}
	}

	// TODO: try/finally or exit block
	// open file
	std::fstream data_file;
	data_file.open(import_definition.input_file.c_str(), std::ios::in);

	if (data_file.is_open())
	{

		char line[MAX_LINE_SIZE];
		char parsedline[MAX_LINE_SIZE];

		// skip first row if necessary
		if (import_definition.first_row_is_header_row && data_file.good())
		{
			data_file.getline(line, MAX_LINE_SIZE - 1);
		}

		if (!data_file.good())
		{
			data_file.close();
			return true;
		}

		std::vector<std::string> colnames;
		boost::split(colnames, line, boost::is_any_of(","));
		std::transform(colnames.begin(), colnames.end(), colnames.begin(), boost::trim<std::string>);
		import_definition.input_schema.ReorderAccToColumnNames(colnames);

		// skip second row if necessary
		if (import_definition.second_row_is_data_type_row && data_file.good())
		{
			data_file.getline(line, MAX_LINE_SIZE - 1);
		}

		if (!data_file.good())
		{
			data_file.close();
			return true;
		}

		InitializeFields();

		// validate
		if (!ValidateMapping())
		{
			// TODO: Error logging
			data_file.close();
			return false;
		}

		std::int64_t current_lines_read = 0;

		int currently_read_lines = 0;

		while (true)
		{
			currently_read_lines = ReadBlockFromFile(data_file, line, parsedline);

			if (currently_read_lines == -1)
			{
				// Todo: log warning
				return false;
			}
			else if (currently_read_lines == 0)
			{
				// nothing to do
				break;
			}
			else
			{
				// Write rows to database here
				bool write_succeeded = table_write_callback(this, model, import_definition, table, output_block, currently_read_lines);

				if (!write_succeeded)
				{
					// Todo: log warning
					return false;
				}
			}
		}

		data_file.close();
	}
	else
	{
		// Todo: log file open failure warning
		return false;
	}

	if (table->table_model_type == Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
	{
		if (!table->ImportEnd(inputModel->getDb(), identifier, import_definition, nullptr, inputModel))
		{
			// Todo: log warning
			return false;
		}

	}

	if (table->table_model_type == Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{
		if (!table->ImportEnd(outputModel->getDb(), identifier, import_definition, outputModel, inputModel))
		{
			// Todo: log warning
			return false;
		}
	}

	return true;

}

void Importer::EatWhitespace(char *& current_line_ptr)
{
	if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__TAB_DELIMITED)
	{
		while (*current_line_ptr == ' ') { ++current_line_ptr; }
	}
	else
	{
		while (*current_line_ptr == ' ' || *current_line_ptr == '\t') { ++current_line_ptr; }
	}
}

void Importer::EatSeparator(char *& current_line_ptr)
{
	if (import_definition.format_qualifiers & ImportDefinition::FORMAT_QUALIFIERS__TAB_DELIMITED)
	{
		if (*current_line_ptr == '\t') { ++current_line_ptr; }
	}
	else
	{
		if (*current_line_ptr == ',') { ++current_line_ptr; }
	}
}

void Importer::ReadOneDataField(SchemaEntry const & column, BaseField & theField, char *& current_line_ptr, char *& parsed_line_ptr, bool & stop)
{
	try
	{
		int number_chars_read = 0;

		switch (column.field_type)
		{
			case FIELD_TYPE_INT32:
				{
					Field<FIELD_TYPE_INT32> & data_entry = dynamic_cast<Field<FIELD_TYPE_INT32>&>(theField);
					sscanf(current_line_ptr, "%d%n", &data_entry.GetValueReference(), &number_chars_read);
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_INT64:
				{
					Field<FIELD_TYPE_INT64> & data_entry = dynamic_cast<Field<FIELD_TYPE_INT64>&>(theField);
					sscanf(current_line_ptr, "%I64d%n", &data_entry.GetValueReference(), &number_chars_read);
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_UINT32:
				{
					Field<FIELD_TYPE_UINT32> & data_entry = dynamic_cast<Field<FIELD_TYPE_UINT32>&>(theField);
					sscanf(current_line_ptr, "%d%n", &data_entry.GetValueReference(), &number_chars_read);
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_UINT64:
				{
					Field<FIELD_TYPE_UINT64> & data_entry = dynamic_cast<Field<FIELD_TYPE_UINT64>&>(theField);
					sscanf(current_line_ptr, "%I64d%n", &data_entry.GetValueReference(), &number_chars_read);
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_STRING_FIXED:
				{
					Field<FIELD_TYPE_STRING_FIXED> & data_entry = dynamic_cast<Field<FIELD_TYPE_STRING_FIXED>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_STRING_VAR:
				{
					Field<FIELD_TYPE_STRING_VAR> & data_entry = dynamic_cast<Field<FIELD_TYPE_STRING_VAR>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_FLOAT:
				{
					Field<FIELD_TYPE_FLOAT> & data_entry = dynamic_cast<Field<FIELD_TYPE_FLOAT>&>(theField);
					float temp;
					sscanf(current_line_ptr, "%f%n", &temp, &number_chars_read);
					data_entry.GetValueReference() = temp;
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_TIMESTAMP:
				{
					Field<FIELD_TYPE_TIMESTAMP> & data_entry = dynamic_cast<Field<FIELD_TYPE_TIMESTAMP>&>(theField);
					sscanf(current_line_ptr, "%I64d%n", &data_entry.GetValueReference());
					current_line_ptr += number_chars_read;
				}
				break;

			case FIELD_TYPE_UUID:
				{
					Field<FIELD_TYPE_UUID> & data_entry = dynamic_cast<Field<FIELD_TYPE_UUID>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_UUID_FOREIGN:
				{
					Field<FIELD_TYPE_UUID_FOREIGN> & data_entry = dynamic_cast<Field<FIELD_TYPE_UUID_FOREIGN>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_STRING_CODE:
				{
					Field<FIELD_TYPE_STRING_CODE> & data_entry = dynamic_cast<Field<FIELD_TYPE_STRING_CODE>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_STRING_LONGHAND:
				{
					Field<FIELD_TYPE_STRING_LONGHAND> & data_entry = dynamic_cast<Field<FIELD_TYPE_STRING_LONGHAND>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_TIME_RANGE:
				{
					Field<FIELD_TYPE_TIME_RANGE> & data_entry = dynamic_cast<Field<FIELD_TYPE_TIME_RANGE>&>(theField);
					sscanf(current_line_ptr, "%I64d%n", &data_entry.GetValueReference(), &number_chars_read);
				}
				break;

			case FIELD_TYPE_NOTES_1:
				{
					Field<FIELD_TYPE_NOTES_1> & data_entry = dynamic_cast<Field<FIELD_TYPE_NOTES_1>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_NOTES_2:
				{
					Field<FIELD_TYPE_NOTES_2> & data_entry = dynamic_cast<Field<FIELD_TYPE_NOTES_2>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;

			case FIELD_TYPE_NOTES_3:
				{
					Field<FIELD_TYPE_NOTES_3> & data_entry = dynamic_cast<Field<FIELD_TYPE_NOTES_3>&>(theField);
					RetrieveStringField(current_line_ptr, parsed_line_ptr, stop);

					if (stop)
					{
						return;
					}

					data_entry.SetValue(parsed_line_ptr);
				}
				break;
		}
	}
	catch (std::bad_cast &)
	{
		// Todo: log warning here
		stop = true;
		return;
	}
}
