#include "../include/stty.hpp"
#include "../include/util.hpp"

/**
	@file src/stty.cpp

	@brief Class instrument::stty method implementation
*/

namespace instrument {

/**
 * @brief Configure the serial interface
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
stty& stty::config() const
{
	__D_ASSERT(m_handle >= 0);
	if ( unlikely(m_handle < 0) ) {
		return const_cast<stty&> (*this);
	}

	u32 real_baud = translate_baud(m_baud);

	struct termios conf;
	util::memset(&conf, 0, sizeof(struct termios));
	conf.c_cflag = real_baud | CS8 | CRTSCTS | CREAD | CLOCAL;
	conf.c_iflag = IGNPAR;
	conf.c_cc[VMIN] = 1;

	i32 retval;
	do {
		retval = tcsetattr(m_handle, TCSANOW, &conf);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to configure serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
	}

	/* Discard all pending data */
	return discard();
}


/**
 * @brief Translate a number to a baud rate as defined in termios.h
 *
 * @param[in] rate the value to translate
 *
 * @returns the translated value
 *
 * @throws instrument::exception
 *
 * @attention
 *	For simplicity, only a subset (most commonly used) of the values defined in
 *	termios.h are translatable
 */
u32 stty::translate_baud(u32 rate)
{
	switch (rate) {
	case 9600:
		return B9600;

	case 19200:
		return B19200;

	case 38400:
		return B38400;

	case 57600:
		return B57600;

	case 115200:
		return B115200;

	case 230400:
		return B230400;

	case 460800:
		return B460800;

	default:
		throw exception("invalid argument: rate (=%d)", rate);
	}
}


/**
 * @brief Object constructor
 *
 * @param[in] port the path of the serial interface device node
 *
 * @param[in] baud the baud rate
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @note The port configuration (except baud rate) is fixed to 8N1
 */
stty::stty(const i8 *port, u32 baud)
try:
stream(),
m_baud(baud),
m_devnode(NULL)
{
	if ( unlikely(port == NULL) ) {
		throw exception("invalid argument: port (=%p)", port);
	}

	m_devnode = new i8[strlen(port) + 1];
	strcpy(m_devnode, port);
}
catch (...) {
	delete[] m_data;
	m_data = NULL;
	m_devnode = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
stty::stty(const stty &src)
try:
stream(src),
m_baud(src.m_baud),
m_devnode(NULL)
{
	m_devnode = new i8[strlen(src.m_devnode) + 1];
	strcpy(m_devnode, src.m_devnode);
}
catch (...) {
	close();

	delete[] m_data;
	m_data = NULL;
	m_devnode = NULL;
}


/**
 * @brief Object destructor
 */
stty::~stty()
{
	delete[] m_devnode;
	m_devnode = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline stty* stty::clone() const
{
	return new stty(*this);
}


/**
 * @brief Get the baud rate
 *
 * @returns this->m_baud
 */
inline u32 stty::baud() const
{
	return m_baud;
}


/**
 * @brief Get the path of the serial interface device node
 *
 * @returns this->m_devnode
 */
inline const i8* stty::devnode() const
{
	return m_devnode;
}


/**
 * @brief Set the baud rate
 *
 * @param[in] baud the new baud rate
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
stty& stty::set_baud(u32 baud)
{
	if ( unlikely(m_baud == baud) ) {
		return *this;
	}

	m_baud = baud;
	if ( unlikely(m_handle < 0) ) {
		return *this;
	}

	return config();
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
stty& stty::operator=(const stty &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Copy the buffer and duplicate the stream descriptor */
	stream::operator=(rval);

	u32 len = strlen(rval.m_devnode);
	if (len > strlen(m_devnode)) {
		delete[] m_devnode;
		m_devnode = NULL;
		m_devnode = new i8[len + 1];
	}

	strcpy(m_devnode, rval.m_devnode);
	return set_baud(rval.m_baud);
}


/**
 * @brief Discard the data cached in the serial interface (in-kernel) buffers
 *
 * @returns *this
 */
stty& stty::discard() const
{
	i32 retval;
	do {
		retval = tcflush(m_handle, TCIOFLUSH);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

#if DBG_LEVEL & DBGL_WARNING
	if ( unlikely(retval < 0) ) {
		util::dbg_warn(
			"failed to clear the buffers of serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
	}
#endif

	return const_cast<stty&> (*this);
}


/**
 * @brief Flush the buffered data to the serial interface
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
stty& stty::flush()
{
	try {
		stream::flush();
		return sync();
	}
	catch (i32 err) {
		discard();

		throw exception(
			"failed to send data to serial interface '%s' (errno %d - %s)",
			m_devnode,
			err,
			strerror(err)
		);
	}
}


/**
 * @brief Check if the device node is a terminal
 *
 * @returns true if the node is a terminal, false otherwise
 */
inline bool stty::is_tty() const
{
	if ( likely(m_handle >= 0) ) {
		return isatty(m_handle);
	}

	return false;
}


/**
 * @brief Open the serial interface for output
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
inline stty& stty::open()
{
	return open(false);
}


/**
 * @brief Open the serial interface for output
 *
 * @param[in] ctty true to make the interface the process controlling terminal
 *
 * @returns *this
 *
 * @throws instrument::exception
 *
 * @note If the interface is already open, it is re-opened with the new settings
 */
stty& stty::open(bool ctty)
{
	if ( unlikely(m_handle >= 0) ) {
		close();
	}

	/* Stat the device node path and make some preliminary checks */
	fileinfo_t inf;
	i32 retval = stat(m_devnode, &inf);
	if ( unlikely(errno == ENOENT) ) {
		throw exception("device node '%s' does not exist", m_devnode);
	}

	/* Stat failed */
	else if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to stat path '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
	}

	/* Device node is not a character device */
	else if ( unlikely(!util::is_chardev(inf)) ) {
		throw exception("'%s' is not a character device", m_devnode);
	}

	/* Device node is not writable */
	else if ( unlikely(!util::is_writable(inf)) ) {
		throw exception("serial interface '%s' is not writable", m_devnode);
	}

	u32 flags = O_WRONLY;
	if ( likely(!ctty) ) {
		flags |= O_NOCTTY;
	}

	/* Open the device node */
	do {
		m_handle = ::open(m_devnode, flags);
	}
	while ( unlikely(m_handle < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(m_handle < 0) ) {
		throw exception(
			"failed to open serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
	}

	return config();
}


/**
 * @brief Commit cached data to the serial interface line
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
stty& stty::sync() const
{
	i32 retval;
	do {
		retval = tcdrain(m_handle);
	}
	while ( unlikely(retval < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to flush serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
	}

	return const_cast<stty&> (*this);
}

}
