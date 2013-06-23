#ifndef DATAWIDGETS_H
#define DATAWIDGETS_H

#include "../globals.h"

enum DATA_WIDGETS
{

	  DATA_WIDGETS_FIRST = 0
	, WIDGET_TYPE_NONE = DATA_WIDGETS_FIRST

	, VARIABLE_GROUPS_SCROLL_AREA
	, VARIABLE_GROUPS_TOOLBOX
	
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
		std::shared_ptr<std::string> notes1;
		std::shared_ptr<std::string> notes2;
		std::shared_ptr<std::string> notes3;
	};

	DataInstanceIdentifier(UUID uuid_, std::string code_, std::string description_)
		: uuid(std::make_shared<UUID>(uuid_))
		, code(std::make_shared<std::string>(code_))
		, description(std::make_shared<std::string>(description_))
	{

	}

	DataInstanceIdentifier(UUID uuid_)
		: uuid(std::make_shared<UUID>(uuid_))
	{

	}

	DataInstanceIdentifier(std::string code_, std::string description_)
		: code(std::make_shared<std::string>(code_))
		, description(std::make_shared<std::string>(description_))
	{

	}

	std::shared_ptr<UUID> uuid;
	std::shared_ptr<std::string> code;
	std::shared_ptr<std::string> description;
	Notes notes;

};

typedef std::vector<DataInstanceIdentifier> DataInstanceIdentifiers;

class WidgetDataItemRequest_base
{

public:

	WidgetDataItemRequest_base(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: reason(reason_)
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

	WidgetDataItem_base(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: request_reason(request_reason_)
	{

	}

	WidgetDataItem_base(WidgetDataItemRequest_base const & request_obj)
		: request_reason(request_obj.reason)
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
	WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: WidgetDataItemRequest_base(reason_)
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
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: WidgetDataItem_base(request_reason_)
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
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: WidgetDataItemRequest_base(reason_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> const & rhs)
		: WidgetDataItemRequest_base(rhs)
		, s(rhs.s)
	{
	}
	std::string s;
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX;

template<>
class WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN)
		: WidgetDataItem_base(request_reason_)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> const & rhs)
		: WidgetDataItem_base(rhs)
		, variable_group_long_names(rhs.variable_group_long_names)
	{
	}
	std::vector<std::string> variable_group_long_names;
};
typedef WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;


#endif
