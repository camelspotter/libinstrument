#ifndef _SYMTAB
#define _SYMTAB 1

/**
	@file include/symtab.hpp

	@brief Class instrument::symtab definition
*/

#include "./list.hpp"
#include "./symbol.hpp"

namespace instrument {

/**
	@brief This class represents a program/library symbol table

	A symtab object can load code from executables or dynamic shared objects, with
	absolute addressing or position independent and of any binary format supported
	by the libbfd backends on the host (target) machine (elf, coff, ecoff e.t.c).

	To optimize lookups the symbol table (as structured in libbfd) is parsed, the
	non-function symbols are discarded and function symbols are demangled once and
	stored in simpler data structures. The symbol list is sorted by address and
	binary searched for even better lookup times, its size is fixed and its random
	access (as opposed to a chain), so single lookups are O(n).

	A symtab can be traversed using callbacks and method symtab::each. The access
	to a symtab is not thread safe, callers must implement thread synchronization

	@todo Sort the table by address and implement binary lookup
*/
class symtab: virtual public object
{
protected:

	/* Protected variables */

	mem_addr_t m_base;							/**< @brief Load base address */

	i8 *m_path;											/**< @brief Objective code file path */

	list<symbol> *m_table;					/**< @brief Function symbol table */

public:

	typedef void (*callback_t)(u32, symbol*);


	/* Constructors, copy constructors and destructor */

	explicit symtab(const i8*, mem_addr_t = 0);

	symtab(const symtab&);

	virtual ~symtab();

	virtual symtab* clone() const;


	/* Accessor methods */

	virtual mem_addr_t base() const;

	virtual const i8* path() const;


	/* Operator overloading methods */

	virtual symtab& operator=(const symtab&);

	virtual const symbol* operator[](mem_addr_t) const;


	/* Generic methods */

	virtual const i8* addr2name(mem_addr_t) const;

	virtual symtab& each(const callback_t) const;

	virtual bool exists(mem_addr_t) const;

	virtual const symbol* lookup(mem_addr_t) const;

	virtual const symbol* lookup(const i8*) const;

	virtual mem_addr_t name2addr(const i8*) const;

	virtual symtab& print(std::ostream& = std::cout) const;

	virtual u32 size() const;
};

}

#endif
