#include "../include/symtab.hpp"
#include "../include/util.hpp"

/**
	@file src/symtab.cpp

	@brief Class instrument::symtab method implementation
*/

namespace instrument {

/**
 * @brief Object constructor
 *
 * @param[in] path the path of the objective code file
 *
 * @param[in] base the load base address
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
symtab::symtab(const i8 *path, mem_addr_t base):
m_base(base),
m_path(NULL),
m_table(NULL)
{
	if ( unlikely(path == NULL) ) {
		throw exception("invalid argument: path (=%p)", path);
	}

	m_path = new i8[strlen(path) + 1];
	strcpy(m_path, path);

	bfd *fd = NULL;
	i8 *nm = NULL;
	symbol *sym = NULL;
	asymbol **tbl = NULL;

	/* If an exception occurs, release resources and rethrow it */
	try {
		/* Open the binary file and obtain a descriptor (the bfd) */
		fd = bfd_openr(m_path, NULL);
		if ( unlikely(fd == NULL) ) {
			bfd_error bfd_errno = bfd_get_error();
			throw exception(
				"failed to open file '%s' (bfd errno %d - %s)",
				m_path,
				bfd_errno,
				bfd_errmsg(bfd_errno));
		}

		/* Verify that the file contains objective code */
		if ( unlikely(!bfd_check_format(fd, bfd_object)) ) {
			bfd_error bfd_errno = bfd_get_error();
			throw exception(
				"failed to verify file '%s' (bfd errno %d - %s)",
				m_path,
				bfd_errno,
				bfd_errmsg(bfd_errno));
		}

		/* Get the symbol table storage size */
		i32 sz = bfd_get_symtab_upper_bound(fd);
		if ( unlikely(sz == 0) ) {
			throw exception("file '%s' is stripped", m_path);
		}
		else if ( unlikely(sz < 0) ) {
			bfd_error bfd_errno = bfd_get_error();
			throw exception(
				"failed to parse file '%s' (bfd errno %d - %s)",
				m_path,
				bfd_errno,
				bfd_errmsg(bfd_errno));
		}

		/* Canonicalize the symbol table and get the symbol count */
		tbl = new asymbol*[sz];
		i32 cnt = bfd_canonicalize_symtab(fd, tbl);
		if ( unlikely(cnt == 0) ) {
			throw exception("file '%s' is stripped", m_path);
		}
		else if ( unlikely(cnt < 0) ) {
			bfd_error bfd_errno = bfd_get_error();
			throw exception(
				"failed to canonicalize the symbol table of '%s' (bfd errno %d - %s)",
				m_path,
				bfd_errno,
				bfd_errmsg(bfd_errno));
		}

		/* Traverse the symbol table, discard non function symbols */
		m_table = new list<symbol>(cnt, true);
		for (i32 i = 0; likely(i < cnt); i++) {
			const asymbol *cur = tbl[i];

			/* If the entry is not in a code section */
			if ( likely((cur->section->flags & SEC_CODE) == 0) ) {
				continue;
			}

			/* If the entry is not a function symbol */
			if ( likely((cur->flags & BSF_FUNCTION) == 0) ) {
				continue;
			}

			/*
			 * A symbol runtime address is the load address, plus the section virtual
			 * memory address, plus the offset from the section base
			 */
			mem_addr_t addr = m_base;
			addr += bfd_get_section_vma(fd, cur->section);
			addr += cur->value;

			/* Demangle and store the symbol */
			nm = abi::__cxa_demangle(cur->name, NULL, NULL, NULL);
			if ( likely(nm != NULL) ) {
				sym = new symbol(addr, nm);
				delete[] nm;
				nm = NULL;
			}

			/* If demangling failed the decorated name is used */
			else {
				sym = new symbol(addr, cur->name);
			}

			m_table->add(sym);
			sym = NULL;
		}

		delete[] tbl;
		bfd_close(fd);

#if DBG_LEVEL & DBGL_INFO
		util::dbg_info("loaded the symbol table of '%s'", m_path);
		util::dbg_info("  base address @ %p", m_base);
		util::dbg_info("  number of symbols: %d", cnt);
		util::dbg_info("  number of function symbols: %d", m_table->size());

#if WITH_SYMBOL_ENUMERATION
		print();
#endif

#endif
	}
	catch (...) {
		delete[] m_path;
		delete[] tbl;
		delete[] nm;

		delete m_table;
		delete sym;

		m_path = NULL;
		m_table = NULL;

		if ( likely(fd != NULL) ) {
			bfd_close(fd);
		}

		throw;
	}
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
symtab::symtab(const symtab &src)
try:
m_base(src.m_base),
m_path(NULL),
m_table(NULL)
{
	m_table = src.m_table->clone();
	m_path = new i8[strlen(src.m_path) + 1];
	strcpy(m_path, src.m_path);
}
catch (...) {
	delete m_table;
	m_table = NULL;
}


/**
 * @brief Object destructor
 */
symtab::~symtab()
{
	delete[] m_path;
	delete m_table;
	m_path = NULL;
	m_table = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline symtab* symtab::clone() const
{
	return new symtab(*this);
}


/**
 * @brief Get the load base address
 *
 * @returns this->m_base
 */
inline mem_addr_t symtab::base() const
{
	return m_base;
}


/**
 * @brief Get the objective code file path
 *
 * @returns this->m_path
 */
inline const i8* symtab::path() const
{
	return m_path;
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
symtab& symtab::operator=(const symtab &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	u32 len = strlen(rval.m_path);
	if (len > strlen(m_path)) {
		delete[] m_path;
		m_path = NULL;
		m_path = new i8[len + 1];
	}

	strcpy(m_path, rval.m_path);
	m_base = rval.m_base;
	*m_table = *rval.m_table;

	return *this;
}


/**
 * @brief Subscript operator, symbol lookup by address
 *
 * @param[in] addr the address to lookup
 *
 * @returns the symbol@addr or NULL if the address is unresolved
 */
inline const symbol* symtab::operator[](mem_addr_t addr) const
{
	return lookup(addr);
}


/**
 * @brief Lookup an address to resolve a symbol name
 *
 * @param[in] addr the address
 *
 * @returns the symbol name or NULL if the address is unresolved
 *
 * @note
 *	If demangling failed upon symbol table loading/parsing the decorated symbol
 *	name is returned
 */
inline const i8* symtab::addr2name(mem_addr_t addr) const
{
	const symbol *sym = lookup(addr);
	if ( likely(sym != NULL) ) {
		return sym->name();
	}

	return NULL;
}


/**
 * @brief Traverse the symbol table with a callback for each symbol
 *
 * @param[in] pfunc the callback (can be NULL)
 *
 * @returns *this
 */
inline symtab& symtab::each(const callback_t pfunc) const
{
	m_table->each(pfunc);
	return const_cast<symtab&> (*this);
}


/**
 * @brief Probe if a symbol exists
 *
 * @param[in] addr the symbol address
 *
 * @returns true if the symbol exists, false otherwise
 */
inline bool symtab::exists(mem_addr_t addr) const
{
	return lookup(addr) != NULL;
}


/**
 * @brief Lookup an address to resolve a symbol
 *
 * @param[in] addr the address
 *
 * @returns the symbol or NULL if the address is unresolved
 */
const symbol* symtab::lookup(mem_addr_t addr) const
{
	for (u32 i = 0, sz = m_table->size(); likely(i < sz); i++) {
		const symbol *sym = m_table->at(i);
		if ( unlikely(sym->addr() == addr) ) {
			return sym;
		}
	}

	/* The address was not resolved */
	return NULL;
}


/**
 * @brief Lookup a name to resolve a symbol
 *
 * @param[in] nm the symbol name (needs exact match)
 *
 * @returns the symbol or NULL if the name was not found
 */
const symbol* symtab::lookup(const i8 *nm) const
{
	for (u32 i = 0, sz = m_table->size(); likely(i < sz); i++) {
		const symbol *sym = m_table->at(i);
		if ( unlikely(strcmp(sym->name(), nm) == 0) ) {
			return sym;
		}
	}

	return NULL;
}


/**
 * @brief Lookup a name to resolve a symbol address
 *
 * @param[in] nm the symbol name (needs exact match)
 *
 * @returns the symbol address or -1 if the name is not found
 */
inline mem_addr_t symtab::name2addr(const i8 *nm) const
{
	const symbol *sym = lookup(nm);
	if ( likely(sym != NULL) ) {
		return sym->addr();
	}

	return -1;
}


/**
 * @brief Enumerate all symbols on an output stream
 *
 * @param[in] out the output stream (std::cout by default)
 *
 * @returns *this
 */
symtab& symtab::print(std::ostream &out) const
{
	out << "Symbol enumeration of '"
			<< m_path
			<< "' ("
			<< size()
			<< " symbols @"
			<< std::hex
			<< m_base
			<< ")\n";

	for (u32 i = 0, sz = m_table->size(); likely(i < sz); i++) {
		const symbol *sym = m_table->at(i);

		out << "  Symbol @"
				<< sym->addr()
				<< ": "
				<< sym->name()
				<< "\n";
	}

	return const_cast<symtab&> (*this);
}


/**
 * @brief Get the number of symbols
 *
 * @returns this->m_table->size()
 */
inline u32 symtab::size() const
{
	return m_table->size();
}

}
