#include "../include/style.hpp"

/**
	@file src/style.cpp

	@brief Class instrument::style method implementation
*/

namespace instrument {

/**
 * @brief Object constructor
 *
 * @param[in] nm the style name
 *
 * @param[in] fg the foreground color
 *
 * @param[in] bg the background color
 *
 * @param[in] set the text formatting attributes
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
style::style(const i8 *nm, color_t fg, color_t bg, attrset_t set):
m_attributes(set),
m_bgcolor(bg),
m_fgcolor(fg),
m_name(NULL)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	m_name = new i8[strlen(nm) + 1];
	strcpy(m_name, nm);
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
style::style(const style &src):
m_attributes(src.m_attributes),
m_bgcolor(src.m_bgcolor),
m_fgcolor(src.m_fgcolor),
m_name(NULL)
{
	m_name = new i8[strlen(src.m_name) + 1];
	strcpy(m_name, src.m_name);
}


/**
 * @brief Object destructor
 */
style::~style()
{
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline style* style::clone() const
{
	return new style(*this);
}


/**
 * @brief Get the text formatting attributes
 *
 * @returns this->m_attributes
 */
inline attrset_t style::attributes() const
{
	return m_attributes;
}


/**
 * @brief Get the background color
 *
 * @returns this->m_bgcolor
 */
inline color_t style::bgcolor() const
{
	return m_bgcolor;
}


/**
 * @brief Get the foreground color
 *
 * @returns this->m_fgcolor
 */
inline color_t style::fgcolor() const
{
	return m_fgcolor;
}


/**
 * @brief Get the style name
 *
 * @returns this->m_name
 */
inline const i8* style::name() const
{
	return m_name;
}


/**
 * @brief Set the text formatting attributes
 *
 * @param[in] set the new attributes
 *
 * @returns *this
 */
inline style& style::set_attributes(attrset_t set)
{
	m_attributes = set;
	return *this;
}


/**
 * @brief Set the background color
 *
 * @param[in] bg the new color
 *
 * @returns *this
 */
inline style& style::set_bgcolor(color_t bg)
{
	m_bgcolor = bg;
	return *this;
}


/**
 * @brief Set the foreground color
 *
 * @param[in] fg the new color
 *
 * @returns *this
 */
inline style& style::set_fgcolor(color_t fg)
{
	m_fgcolor = fg;
	return *this;
}


/**
 * @brief Set the style name
 *
 * @param[in] nm the new name
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
style& style::set_name(const i8 *nm)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	u32 len = strlen(nm);
	if (len > strlen(m_name)) {
		delete[] m_name;
		m_name = NULL;
		m_name = new i8[len + 1];
	}

	strcpy(m_name, nm);
	return *this;
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
style& style::operator=(const style &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	m_attributes = rval.m_attributes;
	m_bgcolor = rval.m_bgcolor;
	m_fgcolor = rval.m_fgcolor;

	return set_name(rval.m_name);
}


/**
 * @brief Apply the style to some text
 *
 * @param[in,out] dst the target text
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline style& style::apply(string &dst) const
{
	string esc;
	to_string(esc);

	dst.insert(0, esc)
		 .append("\e[0m");

	return const_cast<style&> (*this);
}


/**
 * @brief Check if a set of text formatting attributes is enabled
 *
 * @param[in] set the checked attributes
 *
 * @returns true if all attributes in the set are enabled, false otherwise
 */
inline bool style::is_attr_enabled(attrset_t set) const
{
	return (m_attributes & set) == set;
}


/**
 * @brief Enable/disable a set of text formatting attributes
 *
 * @param[in] set the affected attribute set
 *
 * @param[in] how true to enable, false to disable
 *
 * @returns *this
 */
inline style& style::set_attr_enabled(attrset_t set, bool how)
{
	if (how) {
		m_attributes |= set;
	}
	else {
		m_attributes &= ~set;
	}

	return *this;
}


/**
 * @brief Set a string with all the style escape sequences
 *
 * @param[out] dst the destination string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @attention Initial string contents are erased
 */
style& style::to_string(string &dst) const
{
	dst.clear();

	/* Add the background color, if not translucent */
	if ( unlikely(m_bgcolor != CLEAR) ) {
		dst.append("\e[48;5;%dm", m_bgcolor);
	}

	/* Add the foreground color */
	dst.append("\e[38;5;%dm", m_fgcolor);

	/* Add the escape sequence for each text formatting attribute */
	if ( unlikely(is_attr_enabled(BOLD)) ) {
		dst.append("\e[1m");
	}

	if ( unlikely(is_attr_enabled(DIM)) ) {
		dst.append("\e[2m");
	}

	if ( unlikely(is_attr_enabled(UNDERLINED)) ) {
		dst.append("\e[4m");
	}

	if ( unlikely(is_attr_enabled(BLINKING)) ) {
		dst.append("\e[5m");
	}

	if ( unlikely(is_attr_enabled(INVERTED)) ) {
		dst.append("\e[7m");
	}

	if ( unlikely(is_attr_enabled(HIDDEN)) ) {
		dst.append("\e[8m");
	}

	return const_cast<style&> (*this);
}

}
