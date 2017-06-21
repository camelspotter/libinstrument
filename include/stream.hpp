#ifndef _STREAM
#define _STREAM 1

/**
	@file include/stream.hpp

	@brief Class instrument::stream definition
*/

#include "./string.hpp"

namespace instrument {

/**
	@brief
		This abstract class is the base of all buffered output stream types (for
		files, sockets, serial interfaces e.t.c)

	Subclassing class stream is the standard way to create objects that output
	trace and other data to various media. A stream-derived object is both a
	string buffer and an output stream for any type of media that can be accessed
	using an integer descriptor/handle. Currently, libinstrument is shipped with
	three such implementations, instrument::file for <b>files</b>,
	instrument::tcp_socket for <b>TCP/IP sockets</b> and instrument::stty for
	<b>serial interfaces</b>. Class stream is not thread safe, but it implements
	basic stream locking. The buffer part of the object can be manipulated using
	the methods inherited from instrument::string. For example if you need to copy
	only the buffer from one object to another (even of different types) use the
	string::set(const string&) method instead of the overloaded assignment
	operator

	@see
		<a href="index.html#sec5_4">
			<b>5.4 LDP (Libinstrument Debug Protocol)</b>
		</a>
	@see <a href="index.html#sec5_5"><b>5.5 Buffered output streams</b></a>

	@todo Implement the udp_socket subclass
	@todo <b style="color: #ff0000">[ ? ]</b> Add method try_lock
*/
class stream: virtual public string
{
protected:

	/* Protected variables */

	i32 m_handle;										/**< @brief Stream handle (descriptor) */

public:

	/* Constructors, copy constructors and destructor */

	stream();

	stream(const stream&);

	virtual ~stream() = 0;									/**< @brief To be implemented */

	virtual stream* clone() const = 0;			/**< @brief To be implemented */


	/* Accessor methods */

	virtual i32 handle() const;

	virtual bool is_open() const;


	/* Operator overloading methods */

	virtual stream& operator=(const stream&);


	/* Generic methods */

	virtual stream& close();

	virtual stream& flush() = 0;						/**< @brief To be implemented */

	virtual stream& header();

	virtual stream& lock() const;

	virtual stream& open() = 0;							/**< @brief To be implemented */

	virtual stream& sync() const = 0;				/**< @brief To be implemented */

	virtual stream& unlock() const;
};

}

#endif
