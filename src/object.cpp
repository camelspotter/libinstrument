#include "../include/util.hpp"

/**
	@file src/object.cpp

	@brief Class instrument::object method implementation
*/

namespace instrument {

/**
 * @brief Object destructor
 *
 * @note
 *	The default destructor is overloaded to make it virtual and add destructor
 *	polymorphism to the class hierarchy tree. The destructor is not overriden
 *	but extended and an empty implementation is mandatory according to ISO
 *
 * @attention Inlining this method will optimize it out
 */
object::~object()
{
}


/**
 * @brief Query the class name of an object descending from instrument::object
 *
 * @returns the demangled class name (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline const i8* object::class_name() const
{
	return util::type_name(typeid(*this));
}

}
