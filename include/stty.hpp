#ifndef _STTY
#define _STTY 1

/**
	@file include/stty.hpp

	@brief Class instrument::stty definition
*/

#include "./stream.hpp"

namespace instrument {

#define DEFAULT_BAUD 9600


/**
	@brief A buffered output stream for serial interfaces

	An stty object is a buffered output stream used to output LDP or generic data
	to any type of serial interface. The interfaces are configured for 8N1
	transmition, the baud rate is configurable (throughout a session). The class
	is not thread safe, the caller must implement thread synchronization. Basic
	stream locking methods are inherited from instrument::stream

	@see
		<a href="index.html#sec5_4">
			<b>5.4 LDP (Libinstrument Debug Protocol)</b>
		</a>
	@see <a href="index.html#sec5_5_3"><b>5.5.3 Using instrument::stty</b></a>
*/
class stty: virtual public stream
{
protected:

	/* Protected variables */

	u32 m_baud;											/**< @brief Baud rate */

	i8 *m_devnode;									/**< @brief Device node file (devfs) */


	/* Protected generic methods */

	virtual stty& config() const;

public:

	/* Static methods */

	static u32 translate_baud(u32);


	/* Constructors, copy constructors and destructor */

	stty(const i8*, u32 = DEFAULT_BAUD);

	stty(const stty&);

	virtual ~stty();

	virtual stty* clone() const;


	/* Accessor methods */

	virtual u32 baud() const;

	virtual const i8* devnode() const;

	virtual stty& set_baud(u32);


	/* Operator overloading methods */

	virtual stty& operator=(const stty&);


	/* Generic methods */

	virtual stty& discard() const;

	virtual stty& flush();

	virtual bool is_tty() const;

	virtual stty& open();

	virtual stty& open(bool);

	virtual stty& sync() const;
};

}

#endif
