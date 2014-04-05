#include "TimeRangeHelper.h"
#include "./NewGeneException.h"
#ifndef Q_MOC_RUN
#	include <boost/format.hpp>
#endif

// Returns the closest Unix timestamp that is equal to or higher than test_timestamp,
// but aligned on the given time_granularity
std::int64_t TimeRange::determineAligningTimestamp(std::int64_t const test_timestamp, TIME_GRANULARITY const time_granularity, ALIGN_MODE const align_mode)
{

	if (time_granularity == TIME_GRANULARITY::TIME_GRANULARITY__NONE)
	{
		boost::format msg("Logic error: Calling TimeRange::determineNextHighestAligningTimestamp() with TIME_GRANULARITY__NONE");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	namespace bpt = boost::posix_time;

	bpt::ptime incoming_time = bpt::from_time_t(test_timestamp / 1000);
	bpt::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));

	int const milliseconds_into_day = static_cast<int>(incoming_time.time_of_day().total_milliseconds());
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

	bpt::ptime date_rounded(incoming_time.date());

	int const year = date_rounded.date().year();
	boost::posix_time::ptime date_year(boost::gregorian::date(year, 1, 1));
	bpt::time_duration difference_between_incoming_and_year = incoming_time - date_year;

	int const month = date_rounded.date().month();
	boost::posix_time::ptime date_month(boost::gregorian::date(year, month, 1));
	bpt::time_duration difference_between_incoming_and_month = incoming_time - date_month;

	int const day = date_rounded.date().day();
	boost::posix_time::ptime date_day(boost::gregorian::date(year, month, day));
	bpt::time_duration difference_between_incoming_and_day = incoming_time - date_day;

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

				// Set the result, but rounded to nearest second at or above incoming timestamp
				result = bpt::ptime(date_rounded.date(), seconds_into_day_duration);
				if (round_up && align_mode == ALIGN_MODE_UP)
				{
					result += bpt::seconds(1);
				}

			}
			break;

		case TIME_GRANULARITY__MINUTE:
			{

				bool round_up = false;

				// Always round up - do not round to nearest!
				if (difference_minutes_seconds.total_milliseconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded to nearest minute at or above incoming timestamp
				result = bpt::ptime(date_rounded.date(), minutes_into_day_duration);
				if (round_up && align_mode == ALIGN_MODE_UP)
				{
					result += bpt::minutes(1);
				}

			}
			break;

		case TIME_GRANULARITY__HOUR:
			{

				bool round_up = false;

				// Always round up - do not round to nearest!
				if (difference_hours_minutes.total_milliseconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded to nearest hour at or above incoming timestamp
				result = bpt::ptime(date_rounded.date(), hours_into_day_duration);
				if (round_up && align_mode == ALIGN_MODE_UP)
				{
					result += bpt::hours(1);
				}

			}
			break;

		case TIME_GRANULARITY__DAY:
			{

				bool round_up = false;
				if (difference_between_incoming_and_day.total_milliseconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded to nearest hour at or above incoming timestamp
				result = date_day;
				if (round_up && align_mode == ALIGN_MODE_UP)
				{
					result += boost::gregorian::days(1);
				}
		
			}
			break;

		case TIME_GRANULARITY__WEEK:
			{
				boost::format msg("Rounding to nearest week is not currently supported.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

		case TIME_GRANULARITY__MONTH:
			{

				bool round_up = false;
				if (difference_between_incoming_and_month.total_milliseconds() > 0)
				{
					round_up = true;
				}

				// Set the result, but rounded to nearest hour at or above incoming timestamp
				result = date_month;
				if (round_up && align_mode == ALIGN_MODE_UP)
				{
					result += boost::gregorian::months(1);
				}

			}
			break;

		case TIME_GRANULARITY__QUARTER:
			{
				boost::format msg("Rounding to nearest quarter is not currently supported.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

		case TIME_GRANULARITY__YEAR:
			{
				result = date_year;
				if (difference_between_incoming_and_year.total_milliseconds() > 0)
				{
					// round to next year
					result += boost::gregorian::years(1);
				}
		}
			break;

		case TIME_GRANULARITY__BIENNIAL:
			{
				boost::format msg("Rounding to nearest biennial is not currently supported.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

		case TIME_GRANULARITY__QUADRENNIAL:
			{
				boost::format msg("Rounding to nearest quadrennial is not currently supported.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

		case TIME_GRANULARITY__DECADE:
			{

				if (year % 10 != 0)
				{
					int new_decade_year = year - (year % 10);
					if (align_mode == ALIGN_MODE_UP)
					{
						new_decade_year += 10;
					}
					boost::posix_time::ptime date_decade(boost::gregorian::date(new_decade_year, 1, 1));
					bpt::time_duration difference = incoming_time - date_decade;
					if (difference.total_milliseconds() > 0)
					{
						date_decade += boost::gregorian::years(10);
					}
					result = date_decade;
				}
				else
				{
					result = date_year;
					if (difference_between_incoming_and_year.total_milliseconds() > 0)
					{
						// round to next decade
						result += boost::gregorian::years(10);
					}
				}

			}
			break;

		case TIME_GRANULARITY__CENTURY:
			{

				if (year % 100 != 0)
				{
					int new_century_year = year - (year % 100);
					if (align_mode == ALIGN_MODE_UP)
					{
						new_century_year += 100;
					}
					boost::posix_time::ptime date_century(boost::gregorian::date(new_century_year, 1, 1));
					bpt::time_duration difference = incoming_time - date_century;
					if (difference.total_milliseconds() > 0)
					{
						date_century += boost::gregorian::years(100);
					}
					result = date_century;
				}
				else
				{
					result = date_year;
					if (difference_between_incoming_and_year.total_milliseconds() > 0)
					{
						// round to next century
						result += boost::gregorian::years(100);
					}
				}

			}
			break;

		case TIME_GRANULARITY__MILLENIUM:
			{

				if (year % 1000 != 0)
				{
					int new_millenium_year = year - (year % 1000);
					if (align_mode == ALIGN_MODE_UP)
					{
						new_millenium_year += 1000;
					}
					boost::posix_time::ptime date_millenium(boost::gregorian::date(new_millenium_year, 1, 1));
					bpt::time_duration difference = incoming_time - date_millenium;
					if (difference.total_milliseconds() > 0)
					{
						date_millenium += boost::gregorian::years(1000);
					}
					result = date_millenium;
				}
				else
				{
					result = date_year;
					if (difference_between_incoming_and_year.total_milliseconds() > 0)
					{
						// round to next millenium
						result += boost::gregorian::years(1000);
					}
				}
			
			}
			break;

		default:
			{
				boost::format msg("Invalid time range specification in round-to-align calculation.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	bpt::time_duration ms_epoch = result - time_t_epoch__1970;

	return ms_epoch.total_milliseconds();

}
