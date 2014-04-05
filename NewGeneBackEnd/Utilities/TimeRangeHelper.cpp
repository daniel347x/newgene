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

	int const milliseconds_into_day = incoming_time.time_of_day().total_milliseconds();
	bpt::time_duration milliseconds_into_day_duration = bpt::milliseconds(milliseconds_into_day);

	int const seconds_into_day = incoming_time.time_of_day().total_seconds();
	bpt::time_duration seconds_into_day_duration = bpt::seconds(seconds_into_day);

	int minutes_into_day = seconds_into_day / 60;
	bpt::time_duration minutes_into_day_duration = bpt::minutes(minutes_into_day);

	int hours_into_day = minutes_into_day / 60;
	bpt::time_duration hours_into_day_duration = bpt::hours(hours_into_day);

	// Use the difference between the time durations
	// to round as desired
	bpt::time_duration difference_seconds_milliseconds = milliseconds_into_day_duration - seconds_into_day_duration;
	bpt::time_duration difference_minutes_seconds = seconds_into_day_duration - minutes_into_day_duration;
	bpt::time_duration difference_hours_minutes = minutes_into_day_duration - hours_into_day_duration;

	bpt::ptime result;

	switch (time_granularity)
	{

		case TIME_GRANULARITY__SECOND:
			{

				bool round_up = false;

				// Always round up - do not round to nearest!
				if (difference_seconds_milliseconds.total_milliseconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded *down* to the nearest second
				result = bpt::ptime(incoming_time.date(), seconds_into_day_duration);
				if (round_up)
				{
					result += bpt::seconds(1);
				}

			}
			break;

		case TIME_GRANULARITY__MINUTE:
			{

				bool round_up = false;

				// Always round up - do not round to nearest!
				if (difference_minutes_seconds.total_seconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded *down* to the nearest second
				result = bpt::ptime(incoming_time.date(), minutes_into_day_duration);
				if (round_up)
				{
					result += bpt::minutes(1);
				}

			}
			break;

		case TIME_GRANULARITY__HOUR:
			{

				bool round_up = false;

				// Always round up - do not round to nearest!
				if (difference_hours_minutes.total_seconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded *down* to the nearest second
				result = bpt::ptime(incoming_time.date(), hours_into_day_duration);
				if (round_up)
				{
					result += bpt::hours(1);
				}

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

	bpt::time_duration ms_epoch = result - time_t_epoch__1970;

	return ms_epoch.total_milliseconds();

}
