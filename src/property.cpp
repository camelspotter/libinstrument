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

	/* Copy multi-line comments */
	if ( unlikely(m_comments == NULL) ) {
		m_comments = rval.m_comments->clone();
	}
	else {
		*m_comments = *rval.m_comments;
	}

	/* Copy inline comment */
	if ( likely(rval.m_inline_comment == NULL) ) {
		delete m_inline_comment;
		m_inline_comment = NULL;
	}
	else if ( likely(m_inline_comment == NULL) ) {
		m_inline_comment = rval.m_inline_comment->clone();
	}
	else {
		*m_inline_comment = *rval.m_inline_comment;
	}

	/* Copy name */
	if ( unlikely(rval.m_name == NULL) ) {
		delete m_name;
		m_name = NULL;
	}
	else if ( unlikely(m_name == NULL) ) {
		m_name = rval.m_name->clone();
	}
	else {
		*m_name = *rval.m_name;
	}

	/* Copy value */
	if ( unlikely(rval.m_value == NULL) ) {
		delete m_value;
		m_value = NULL;
	}
	else if ( unlikely(m_value == NULL) ) {
		m_value = rval.m_value->clone();
	}
	else {
		*m_value = *rval.m_value;
	}

	return *this;
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
	m_inline_comment = m_name = m_value = NULL;

	m_comments = new list<string>(1, true);
	return *this;
}


/**
 * @brief Check if the property is totally empty
 *
 * @returns true if all the members are uninitialized, false otherwise
 */
bool property::is_empty() const
{
	if ( unlikely(m_comments != NULL && m_comments->size() > 0) ) {
		return false;
	}

	if ( unlikely(m_inline_comment != NULL && m_inline_comment->length() > 0) ) {
		return false;
	}

	if ( unlikely(m_name != NULL && m_name->length() > 0) ) {
		return false;
	}

	if ( unlikely(m_value != NULL && m_value->length() > 0) ) {
		return false;
	}

	return true;
}


/**
 * @brief Check if the property is valid
 *
 * @returns true if valid, false otherwise
 */
bool property::validate() const
{
	if ( unlikely(	m_name == NULL ||
									m_name->length() == 0 ||
									!m_name->match(PROPERTY_KEY_FORMAT)) ) {
		return false;
	}

	if ( unlikely(	m_value == NULL ||
									m_value->length() == 0 ||
									!m_value->match(PROPERTY_VALUE_FORMAT)) ) {
		return false;
	}

	return true;
}

}
