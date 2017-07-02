#ifndef _TCP_SOCKET
#define _TCP_SOCKET 1

/**
	@file include/tcp_socket.hpp

	@brief Class instrument::tcp_socket definition
*/

#include "./stream.hpp"

namespace instrument {

/**
	@brief A buffered TCP/IP socket output stream

	A tcp_socket object is a buffered TCP/IP client socket, designed specifically
	to implement the client side of IDP, or any other unidirectional application
	protocol (write only). The class currently supports only IPv4 addresses. This
	class is not thread safe, the caller must implement thread synchronization,
	nevertheless basic stream locking is inherited from instrument::stream

	@see
		<a href="index.html#sec5_4">
			<b>5.4 IDP (Instrumentation Data Protocol)</b>
		</a>
	@see
		<a href="index.html#sec5_5_2">
			<b>5.5.2 Using instrument::tcp_socket</b>
		</a>

	@todo Add domain name lookup (getaddrinfo will also resolve IPv4 vs IPv6)
	@todo Implement connection drop detection (SO_KEEPALIVE, SIGPIPE)
	@todo Fine tune socket options (buffer size, linger, no-delay e.t.c)

	@test Stream locking
	@test TCP_NODELAY option or other means to flush cached network data
	@test Exploit shutdown on close
*/
class tcp_socket: virtual public stream
{
protected:

	/* Protected variables */

	i8 *m_address;							/**< @brief Peer IP address (numerical, IPv4) */

	i32 m_port;									/**< @brief Peer TCP port */

public:

	/* Constructors, copy constructors and destructor */

	explicit tcp_socket(const i8*, i32 = g_idp_port);

	tcp_socket(const tcp_socket&);

	virtual ~tcp_socket();

	virtual tcp_socket* clone() const;


	/* Accessor methods */

	virtual const i8* address() const;

	virtual bool is_connected() const;

	virtual i32 port() const;


	/* Operator overloading methods */

	virtual tcp_socket& operator=(const tcp_socket&);


	/* Generic methods */

	virtual tcp_socket& flush();

	virtual tcp_socket& open();

	virtual tcp_socket& set_option(i32, const void*, u32);

	virtual tcp_socket& shutdown(i32) const;

	virtual tcp_socket& sync() const;
};

}

#endif
