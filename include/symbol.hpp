#ifndef _SYMBOL
#define _SYMBOL 1

/**
	@file include/symbol.hpp

	@brief Class instrument::symbol definition
*/

#include "./object.hpp"

namespace instrument {

/**
	@brief This class represents a program/library function symbol
*/
class symbol: virtual public object
{
protected:

	/* Protected variables */

	mem_addr_t m_addr;									/**< @brief Symbol address */

	i8 *m_name;													/**< @brief Symbol name */

public:

	/* Constructors, copy constructors and destructor */

	explicit symbol(mem_addr_t, const i8* = NULL);

	symbol(const symbol&);

	virtual ~symbol();

	virtual symbol* clone() const;


	/* Accessor methods */

	virtual mem_addr_t addr() const;

	virtual bool is_resolved() const;

	virtual const i8* name() const;

	virtual symbol& set_name(const i8* = NULL);


	/* Operator overloading methods */

	virtual symbol& operator=(const symbol&);
};

}

#endif
