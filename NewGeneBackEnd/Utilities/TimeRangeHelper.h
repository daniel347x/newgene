#ifndef TIMERANGEHELPER_H
#define TIMERANGEHELPER_H

#include <string>
#include <cstdint>

#ifndef Q_MOC_RUN
#	include <boost/date_time/local_time/local_time.hpp>
#endif
#include "../Model/TimeGranularity.h"

namespace TimeRange
{

	enum TimeRangeImportMode
	{

		  TIME_RANGE_IMPORT_MODE__NONE = 0
		, TIME_RANGE_IMPORT_MODE__YEAR_MONTH_DAY
		, TIME_RANGE_IMPORT_MODE__YEAR

	};

	enum ALIGN_MODE
	{
		ALIGN_MODE_UP = 0
		, ALIGN_MODE_DOWN
	};

	inline std::string convertTimestampToString(std::int64_t const timestamp)
	{
		boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));
		boost::posix_time::ptime time_start_database = time_t_epoch__1970 + boost::posix_time::milliseconds(timestamp);
		return boost::posix_time::to_simple_string(time_start_database);
	}

	// Returns the closest Unix timestamp that is equal to or higher than test_timestamp,
	// but aligned on the given time_granularity
	std::int64_t determineAligningTimestamp(std::int64_t const test_timestamp, TIME_GRANULARITY const time_granularity, ALIGN_MODE const align_mode);

}

#endif
