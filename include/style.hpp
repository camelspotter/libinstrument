#ifndef _STYLE
#define _STYLE 1

/**
	@file include/style.hpp

	@brief Class instrument::style definition
*/

#include "./string.hpp"

namespace instrument {

/**
	@brief A set of formatting attributes for VT100 (and compatible) terminals

	@see instrument::parser
	@see
		<a href="index.html#sec5_7">
			<b>5.7 Using the stack trace parser (syntax highlighter)</b>
		</a>
*/
class style: virtual public object
{
protected:

	/* Protected variables */

	attrset_t m_attributes;				/**< @brief Text formatting attribute bitmask */

	color_t m_bgcolor;						/**< @brief Background color */

	color_t m_fgcolor;						/**< @brief Foreground (text) color */

	i8 *m_name;										/**< @brief Style name */

public:

	/* Constructors, copy constructors and destructor */

	style(const i8*, color_t = WHITE, color_t = CLEAR, attrset_t = 0);

	style(const style&);

	virtual ~style();

	virtual style* clone() const;


	/* Accessor methods */

	virtual attrset_t attributes() const;

	virtual color_t bgcolor() const;

	virtual color_t fgcolor() const;

	virtual const i8* name() const;

	virtual style& set_attributes(attrset_t);

	virtual style& set_bgcolor(color_t);

	virtual style& set_fgcolor(color_t);

	virtual style& set_name(const i8*);


	/* Operator overloading methods */

	virtual style& operator=(const style&);


	/* Generic methods */

	virtual style& apply(string&) const;

	virtual bool is_attr_enabled(attrset_t) const;

	virtual style& set_attr_enabled(attrset_t, bool);

	virtual style& to_string(string&) const;


	/* Public static variables */

	/**
		@brief Text formatting attributes of VT100 terminals
	*/
	static const enum {

		BLINKING		= 0x01,		BOLD				= 0x02,		DIM					= 0x04,

		HIDDEN			= 0x08,		INVERTED		= 0x10,		JOINED			= 0x3F,

		UNDERLINED 	= 0x20

	} vt100_attributes;

	/**
		@brief Basic palette of VT100 terminals
	*/
	static const enum {

		BLACK			 	= 0x10,		BLUE				= 0x0C,		CLEAR				=	0x00,

		CYAN				= 0x0E,		GRAY				= 0x08,		GREEN				= 0x0A,

		MAGENTA			= 0x0D,		RED					= 0x09,		WHITE				= 0x0F,

		YELLOW			= 0x0B

	} vt100_palette;
};

}

#endif
