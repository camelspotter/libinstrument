#include "../include/exception.hpp"
#include "../include/util.hpp"

/**
	@file src/exception.cpp

	@brief Class instrument::exception method implementation
*/

namespace instrument {

/**
 * @brief Stream insertion operator for std::exception objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 *
 * @relatedalso instrument::exception
 */
std::ostream& operator<<(std::ostream &lval, const std::exception &rval)
{
	util::lock();
	util::header(lval, EXCEPTION_TAG);

	lval	<< rval.what()
				<< std::endl;

	util::unlock();
	return lval;
}


/**
 * @brief Stream insertion operator for instrument::exception objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, const exception &rval)
{
	util::lock();
	util::header(lval, EXCEPTION_TAG);

	if ( likely(rval.m_msg != NULL) ) {
		lval	<< rval.m_msg
					<< std::endl;
	}
	else {
		lval	<< "n/a"
					<< std::endl;
	}

	util::unlock();
	return lval;
}


/**
 * @brief Object constructor
 *
 * @param[in] fmt a printf-style format string (the error description)
 *
 * @param[in] ... a variable argument list
 */
exception::exception(const i8 *fmt, ...):
m_msg(NULL)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return;
	}

	try {
		va_list args;
		va_start(args, fmt);
		m_msg = util::va_format(fmt, args);
	}
	catch (...) {
		__D_ASSERT(m_msg != NULL);
	}
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 */
exception::exception(const exception &src):
m_msg(NULL)
{
	*this = src;
}


/**
 * @brief Object destructor
 */
exception::~exception()
{
	delete[] m_msg;
	m_msg = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @note The return value must be checked for allocation errors
 */
inline exception* exception::clone() const
{
	return new (std::nothrow) exception(*this);
}


/**
 * @brief Get the exception message
 *
 * @returns this->m_msg
 */
inline const i8* exception::msg() const
{
	return m_msg;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 */
exception& exception::operator=(const exception &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	const i8 *buf = rval.m_msg;
	if ( unlikely(buf == NULL) ) {
		delete[] m_msg;
		m_msg = NULL;
		return *this;
	}

	u32 len = strlen(buf);
	if ( likely(m_msg == NULL || len > strlen(m_msg)) ) {
		delete[] m_msg;
		m_msg = NULL;
		m_msg = new (std::nothrow) i8[len + 1];
	}

	__D_ASSERT(m_msg != NULL);
	if ( likely(m_msg != NULL) ) {
		strcpy(m_msg, buf);
	}

	return *this;
}

}
