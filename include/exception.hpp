#ifndef _EXCEPTION
#define _EXCEPTION 1

/**
	@file include/exception.hpp

	@brief Class instrument::exception definition
*/

#include "./object.hpp"

namespace instrument {

std::ostream& operator<<(std::ostream&, const std::exception&);

/**
	@brief This class is a throwable with a textual description of an error

	An exception object can be constructed using a printf-style format string and
	a variable argument list (for its error message). If a recursive exception
	occurs in the object constructor, it is caught and silently ignored. In this
	case the object is still safe to use by ignoring its message. Although objects
	of this class are not thread safe, overloaded std::ostream insertion operators
	that output exceptions synchronize thread access

	@attention
		When you output to the console directly (not through libinstrument methods),
		use util::lock/unlock to synchronize thread access
*/
class exception: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_msg;										/**< @brief Error description message */

public:

	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, const exception&);


	/* Constructors, copy constructors and destructor */

	explicit exception(const i8*, ...);

	exception(const exception&);

	virtual	~exception();

	virtual exception* clone() const;


	/* Accessor methods */

	virtual const i8* msg() const;


	/* Operator overloading methods */

	virtual exception& operator=(const exception&);
};

}

#endif
