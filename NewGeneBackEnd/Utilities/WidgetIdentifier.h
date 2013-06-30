#ifndef WIDGETIDENTIFIER_H
#define WIDGETIDENTIFIER_H

#include <memory>
#include <vector>
#include <string>

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
	{

	}

	WidgetInstanceIdentifier(UUID const uuid_, std::string const code_, std::string const description_, int const sequence_number_, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, notes(notes_)
	{

	}

	WidgetInstanceIdentifier(UUID uuid_)
		: uuid(std::make_shared<UUID>(uuid_))
		, sequence_number(0)
	{

	}

	WidgetInstanceIdentifier(std::string code_, std::string description_, int const sequence_number_, Notes notes_ = Notes())
		: code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, notes(notes_)
	{

	}

	WidgetInstanceIdentifier(WidgetInstanceIdentifier const & rhs)
		: uuid(rhs.uuid)
		, code(rhs.code)
		, sequence_number(rhs.sequence_number)
		, longhand(rhs.longhand)
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
		fkuuids = rhs.fkuuids;
		code = rhs.code;
		longhand = rhs.longhand;
		sequence_number = rhs.sequence_number;
		notes = rhs.notes;
		return *this;
	}

	std::shared_ptr<UUID> uuid;
	std::shared_ptr<UUIDVector> fkuuids;
	std::shared_ptr<std::string> code;
	std::shared_ptr<std::string> longhand;
	int sequence_number;
	Notes notes;

};

typedef std::vector<WidgetInstanceIdentifier> WidgetInstanceIdentifiers;

WidgetInstanceIdentifier::Notes MakeNotes(char const * const notes1, char const * const notes2, char const * const notes3);
WidgetInstanceIdentifier::Notes MakeNotes(std::string notes1, std::string notes2, std::string notes3);

#endif
