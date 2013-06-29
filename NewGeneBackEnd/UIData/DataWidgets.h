#ifndef DATAWIDGETS_H
#define DATAWIDGETS_H

#include <memory>
#include <vector>
#include <string>

typedef std::string UUID;
typedef std::vector<UUID> UUIDVector;

int const UUID_LENGTH = 36;

enum DATA_WIDGETS
{

	  DATA_WIDGETS_FIRST = 0
	, WIDGET_TYPE_NONE = DATA_WIDGETS_FIRST

	, VARIABLE_GROUPS_SCROLL_AREA
	, VARIABLE_GROUPS_TOOLBOX
	, VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE

	, DATA_WIDGETS_LAST

};

enum WIDGET_DATA_ITEM_REQUEST_REASON
{

	  WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN
	, WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS

};

class DataInstanceIdentifier
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

	DataInstanceIdentifier()
		: sequence_number(0)
	{

	}

	DataInstanceIdentifier(UUID const uuid_, std::string const code_, std::string const description_, int const sequence_number_, Notes notes_ = Notes())
		: uuid(std::make_shared<UUID>(uuid_))
		, code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, notes(notes_)
	{

	}

	DataInstanceIdentifier(UUID uuid_)
		: uuid(std::make_shared<UUID>(uuid_))
		, sequence_number(0)
	{

	}

	DataInstanceIdentifier(std::string code_, std::string description_, int const sequence_number_, Notes notes_ = Notes())
		: code(std::make_shared<std::string>(code_))
		, sequence_number(sequence_number_)
		, longhand(std::make_shared<std::string>(description_))
		, notes(notes_)
	{

	}

	DataInstanceIdentifier(DataInstanceIdentifier const & rhs)
		: uuid(rhs.uuid)
		, code(rhs.code)
		, sequence_number(rhs.sequence_number)
		, longhand(rhs.longhand)
		, notes(rhs.notes)
	{

	}

	DataInstanceIdentifier & operator=(DataInstanceIdentifier const & rhs)
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

typedef std::vector<DataInstanceIdentifier> DataInstanceIdentifiers;

DataInstanceIdentifier::Notes MakeNotes(char const * const notes1, char const * const notes2, char const * const notes3);
DataInstanceIdentifier::Notes MakeNotes(std::string notes1, std::string notes2, std::string notes3);

class WidgetDataItemRequest_base
{

public:

	WidgetDataItemRequest_base(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: reason(reason_)
		, identifier(std::make_shared<DataInstanceIdentifier>(identifier_))
	{

	}

	WidgetDataItemRequest_base(WidgetDataItemRequest_base const & rhs)
		: reason(rhs.reason)
		, identifier(rhs.identifier)
	{

	}

	WIDGET_DATA_ITEM_REQUEST_REASON reason;
	std::shared_ptr<DataInstanceIdentifier> identifier;

};

class WidgetDataItem_base
{

public:

	WidgetDataItem_base(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: request_reason(request_reason_)
		, identifier(std::make_shared<DataInstanceIdentifier>(identifier_))
	{

	}

	WidgetDataItem_base(WidgetDataItemRequest_base const & request_obj)
		: request_reason(request_obj.reason)
		, identifier(request_obj.identifier)
	{

	}

	WidgetDataItem_base(WidgetDataItem_base const & rhs)
		: request_reason(rhs.request_reason)
		, identifier(rhs.identifier)
	{

	}

	WIDGET_DATA_ITEM_REQUEST_REASON request_reason;
	std::shared_ptr<DataInstanceIdentifier> identifier;

};

template<DATA_WIDGETS WIDGET>
class WidgetDataItemRequest : public WidgetDataItemRequest_base
{

};

template<DATA_WIDGETS WIDGET>
class WidgetDataItem : public WidgetDataItem_base
{

};

/************************************************************************/
// VARIABLE_GROUPS_SCROLL_AREA
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA>(WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA> WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA;

template<>
class WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>(WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA> const & rhs)
		: WidgetDataItem_base(rhs)
	{
	}
};
typedef WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA> WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA;


/************************************************************************/
// VARIABLE_GROUPS_TOOLBOX
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX;

template<>
class WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> const & rhs)
		: WidgetDataItem_base(rhs)
		, identifiers(rhs.identifiers)
	{
	}
	DataInstanceIdentifiers identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;


/************************************************************************/
// VARIABLE_GROUPS_TOOLBOX
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE;

template<>
class WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, DataInstanceIdentifier identifier_ = DataInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> const & rhs)
		: WidgetDataItem_base(rhs)
		, identifiers(rhs.identifiers)
	{
	}
	DataInstanceIdentifiers identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE;


#endif
