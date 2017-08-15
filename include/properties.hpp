#ifndef _PROPERTIES
#define _PROPERTIES 1

/**
	@file include/properties.hpp

	@brief Class instrument::properties definition
*/

#include "./string.hpp"

namespace instrument {

/**
	@brief
*/
class properties: virtual public object
{
protected:

	/* Protected variables */

	pid_t m_pid;												/**< @brief Process ID */

public:

	/* Constructors, copy constructors and destructor */

	properties();

	properties(const properties&);

	virtual	~properties();

	virtual properties* clone() const;


	/* Accessor methods */


	/* Operator overloading methods */

	virtual properties& operator=(const properties&);


	/* Generic methods */
};

}

#endif
