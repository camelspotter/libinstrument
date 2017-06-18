#include "../include/call.hpp"

/**
	@file src/call.cpp

	@brief Class instrument::call method implementation
*/

namespace instrument {

/**
 * @brief Object constructor
 *
 * @param[in] addr the called function address
 *
 * @param[in] site the call site address
 *
 * @param[in] nm the called function name (NULL if unresolved)
 *
 * @throws std::bad_alloc
 */
call::call(mem_addr_t addr, mem_addr_t site, const i8 *nm):
symbol(addr, nm),
m_site(site)
{
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
call::call(const call &src):
symbol(src),
m_site(src.m_site)
{
}


/**
 * @brief Object destructor
 *
 * @attention Inlining this method will optimize it out
 */
call::~call()
{
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline call* call::clone() const
{
	return new call(*this);
}


/**
 * @brief Get the call site address
 *
 * @returns this->m_site
 */
inline mem_addr_t call::site() const
{
	return m_site;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
call& call::operator=(const call &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	symbol::operator=(rval);
	m_site = rval.m_site;

	return *this;
}

}
