#ifndef DATAWIDGETS_H
#define DATAWIDGETS_H

#include "../Utilities/WidgetIdentifier.h"
#include <cstdint>
#include <set>
#ifndef Q_MOC_RUN
#endif

enum DATA_WIDGETS
{

	  DATA_WIDGETS_FIRST = 0
	, WIDGET_TYPE_NONE = DATA_WIDGETS_FIRST

	// Output project
	, VARIABLE_GROUPS_SCROLL_AREA
	, VARIABLE_GROUPS_TOOLBOX
	, VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE
	, VARIABLE_GROUPS_SUMMARY
	, VARIABLE_GROUPS_SUMMARY_SCROLL_AREA
	, VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
	, KAD_SPIN_CONTROLS_AREA
	, KAD_SPIN_CONTROL_WIDGET
	, TIMERANGE_REGION_WIDGET
	, DATETIME_WIDGET
	, GENERATE_OUTPUT_TAB
	, LIMIT_DMUS_TAB

	// Input project
	, MANAGE_DMUS_WIDGET
	, MANAGE_UOAS_WIDGET
	, MANAGE_VGS_WIDGET

	, DATA_WIDGETS_LAST

};

enum WIDGET_DATA_ITEM_REQUEST_REASON
{

	  WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN
	, WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS

};

class WidgetDataItemRequest_base
{

public:

	WidgetDataItemRequest_base(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: reason(reason_)
		, identifier(std::make_shared<WidgetInstanceIdentifier>(identifier_))
	{

	}

	WidgetDataItemRequest_base(WidgetDataItemRequest_base const & rhs)
		: reason(rhs.reason)
		, identifier(rhs.identifier)
	{

	}

	virtual ~WidgetDataItemRequest_base()
	{

	}

	WIDGET_DATA_ITEM_REQUEST_REASON reason;
	std::shared_ptr<WidgetInstanceIdentifier> identifier;

};

class WidgetDataItem_base
{

public:

	WidgetDataItem_base(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: request_reason(request_reason_)
		, identifier(std::make_shared<WidgetInstanceIdentifier>(identifier_))
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
	std::shared_ptr<WidgetInstanceIdentifier> identifier;

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
	WidgetDataItemRequest<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	WidgetDataItem<VARIABLE_GROUPS_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	WidgetDataItemRequest<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	WidgetDataItem<VARIABLE_GROUPS_TOOLBOX>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	WidgetInstanceIdentifiers identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUPS_TOOLBOX> WidgetDataItem_VARIABLE_GROUPS_TOOLBOX;

typedef std::pair<WidgetInstanceIdentifier, bool> WidgetInstanceIdentifier_Bool_Pair;

/************************************************************************/
// VARIABLE_GROUPS_VARIABLE_GROUP_INSTANCE
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
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
	std::vector<WidgetInstanceIdentifier_Bool_Pair> identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE> WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE;


/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_SCROLL_AREA
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA;

template<>
class WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> const & rhs)
		: WidgetDataItem_base(rhs)
		, identifiers(rhs.identifiers)
	{
	}
	WidgetInstanceIdentifiers identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA> WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA;


/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
template<>
class WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE;

template<>
class WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> : public WidgetDataItem_base
{
public:
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> const & rhs)
		: WidgetDataItem_base(rhs)
		, identifiers(rhs.identifiers)
	{
	}
	WidgetInstanceIdentifiers identifiers;
};
typedef WidgetDataItem<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE> WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE;

/************************************************************************/
// KAD_SPIN_CONTROLS_AREA
/************************************************************************/
template<>
class WidgetDataItemRequest<KAD_SPIN_CONTROLS_AREA> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<KAD_SPIN_CONTROLS_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<KAD_SPIN_CONTROLS_AREA>(WidgetDataItemRequest<KAD_SPIN_CONTROLS_AREA> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<KAD_SPIN_CONTROLS_AREA> WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA;

template<>
class WidgetDataItem<KAD_SPIN_CONTROLS_AREA> : public WidgetDataItem_base
{
public:
	WidgetDataItem<KAD_SPIN_CONTROLS_AREA>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<KAD_SPIN_CONTROLS_AREA>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<KAD_SPIN_CONTROLS_AREA>(WidgetDataItem<KAD_SPIN_CONTROLS_AREA> const & rhs)
		: WidgetDataItem_base(rhs)
		, identifiers(rhs.identifiers)
		, active_dmus(rhs.active_dmus)
	{
	}
	WidgetInstanceIdentifiers identifiers;
	std::set<WidgetInstanceIdentifier> active_dmus;
};
typedef WidgetDataItem<KAD_SPIN_CONTROLS_AREA> WidgetDataItem_KAD_SPIN_CONTROLS_AREA;


/************************************************************************/
// KAD_SPIN_CONTROL_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<KAD_SPIN_CONTROL_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<KAD_SPIN_CONTROL_WIDGET>(int const count_ = 0, WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
		, count(count_)
	{
	}
	WidgetDataItemRequest<KAD_SPIN_CONTROL_WIDGET>(WidgetDataItemRequest<KAD_SPIN_CONTROL_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
		, count(rhs.count)
	{
	}
	int count;
};
typedef WidgetDataItemRequest<KAD_SPIN_CONTROL_WIDGET> WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET;

template<>
class WidgetDataItem<KAD_SPIN_CONTROL_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<KAD_SPIN_CONTROL_WIDGET>(int count_ = 0, WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
		, count(count_)
	{
	}
	WidgetDataItem<KAD_SPIN_CONTROL_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET const & spinRequest = dynamic_cast<WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET const &>(request_obj);
			count = spinRequest.count;
		}
		catch (std::bad_cast &)
		{
			count = 0;
		}
	}
	WidgetDataItem<KAD_SPIN_CONTROL_WIDGET>(WidgetDataItem<KAD_SPIN_CONTROL_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, count(rhs.count)
	{
	}
	int count;
};
typedef WidgetDataItem<KAD_SPIN_CONTROL_WIDGET> WidgetDataItem_KAD_SPIN_CONTROL_WIDGET;


/************************************************************************/
// TIMERANGE_REGION_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<TIMERANGE_REGION_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<TIMERANGE_REGION_WIDGET>(bool const do_random_sampling_ = false, std::int64_t random_sampling_count_per_stage_ = 1, bool consolidate_rows_ = true, WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
		, do_random_sampling(do_random_sampling_)
		, random_sampling_count_per_stage(random_sampling_count_per_stage_)
		, consolidate_rows(consolidate_rows_)
	{
	}
	WidgetDataItemRequest<TIMERANGE_REGION_WIDGET>(WidgetDataItemRequest<TIMERANGE_REGION_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
		, do_random_sampling(rhs.do_random_sampling)
		, random_sampling_count_per_stage(rhs.random_sampling_count_per_stage)
		, consolidate_rows(rhs.consolidate_rows)
	{
	}
	bool do_random_sampling;
	std::int64_t random_sampling_count_per_stage;
	bool consolidate_rows;
};
typedef WidgetDataItemRequest<TIMERANGE_REGION_WIDGET> WidgetDataItemRequest_TIMERANGE_REGION_WIDGET;

template<>
class WidgetDataItem<TIMERANGE_REGION_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<TIMERANGE_REGION_WIDGET>(bool const do_random_sampling_ = false, std::int64_t random_sampling_count_per_stage_ = 1, bool const consolidate_rows_ = true, WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
		, do_random_sampling(do_random_sampling_)
		, random_sampling_count_per_stage(random_sampling_count_per_stage_)
		, consolidate_rows(consolidate_rows_)
	{
	}
	WidgetDataItem<TIMERANGE_REGION_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_TIMERANGE_REGION_WIDGET const & date_time_request = dynamic_cast<WidgetDataItemRequest_TIMERANGE_REGION_WIDGET const &>(request_obj);
			do_random_sampling = date_time_request.do_random_sampling;
			random_sampling_count_per_stage = date_time_request.random_sampling_count_per_stage;
			consolidate_rows = date_time_request.consolidate_rows;
		}
		catch (std::bad_cast &)
		{
			do_random_sampling = false;
			random_sampling_count_per_stage = 1;
			consolidate_rows = true;
		}
	}
	WidgetDataItem<TIMERANGE_REGION_WIDGET>(WidgetDataItem<TIMERANGE_REGION_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, do_random_sampling(rhs.do_random_sampling)
		, random_sampling_count_per_stage(rhs.random_sampling_count_per_stage)
		, consolidate_rows(rhs.consolidate_rows)
	{
	}
	bool do_random_sampling;
	std::int64_t random_sampling_count_per_stage;
	bool consolidate_rows;
};
typedef WidgetDataItem<TIMERANGE_REGION_WIDGET> WidgetDataItem_TIMERANGE_REGION_WIDGET;


/************************************************************************/
// DATETIME_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<DATETIME_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<DATETIME_WIDGET>(std::int64_t const the_date_time_ = 0, WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
		, the_date_time(the_date_time_)
	{
	}
	WidgetDataItemRequest<DATETIME_WIDGET>(WidgetDataItemRequest<DATETIME_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
		, the_date_time(rhs.the_date_time)
	{
	}
	std::int64_t the_date_time;
};
typedef WidgetDataItemRequest<DATETIME_WIDGET> WidgetDataItemRequest_DATETIME_WIDGET;

template<>
class WidgetDataItem<DATETIME_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<DATETIME_WIDGET>(std::int64_t the_date_time_ = 0, WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
		, the_date_time(the_date_time_)
	{
	}
	WidgetDataItem<DATETIME_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_DATETIME_WIDGET const & date_time_request = dynamic_cast<WidgetDataItemRequest_DATETIME_WIDGET const &>(request_obj);
			the_date_time = date_time_request.the_date_time;
		}
		catch (std::bad_cast &)
		{
			the_date_time = 0;
		}
	}
	WidgetDataItem<DATETIME_WIDGET>(WidgetDataItem<DATETIME_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, the_date_time(rhs.the_date_time)
	{
	}
	std::int64_t the_date_time;
};
typedef WidgetDataItem<DATETIME_WIDGET> WidgetDataItem_DATETIME_WIDGET;


/************************************************************************/
// GENERATE_OUTPUT_TAB
/************************************************************************/
template<>
class WidgetDataItemRequest<GENERATE_OUTPUT_TAB> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<GENERATE_OUTPUT_TAB>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<GENERATE_OUTPUT_TAB>(WidgetDataItemRequest<GENERATE_OUTPUT_TAB> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<GENERATE_OUTPUT_TAB> WidgetDataItemRequest_GENERATE_OUTPUT_TAB;

template<>
class WidgetDataItem<GENERATE_OUTPUT_TAB> : public WidgetDataItem_base
{
public:
	WidgetDataItem<GENERATE_OUTPUT_TAB>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<GENERATE_OUTPUT_TAB>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			//WidgetDataItemRequest_GENERATE_OUTPUT_TAB const & date_time_request = dynamic_cast<WidgetDataItemRequest_GENERATE_OUTPUT_TAB const &>(request_obj);
		}
		catch (std::bad_cast &)
		{
		}
	}
	WidgetDataItem<GENERATE_OUTPUT_TAB>(WidgetDataItem<GENERATE_OUTPUT_TAB> const & rhs)
		: WidgetDataItem_base(rhs)
	{
	}
};
typedef WidgetDataItem<GENERATE_OUTPUT_TAB> WidgetDataItem_GENERATE_OUTPUT_TAB;


/************************************************************************/
// MANAGE_DMUS_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<MANAGE_DMUS_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<MANAGE_DMUS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<MANAGE_DMUS_WIDGET>(WidgetDataItemRequest<MANAGE_DMUS_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<MANAGE_DMUS_WIDGET> WidgetDataItemRequest_MANAGE_DMUS_WIDGET;

template<>
class WidgetDataItem<MANAGE_DMUS_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<MANAGE_DMUS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<MANAGE_DMUS_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_MANAGE_DMUS_WIDGET const & dmu_management_request = dynamic_cast<WidgetDataItemRequest_MANAGE_DMUS_WIDGET const &>(request_obj);
		}
		catch (std::bad_cast &)
		{
		}
	}
	WidgetDataItem<MANAGE_DMUS_WIDGET>(WidgetDataItem<MANAGE_DMUS_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, dmus_and_members(rhs.dmus_and_members)
	{
	}
	std::vector<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>> dmus_and_members;
};
typedef WidgetDataItem<MANAGE_DMUS_WIDGET> WidgetDataItem_MANAGE_DMUS_WIDGET;


/************************************************************************/
// MANAGE_UOAS_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<MANAGE_UOAS_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<MANAGE_UOAS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<MANAGE_UOAS_WIDGET>(WidgetDataItemRequest<MANAGE_UOAS_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<MANAGE_UOAS_WIDGET> WidgetDataItemRequest_MANAGE_UOAS_WIDGET;

template<>
class WidgetDataItem<MANAGE_UOAS_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<MANAGE_UOAS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<MANAGE_UOAS_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_MANAGE_UOAS_WIDGET const & uoa_management_request = dynamic_cast<WidgetDataItemRequest_MANAGE_UOAS_WIDGET const &>(request_obj);
		}
		catch (std::bad_cast &)
		{
		}
	}
	WidgetDataItem<MANAGE_UOAS_WIDGET>(WidgetDataItem<MANAGE_UOAS_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, uoas_and_dmu_categories(rhs.uoas_and_dmu_categories)
	{
	}
	std::vector<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>> uoas_and_dmu_categories;
};
typedef WidgetDataItem<MANAGE_UOAS_WIDGET> WidgetDataItem_MANAGE_UOAS_WIDGET;


/************************************************************************/
// MANAGE_VGS_WIDGET
/************************************************************************/
template<>
class WidgetDataItemRequest<MANAGE_VGS_WIDGET> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<MANAGE_VGS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<MANAGE_VGS_WIDGET>(WidgetDataItemRequest<MANAGE_VGS_WIDGET> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<MANAGE_VGS_WIDGET> WidgetDataItemRequest_MANAGE_VGS_WIDGET;

template<>
class WidgetDataItem<MANAGE_VGS_WIDGET> : public WidgetDataItem_base
{
public:
	WidgetDataItem<MANAGE_VGS_WIDGET>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<MANAGE_VGS_WIDGET>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
		try
		{
			WidgetDataItemRequest_MANAGE_VGS_WIDGET const & vg_management_request = dynamic_cast<WidgetDataItemRequest_MANAGE_VGS_WIDGET const &>(request_obj);
		}
		catch (std::bad_cast &)
		{
		}
	}
	WidgetDataItem<MANAGE_VGS_WIDGET>(WidgetDataItem<MANAGE_VGS_WIDGET> const & rhs)
		: WidgetDataItem_base(rhs)
		, vgs_and_uoa(rhs.vgs_and_uoa)
	{
	}
	std::vector<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier>> vgs_and_uoa;
};
typedef WidgetDataItem<MANAGE_VGS_WIDGET> WidgetDataItem_MANAGE_VGS_WIDGET;


/************************************************************************/
// LIMIT_DMUS_TAB
/************************************************************************/
template<>
class WidgetDataItemRequest<LIMIT_DMUS_TAB> : public WidgetDataItemRequest_base
{
public:
	WidgetDataItemRequest<LIMIT_DMUS_TAB>(WIDGET_DATA_ITEM_REQUEST_REASON const reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItemRequest_base(reason_, identifier_)
	{
	}
	WidgetDataItemRequest<LIMIT_DMUS_TAB>(WidgetDataItemRequest<LIMIT_DMUS_TAB> const & rhs)
		: WidgetDataItemRequest_base(rhs)
	{
	}
};
typedef WidgetDataItemRequest<LIMIT_DMUS_TAB> WidgetDataItemRequest_LIMIT_DMUS_TAB;

//                 dmu_category            , is_limited, all_dmu_members,   , not_limited_members      , limited_members
typedef std::tuple<WidgetInstanceIdentifier, bool, WidgetInstanceIdentifiers, WidgetInstanceIdentifiers, WidgetInstanceIdentifiers> dmu_category_limit_members_info_tuple;
template<>
class WidgetDataItem<LIMIT_DMUS_TAB> : public WidgetDataItem_base
{
public:
	WidgetDataItem<LIMIT_DMUS_TAB>(WIDGET_DATA_ITEM_REQUEST_REASON const request_reason_ = WIDGET_DATA_ITEM_REQUEST_REASON__UNKNOWN, WidgetInstanceIdentifier identifier_ = WidgetInstanceIdentifier())
		: WidgetDataItem_base(request_reason_, identifier_)
	{
	}
	WidgetDataItem<LIMIT_DMUS_TAB>(WidgetDataItemRequest_base const & request_obj)
		: WidgetDataItem_base(request_obj)
	{
	}
	WidgetDataItem<LIMIT_DMUS_TAB>(WidgetDataItem<LIMIT_DMUS_TAB> const & rhs)
		: WidgetDataItem_base(rhs)
		, dmu_category_limit_members_info(rhs.dmu_category_limit_members_info)
	{
	}
	std::vector<dmu_category_limit_members_info_tuple> dmu_category_limit_members_info;
};
typedef WidgetDataItem<LIMIT_DMUS_TAB> WidgetDataItem_LIMIT_DMUS_TAB;


#endif
