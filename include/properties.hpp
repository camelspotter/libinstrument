#ifndef _PROPERTIES
#define _PROPERTIES 1

/**
	@file include/properties.hpp

	@brief Class instrument::properties definition
*/

#include "./property.hpp"

namespace instrument {

/**
	@brief A collection of properties loaded from a .properties file
*/
class properties: virtual public list<property>
{
protected:

	/* Protected variables */

	i8 *m_path;												/**< @brief Properties file path */

public:

	/* Constructors, copy constructors and destructor */

	properties();

	explicit properties(const i8*);
	
	properties(const properties&);

	virtual	~properties();

	virtual properties* clone() const;


	/* Accessor methods */
	
	virtual const i8* path() const;


	/* Operator overloading methods */

	virtual properties& operator=(const properties&);


	/* Generic methods */
	
	virtual properties& deserialize();
	
	virtual properties& serialize();
};

}

#endif
