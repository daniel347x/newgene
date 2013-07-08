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
		: sequence_number(0)
		, time_granularity(TIME_GRANULARITY__NONE)
	{

	}

	WidgetInstanceIdentifier(UUID const uuid_, std::string const code_, std::string const description_, int const sequence_number_, char const * const flags_ = "", TIME_GRANULARITY time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, time_granularity(time_granularity_)
		, notes(notes_)
	{
//		if (flags_)
//		{
//			flags = flags_;
//		}
//		else
//		{
//			flags = "";
//		}
	}

	//WidgetInstanceIdentifier(UUID const uuid_, UUID const uuid_parent_, std::string const code_, std::string const description_, int const sequence_number_, Notes notes_ = Notes())
	WidgetInstanceIdentifier(UUID const uuid_, WidgetInstanceIdentifier const & identifier_parent_, std::string const code_, std::string const description_, int const sequence_number_, char const * const flags_ = "", TIME_GRANULARITY time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		//, uuid_parent(std::make_shared<UUID>(uuid_parent_))
		, identifier_parent(std::make_shared<WidgetInstanceIdentifier>(identifier_parent_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, notes(notes_)
	{

	}

	//WidgetInstanceIdentifier(UUID uuid_)
	//	: uuid(std::make_shared<UUID>(uuid_))
	//	, sequence_number(0)
	//{

	//}

	//WidgetInstanceIdentifier(UUID uuid_, UUID uuid_parent_)
	//	: uuid(std::make_shared<UUID>(uuid_))
	//	, uuid_parent(std::make_shared<UUID>(uuid_parent_))
	//	, sequence_number(0)
	//{

	//}

	WidgetInstanceIdentifier(std::string code_, std::string description_, int const sequence_number_, char const * const flags_ = "", TIME_GRANULARITY const time_granularity_ = TIME_GRANULARITY__NONE, Notes notes_ = Notes())
		: code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, flags(flags_ ? flags_ : "")
		, time_granularity(time_granularity_)
		, notes(notes_)
	{

	}

	WidgetInstanceIdentifier(WidgetInstanceIdentifier const & rhs)
		: uuid(rhs.uuid)
		//, uuid_parent(rhs.uuid_parent)
		//, fkuuids(rhs.fkuuids)
		, identifier_parent(rhs.identifier_parent)
		, foreign_key_identifiers(rhs.foreign_key_identifiers)
		, code(rhs.code)
		, sequence_number(rhs.sequence_number)
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
		//uuid_parent = rhs.uuid;
		//fkuuids = rhs.fkuuids;
		identifier_parent = rhs.identifier_parent;
		foreign_key_identifiers = rhs.foreign_key_identifiers;
		code = rhs.code;
		longhand = rhs.longhand;
		sequence_number = rhs.sequence_number;
		flags = rhs.flags;
		time_granularity = rhs.time_granularity;
		notes = rhs.notes;
		return *this;
	}

	std::shared_ptr<UUID> uuid;
	//std::shared_ptr<UUID> uuid_parent;
	std::shared_ptr<WidgetInstanceIdentifier> identifier_parent;
	//std::shared_ptr<UUIDVector> fkuuids;
	std::shared_ptr<std::vector<WidgetInstanceIdentifier>> foreign_key_identifiers;
	std::shared_ptr<std::string> code;
	std::shared_ptr<std::string> longhand;
	int sequence_number;
	std::string flags;
	TIME_GRANULARITY time_granularity;
	Notes notes;

};

typedef std::vector<WidgetInstanceIdentifier> WidgetInstanceIdentifiers;

WidgetInstanceIdentifier::Notes MakeNotes(char const * const notes1, char const * const notes2, char const * const notes3);
WidgetInstanceIdentifier::Notes MakeNotes(std::string notes1, std::string notes2, std::string notes3);

#endif
