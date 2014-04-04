#include "TimeRangeHelper.h"
#include "./NewGeneException.h"
#ifndef Q_MOC_RUN
#	include <boost/format.hpp>
#endif

// Returns the closest Unix timestamp that is equal to or higher than test_timestamp,
// but aligned on the given time_granularity
std::int64_t TimeRange::determineNextHighestAligningTimestamp(std::int64_t const test_timestamp, TIME_GRANULARITY const time_granularity)
{

	if (time_granularity == TIME_GRANULARITY::TIME_GRANULARITY__NONE)
	{
		boost::format msg("Logic error: Calling TimeRange::determineNextHighestAligningTimestamp() with TIME_GRANULARITY__NONE");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (time_granularity)
	{

		case TIME_GRANULARITY__SECOND:
			{
				result = second;
			}
			break;

		case TIME_GRANULARITY__MINUTE:
			{
				result = minute;
			}
			break;

		case TIME_GRANULARITY__HOUR:
			{
				result = hour;
			}
			break;

		case TIME_GRANULARITY__DAY:
			{
				result = day;
			}
			break;

		case TIME_GRANULARITY__WEEK:
			{
				result = week;
			}
			break;

		case TIME_GRANULARITY__MONTH:
			{
				result = month;
			}
			break;

		case TIME_GRANULARITY__QUARTER:
			{
				result = quarter;
			}
			break;

		case TIME_GRANULARITY__YEAR:
			{
				result = year;
			}
			break;

		case TIME_GRANULARITY__BIENNIAL:
			{
				result = biennial;
			}
			break;

		case TIME_GRANULARITY__QUADRENNIAL:
			{
				result = quadrennial;
			}
			break;

		case TIME_GRANULARITY__DECADE:
			{
				result = decade;
			}
			break;

		case TIME_GRANULARITY__CENTURY:
			{
				result = century;
			}
			break;

		case TIME_GRANULARITY__MILLENIUM:
			{
				result = millenium;
			}
			break;

		default:
			{
				// no-op;
			}
			break;

	}

}
