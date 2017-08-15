#include "../include/property.hpp"

/**
	@file src/property.cpp

	@brief Class instrument::property method implementation
*/

namespace instrument {

/**
 * @brief Free object resources
 *
 * @returns *this
 */
property& property::clear()
{
	delete m_comments;
	delete m_inline_comment;
	delete m_name;
	delete m_value;
	
	m_comments = NULL;
	m_inline_comment = NULL;
	m_name = NULL;
	m_value = NULL;

	return *this;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
property::property()
try:
m_comments(NULL),
m_inline_comment(NULL),
m_invalid(false),
m_name(NULL),
m_value(NULL)
{
}
catch (...) {
	clear();
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
property::property(string *name)
try:
m_comments(NULL),
m_inline_comment(NULL),
m_invalid(false),
m_name(NULL),
m_value(NULL)
{
}
catch (...) {
	clear();
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
property::property(string *name, string *value)
try:
m_comments(NULL),
m_inline_comment(NULL),
m_invalid(false),
m_name(NULL),
m_value(NULL)
{
}
catch (...) {
	clear();
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
property::property(const property &src)
try:
m_comments(NULL),
m_inline_comment(NULL),
m_invalid(src.m_invalid),
m_name(NULL),
m_value(NULL)
{
	if ( likely(src.m_comments != NULL) ) {
		m_comments = src.m_comments->clone();
	}
	
	if ( unlikely(src.m_inline_comment != NULL) ) {
		m_inline_comment = src.m_inline_comment->clone();
	}
	
	if ( likely(src.m_name != NULL) ) {
		m_name = src.m_name->clone();
	}
	
	if ( likely(src.m_value != NULL) ) {
		m_value = src.m_value->clone();
	}
}
catch (...) {
	clear();
}


/**
 * @brief Object destructor
 */
property::~property()
{
	clear();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline property* property::clone() const
{
	return new property(*this);
}


/**
 * @brief Check if the property is valid (format, type, complete)
 *
 * @returns !this->m_invalid
 */
inline bool property::is_valid() const
{
	return !m_invalid;
}


/**
 * @brief Get the property name
 *
 * @returns this->m_name
 */
inline const string* property::name() const
{
	return m_name;
}


/**
 * @brief Get the property value
 *
 * @returns this->m_value
 */
inline const string* property::value() const
{
	return m_value;
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
property& property::operator=(const property &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* todo Implement this */

	return *this;
}

}
