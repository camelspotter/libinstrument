#ifndef _DICTIONARY
#define _DICTIONARY 1

/**
	@file include/dictionary.hpp

	@brief Class instrument::dictionary definition
*/

#include "./string.hpp"
#include "./list.hpp"

namespace instrument {

#define REGEXP_LOOKUP_MODE	true

#define SIMPLE_LOOKUP_MODE	false


/**
	@brief A named collection of words (for syntax highlighters)

	A dictionary object is used to create a collection of tokens, under a common
	name. Dictionary data can be loaded from regular text files (.dict extension).
	Each non-empty line in the source file is translated as a single token. A line
	with only whitespace characters is considered an empty line. The tokens are
	trimmed to remove leading and trailing whitespace characters. If the source
	file is empty no tokens are loaded, but the dictionary object remains valid.
	The dictionary class inherits from instrument::list (T = instrument::string)
	all its methods for item management. A dictionary can be looked up for literal
	strings or for POSIX extended regular expressions (with or without case
	sensitivity). If a word appears more than once, its first occurence is used. A
	dictionary is not thread safe, users must implement thread synchronization

	@see instrument::parser
	@see
		<a href="index.html#sec5_7">
			<b>5.7 Using the stack trace parser (syntax highlighter)</b>
		</a>
*/
class dictionary: virtual public list<string>
{
protected:

	/* Protected variables */

	bool m_mode;							/**< @brief Lookup mode */

	i8 *m_name;								/**< @brief Dictionary name */

public:

	/* Constructors, copy constructors and destructor */

	dictionary(const i8*, const i8* = NULL, bool = SIMPLE_LOOKUP_MODE);

	dictionary(const dictionary&);

	virtual	~dictionary();

	virtual dictionary* clone() const;


	/* Accessor methods */

	virtual bool mode() const;

	virtual const i8* name() const;

	virtual dictionary& set_mode(bool);

	virtual dictionary& set_name(const i8*);


	/* Operator overloading methods */

	virtual dictionary& operator=(const dictionary&);


	/* Generic methods */

	virtual dictionary& load_file(const i8*);

	virtual const string* lookup(const string&, bool = false) const;
};

}

#endif
