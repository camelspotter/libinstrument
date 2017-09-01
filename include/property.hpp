#ifndef _PROPERTY
#define _PROPERTY 1

/**
	@file include/property.hpp

	@brief Class instrument::property definition
*/

#include "./list.hpp"
#include "./string.hpp"

namespace instrument {

/**
	@brief This class represents an entry in a .properties file
*/
class property: virtual public object
{
protected:

	/* Protected variables */

	list<string> *m_comments;

	string *m_inline_comment;

	bool m_invalid;

	string *m_name;												/**< @brief Property name */

	string *m_value;											/**< @brief Property value */

public:

	friend class properties;


	/* Constructors, copy constructors and destructor */

	property();

	property(const property&);

	virtual	~property();

	virtual property* clone() const;


	/* Accessor methods */

	virtual const list<string>* comments() const;

	virtual const string* inline_comment() const;

	virtual bool is_valid() const;

	virtual const string* name() const;

	virtual const string* value() const;


	/* Operator overloading methods */

	virtual property& operator=(const property&);


	/* Generic methods */

	virtual const string* comment(u32) const;

	virtual u32 comment_count() const;

	virtual property& empty();

	virtual bool is_empty() const;
};

}

#endif
