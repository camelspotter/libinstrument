#include "../include/string.hpp"
#include "../include/util.hpp"

/**
	@file src/string.cpp

	@brief Class instrument::string method implementation
*/

namespace instrument {

/**
 * @brief
 *	Fill with a printf-style format C-string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format C-string (can be NULL)
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string& string::format(const i8 *fmt, va_list args)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		return memalign(0);
	}

	try {
		va_list cpargs;
		va_copy(cpargs, args);
		u32 len = util::va_size(fmt, cpargs);
		memalign(len);

		util::va_format(m_data, fmt, args);
		m_length = len;
		return *this;
	}
	catch (...) {
		va_end(args);
		throw;
	}
}


/**
 * @brief Allocate aligned memory, mandate a minimum buffer size
 *
 * @param[in] len the mandatory length (without the trailing \\0)
 *
 * @param[in] keep true to keep the current data
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::memalign(u32 len, bool keep)
{
	if ( unlikely(len < m_size) ) {
		return (unlikely(keep)) ? *this : clear();
	}

	/* Aligned size */
	m_size = (len + g_memblock_sz) / g_memblock_sz;
	m_size *= g_memblock_sz;

	i8 *aligned = new i8[m_size];
	if ( unlikely(keep) ) {
		__D_ASSERT(m_data != NULL);
		__D_ASSERT(strlen(m_data) == m_length);

		strcpy(aligned, m_data);
	}
	else {
		aligned[0] = '\0';
		m_length = 0;
	}

	delete[] m_data;
	m_data = aligned;
	return *this;
}


/**
 * @brief Stream insertion operator for instrument::string objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, const string &rval)
{
	return lval << rval.m_data;
}


/**
 * @brief Object constructor
 *
 * @param[in] sz the minimum mandated buffer size
 *
 * @throws std::bad_alloc
 */
string::string(u32 sz):
m_data(NULL),
m_length(0),
m_locale(NULL),
m_size(0)
{
	memalign(sz);
}


/**
 * @brief Object constructor
 *
 * @param[in] fmt a printf-style format C-string (can be NULL)
 *
 * @param[in] ... a variable argument list
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @note
 *	This constructor is also an implicit type conversion operator between i8*
 *	and instrument::string
 */
string::string(const i8 *fmt, ...)
try:
m_data(NULL),
m_length(0),
m_locale(NULL),
m_size(0)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		memalign(0);
	}
	else {
		va_list args;
		va_start(args, fmt);
		format(fmt, args);
	}
}
catch(...) {
	delete[] m_data;
	m_data = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
string::string(const string &src):
m_data(NULL),
m_length(0),
m_locale(NULL),
m_size(0)
{
	*this = src;
}


/**
 * @brief Object destructor
 */
string::~string()
{
	delete[] m_data;
	delete[] m_locale;
	m_data = NULL;
	m_locale = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline string* string::clone() const
{
	return new string(*this);
}


/**
 * @brief Get/set the character at an offset
 *
 * @param[in] i the offset
 *
 * @returns &this->m_data[i]
 *
 * @throws instrument::exception
 */
inline i8& string::at(u32 i)
{
	if ( unlikely(i >= m_length) ) {
		throw exception("offset out of string bounds (%d >= %d)", i, m_length);
	}

	return m_data[i];
}


/**
 * @brief Get the buffer size
 *
 * @returns this->m_size
 */
inline u32 string::buffer_size() const
{
	return m_size;
}


/**
 * @brief Get the C-string equivalent
 *
 * @returns this->m_data
 */
inline const i8* string::cstring() const
{
	return m_data;
}


/**
 * @brief Get the character count
 *
 * @returns this->m_length
 */
inline u32 string::length() const
{
	return m_length;
}


/**
 * @brief Get the codepage
 *
 * @returns this->m_locale
 */
inline const codepage_t string::locale() const
{
	return m_locale;
}


/**
 * @brief
 *	Fill with a printf-style format C-string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format C-string (can be NULL)
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string& string::set(const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return memalign(0);
	}

	__D_ASSERT(fmt != m_data);
	if ( unlikely(fmt == m_data) ) {
		return *this;
	}

	va_list args;
	va_start(args, fmt);
	return format(fmt, args);
}


/**
 * @brief Copy a string
 *
 * @param[in] src the source string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::set(const string &src)
{
	if ( unlikely(this == &src) ) {
		return *this;
	}

	memalign(src.m_length);
	strcpy(m_data, src.m_data);
	m_length = src.m_length;

	return *this;
}


/**
 * @brief Set the locale
 *
 * @param[in] locale the new locale
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
inline string& string::set_locale(const codepage_t locale)
{
	/* @todo Implement */
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
inline string& string::operator=(const string &rval)
{
	return set(rval);
}


/**
 * @brief Compound addition-assignment operator (append)
 *
 * @param[in] rval the appended string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
inline string& string::operator+=(const string &rval)
{
	return append(rval);
}


/**
 * @brief Equality operator
 *
 * @param[in] rval the compared string
 *
 * @returns true if the two string are equal, false otherwise
 */
inline bool string::operator==(const string &rval) const
{
	return equals(rval);
}


/**
 * @brief Subscript operator
 *
 * @param[in] i the index
 *
 * @returns &this->m_data[i]
 *
 * @throws instrument::exception
 */
inline i8& string::operator[](u32 i)
{
	return at(i);
}


/**
 * @brief Append a string
 *
 * @param[in] tail the appended string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::append(const string &tail)
{
	u32 len = m_length + tail.m_length;
	memalign(len, true);

	strcpy(m_data + m_length, tail.m_data);
	m_length = len;

	return *this;
}


/**
 * @brief
 *	Append a printf-style format C-string expanded with the values of a variable
 *	argument list
 *
 * @param[in] fmt a printf-style format C-string (can be NULL)
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string& string::append(const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return *this;
	}

	string tmp;
	va_list args;
	va_start(args, fmt);
	tmp.format(fmt, args);

	return append(tmp);
}


/**
 * @brief Append a character
 *
 * @param[in] ch the appended character
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline string& string::append(i8 ch)
{
	return append("%c", ch);
}


/**
 * @brief
 *	Get the available buffer size, the number of characters that can be appended
 *	without reallocation
 *
 * @returns this->m_size - (this->m_length + 1)
 */
inline u32 string::available() const
{
	return m_size - m_length - 1;
}


/**
 * @brief Clear contents
 *
 * @returns *this
 */
inline string& string::clear()
{
	m_data[0] = '\0';
	m_length = 0;
	return *this;
}


/**
 * @brief Clip the string to a new length
 *
 * param[in] offset the clipping offset
 *
 * @returns *this
 */
inline string& string::clip(u32 pffset)
{
	if ( unlikely(offset >= m_length) ) {
		return *this;
	}

	m_data[offset] = '\0';
	m_length = offset;
	return *this;
}


/**
 * @brief Compare to another string
 *
 * @param[in] rval the compared string
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns
 *	<0, zero, or >0 if this is respectively less than, equal, or greater than
 *	the compared string, lexicographically
 */
inline i32 string::compare(const string &rval, bool icase) const
{
	if ( unlikely(icase) ) {
		return strcasecmp(m_data, rval.m_data);
	}

	return strcmp(m_data, rval.m_data);
}


/**
 * @brief Compare to another string for equality
 *
 * @param[in] rval the compared string
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns true if the strings are equal, false otherwise
 */
inline bool string::equals(const string &rval, bool icase) const
{
	return compare(rval, icase) == 0;
}


/**
 * @brief Insert a string at a specified position
 *
 * @param[in] pos the insertion position
 *
 * @param[in] rval the inserted text
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::insert(u32 pos, const string &rval)
{
	if ( unlikely(pos >= m_length) ) {
		return append(rval);
	}

	u32 len = m_length + rval.m_length;
	memalign(len, true);

	/* Shift the tail substring to make place for the inserted text */
	u32 i = m_length;
	u32 j = len;
	i32 tail = m_length - pos;

	while ( likely(tail-- >= 0) ) {
		m_data[j--] = m_data[i--];
	}

	/* Insert text */
	strncpy(m_data + pos, rval.m_data, rval.m_length);

	m_length = len;
	return *this;
}


/**
 * @brief
 *	Insert a printf-style format C-string expanded with the values of a variable
 *	argument list, at a specified position
 *
 * @param[in] pos the insertion position
 *
 * @param[in] fmt a printf-style format C-string (can by NULL)
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string& string::insert(u32 pos, const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return *this;
	}

	/* Format a temporary string */
	string tmp;
	va_list args;
	va_start(args, fmt);
	tmp.format(fmt, args);

	return insert(pos, tmp);
}


/**
 * @brief Match against a POSIX extended regular expression
 *
 * @param[in] expr the regular expression
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns true if there is a match, false otherwise
 *
 * @throws instrument::exception
 */
bool string::match(const string &expr, bool icase) const
{
	i32 flags = REG_EXTENDED | REG_NOSUB;
	if ( unlikely(icase) ) {
		flags |= REG_ICASE;
	}

	/* Compile the regular expression and perform the matching */
	regex_t regexp;
	i32 retval = regcomp(&regexp, expr.cstring(), flags);
	if ( likely(retval == 0) ) {
		retval = regexec(&regexp, m_data, 0, NULL, 0);
		regfree(&regexp);
		return !retval;
	}

	/* If the expression compilation failed */
	i32 len = regerror(retval, &regexp, NULL, 0);
	i8 errbuf[len];
	regerror(retval, &regexp, errbuf, len);
	regfree(&regexp);

	throw exception(
		"failed to compile regexp '%s' (regex errno %d - %s)",
		expr.cstring(),
		retval,
		errbuf
	);
}


/**
 * @brief Remove a substring
 *
 * @param[in] from the substring start offset
 *
 * @param[in] len the substring length
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
string& string::reduce(u32 from, u32 len)
{
	if ( unlikely(from >= m_length) ) {
		return *this;
	}

	if ( unlikely(len + from > m_length) ) {
		return clip(from);
	}

	if ( unlikely(len == 0) ) {
		return *this;
	}
	else if ( unlikely(len == m_length) ) {
		return clear();
	}

	for (u32 i = from, j = from + len; likely(j <= m_length); i++, j++) {
		m_data[i] = m_data[j];
	}

	m_length -= len;
	return *this;
}


/**
 * @brief Fill the whole buffer with a constant byte
 *
 * @param[in] ch the constant byte
 *
 * @returns *this
 *
 * @attention No matter how the string is shred, it stays valid (cleared)
 */
inline string& string::shred(u8 ch)
{
	util::memset(m_data, ch, m_size);
	return clear();
}


/**
 * @brief Tokenize using a POSIX extended regular expression
 *
 * @param[in] expr the delimiter expression
 *
 * @param[in] imatch false to include the actual matches in the result
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns the list of tokens (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
chain<string>* string::split(const string &expr, bool imatch, bool icase) const
{
	chain<string> *tokens = NULL;
	string *word = NULL;
	regex_t regexp;

	/* If an exception occurs, release resources and rethrow it */
	try {
		tokens = new chain<string>;

		/* Compile the regular expression */
		i32 flags = REG_EXTENDED;
		if ( unlikely(icase) ) {
			flags |= REG_ICASE;
		}

		i32 retval = regcomp(&regexp, expr.cstring(), flags);
		if ( unlikely(retval != 0) ) {
			i32 len = regerror(retval, &regexp, NULL, 0);
			i8 errbuf[len];
			regerror(retval, &regexp, errbuf, len);

			throw exception(
				"failed to compile regexp '%s' (regex errno %d - %s)",
				expr.cstring(),
				retval,
				errbuf
			);
		}

		regmatch_t match;
		regoff_t offset = 0;
		i32 len = m_length;
		do {
			bool found = !regexec(&regexp, m_data + offset, 1, &match, 0);

			/*
			 * The delimiter pattern is found. The left token is from the beginning of
			 * the text plus an offset, to the beginning of the matched text. The
			 * right token is from the end of the matched text to the end of the text
			 * or to the beginning of the next matched text. This will be evaluated on
			 * the next loop pass
			 */
			if ( likely(found) ) {
				i32 bgn = match.rm_so;
				i32 end = match.rm_eo;
				if ( unlikely(end == 0) ) {
					throw exception(
						"logic error in regular expression '%s'",
						expr.cstring());
				}

				word = new string("%.*s", bgn, m_data + offset);
				tokens->add(word);
				word = NULL;

				/* Include matched text in tokens */
				if ( unlikely(!imatch) ) {
					word = new string("%.*s", end - bgn, m_data + offset + bgn);
					tokens->add(word);
					word = NULL;
				}

				offset += end;
				if ( unlikely(offset > len) ) {
					break;
				}
			}

			/*
			 * The pattern isn't found. That means that either the delimiter was never
			 * in the text, so the whole text is the one and only token, or there is
			 * some text after the last delimiter. In that case this trailing text is
			 * the last token
			 */
			else if ( likely(offset <= len) ) {
				word = new string(m_data + offset);
				tokens->add(word);
				word = NULL;
				break;
			}

			/* No more tokens */
			else {
				break;
			}
		}
		while ( likely(true) );

		regfree(&regexp);
		return tokens;
	}
	catch (...) {
		delete tokens;
		delete word;
		regfree(&regexp);
		throw;
	}
}


/**
 * @brief Get a substring
 *
 * @param[in] from the substring start offset
 *
 * @param[in] len the substring length
 *
 * @param[in] inplace true to store the substring in place
 *
 * @returns the substring (heap allocated), or this if inplace is true
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string* string::substring(u32 from, u32 len, bool inplace)
{
	if ( unlikely(from >= m_length) ) {
		throw exception("offset out of string bounds (%d >= %d)", from, m_length);
	}

	if ( unlikely(len + from > m_length) ) {
		len = m_length - from;
	}

	i8 *substr = new i8[len + 1];
	strncpy(substr, m_data + from, len);
	substr[len] = '\0';

	try {
		string *retval = this;

		if ( likely(!inplace) ) {
			retval = new string(substr);
		}
		else {
			strcpy(m_data, substr);
			m_length = len;
		}

		delete[] substr;
		return retval;
	}
	catch (...) {
		delete[] substr;
		throw;
	}
}


/**
 * @brief Remove leading and/or trailing whitespace characters
 *
 * @param[in] which one of TRIM_LEADING, TRIM_TRAILING, TRIM_ALL (default)
 *
 * @returns *this
 */
string& string::trim(i32 which)
{
	if ( likely(which <= TRIM_ALL) ) {
		/* Estimate the number of leading whitespace characters */
		u32 i;
		for (i = 0; likely(i < m_length); i++) {
			if ( likely(!isspace(m_data[i])) ) {
				break;
			}
		}

		/* Remove them */
		if ( unlikely(i > 0 && i < m_length) ) {
			strcpy(m_data, m_data + i);
			m_length -= i;
		}

		/* If the string is filled with whitespace characters */
		else if ( unlikely(i == m_length) ) {
			return clear();
		}
	}

	if ( likely(which >= TRIM_ALL) ) {
		/* Estimate the number of trailing whitespace characters */
		i32 i;
		for (i = m_length - 1; likely(i >= 0); i--) {
			if ( likely(!isspace(m_data[i])) ) {
				break;
			}
		}

		m_data[++i] = '\0';
		m_length = i;
	}

	return *this;
}

}
