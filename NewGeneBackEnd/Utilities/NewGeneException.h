#ifndef NEWGENEEXCEPTION_H
#define NEWGENEEXCEPTION_H

#ifndef Q_MOC_RUN
#	include <boost/exception/all.hpp>
#	include <boost/format.hpp>
#endif

typedef boost::error_info<struct tag_description, std::string> newgene_error_description;

struct NewGeneException : virtual std::exception, virtual boost::exception
{
public:
	NewGeneException(void);
	~NewGeneException(void);
};

#endif
