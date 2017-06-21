#ifndef _FILTER
#define _FILTER 1

/**
	@file include/filter.hpp

	@brief Class instrument::filter definition
*/

#include "./object.hpp"

namespace instrument {

#define MODULE_FILTER		false

#define SYMBOL_FILTER		true


/**
	@brief Instrumentation filter

	An object of this class is used to filter out whole modules or sets of methods
	by matching a POSIX extended regular expression against their absolute path or
	their signature, respectively

	@todo Make the object copyable
*/
class filter: virtual public object
{
protected:

	/* Protected variables */

	regex_t m_expr;										/**< @brief Filter expression */

	bool m_mode;											/**< @brief Filter type switch */


	/* Protected copy constructors */

	filter(const filter&)															__attribute((noreturn));

	virtual filter* clone() const											__attribute((noreturn));


	/* Protected operator overloading methods */

	virtual filter& operator=(const filter&)					__attribute((noreturn));

public:

	/* Friend classes and functions */

	template <class F> friend class list;


	/* Constructors, copy constructors and destructor */

	filter(const i8*, bool, bool = true);

	virtual ~filter();


	/* Accessor methods */

	virtual bool mode() const;

	virtual filter& set_expr(const i8*, bool);

	virtual filter& set_mode(bool);


	/* Generic methods */

	virtual bool apply(const i8*) const;
};

}

#endif
