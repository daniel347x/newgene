#ifndef DATACHANGES_H
#define DATACHANGES_H

#include <vector>
#include "../Utilities/WidgetIdentifier.h"

enum DATA_CHANGE_TYPE
{

	  DATA_CHANGE_TYPE__FIRST = 0

	, DATA_CHANGE_TYPE__UNKNOWN = DATA_CHANGE_TYPE__FIRST




	// Input model
	, DATA_CHANGE_TYPE__INPUT_MODEL__FIRST



	, DATA_CHANGE_TYPE__INPUT_MODEL__LAST




	// Output model
	, DATA_CHANGE_TYPE__OUTPUT_MODEL__FIRST

	, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION

	, DATA_CHANGE_TYPE__OUTPUT_MODEL__LAST




	, DATA_CHANGE_TYPE__LAST

};


enum DATA_CHANGE_INTENTION
{

	  DATA_CHANGE_INTENTION__ADD
	, DATA_CHANGE_INTENTION__REMOVE
	, DATA_CHANGE_INTENTION__UPDATE
	, DATA_CHANGE_INTENTION__RESET_ALL

};

class DataChange
{

	public:

		DataChange()
		{

		}

		DataChange(DATA_CHANGE_TYPE const & type, DATA_CHANGE_INTENTION const & intention, WidgetInstanceIdentifier const & parent_identifier_, WidgetInstanceIdentifiers const & child_identifiers_)
			: change_type(type)
			, change_intention(intention)
			, parent_identifier(parent_identifier_)
			, child_identifiers(child_identifiers_)
		{

		}

		DataChange(DataChange const & rhs)
			: change_type(rhs.change_type)
			, change_intention(rhs.change_intention)
			, parent_identifier(rhs.parent_identifier)
			, child_identifiers(rhs.child_identifiers)
		{

		}

		DATA_CHANGE_TYPE change_type;
		DATA_CHANGE_INTENTION change_intention;
		WidgetInstanceIdentifier parent_identifier;
		WidgetInstanceIdentifiers child_identifiers;

};

typedef std::vector<DataChange> DataChanges;

//class DataChange__DATA_CHANGES__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION : virtual public DataChange
//{
//
//	public:
//
//		DataChange__DATA_CHANGES__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION()
//			: DataChange()
//		{
//
//		}
//
//		DataChange__DATA_CHANGES__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION(DATA_CHANGE_TYPE const & type, DATA_CHANGE_INTENTION const & intention, WidgetInstanceIdentifier const & parent_identifier_, WidgetInstanceIdentifiers const & child_identifiers_)
//			: DataChange(type, intention, parent_identifier_, child_identifiers_)
//		{
//
//		}
//
//		DataChange__DATA_CHANGES__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION(DataChange__DATA_CHANGES__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION const & rhs)
//			: DataChange(rhs)
//		{
//
//		}
//
//};

#endif
