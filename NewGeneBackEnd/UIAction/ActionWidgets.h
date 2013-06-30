#ifndef ACTIONWIDGETS_H
#define ACTIONWIDGETS_H

#include "..\Utilities\WidgetIdentifier.h"

enum WIDGET_ACTIONS
{

	  WIDGET_ACTIONS_FIRST = 0
	, ACTION_TYPE_NONE = WIDGET_ACTIONS_FIRST

	, ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED

	, WIDGET_ACTIONS_LAST

};

enum WIDGET_ACTION_ITEM_REQUEST_REASON
{

	  WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN

	, WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS
	, WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS
	, WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS

	, WIDGET_ACTION_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS

};

class WidgetActionItemRequest_base
{

public:

	WidgetActionItemRequest_base(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifiers identifiers_ = WidgetInstanceIdentifiers())
		: reason(reason_)
		, identifiers(std::make_shared<WidgetInstanceIdentifiers>(identifiers_))
	{

	}

	WidgetActionItemRequest_base(WidgetActionItemRequest_base const & rhs)
		: reason(rhs.reason)
		, identifiers(rhs.identifiers)
	{

	}

	WIDGET_ACTION_ITEM_REQUEST_REASON reason;
	std::shared_ptr<WidgetInstanceIdentifiers> identifiers;

};

class WidgetActionItem_base
{

public:

	WidgetActionItem_base(WIDGET_ACTION_ITEM_REQUEST_REASON const request_reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifiers identifiers_ = WidgetInstanceIdentifiers())
		: request_reason(request_reason_)
		, identifiers(std::make_shared<WidgetInstanceIdentifiers>(identifiers_))
	{

	}

	WidgetActionItem_base(WidgetActionItemRequest_base const & request_obj)
		: request_reason(request_obj.reason)
		, identifiers(request_obj.identifiers)
	{

	}

	WidgetActionItem_base(WidgetActionItem_base const & rhs)
		: request_reason(rhs.request_reason)
		, identifiers(rhs.identifiers)
	{

	}

	WIDGET_ACTION_ITEM_REQUEST_REASON request_reason;
	std::shared_ptr<WidgetInstanceIdentifiers> identifiers;

};

template<WIDGET_ACTIONS WIDGET>
class WidgetActionItemRequest : public WidgetActionItemRequest_base
{

};

template<WIDGET_ACTIONS WIDGET>
class WidgetActionItem : public WidgetActionItem_base
{

};

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
template<>
class WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> : public WidgetActionItemRequest_base
{
public:
	WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WIDGET_ACTION_ITEM_REQUEST_REASON const reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifiers identifiers_ = WidgetInstanceIdentifiers())
		: WidgetActionItemRequest_base(reason_, identifiers_)
	{
	}
	WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> const & rhs)
		: WidgetActionItemRequest_base(rhs)
	{
	}
};
typedef WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED;

template<>
class WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> : public WidgetActionItem_base
{
public:
	WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WIDGET_ACTION_ITEM_REQUEST_REASON const request_reason_ = WIDGET_ACTION_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifiers identifiers_ = WidgetInstanceIdentifiers())
		: WidgetActionItem_base(request_reason_, identifiers_)
	{
	}
	WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WidgetActionItemRequest_base const & request_obj)
		: WidgetActionItem_base(request_obj)
	{
	}
	WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> const & rhs)
		: WidgetActionItem_base(rhs)
	{
	}
};
typedef WidgetActionItem<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED> WidgetActionItem_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED;


#endif
