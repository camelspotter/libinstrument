#include "../include/plugin.hpp"
#include "../include/string.hpp"
#include "../include/util.hpp"

/**
	@file src/plugin.cpp

	@brief Class instrument::plugin method implementation
*/

namespace instrument {

/**
 * @brief Object deconstruction
 *
 * @returns *this
 */
plugin& plugin::destroy()
{
	if ( likely(m_handle != NULL) ) {
		dlclose(m_handle);
	}

	delete[] m_path;
	m_path = NULL;
	m_handle = NULL;
	m_begin = m_end = NULL;

	return *this;
}


/**
 * @brief Object constructor
 *
 * @param[in] path the path of the module file
 *
 * @param[in] scope
 *	the full scope of the callbacks (namespace and/or class). Pass NULL for <b>C
 *	type linkage and ABI</b>
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
plugin::plugin(const i8 *path, const i8 *scope)
try:
m_begin(NULL),
m_end(NULL),
m_handle(NULL),
m_path(NULL)
{
	if ( unlikely(path == NULL) ) {
		throw exception("invalid argument: path (=%p)", path);
	}

	m_path = new i8[strlen(path) + 1];
	strcpy(m_path, path);

	/* Check if the module is already loaded */
	u32 flags = RTLD_LOCAL | RTLD_LAZY;
	m_handle = dlopen(m_path, flags | RTLD_NOLOAD);

	/* If the module is not yet loaded, load it explicitly */
	if ( likely(m_handle == NULL) ) {
		dlerror();
		m_handle = dlopen(m_path, flags);
		if ( unlikely(m_handle == NULL) ) {
			throw exception("failed to load plugin '%s' (%s)", m_path, dlerror());
		}

		util::dbg_info("plugin '%s' loaded/linked", m_path);
	}
#if DBG_LEVEL & DBGL_INFO
	else {
		util::dbg_info("plugin '%s' linked", m_path);
	}
#endif

	/* Resolve the instrumentation functions */
	m_begin = resolve("mod_enter", scope);
	m_end = resolve("mod_exit", scope);
}
catch (...) {
	destroy();
}


/**
 * @brief Object constructor
 *
 * @param[in] bgn instrumentation starting function
 *
 * @param[in] end instrumentation ending function
 */
plugin::plugin(modsym_t bgn, modsym_t end):
m_begin(bgn),
m_end(end),
m_handle(NULL),
m_path(NULL)
{
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
plugin::plugin(const plugin &src)
try:
m_begin(NULL),
m_end(NULL),
m_handle(NULL),
m_path(NULL)
{
	*this = src;
}
catch (...) {
	destroy();
}


/**
 * @brief Object destructor
 */
plugin::~plugin()
{
#if DBG_LEVEL & DBGL_INFO
	if ( likely(m_handle != NULL && m_path != NULL) ) {
 		util::dbg_info("plugin '%s' unlinked", m_path);
 	}
#endif

	destroy();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline plugin* plugin::clone() const
{
	return new plugin(*this);
}


/**
 * @brief Get the module file path
 *
 * @returns this->m_path
 */
inline const i8* plugin::path() const
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
 * @throws instrument::exception
 */
plugin& plugin::operator=(const plugin &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	m_begin = rval.m_begin;
	m_end = rval.m_end;

	/* If this object has called dlopen and holds a handle, close it */
	if ( likely(m_handle != NULL) ) {
		dlclose(m_handle);
		util::dbg_info("plugin '%s' unlinked", m_path);

		delete[] m_path;
		m_path = NULL;
		m_handle = NULL;
	}

	if ( unlikely(rval.m_handle == NULL) ) {
		return *this;
	}

	m_path = new i8[strlen(rval.m_path) + 1];
	strcpy(m_path, rval.m_path);

	/* Obtain a handle to an already loaded module */
	dlerror();
	m_handle = dlopen(m_path, RTLD_LOCAL | RTLD_LAZY | RTLD_NOLOAD);
	if ( unlikely(m_handle == NULL) ) {
		throw exception("failed to link plugin '%s' (%s)", m_path, dlerror());
	}

	util::dbg_info("plugin '%s' linked", m_path);
	return *this;
}


/**
 * @brief Begin instrumenting a function
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @returns *this
 */
inline plugin& plugin::begin(void *this_fn, void *call_site) const
{
	__D_ASSERT(m_begin != NULL);
	if ( likely(m_begin != NULL) ) {
		m_begin(this_fn, call_site);
	}

	return const_cast<plugin&> (*this);
}


/**
 * @brief End a function instrumentation
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address that the program counter will return to
 *
 * @returns *this
 */
inline plugin& plugin::end(void *this_fn, void *call_site) const
{
	__D_ASSERT(m_end != NULL);
	if ( likely(m_end != NULL) ) {
		m_end(this_fn, call_site);
	}

	return const_cast<plugin&> (*this);
}


/**
 * @brief Resolve a module symbol
 *
 * @param[in] nm the symbol name
 *
 * @param[in] scope
 *	the full scope of the symbol (namespace and/or class). Pass NULL for <b>C
 *	type linkage and ABI</b>
 *
 * @returns the symbol address
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @note
 *	The modsym_t type is a pointer to a function that takes two void* arguments
 *	and returns void. All module entry points should conform to this signature
 *	in order to be callable from a plugin object
 */
modsym_t plugin::resolve(const i8 *nm, const i8 *scope) const
{
	if ( unlikely(m_handle == NULL) ) {
		throw exception("no selected module, this is an inline plugin");
	}

	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	string *mangled = NULL;
	if ( likely(scope == NULL) ) {
		mangled = new string(nm);
	}

	/* Mangle the symbol (g++ ABI mangling) */
	else {
		string tmp(scope);
		chain<string> *parts = NULL;

		try {
			mangled = new string("_ZN");
			parts = tmp.split("::");

			for (u32 i = 0, sz = parts->size(); likely(i < sz); i++) {
				const string *token = parts->at(i);
				mangled->append("%d%s", token->length(), token->cstring());
			}

			mangled->append("%d%s", strlen(nm), nm);
			mangled->append("EPvS%d_", parts->size() - 1);

			delete parts;
			parts = NULL;
		}
		catch (...) {
			delete parts;
			delete mangled;
			throw;
		}
	}

	/* Resolve the symbol address */
	dlerror();
	void *sym = dlsym(m_handle, mangled->cstring());
	modsym_t retval = reinterpret_cast<modsym_t> (sym);

	i8 *err = dlerror();
	if ( unlikely(err != NULL) ) {
		exception x(
			"failed to resolve symbol %s in object '%s' (%s)",
			mangled->cstring(),
			m_path,
			err
		);

		delete mangled;
		throw x;
	}

	delete mangled;
	return retval;
}

}
