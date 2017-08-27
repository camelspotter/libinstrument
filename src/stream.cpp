#include "../include/stream.hpp"
#include "../include/util.hpp"

/**
	@file src/stream.cpp

	@brief Class instrument::stream method implementation
*/

namespace instrument {

/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
stream::stream()
try:
string(),
m_handle(-1)
{
}
catch (...) {
	m_handle = -1;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
stream::stream(const stream &src)
try:
string(),
m_handle(-1)
{
	*this = src;
}
catch (...) {
	delete[] m_data;
	m_data = NULL;
	m_handle = -1;
}


/**
 * @brief Object destructor
 */
stream::~stream()
{
	close();
}


/**
 * @brief Get the handle
 *
 * @returns this->m_handle
 */
inline i32 stream::handle() const
{
	return m_handle;
}


/**
 * @brief Check if the stream is opened for output
 *
 * @returns true if the stream is open, false otherwise
 */
inline bool stream::is_open() const
{
	return m_handle >= 0;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
stream& stream::operator=(const stream &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Close the current stream (to sync current data) */
	close();

	/* Copy the buffer */
	string::operator=(rval);

	i32 fd = rval.m_handle;
	if ( unlikely(fd < 0) ) {
		return *this;
	}

	/* Duplicate the handle (descriptor) */
	m_handle = dup(fd);
	if ( unlikely(m_handle < 0) ) {
		throw exception(
			"failed to duplicate descriptor %d (errno %d - %s)",
			fd,
			errno,
			strerror(errno)
		);
	}

	return *this;
}


/**
 * @brief Close the stream
 *
 * @returns *this
 */
stream& stream::close()
{
	if ( likely(m_handle < 0) ) {
		return *this;
	}

	i32 retval;
	do {
		retval = ::close(m_handle);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	m_handle = -1;
	return *this;
}


/**
 * @brief Flush the buffered data to the stream
 *
 * @returns *this
 *
 * @throws i32 (errno)
 *
 * @note The buffer remains as is, if the stream isn't open
 * @note Synchronous output is enforced (even if O_NONBLOCK is specified)
 */
stream& stream::flush()
{
	i32 offset = 0, sz = m_length;
	while ( likely(sz > 0) ) {
		i32 written = write(m_handle, m_data + offset, sz);
		if ( unlikely(written < 0) ) {
			switch (errno) {
			case EINTR:
			case EAGAIN:
				continue;

			default:
				throw errno;
			}
		}

		sz -= written;
		offset += written;
	}

	/* Clear the buffer */
	clear();
	return *this;
}


/**
 * @brief
 *	Append <a href="index.html#sec5_4"><b>IDP</b></a> headers to the buffer
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @note
 *	The appended headers are for:<br><br>
 *	<ol>
 *		<li>executable absolute path
 *		<li>process ID
 *		<li>thread ID
 *		<li>timestamp (in microseconds)
 *	</ol><br>
 *
 *	Each header is formatted as 'name: value\\r\\n'. All the numeric values are
 *	hexadecimal. In IDP, the header section is terminated with a double \\r\\n
 *	followed by the message body (trace data). This method just appends the four
 *	headers (not the extra \\r\\n delimiter) to allow for custom headers before
 *	the trace data (exception headers, custom user headers e.t.c)
 */
stream& stream::header()
{
	const i8 *path = util::executable_path();

	struct timeval now;
	gettimeofday(&now, NULL);
	u64 tstamp = static_cast<u64> (now.tv_sec) * 10e+5 + now.tv_usec;

	try {
		append("path: %s\r\n", path);
		append("pid: %x\r\n", getpid());
		append("tid: %lx\r\n", pthread_self());
		append("tstamp: %lx\r\n", tstamp);

		delete[] path;
		return *this;
	}
	catch (...) {
		delete[] path;
		throw;
	}
}


/**
 * @brief Lock the stream (exclusively)
 *
 * @returns *this
 *
 * @throws i32 (errno)
 */
stream& stream::lock() const
{
	i32 retval;
	do {
		retval = flock(m_handle, LOCK_EX);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(retval < 0) ) {
		throw errno;
	}

	return const_cast<stream&> (*this);
}


/**
 * @brief Unlock the stream
 *
 * @returns *this
 *
 * @throws i32 (errno)
 */
stream& stream::unlock() const
{
	i32 retval;
	do {
		retval = flock(m_handle, LOCK_UN);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(retval < 0) ) {
		throw errno;
	}

	return const_cast<stream&> (*this);
}

}
