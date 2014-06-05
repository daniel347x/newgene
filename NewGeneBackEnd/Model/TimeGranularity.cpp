#include "TimeGranularity.h"
#include "../Utilities/NewGeneException.h"

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
