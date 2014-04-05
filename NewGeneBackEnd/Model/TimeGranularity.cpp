#include "TimeGranularity.h"
#include "../Utilities/NewGeneException.h"

std::int64_t AvgMsperUnit(TIME_GRANULARITY const time_granularity)
{

	static std::int64_t ms = 1;
	static std::int64_t second = ms * 1000;
	static std::int64_t minute = second * 60;
	static std::int64_t hour = minute * 60;
	static std::int64_t day = hour * 24;
	static std::int64_t week = day * 7;
	static std::int64_t month = (std::int64_t)((double)week * 4.354166666666666670); // for AVERAGE purposes only - 52.25 / 12; average includes leap year
	static std::int64_t quarter = (std::int64_t)((double)week * 13.0625); // Average includes leap year
	static std::int64_t year = (std::int64_t)((double)week * 52.25);   // Average includes leap year
	static std::int64_t biennial = year * 2;
	static std::int64_t quadrennial = year * 4;
	static std::int64_t decade = year * 10;
	static std::int64_t century = year * 100;
	static std::int64_t millenium = year * 1000;

	std::int64_t result = 0;

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
				// 0 is acceptable - calling function will handle this case
				//boost::format msg("Invalid time granularity in AvgMsperUnit()");
				//throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	return result;

}

std::string GetTimeGranularityText(TIME_GRANULARITY const time_granularity)
{

	std::string result;

	switch (time_granularity)
	{

		case TIME_GRANULARITY__SECOND:
			{
				result = "TIME_GRANULARITY__SECOND";
			}
			break;

		case TIME_GRANULARITY__MINUTE:
			{
				result = "TIME_GRANULARITY__MINUTE";
			}
			break;

		case TIME_GRANULARITY__HOUR:
			{
				result = "TIME_GRANULARITY__HOUR";
			}
			break;

		case TIME_GRANULARITY__DAY:
			{
				result = "TIME_GRANULARITY__DAY";
			}
			break;

		case TIME_GRANULARITY__WEEK:
			{
				result = "TIME_GRANULARITY__WEEK";
			}
			break;

		case TIME_GRANULARITY__MONTH:
			{
				result = "TIME_GRANULARITY__MONTH";
			}
			break;

		case TIME_GRANULARITY__QUARTER:
			{
				result = "TIME_GRANULARITY__QUARTER";
			}
			break;

		case TIME_GRANULARITY__YEAR:
			{
				result = "TIME_GRANULARITY__YEAR";
			}
			break;

		case TIME_GRANULARITY__BIENNIAL:
			{
				result = "TIME_GRANULARITY__BIENNIAL";
			}
			break;

		case TIME_GRANULARITY__QUADRENNIAL:
			{
				result = "TIME_GRANULARITY__QUADRENNIAL";
			}
			break;

		case TIME_GRANULARITY__DECADE:
			{
				result = "TIME_GRANULARITY__DECADE";
			}
			break;

		case TIME_GRANULARITY__CENTURY:
			{
				result = "TIME_GRANULARITY__CENTURY";
			}
			break;

		case TIME_GRANULARITY__MILLENIUM:
			{
				result = "TIME_GRANULARITY__MILLENIUM";
			}
			break;

		default:
			{
				result = "TIME_GRANULARITY__NONE";
			}
			break;

	}

	return result;

}
