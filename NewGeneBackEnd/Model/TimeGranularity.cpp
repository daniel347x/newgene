#include "TimeGranularity.h"
#include "../Utilities/NewGeneException.h"
#ifndef Q_MOC_RUN
	#include <boost/date_time/local_time/local_time.hpp>
	#include "boost/date_time/posix_time/posix_time.hpp"
#endif

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

std::string GetTimeUnitValue(TIME_GRANULARITY const time_granularity, std::int64_t timestamp)
{

	std::string result;

	boost::posix_time::ptime time_epoch(boost::gregorian::date(1970, 1, 1));
	boost::posix_time::ptime t = time_epoch + boost::posix_time::milliseconds(timestamp);

	switch (time_granularity)
	{

		case TIME_GRANULARITY__SECOND:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__MINUTE:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__HOUR:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__DAY:
			{
				result = boost::gregorian::to_simple_string(t.date());
			}
			break;

		case TIME_GRANULARITY__WEEK:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__MONTH:
			{
				result = t.date().month() + "/" + t.date().year();
			}
			break;

		case TIME_GRANULARITY__QUARTER:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__YEAR:
			{
				result = std::to_string(t.date().year());
			}
			break;

		case TIME_GRANULARITY__BIENNIAL:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__QUADRENNIAL:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__DECADE:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__CENTURY:
			{
				result = "";
			}
			break;

		case TIME_GRANULARITY__MILLENIUM:
			{
				result = "";
			}
			break;

		default:
			{
				result = "";
			}
			break;

	}

	return result;

}
