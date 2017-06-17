#ifndef _OBJECT
#define _OBJECT 1

/**
	@file include/object.hpp

	@brief Class instrument::object definition
*/

#include "./config.hpp"

namespace instrument {

/**
	@brief This abstract class serves as the root of the class hierarchy tree
*/
class object
{
public:

	/* Constructors, copy constructors and destructor */

	virtual ~object() = 0;											/**< @brief To be implemented */

	virtual object* clone() const = 0;					/**< @brief To be implemented */


	/* Generic methods */

	virtual const i8* class_name() const;
};

}

#endif
