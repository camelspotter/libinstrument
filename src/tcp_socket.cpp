#include "../include/tcp_socket.hpp"
#include "../include/util.hpp"

/**
	@file src/tcp_socket.cpp

	@brief Class instrument::tcp_socket method implementation
*/

namespace instrument {

/**
 * @brief Object constructor
 *
 * @param[in] addr the peer (server) IP address (localhost if NULL is passed)
 *
 * @param[in] port the peer TCP port
 *
 * @throws std::bad_alloc
 */
tcp_socket::tcp_socket(const i8 *addr, i32 port)
try:
stream(),
m_address(NULL),
m_port(port)
{
	if ( unlikely(addr == NULL || strlen(addr) == 0) ) {
		addr = "127.0.0.1";
	}

	m_address = new i8[strlen(addr) + 1];
	strcpy(m_address, addr);
}
catch (...) {
	delete[] m_data;
	m_data = NULL;
	m_address = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
tcp_socket::tcp_socket(const tcp_socket &src)
try:
stream(src),
m_address(NULL),
m_port(src.m_port)
{
	m_address = new i8[strlen(src.m_address) + 1];
	strcpy(m_address, src.m_address);
}
catch (...) {
	close();

	delete[] m_data;
	m_data = NULL;
	m_address = NULL;
}


/**
 * @brief Object destructor
 */
tcp_socket::~tcp_socket()
{
	delete[] m_address;
	m_address = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline tcp_socket* tcp_socket::clone() const
{
	return new tcp_socket(*this);
}


/**
 * @brief Get the peer IP address
 *
 * @returns this->m_address
 */
inline const i8* tcp_socket::address() const
{
	return m_address;
}


/**
 * @brief Check if the socket is connected to its peer
 *
 * @returns true if the socket is connected, false otherwise
 */
inline bool tcp_socket::is_connected() const
{
	return m_handle >= 0;
}


/**
 * @brief Get the peer TCP port
 *
 * @returns this->m_port
 */
inline i32 tcp_socket::port() const
{
	return m_port;
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
tcp_socket& tcp_socket::operator=(const tcp_socket &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Copy the buffer and duplicate the stream descriptor */
	stream::operator=(rval);

	u32 len = strlen(rval.m_address);
	if (len > strlen(m_address)) {
		delete[] m_address;
		m_address = NULL;
		m_address = new i8[len + 1];
	}

	strcpy(m_address, rval.m_address);
	m_port = rval.m_port;
	return *this;
}


/**
 * @brief Flush the buffered data to the socket
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
tcp_socket& tcp_socket::flush()
{
	try {
		stream::flush();
		return sync();
	}
	catch (i32 err) {
		throw exception(
			"failed to send data @ %s:%d (errno %d - %s)",
			m_address,
			m_port,
			err,
			strerror(err)
		);
	}
}


/**
 * @brief Connect the socket to its peer
 *
 * @returns *this
 *
 * @throws instrument::exception
 *
 * @note
 *	If the socket is already connected, it is closed and re-connected to the new
 *	address/port
 */
tcp_socket& tcp_socket::open()
{
	if ( unlikely(m_handle >= 0) ) {
		close();
	}

	/* Create the stream socket */
	m_handle = socket(AF_INET, SOCK_STREAM, 0);
	if ( unlikely(m_handle < 0) ) {
		throw exception(
			"failed to create stream socket (errno %d - %s)",
			errno,
			strerror(errno)
		);
	}

	tcp_addr_t addr;
	util::memset(&addr, 0, sizeof(tcp_addr_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.s_addr = inet_addr(m_address);

	/* Connect the socket to its peer */
	ip_addr_t *ip = reinterpret_cast<ip_addr_t*> (&addr);
	i32 retval;
	do {
		retval = connect(m_handle, ip, sizeof(tcp_addr_t));
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(retval < 0) ) {
		close();

		throw exception(
			"failed to connect TCP/IP socket @ %s:%d (errno %d - %s)",
			m_address,
			m_port,
			errno,
			strerror(errno)
		);
	}

	return *this;
}


/**
 * @brief Set a socket option (applies only for the SOL_SOCKET ioctl level)
 *
 * @param[in] nm the option name
 *
 * @param[in] val the new option value (can be NULL for NO-OP)
 *
 * @param[in] sz the sizeof val
 *
 * @returns *this
 *
 * @throws instrument::exception
 *
 * @attention This method flushes the current buffer
 */
tcp_socket& tcp_socket::set_option(i32 nm, const void *val, u32 sz)
{
	__D_ASSERT(val != NULL);
	__D_ASSERT(sz > 0);
	if ( unlikely(val == NULL || sz == 0) ) {
		return *this;
	}

	/* Flush the buffer to avoid data loss or data corruption */
	flush();

	i32 retval = setsockopt(m_handle, SOL_SOCKET, nm, val, sz);
	if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to set socket option %d (errno %d - %s)",
			nm,
			errno,
			strerror(errno)
		);
	}

	return *this;
}


/**
 * @brief Shutdown one or both socket channels
 *
 * @param[in] ch the channel(s) to shutdown
 *
 * @returns *this
 *
 * @note
 *	If the object implements LDP, that is a unidirectional protocol, the read
 *	channel can be shutdown right after connection establishment
 */
inline tcp_socket& tcp_socket::shutdown(i32 ch) const
{
	if ( likely(m_handle >= 0) ) {
		::shutdown(m_handle, ch);
	}

	return const_cast<tcp_socket&> (*this);
}


/**
 * @brief Commit cached data to the network
 *
 * @returns *this
 */
inline tcp_socket& tcp_socket::sync() const
{
	return const_cast<tcp_socket&> (*this);
}

}
