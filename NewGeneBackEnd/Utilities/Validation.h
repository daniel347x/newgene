#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>

namespace Validation
{

	bool ValidateDmuMemberUUID(std::string & proposed_dmu_member_uuid, std::string & errorMsg);
	bool ValidateDmuMemberCode(std::string & proposed_dmu_member_code, std::string & errorMsg);
	bool ValidateDmuMemberDescription(std::string & proposed_dmu_member_description, std::string & errorMsg);

	bool ValidateColumnName(std::string & proposed_column_name, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg);

}

#endif
