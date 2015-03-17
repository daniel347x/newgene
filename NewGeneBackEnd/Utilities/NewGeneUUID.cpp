// See http://stackoverflow.com/questions/1301277/c-boost-whats-the-cause-of-this-warning
// for the reason for this file.
// This file exists only to suppress an absolutely unavoidable C4996 error
// produced by the overly-"secure"-aware Microsoft compiler
// SIMPLY by #include-ing <iostream> followed by <boost/uuid/uuid.hpp>, and nothing else (i.e., no code)
// ... Because this error is actually issued by a line of code in a file included within <iostream>,
// rather than by a line of code within <boost/uuid/uuid.hpp>,
// the warning needs to be suppressed in <iostream>.  But, <iostream> is #include-ed in a single place,
// which is in turn #include-ed by many of this project's source files, so wrapping the pragma
// around <iostream> disabled the error globally.  To avoid this, we must create a separate
// header/source file that #includes <iostream> independently, and for the purpose of this
// single function call only.

#pragma warning(push)
#pragma warning(disable:4996)

#ifndef Q_MOC_RUN
	#include <boost/uuid/uuid.hpp>
	#include <boost/uuid/uuid_generators.hpp>
	#include <boost/uuid/uuid_io.hpp>
	#include <boost/lexical_cast.hpp>
#endif

const std::string newUUID(bool noDashes)
{
	boost::uuids::uuid u = boost::uuids::random_generator()();
	std::string su = boost::uuids::to_string(u);

	if (noDashes)
	{
		std::replace(su.begin(), su.end(), '-', '_'); // replace all '-' to underscore
	}

	return su;
}

#pragma warning(pop)
