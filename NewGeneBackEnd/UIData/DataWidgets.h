#ifndef DATAWIDGETS_H
#define DATAWIDGETS_H

enum DATA_WIDGETS
{

	  DATA_WIDGETS_FIRST = 0
	, WIDGET_TYPE_NONE = DATA_WIDGETS_FIRST

	, VARIABLE_GROUPS_SCROLL_AREA
	, VARIABLE_GROUPS_TOOLBOX
	
	, DATA_WIDGETS_LAST

};

class WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest_base()
	{

	}
};

class WidgetDataItem_base
{
public:
	WidgetDataItem_base()
	{

	}
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
	WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA>()
		: WidgetDataItemRequest_base()
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA> WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA;

template<>
class WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>()
		: WidgetDataItem_base()
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
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>()
		: WidgetDataItemRequest_base()
	{
	}
	std::string s;
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX> WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX;

template<>
class WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>()
		: WidgetDataItem_base()
	{
	}
	std::vector<std::string> variable_group_long_names;
};
typedef WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;


#endif
