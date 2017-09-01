#include "../include/property.hpp"

/**
	@file src/property.cpp

	@brief Class instrument::property method implementation
*/

namespace instrument {

/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
property::property():
m_comments(NULL),
m_inline_comment(NULL),
m_invalid(false),
m_name(NULL),
m_value(NULL)
{
	m_comments = new list<string>(1, true);
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
	*this = src;
}
catch (...) {
	empty();
}


/**
 * @brief Object destructor
 */
property::~property()
{
	empty();
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
 * @brief Get all the property comments
 *
 * @returns this->m_comments
 */
inline const list<string>* property::comments() const
{
	return m_comments;
}


/**
 * @brief Get the property inline comment
 *
 * @returns this->m_inline_comment
 */
inline const string* property::inline_comment() const
{
	return m_inline_comment;
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

	if ( likely(rval.m_comments != NULL) ) {
		m_comments = rval.m_comments->clone();
	}

	if ( unlikely(rval.m_inline_comment != NULL) ) {
		m_inline_comment = rval.m_inline_comment->clone();
	}

	if ( likely(rval.m_name != NULL) ) {
		m_name = rval.m_name->clone();
	}

	if ( likely(rval.m_value != NULL) ) {
		m_value = rval.m_value->clone();
	}

	m_invalid = rval.m_invalid;
	return *this;
}


/**
 * @brief Get the i-th comment
 *
 * @param[in] i the comment index
 *
 * @returns this->m_comments->at(i)
 *
 * @throws instrument::exception
 */
inline const string* property::comment(u32 i) const
{
	if ( likely(m_comments == NULL) ) {
		throw exception("offset out of list bounds (%d >= 0)", i);
	}

	return m_comments->at(i);
}


/**
 * @brief Get the number of comments
 *
 * @returns this->m_comments->size()
 */
inline u32 property::comment_count() const
{
	return m_comments->size();
}


/**
 * @brief Free object resources
 *
 * @returns *this
 */
property& property::empty()
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
 * @brief Check if the property is totally empty
 *
 * @returns true if all the members are uninitialized, false otherwise
 */
bool property::is_empty() const
{
	return false;
}

}
