#ifndef WIDGETIDENTIFIER_H
#define WIDGETIDENTIFIER_H

#include <memory>
#include <vector>
#include <string>
#include "../Model/TimeGranularity.h"

typedef std::string UUID;
typedef std::vector<UUID> UUIDVector;

int const UUID_LENGTH = 36;

class WidgetInstanceIdentifier
{

	// ***************************************************************************//
	// A class corresponding to one or more rows in the NewGene backend model tables
	// ***************************************************************************//

public:

	class Notes
	{

	public:
		Notes()
		{

		}
		Notes(Notes const & rhs)
			: notes1(rhs.notes1)
			, notes2(rhs.notes2)
			, notes3(rhs.notes3)
		{

		}
		Notes(std::string notes1, std::string notes2, std::string notes3)
			: notes1(std::make_shared<std::string>(notes1))
			, notes2(std::make_shared<std::string>(notes2))
			, notes3(std::make_shared<std::string>(notes3))
		{

		}
		Notes(std::shared_ptr<std::string> notes1, std::shared_ptr<std::string> notes2, std::shared_ptr<std::string> notes3)
			: notes1(notes1)
			, notes2(notes2)
			, notes3(notes3)
		{

		}
		Notes & operator=(Notes const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}
			notes1 = rhs.notes1;
			notes2 = rhs.notes2;
			notes3 = rhs.notes3;
			return *this;
		}
		std::shared_ptr<std::string> notes1;
		std::shared_ptr<std::string> notes2;
		std::shared_ptr<std::string> notes3;

	};

	WidgetInstanceIdentifier()
		: sequence_number_or_count(0)
		, time_granularity(TIME_GRANULARITY__NONE)
	{

	}

	WidgetInstanceIdentifier(UUID const uuid_, std::string const code_, std::string const description_, int const sequence_number_or_count_, char const * const flags_ = "", TIME_GRANULARITY time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number_or_count(sequence_number_or_count_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, time_granularity(time_granularity_)
		, notes(notes_)
	{
	}

	WidgetInstanceIdentifier(UUID const uuid_, WidgetInstanceIdentifier const & identifier_parent_, std::string const code_, std::string const description_, int const sequence_number_or_count_, char const * const flags_ = "", TIME_GRANULARITY time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		, identifier_parent(std::make_shared<WidgetInstanceIdentifier>(identifier_parent_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number_or_count(sequence_number_or_count_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, notes(notes_)
	{

	}

	WidgetInstanceIdentifier(std::string code_, std::string description_, int const sequence_number_or_count_, char const * const flags_ = "", TIME_GRANULARITY const time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: code(std::make_shared<std::string>(code_))
		, sequence_number_or_count(sequence_number_or_count_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, time_granularity(time_granularity_)
		, notes(notes_)
	{

	}

	WidgetInstanceIdentifier(WidgetInstanceIdentifier const & rhs)
		: uuid(rhs.uuid)
		, identifier_parent(rhs.identifier_parent)
		, foreign_key_identifiers(rhs.foreign_key_identifiers)
		, code(rhs.code)
		, sequence_number_or_count(rhs.sequence_number_or_count)
		, longhand(rhs.longhand)
		, flags(rhs.flags)
		, time_granularity(rhs.time_granularity)
		, notes(rhs.notes)
	{

	}

	WidgetInstanceIdentifier & operator=(WidgetInstanceIdentifier const & rhs)
	{
		if (&rhs == this)
		{
			return *this;
		}
		uuid = rhs.uuid;
		identifier_parent = rhs.identifier_parent;
		foreign_key_identifiers = rhs.foreign_key_identifiers;
		code = rhs.code;
		longhand = rhs.longhand;
		sequence_number_or_count = rhs.sequence_number_or_count;
		flags = rhs.flags;
		time_granularity = rhs.time_granularity;
		notes = rhs.notes;
		return *this;
	}

	std::shared_ptr<UUID> uuid; // In case of ambiguity, sometimes identifier_parent or foreign_key_identifiers are necessary to disambiguate this identifier

	std::shared_ptr<WidgetInstanceIdentifier> identifier_parent; // Parent in the data model (i.e., foreign key in the case of a simple parent-child data relationship), not parent widget in the user interface

	// Regarding comments for this data member: This class is named "WidgetInstanceIdentifier", not just "InstanceIdentifier".
	// The "Instance" in the class name corresponds to a row in the backend model tables.
	// But the "Widget" in the class name corresponds to a widget in the user interface, which usually corresponds to a single instance (row) of a backend model table, but might correspond to a number of different rows from one or more tables.
	std::shared_ptr<std::vector<WidgetInstanceIdentifier>> foreign_key_identifiers; // For more complex foreign key relationships required by this identifier than simple parent-child data relationships, this vector of identifiers can be used.  Note: these identifiers can be used for any application-specific purpose, either directly modeling a row in a backend table, modeling multiple rows in one identifier, representing one or more user-interface widget UUID's, or any other purpose.

	// For simple identifiers, these are frequently used.  Simple identifiers cover the bulk of uses of this class.
	std::shared_ptr<std::string> code;
	std::shared_ptr<std::string> longhand;
	int sequence_number_or_count;
	std::string flags;
	TIME_GRANULARITY time_granularity;
	Notes notes;

};

typedef std::vector<WidgetInstanceIdentifier> WidgetInstanceIdentifiers;
typedef std::pair<WidgetInstanceIdentifier, int> WidgetInstanceIdentifier_Int_Pair;
typedef std::vector<WidgetInstanceIdentifier_Int_Pair> WidgetInstanceIdentifiers_WithInts;

WidgetInstanceIdentifier::Notes MakeNotes(char const * const notes1, char const * const notes2, char const * const notes3);
WidgetInstanceIdentifier::Notes MakeNotes(std::string notes1, std::string notes2, std::string notes3);

#endif
