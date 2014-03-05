#ifndef ACTIONWIDGETS_H
#define ACTIONWIDGETS_H

#include "../Utilities/WidgetIdentifier.h"

enum WIDGET_ACTIONS
{

	  WIDGET_ACTIONS_FIRST = 0
	, ACTION_TYPE_NONE = WIDGET_ACTIONS_FIRST

	, ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
	, ACTION_KAD_COUNT_CHANGE
	, ACTION_DO_RANDOM_SAMPLING_CHANGE
	, ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE
	, ACTION_DATETIME_RANGE_CHANGE
	, ACTION_GENERATE_OUTPUT

	, ACTION_ADD_DMU
	, ACTION_DELETE_DMU
	, ACTION_ADD_DMU_MEMBERS
	, ACTION_DELETE_DMU_MEMBERS
	, ACTION_REFRESH_DMUS_FROM_FILE
	, ACTION_ADD_UOA
	, ACTION_DELETE_UOA

	, WIDGET_ACTIONS_LAST

};

enum WIDGET_ACTION_ITEM_REQUEST_REASON
{

	  WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN

	, WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS
	, WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS
	, WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS
	, WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION // for actions not associated with an item

	, WIDGET_ACTION_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS

};

class WidgetActionItem
{

	public:

		WidgetActionItem()
		{

		}

		virtual ~WidgetActionItem()
		{

		}

		WidgetActionItem(WidgetActionItem const &)
		{

		}

};

typedef std::shared_ptr<WidgetActionItem> WidgetActionItemSharedPtr;
typedef std::pair<WidgetInstanceIdentifier, WidgetActionItemSharedPtr> InstanceActionItem;
typedef std::vector<InstanceActionItem> InstanceActionItems;


class WidgetActionItem__Checkbox : public WidgetActionItem
{

	public:

		WidgetActionItem__Checkbox(bool const checked_)
			: WidgetActionItem()
			, checked(checked_)
		{

		}

		WidgetActionItem__Checkbox(WidgetActionItem__Checkbox const & rhs)
			: WidgetActionItem(rhs)
			, checked(rhs.checked)
		{

		}

		~WidgetActionItem__Checkbox()
		{

		}

		void setChecked(bool const checked_)
		{
			checked = checked_;
		}

		bool isChecked() const
		{
			return checked;
		}

	protected:

		bool checked;

};

class WidgetActionItem__Spinbox : public WidgetActionItem
{

	public:

		WidgetActionItem__Spinbox(int const value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__Spinbox(WidgetActionItem__Spinbox const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__Spinbox()
		{

		}

		void setValue(int const value__)
		{
			value_ = value__;
		}

		int getValue() const
		{
			return value_;
		}

	protected:

		int value_;

};

class WidgetActionItem__Int64 : public WidgetActionItem
{

	public:

		WidgetActionItem__Int64(std::int64_t const value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__Int64(WidgetActionItem__Int64 const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__Int64()
		{

		}

		void setValue(std::int64_t const value__)
		{
			value_ = value__;
		}

		std::int64_t getValue() const
		{
			return value_;
		}

	protected:

		std::int64_t value_;

};

class WidgetActionItem__WidgetInstanceIdentifier : public WidgetActionItem
{

	public:

		WidgetActionItem__WidgetInstanceIdentifier(WidgetInstanceIdentifier const & value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__WidgetInstanceIdentifier(WidgetActionItem__WidgetInstanceIdentifier const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__WidgetInstanceIdentifier()
		{

		}

		void setValue(WidgetInstanceIdentifier const & value__)
		{
			value_ = value__;
		}

		WidgetInstanceIdentifier getValue() const
		{
			return value_;
		}

	protected:

		WidgetInstanceIdentifier value_;

};

class WidgetActionItem__WidgetInstanceIdentifiers : public WidgetActionItem
{

	public:

		WidgetActionItem__WidgetInstanceIdentifiers(WidgetInstanceIdentifiers const & value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__WidgetInstanceIdentifiers(WidgetActionItem__WidgetInstanceIdentifiers const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__WidgetInstanceIdentifiers()
		{

		}

		void setValue(WidgetInstanceIdentifiers const & value__)
		{
			value_ = value__;
		}

		WidgetInstanceIdentifiers getValue() const
		{
			return value_;
		}

	protected:

		WidgetInstanceIdentifiers value_;

};

class WidgetActionItem__WidgetInstanceIdentifiers_Plus_String : public WidgetActionItem__WidgetInstanceIdentifiers
{

public:

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(WidgetInstanceIdentifiers const & value__, std::string const & the_string__)
		: WidgetActionItem__WidgetInstanceIdentifiers(value__)
		, the_string_(the_string__)
	{

	}

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(WidgetActionItem__WidgetInstanceIdentifiers_Plus_String const & rhs)
		: WidgetActionItem__WidgetInstanceIdentifiers(rhs)
		, the_string_(rhs.the_string_)
	{

	}

	~WidgetActionItem__WidgetInstanceIdentifiers_Plus_String()
	{

	}

	void setValueString(std::string const & the_string__)
	{
		the_string_ = the_string__;
	}

	std::string getValueString() const
	{
		return the_string_;
	}

protected:

	std::string the_string_;

};

class WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int : public WidgetActionItem__WidgetInstanceIdentifiers_Plus_String
{

public:

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int(WidgetInstanceIdentifiers const & value__, std::string const & the_string__, int const & the_int__)
		: WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(value__, the_string__)
		, the_int_(the_int__)
	{

	}

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int(WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int const & rhs)
		: WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(rhs)
		, the_int_(rhs.the_int_)
	{

	}

	~WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int()
	{

	}

	void setValueInt(int const & the_int__)
	{
		the_int_ = the_int__;
	}

	int getValueInt() const
	{
		return the_int_;
	}

protected:

	int the_int_;

};

class WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int : public WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int
{

public:

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int(WidgetInstanceIdentifiers const & value__, std::string const & the_string__, std::string const & the_string2__, int const & the_int__)
		: WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int(value__, the_string__, the_int__)
		, the_string2_(the_string2__)
	{

	}

	WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int(WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int const & rhs)
		: WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_And_Int(rhs)
		, the_string2_(rhs.the_string2_)
	{

	}

	~WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int()
	{

	}

	void setValueString2(std::string const & the_string2__)
	{
		the_string2_ = the_string2__;
	}

	std::string getValueString2() const
	{
		return the_string2_;
	}

protected:

	std::string the_string2_;

};

class WidgetActionItem__String : public WidgetActionItem
{

	public:

		WidgetActionItem__String(std::string const value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__String(WidgetActionItem__String const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__String()
		{

		}

		void setValue(std::string const value__)
		{
			value_ = value__;
		}

		std::string getValue() const
		{
			return value_;
		}

	protected:

		std::string value_;

};

class WidgetActionItem__StringVector : public WidgetActionItem
{

public:

	WidgetActionItem__StringVector(std::vector<std::string> const value__)
		: WidgetActionItem()
		, value_(value__)
	{

	}

	WidgetActionItem__StringVector(WidgetActionItem__StringVector const & rhs)
		: WidgetActionItem(rhs)
		, value_(rhs.value_)
	{

	}

	~WidgetActionItem__StringVector()
	{

	}

	void setValue(std::vector<std::string> const & value__)
	{
		value_ = value__;
	}

	std::vector<std::string> getValue() const
	{
		return value_;
	}

protected:

	std::vector<std::string> value_;

};

class WidgetActionItem__StringVector_Plus_Int : public WidgetActionItem__StringVector
{

	public:

		WidgetActionItem__StringVector_Plus_Int(std::vector<std::string> const value__, int const intValue__)
			: WidgetActionItem__StringVector(value__)
			, intValue_(intValue__)
		{

		}

		WidgetActionItem__StringVector_Plus_Int(WidgetActionItem__StringVector_Plus_Int const & rhs)
			: WidgetActionItem__StringVector(rhs)
			, intValue_(rhs.intValue_)
		{

		}

		~WidgetActionItem__StringVector_Plus_Int()
		{

		}

		void setIntValue(int const & value__)
		{
			intValue_ = value__;
		}

		int getIntValue() const
		{
			return intValue_;
		}

	protected:

		int intValue_;

};

class WidgetActionItem__DateTime : public WidgetActionItem
{

	public:

		WidgetActionItem__DateTime(std::int64_t const value__)
			: WidgetActionItem()
			, value_(value__)
		{

		}

		WidgetActionItem__DateTime(WidgetActionItem__DateTime const & rhs)
			: WidgetActionItem(rhs)
			, value_(rhs.value_)
		{

		}

		~WidgetActionItem__DateTime()
		{

		}

		void setValue(std::int64_t const value__)
		{
			value_ = value__;
		}

		std::int64_t getValue() const
		{
			return value_;
		}

	protected:

		std::int64_t value_;

};

class WidgetActionItem__GenerateOutput : public WidgetActionItem
{

public:

	WidgetActionItem__GenerateOutput()
		: WidgetActionItem()
	{

	}

	WidgetActionItem__GenerateOutput(WidgetActionItem__GenerateOutput const & rhs)
		: WidgetActionItem(rhs)
	{

	}

	~WidgetActionItem__GenerateOutput()
	{

	}

};

class WidgetActionItemRequest_base
{

	public:

		WidgetActionItemRequest_base(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: reason(reason_)
			, items(std::make_shared<InstanceActionItems>(items_))
		{

		}

		WidgetActionItemRequest_base(WidgetActionItemRequest_base const & rhs)
			: reason(rhs.reason)
			, items(rhs.items)
		{

		}

		WIDGET_ACTION_ITEM_REQUEST_REASON reason;
		std::shared_ptr<InstanceActionItems> items;

};

template<WIDGET_ACTIONS WIDGET>
class WidgetActionItemRequest : public WidgetActionItemRequest_base
{

};


/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> : public WidgetActionItemRequest_base
{
	public:
		WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: WidgetActionItemRequest_base(reason_, items_)
		{
		}
		WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> const & rhs)
			: WidgetActionItemRequest_base(rhs)
		{
		}
};
typedef WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED;

/************************************************************************/
// ACTION_KAD_COUNT_CHANGE
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE> : public WidgetActionItemRequest_base
{
	public:
		WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: WidgetActionItemRequest_base(reason_, items_)
		{
		}
		WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE>(WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE> const & rhs)
			: WidgetActionItemRequest_base(rhs)
		{
		}
};
typedef WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE> WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE;

/************************************************************************/
// ACTION_DO_RANDOM_SAMPLING_CHANGE
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE> : public WidgetActionItemRequest_base
{
	public:
		WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: WidgetActionItemRequest_base(reason_, items_)
		{
		}
		WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE>(WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE> const & rhs)
			: WidgetActionItemRequest_base(rhs)
		{
		}
};
typedef WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE> WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE;

/************************************************************************/
// ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE> : public WidgetActionItemRequest_base
{
	public:
		WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: WidgetActionItemRequest_base(reason_, items_)
		{
		}
		WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>(WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE> const & rhs)
			: WidgetActionItemRequest_base(rhs)
		{
		}
};
typedef WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE> WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE;

/************************************************************************/
// ACTION_DATETIME_RANGE_CHANGE
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE> : public WidgetActionItemRequest_base
{
	public:
		WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, InstanceActionItems items_ = InstanceActionItems())
			: WidgetActionItemRequest_base(reason_, items_)
		{
		}
		WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE>(WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE> const & rhs)
			: WidgetActionItemRequest_base(rhs)
		{
		}
};
typedef WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE> WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE;

/************************************************************************/
// ACTION_GENERATE_OUTPUT
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_GENERATE_OUTPUT> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_GENERATE_OUTPUT>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_GENERATE_OUTPUT>(WidgetActionItemRequest<ACTION_GENERATE_OUTPUT> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_GENERATE_OUTPUT> WidgetActionItemRequest_ACTION_GENERATE_OUTPUT;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_ADD_DMU> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_ADD_DMU>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_ADD_DMU>(WidgetActionItemRequest<ACTION_ADD_DMU> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_ADD_DMU> WidgetActionItemRequest_ACTION_ADD_DMU;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_DELETE_DMU> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_DELETE_DMU>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_DELETE_DMU>(WidgetActionItemRequest<ACTION_DELETE_DMU> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_DELETE_DMU> WidgetActionItemRequest_ACTION_DELETE_DMU;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_ADD_DMU_MEMBERS> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_ADD_DMU_MEMBERS>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_ADD_DMU_MEMBERS>(WidgetActionItemRequest<ACTION_ADD_DMU_MEMBERS> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_ADD_DMU_MEMBERS> WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_DELETE_DMU_MEMBERS> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_DELETE_DMU_MEMBERS>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_DELETE_DMU_MEMBERS>(WidgetActionItemRequest<ACTION_DELETE_DMU_MEMBERS> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_DELETE_DMU_MEMBERS> WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_REFRESH_DMUS_FROM_FILE> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_REFRESH_DMUS_FROM_FILE>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_REFRESH_DMUS_FROM_FILE>(WidgetActionItemRequest<ACTION_REFRESH_DMUS_FROM_FILE> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_REFRESH_DMUS_FROM_FILE> WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_ADD_UOA> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_ADD_UOA>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_ADD_UOA>(WidgetActionItemRequest<ACTION_ADD_UOA> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_ADD_UOA> WidgetActionItemRequest_ACTION_ADD_UOA;

/************************************************************************/
//
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_DELETE_UOA> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_DELETE_UOA>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, InstanceActionItems items_ = InstanceActionItems())
		: WidgetActionItemRequest_base(reason_, items_)
	{
	}
	WidgetActionItemRequest<ACTION_DELETE_UOA>(WidgetActionItemRequest<ACTION_DELETE_UOA> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_DELETE_UOA> WidgetActionItemRequest_ACTION_DELETE_UOA;

#endif
