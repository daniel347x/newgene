#include "Validation.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/regex.hpp>
#	include <boost/format.hpp>
#endif

bool Validation::ValidateDmuMemberUUID(std::string & proposed_dmu_member_uuid, std::string & errorMsg)
{

	boost::trim(proposed_dmu_member_uuid);

	if (proposed_dmu_member_uuid.empty())
	{
		boost::format msg("The DMU member you entered is empty.");
		errorMsg = msg.str();
		return false;
	}

	std::string regex_string_uuid("([a-zA-Z_0-9]*)");
	boost::regex regex_uuid(regex_string_uuid);
	boost::cmatch matches_uuid;

	bool valid = false;
	std::string invalid_string;
	if (boost::regex_match(proposed_dmu_member_uuid.c_str(), matches_uuid, regex_uuid))
	{
		// matches[0] contains the original string.  matches[n]
		// contains a sub_match object for each matching
		// subexpression
		// ... see http://www.onlamp.com/pub/a/onlamp/2006/04/06/boostregex.html?page=3
		// for an example usage
		if (matches_uuid.size() == 2)
		{
			std::string the_dmu_member_uuid_match(matches_uuid[1].first, matches_uuid[1].second);

			if (the_dmu_member_uuid_match.size() <= 36)
			{
				if (the_dmu_member_uuid_match == proposed_dmu_member_uuid)
				{
					valid = true;
				}
			}
			else
			{
				invalid_string = ": The length is too long (maximum length: 36).";
			}

		}
	}

	if (!valid)
	{
		boost::format msg("The DMU member code you entered is invalid%1%");
		msg % invalid_string;
		errorMsg = msg.str();
	}

	return valid;

}

bool Validation::ValidateDmuMemberCode(std::string & proposed_dmu_member_code, std::string & errorMsg)
{

	boost::trim(proposed_dmu_member_code);

	std::string regex_string_code("([a-zA-Z_0-9]*)");
	boost::regex regex_code(regex_string_code);
	boost::cmatch matches_code;

	bool valid = true;
	if (valid && boost::regex_match(proposed_dmu_member_code.c_str(), matches_code, regex_code))
	{
		// matches[0] contains the original string.  matches[n]
		// contains a sub_match object for each matching
		// subexpression
		// ... see http://www.onlamp.com/pub/a/onlamp/2006/04/06/boostregex.html?page=3
		// for an example usage
		if (valid && matches_code.size() == 2)
		{
			std::string the_dmu_member_code_match(matches_code[1].first, matches_code[1].second);

			if (valid && the_dmu_member_code_match == proposed_dmu_member_code)
			{
				 // no-op
			}
			else
			{
				boost::format msg("The abbreviation is invalid.");
				errorMsg = msg.str();
				valid = false;
			}

		}
		else
		{
			boost::format msg("The abbreviation is invalid.");
			errorMsg = msg.str();
			valid = false;
		}
	}

	if (valid)
	{
		if (valid && proposed_dmu_member_code.size() > 128)
		{
			boost::format msg("The abbreviation is too long (maximum length: 128).");
			errorMsg = msg.str();
			valid = false;
		}
	}

	return valid;

}

bool Validation::ValidateDmuMemberDescription(std::string & proposed_dmu_member_description, std::string & errorMsg)
{

	boost::trim(proposed_dmu_member_description);

	bool valid = true;
	if (proposed_dmu_member_description.size() > 4096)
	{
		boost::format msg("The description is too long (maximum length: 4096).");
		errorMsg = msg.str();
		valid = false;
	}

	return valid;

}

bool Validation::ValidateColumnName(std::string & proposed_column_name, std::string const & column_description_for_invalid_message, bool const required, std::string & errorMsg)
{

	boost::trim(proposed_column_name);

	bool valid = true;
	if (proposed_column_name.empty())
	{
		if (required)
		{
			boost::format msg("The '%1%' column name cannot be empty.");
			msg % column_description_for_invalid_message;
			errorMsg = msg.str();
			valid = false;
		}
	}

	if (proposed_column_name.size() > 1024)
	{
		boost::format msg("The '%1%' column name is too long.");
		msg % column_description_for_invalid_message;
		errorMsg = msg.str();
		valid = false;
	}

	std::string regex_string("([a-zA-Z_0-9]*)");
	boost::regex regex(regex_string);
	boost::cmatch matches;

	bool valid = true;
	if (valid && boost::regex_match(proposed_column_name.c_str(), matches, regex))
	{
		// matches[0] contains the original string.  matches[n]
		// contains a sub_match object for each matching
		// subexpression
		// ... see http://www.onlamp.com/pub/a/onlamp/2006/04/06/boostregex.html?page=3
		// for an example usage
		if (valid && matches.size() == 2)
		{
			std::string the_match(matches[1].first, matches[1].second);

			if (valid && the_match == proposed_column_name)
			{
				// no-op
			}
			else
			{
				boost::format msg("The '%1%' column name is invalid.");
				msg % column_description_for_invalid_message;
				errorMsg = msg.str();
				valid = false;
			}

		}
		else
		{
			boost::format msg("The '%1%' column name is invalid.");
			msg % column_description_for_invalid_message;
			errorMsg = msg.str();
			valid = false;
		}
	}

	return valid;

}
