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

	namespace bpt = boost::posix_time;

	bpt::ptime incoming_time = bpt::from_time_t(test_timestamp / 1000);
	bpt::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));

	switch (time_granularity)
	{

		case TIME_GRANULARITY__SECOND:
			{
				// Get number of seconds into the day represented by incoming_time
				bpt::time_duration seconds_into_day = bpt::seconds(incoming_time.time_of_day().total_seconds());
				bpt::time_duration milliseconds_into_day = bpt::seconds(incoming_time.time_of_day().total_milliseconds());

				// Use the difference between the time duration in seconds
				// and the time duration in milliseconds
				// to round to the nearest second
				bpt::time_duration difference_seconds_milliseconds = milliseconds_into_day - seconds_into_day;
				bool round_up = false;
				if (difference_seconds_milliseconds.total_milliseconds() > 500)
				{
					round_up = true;
				}

				// Set the result, but rounded *down* to the nearest second
				bpt::ptime result(incoming_time.date(), seconds_into_day);
				if (round_up)
				{
					result += bpt::seconds(1);
				}

				boost::posix_time::time_duration ms_epoch = result - time_t_epoch__1970;

				return ms_epoch.total_milliseconds();
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
