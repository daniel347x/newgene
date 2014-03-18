#ifndef DATACHANGES_H
#define DATACHANGES_H

#include <vector>
#include <set>
#include "../Utilities/WidgetIdentifier.h"
#include <cstdint>

enum DATA_CHANGE_TYPE
{

	  DATA_CHANGE_TYPE__FIRST = 0

	, DATA_CHANGE_TYPE__UNKNOWN = DATA_CHANGE_TYPE__FIRST




	// Input model
	, DATA_CHANGE_TYPE__INPUT_MODEL__FIRST

	, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE
	, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE
	, DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE
	, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE

	, DATA_CHANGE_TYPE__INPUT_MODEL__LAST




	// Output model
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__FIRST

	, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__KAD_COUNT_CHANGE
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__DO_RANDOM_SAMPLING_CHANGE
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__GENERATE_OUTPUT
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__ACTIVE_DMU_CHANGE

	, DATA_CHANGE_TYPE__OUTPUT_MODEL__LAST




	, DATA_CHANGE_TYPE__LAST

};


enum DATA_CHANGE_INTENTION
{

	  DATA_CHANGE_INTENTION__ADD
	, DATA_CHANGE_INTENTION__REMOVE
	, DATA_CHANGE_INTENTION__UPDATE
	, DATA_CHANGE_INTENTION__RESET_ALL
	, DATA_CHANGE_INTENTION__NONE

};

class DataChangePacket
{

	public:

		DataChangePacket()
		{

		}

        DataChangePacket(DataChangePacket const &)
		{

		}

};

class DataChangePacket_bool : public DataChangePacket
{

public:

	DataChangePacket_bool(bool const b_)
		: DataChangePacket()
		, b(b_)
	{

	}

	DataChangePacket_bool(DataChangePacket_bool const & rhs)
		: DataChangePacket(rhs)
		, b(rhs.b)
	{

	}

	void setValue(bool const b_)
	{
		b = b_;
	}

	bool getValue() const
	{
		return b;
	}

	bool b;

};

class DataChangePacket_int : public DataChangePacket
{

	public:

		DataChangePacket_int(int const n_)
			: DataChangePacket()
			, n(n_)
		{

		}

		DataChangePacket_int(DataChangePacket_int const & rhs)
			: DataChangePacket(rhs)
			, n(rhs.n)
		{

		}

		void setValue(int const n_)
		{
			n = n_;
		}

		int getValue() const
		{
			return n;
		}

		int n;

};

class DataChangePacket_int64 : public DataChangePacket
{

	public:

		DataChangePacket_int64(std::int64_t const n_)
			: DataChangePacket()
			, n(n_)
		{

		}

		DataChangePacket_int64(DataChangePacket_int64 const & rhs)
			: DataChangePacket(rhs)
			, n(rhs.n)
		{

		}

		void setValue(std::int64_t const n_)
		{
			n = n_;
		}

		std::int64_t getValue() const
		{
			return n;
		}

		std::int64_t n;

};

class DataChangePacket_GenerateOutput : public DataChangePacket
{

public:

	DataChangePacket_GenerateOutput()
		: DataChangePacket()
	{

	}

	DataChangePacket_GenerateOutput(DataChangePacket_GenerateOutput const & rhs)
		: DataChangePacket(rhs)
	{

	}

};

class DataChange
{

	public:

		DataChange()
		{

		}

		DataChange(DATA_CHANGE_TYPE const & type, DATA_CHANGE_INTENTION const & intention, WidgetInstanceIdentifier const & parent_identifier_ = WidgetInstanceIdentifier(), WidgetInstanceIdentifiers const & child_identifiers_ = WidgetInstanceIdentifiers())
			: change_type(type)
			, change_intention(intention)
			, parent_identifier(parent_identifier_)
			, child_identifiers(child_identifiers_)
			// set_of_identifiers: No.  A hint that it's minor enough that it should be required to be set outside the constructor.
		{

		}

		DataChange(DataChange const & rhs)
			: change_type(rhs.change_type)
			, change_intention(rhs.change_intention)
			, parent_identifier(rhs.parent_identifier)
			, child_identifiers(rhs.child_identifiers)
			, set_of_identifiers(rhs.set_of_identifiers)
			, change_packet(rhs.change_packet)
		{

		}

		void SetPacket(std::shared_ptr<DataChangePacket> packet_)
		{
			change_packet = packet_;
		}

		DataChangePacket * getPacket() const
		{
			if (change_packet)
			{
				return change_packet.get();
			}
			return nullptr;
		}

		DATA_CHANGE_TYPE change_type;
		DATA_CHANGE_INTENTION change_intention;
		WidgetInstanceIdentifier parent_identifier;
		WidgetInstanceIdentifiers child_identifiers;
		std::set<WidgetInstanceIdentifier> set_of_identifiers; // only when needed

		std::shared_ptr<DataChangePacket> change_packet;

};

typedef std::vector<DataChange> DataChanges;

#endif
