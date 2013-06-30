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

class WidgetActionItem
{

	public:

		WidgetActionItem()
		{

		}

		WidgetActionItem(WidgetActionItem const & rhs)
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

		void setChecked(bool const checked_)
		{
			checked = checked_;
		}

		bool isChecked()
		{
			return checked;
		}

	protected:
	
		bool checked;

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

#endif
