#ifndef _STRING
#define _STRING 1

/**
	@file include/string.hpp

	@brief Class instrument::string definition
*/

#include "./chain.hpp"

namespace instrument {

#define TRIM_LEADING		-1

#define TRIM_TRAILING		1

#define TRIM_ALL				0


/**
	@brief Lightweight string buffer class (for ISO-8859-1 text by default)

	A string object is mainly used to store trace text. Text is easily appended
	using printf-style format strings and variable argument lists. Memory can be
	allocated in blocks (aligning) to reduce overhead when appending multiple
	small strings. It is comparable against POSIX extended regular expressions.

	It is very easy to direct library output to any kind of stream (console, file,
	serial, network, plugin, device e.t.c), by storing that output (trace) in
	string buffers or other subclassed objects.

	Apart from traces a string can be used for generic, dynamic text manipulation.
	This class is not thread safe, the caller must implement thread sychronization

	@todo Use std::regex (C++11) class for portability
	@todo Add full i18n and l10n
*/
class string: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_data;								/**< @brief String data */

	u32 m_length;							/**< @brief Character count */

	codepage_t m_locale; 			/**< @brief Data codepage */

	u32 m_size;								/**< @brief Buffer size */


	/* Protected generic methods */

	virtual string& format(const i8*, va_list);

	virtual string& memalign(u32, bool = false);

public:

	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, const string&);


	/* Constructors, copy constructors and destructor */

	explicit string(u32 = 0);

	string(const i8*, ...);

	string(const string&);

	virtual	~string();

	virtual string* clone() const;


	/* Accessor methods */

	virtual i8& at(u32);

	virtual	u32 buffer_size() const;

	virtual	const i8* cstring() const;

	virtual	u32 length() const;

	virtual const codepage_t locale() const;

	virtual	string& set(const i8*, ...);

	virtual string& set(const string&);

	virtual string& set_locale(const codepage_t);


	/* Operator overloading methods */

	virtual string& operator=(const string&);

	virtual string& operator+=(const string&);

	virtual bool operator==(const string&) const;

	virtual i8& operator[](u32);


	/* Generic methods */

	virtual string& append(const string&);

	virtual string& append(const i8*, ...);

	virtual string& append(i8);

	virtual u32 available() const;

	virtual string& clear();

	virtual i32 compare(const string&, bool = false) const;

	virtual bool equals(const string&, bool = false) const;

	virtual string& insert(u32, const string&);

	virtual string& insert(u32, const i8*, ...);

	virtual bool match(const string&, bool = false) const;

	virtual string& shred(u8 = 0);

	virtual chain<string>* split(const string&, bool = true, bool = false) const;

	virtual string& trim(i32 = TRIM_ALL);
};

}

#endif
