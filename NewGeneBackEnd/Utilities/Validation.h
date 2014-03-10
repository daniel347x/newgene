#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>

namespace Validation
{

	bool ValidateDmuCode(std::string & proposed_dmu_code, std::string & errorMsg);
	bool ValidateDmuDescription(std::string & proposed_dmu_member_description, std::string & errorMsg);

	bool ValidateDmuMemberUUID(std::string & proposed_dmu_member_uuid, bool const isIntegerType, std::string & errorMsg);
	bool ValidateDmuMemberCode(std::string & proposed_dmu_member_code, std::string & errorMsg);
	bool ValidateDmuMemberDescription(std::string & proposed_dmu_member_description, std::string & errorMsg);

	bool ValidateUoaCode(std::string & proposed_uoa_code, std::string & errorMsg);
	bool ValidateUoaDescription(std::string & proposed_uoa_description, std::string & errorMsg);

	bool ValidateVgCode(std::string & proposed_vg_code, std::string & errorMsg);
	bool ValidateVgDescription(std::string & proposed_vg_description, std::string & errorMsg);

	bool ValidateColumnName(std::string & proposed_column_name, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);
	bool ValidateColumnDescription(std::string & proposed_column_description, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);

	bool ValidateYearInteger(std::string & proposed_year_integer, short & theYear, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);
	bool ValidateMonthInteger(std::string & proposed_month_integer, short & theMonth, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);
	bool ValidateDayInteger(short const theYear, short const theMonth, std::string & proposed_day_integer, short & theDay, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);

	bool ValidateDate1beforeDate2(short const y1, short const m1, short const d1, short const y2, short const m2, short const d2, std::string & errorMsg);

	bool ValidateDatePair(std::string & y1, std::string & m1, std::string & d1, std::string & y2, std::string & m2, std::string & d2, std::string & errorMsg);

	bool ValidateGenericStringField(std::string & generic_field, std::string & errorMsg, bool required = true);

}

#endif
