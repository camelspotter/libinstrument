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
	@brief
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


	/* Protected generic methods */

	virtual property& clear();

public:

	/* Constructors, copy constructors and destructor */

	property();

	explicit property(string*);

	property(string*, string*);

	property(const property&);

	virtual	~property();

	virtual property* clone() const;


	/* Accessor methods */

	virtual bool is_valid() const;

	virtual const string* name() const;

	virtual const string* value() const;


	/* Operator overloading methods */

	virtual property& operator=(const property&);


	/* Generic methods */
};

}

#endif
