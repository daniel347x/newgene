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

template<DATA_WIDGETS WIDGET>
class WidgetDataItem
{

};

template<>
class WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>
{
public:
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>()
		: n(0)
	{
	}
	int n;
};
typedef WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA> WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA;


template<>
class WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>
{
public:
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>()
	{
	}
	std::vector<std::string> variable_group_long_names;
};
typedef WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;

#endif
