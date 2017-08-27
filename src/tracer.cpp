#include "../include/tracer.hpp"
#include "../include/util.hpp"

/**
	@file src/tracer.cpp

	@brief Class instrument::tracer method implementation
*/

namespace instrument {

/* Static member variable definition */

tracer *tracer::s_iface = NULL;

pthread_mutex_t tracer::s_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/* Link the instrumentation functions with C-style linking */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the beginning of instrumented functions. By implementing this
 *	function (and __cyg_profile_func_exit), libinstrument simulates the call
 *	stack of each thread
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	tracer *iface = tracer::interface();

	__D_ASSERT(this_fn != NULL);
	__D_ASSERT(call_site != NULL);
	__D_ASSERT(iface != NULL);
	if ( unlikely(iface == NULL) ) {
		return;
	}

#ifdef WITH_PLUGIN
	iface->begin_plugins(this_fn, call_site);
#endif

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);
		mem_addr_t site = reinterpret_cast<mem_addr_t> (call_site);

		tracer::lock();

		iface->proc()
				 ->current_thread()
				 ->called(addr, site);

		tracer::unlock();
		return;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	tracer::unlock();
	exit(EXIT_FAILURE);
}


/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the end of instrumented functions. By implementing this function
 *	(and __cyg_profile_func_enter), libinstrument simulates the call stack of
 *	each thread
 *
 * @param[in] this_fn the address of the returning function
 *
 * @param[in] call_site the address that the program counter will return to
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	tracer *iface = tracer::interface();

	__D_ASSERT(iface != NULL);
	if ( unlikely(iface == NULL) ) {
		return;
	}

#ifdef WITH_PLUGIN
	iface->end_plugins(this_fn, call_site);
#endif

	try {
		tracer::lock();

		iface->proc()
				 ->current_thread()
				 ->returned();

		tracer::unlock();
		return;
	}
	catch (exception x) {
		std::cerr << x;
	}
	catch (std::exception x) {
		std::cerr << x;
	}

	tracer::unlock();
	exit(EXIT_FAILURE);
}

#ifdef __cplusplus
}
#endif


/**
 * @brief Library constructor
 *
 * @note If an exception occurs, the process exits
 */
void tracer::__on_lib_load()
{
	/* Initialize libbfd internals and backends */
	bfd_init();

	try {
		s_iface = new tracer;

		/* Load the symbol table of the executable */
		const i8 *path = util::executable_path();
		s_iface->m_proc->add_module(path, 0);
		delete[] path;

		/* Load the symbol tables of the selected DSO */
		chain<string> *libs = util::getenv(g_libs_env);
		dl_iterate_phdr(on_dso_load, libs);
		delete libs;

		util::dbg_info("libinstrument.so.%d.%d initialized", g_major, g_minor);
		return;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	exit(EXIT_FAILURE);
}


/**
 * @brief Library destructor
 */
void tracer::__on_lib_unload()
{
	delete s_iface;
	s_iface = NULL;
	util::dbg_info("libinstrument.so.%d.%d finalized", g_major, g_minor);
}


/**
 * @brief
 *	Given an address in an objective code file, extract from the gdb-related
 *	debug information, the equivalent source code file name and line and append
 *	it to a string buffer
 *
 * @param[in,out] dst the destination string
 *
 * @param[in] path the path of the objective code file (can be NULL for NO-OP)
 *
 * @param[in] addr the address
 *
 * @returns the first argument
 *
 * @note
 *	If the addr2line program fails to retreive the debug information, or if any
 *	other error or exception occurs, nothing is appended to the destination
 *	string
 *
 * @see man addr2line
 * @see man g++ (-g family options)
 */
string& tracer::addr2line(string &dst, const i8 *path, mem_addr_t addr)
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) ) {
		return dst;
	}

	FILE *pipe = NULL;
	try {
		/* Command to be executed by the child process */
		string cmd("addr2line -se %s 0x%x", path, addr);

		/* Open a readonly pipe to the child process */
		pipe = popen(cmd.cstring(), "r");
		if ( unlikely(pipe == NULL) ) {
			throw exception(
				"failed to open pipe for command '%s' (errno %d - %s)",
				cmd.cstring(),
				errno,
				strerror(errno));
		}

		/* Read a line of output from the pipe to a buffer */
		string buf;
		i8 ch = fgetc(pipe);
		while ( likely(ch != '\n' && ch != EOF) ) {
			buf.append(ch);
			ch = fgetc(pipe);
		}

		if ( unlikely(ferror(pipe) != 0) ) {
			throw exception("failed to read pipe for command '%s'", cmd.cstring());
		}

		if ( likely(buf.compare("??:0") != 0) ) {
			dst.append(" (%s)", buf.cstring());
		}
	}
	catch (exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.msg());
	}
	catch (std::exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.what());
	}

	if ( likely(pipe != NULL) ) {
		pclose(pipe);
	}

	return dst;
}


/**
 * @brief
 *	This is a dl_iterate_phdr (libdl) callback, called for each linked shared
 *	object. It loads the symbol table of the DSO (if it's not filtered out) to
 *	tracer::s_iface->m_proc
 *
 * @param[in] dso
 *	a dl_phdr_info struct (libdl) that describes the shared object (file path,
 *	load address e.t.c)
 *
 * @param[in] sz the sizeof dso
 *
 * @param[in] arg
 *	a chain of POSIX extended regular expressions used to select the shared
 *	objects that will participate in the call stack simulation. The absolute
 *	path of each DSO is matched against each regexp. If NULL, all linked DSO
 *	symbol tables will be loaded. If not NULL but empty, all DSO are filtered
 *	out from instrumentation
 *
 * @returns 0
 *
 * @note
 *	If an exception occurs, it's caught and handled. 0 is returned, signaling to
 *	the iterator (dl_iterate_phdr) to continue with the next DSO
 */
i32 tracer::on_dso_load(dl_phdr_info *dso, size_t sz, void *arg)
{
	try {
		if ( unlikely(dso == NULL) ) {
			throw exception("invalid argument: dso (=%p)", dso);
		}

		/* If the DSO path is undefined */
		string path(dso->dlpi_name);
		if ( unlikely(path.length() == 0) ) {
			throw exception("undefined DSO path");
		}

		/* If the DSO has no segments */
		if ( unlikely(dso->dlpi_phnum == 0) ) {
			throw exception("'%s' has 0 segments", path.cstring());
		}

		/* Check if the DSO is filtered out */
		bool found = false;
		if ( likely(arg != NULL) ) {
			const chain<string> *filters = static_cast<chain<string>*> (arg);

			for (u32 i = 0, sz = filters->size(); likely(i < sz); i++) {
				const string *flt = filters->at(i);

				if ( unlikely(path.match(*flt)) ) {
					found = true;
					break;
				}
			}
		}
		else {
			found = true;
		}

		if ( likely(!found) ) {
			util::dbg_warn("filtered out '%s'", path.cstring());
			return 0;
		}

		/* Load the DSO symbol table */
		mem_addr_t base = dso->dlpi_addr + dso->dlpi_phdr[0].p_vaddr;
		s_iface->m_proc
					 ->add_module(path.cstring(), base);
	}
	catch (exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.msg());
	}
	catch (std::exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.what());
	}

	return 0;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
tracer::tracer()
try:
#ifdef WITH_FILTER
m_filters(NULL),
#endif
#ifdef WITH_PLUGIN
m_plugins(NULL),
#endif
m_proc(NULL)
{
#ifdef WITH_FILTER
	m_filters = new list<filter>;
#endif

#ifdef WITH_PLUGIN
	m_plugins = new list<plugin>;
#endif

	m_proc = new process;
}
catch (...) {
	destroy();
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
tracer::tracer(const tracer &src)
try:
#ifdef WITH_FILTER
m_filters(NULL),
#endif
#ifdef WITH_PLUGIN
m_plugins(NULL),
#endif
m_proc(NULL)
{
/* todo Copy if made copyable */
#ifdef WITH_FILTER
	m_filters = new list<filter>;
#endif

#ifdef WITH_PLUGIN
	m_plugins = src.m_plugins->clone();
#endif

	m_proc = src.m_proc->clone();
}
catch (...) {
	destroy();
}


/**
 * @brief Object destructor
 */
tracer::~tracer()
{
	destroy();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline tracer* tracer::clone() const
{
	return new tracer(*this);
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
inline tracer& tracer::operator=(const tracer &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* todo Copy filters if they are made copyable */

#ifdef WITH_PLUGIN
	*m_plugins = *rval.m_plugins;
#endif

	*m_proc = *rval.m_proc;
	return *this;
}


/**
 * @brief Release object resources
 *
 * @returns *this
 */
inline tracer& tracer::destroy()
{
#ifdef WITH_FILTER
	delete m_filters;
	m_filters = NULL;
#endif

#ifdef WITH_PLUGIN
	delete m_plugins;
	m_plugins = NULL;
#endif

	delete m_proc;
	m_proc = NULL;

	return *this;
}


/**
 * @brief Get the interface object
 *
 * @returns tracer::s_iface if the interface object is enabled, NULL otherwise
 */
tracer* tracer::interface()
{
	/* The interface is not yet initialized */
	if ( unlikely(s_iface == NULL) ) {
		return NULL;
	}

	const process *proc = s_iface->m_proc;

	/* The interface process handle is not yet initialized */
	if ( unlikely(proc == NULL) ) {
		return NULL;
	}

	/* No modules are loaded yet */
	else if ( unlikely(proc->module_count() == 0) ) {
		return NULL;
	}

	/* No symbols are loaded yet */
	else if ( unlikely(proc->symbol_count() == 0) ) {
		return NULL;
	}

	return s_iface;
}


/**
 * @brief Lock the access mutex
 *
 * @note Recursive locking is supported
 */
void tracer::lock()
{
	pthread_mutex_lock(&s_lock);
}


/**
 * @brief Unlock the access mutex
 */
void tracer::unlock()
{
	pthread_mutex_unlock(&s_lock);
}


/**
 * @brief Stream insertion operator for instrument::tracer objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, tracer &rval)
{
	try {
		string buf;
		rval.trace(buf);

		util::lock();
		lval << buf;
		util::unlock();

		return lval;
	}
	catch (exception &x) {
		lval << x;
	}
	catch (std::exception &x) {
		lval << x;
	}

	rval.unwind();
	util::unlock();
	return lval;
}


/**
 * @brief Get the process handle
 *
 * @returns this->m_proc
 */
inline process* tracer::proc() const
{
	return m_proc;
}


/**
 * @brief
 *	Create multiple stack traces using the simulated call stack of each thread.
 *	The traces are appended to a string. The stacks are not unwinded
 *
 * @param[in,out] dst the trace destination string
 *
 * @returns *this
 *
 * @throw std::bad_alloc
 * @throw instrument::exception
 */
tracer& tracer::dump(string &dst) const
{
	try {
		tracer::lock();

		for (u32 i = 0, sz = m_proc->thread_count(); likely(i < sz); i++) {
			const thread *thr = m_proc->get_thread(i);
			trace(dst, thr->handle());

			if ( likely(i < sz - 1) ) {
				dst.append("\r\n");
			}
		}

		tracer::unlock();
		return const_cast<tracer&> (*this);
	}
	catch (...) {
		tracer::unlock();
		throw;
	}
}


/**
 * @brief
 *	Create an exception stack trace using the simulated call stack of the
 *	current thread. The trace is appended to a string and the simulated stack is
 *	unwinded
 *
 * @param[in,out] dst the trace destination string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @attention
 *	The simulated call stack is <b>unwinded even if the method fails, in any way
 *	to produce a trace</b>
 *
 * @todo Set the symbol name on first lookup (optimization)
 */
tracer& tracer::trace(string &dst)
{
	/* If an exception occurs, unwind, unlock and rethrow it */
	try {
		tracer::lock();
		thread *thr = m_proc->current_thread();

		const i8 *nm = thr->name();
		if ( likely(nm == NULL) ) {
			nm = "anonymous";
		}

		dst.append("at '%s' thread (0x%lx) {\r\n", nm, thr->handle());

		/* For each function call */
		for (i32 i = thr->lag(); likely(i >= 0); i--) {
			call *cur = const_cast<call*> (thr->backtrace(i));

			const i8 *nm = cur->name();
			if ( likely(nm == NULL) ) {
				nm = m_proc->lookup(cur->addr());
			}

			if ( likely(nm != NULL) ) {
				cur->set_name(nm);
				dst.append("  at %s", nm);
			}
			else {
#ifdef WITH_UNRESOLVED
				dst.append("  at UNRESOLVED");
#endif
			}

			/* Append addr2line debug information */
			u32 prev = i + 1;
			if ( likely (prev < thr->call_depth()) ) {
				const call *caller = thr->backtrace(prev);
				mem_addr_t base = 0;

				const i8 *path = m_proc->inverse_lookup(caller->addr(), base);
				addr2line(dst, path, cur->site() - base);
			}

			dst.append("\r\n");
		}

		dst.append("}\r\n");
		thr->unwind();
		tracer::unlock();

		return *this;
	}
	catch (...) {
		unwind();
		tracer::unlock();
		throw;
	}
}


/**
 * @brief
 *	Create the stack trace of a thread indexed by its ID and append it to a
 *	string
 *
 * @param[in,out] dst the trace destination string
 *
 * @param[in] id the thread ID
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
tracer& tracer::trace(string &dst, pthread_t id) const
{
	/* If an exception occurs, unlock and rethrow it */
	try {
		tracer::lock();
		thread *thr = m_proc->get_thread(id);
		if ( unlikely(thr == NULL) ) {
			tracer::unlock();
			return const_cast<tracer&> (*this);
		}

		const i8 *nm = thr->name();
		if ( likely(nm == NULL) ) {
			nm = "anonymous";
		}

		dst.append("at '%s' thread (0x%lx) {\r\n", nm, thr->handle());

		/* For each function call */
		for (i32 i = thr->call_depth() - 1; likely(i >= 0); i--) {
			call *cur = const_cast<call*> (thr->backtrace(i));

			const i8 *nm = cur->name();
			if ( likely(nm == NULL) ) {
				nm = m_proc->lookup(cur->addr());
			}

			if ( likely(nm != NULL) ) {
				cur->set_name(nm);
				dst.append("  at %s", nm);
			}
			else {
#ifdef WITH_UNRESOLVED
				dst.append("  at UNRESOLVED");
#endif
			}

			/* Append addr2line debug information */
			u32 prev = i + 1;
			if ( likely (prev < thr->call_depth()) ) {
				const call *caller = thr->backtrace(prev);
				mem_addr_t base = 0;

				const i8 *path = m_proc->inverse_lookup(caller->addr(), base);
				addr2line(dst, path, cur->site() - base);
			}

			dst.append("\r\n");
		}

		dst.append("}\r\n");
		tracer::unlock();

		return const_cast<tracer&> (*this);
	}
	catch (...) {
		tracer::unlock();
		throw;
	}
}


/**
 * @brief Unwind the simulated call stack of the current thread
 *
 * @returns *this
 *
 * @throw std::bad_alloc
 *
 * @attention
 *	If an exception trace is not produced, before a new exception occurs, you
 *	must perform an explicit simulated call stack unwinding, to discard the
 *	current exception trace. If you don't properly unwind the simulated stack,
 *	the stored trace will mess with the next attempt to obtain a stack trace.
 *	Nevertheless, if the trace was actually created, a call to unwind doesn't
 *	affect the tracer object state at all (nothing to dispose), so it is not an
 *	error to call it once or even more times even when the trace was produced
 */
tracer& tracer::unwind()
{
	try {
		tracer::lock();

		m_proc->current_thread()
					->unwind();

		tracer::unlock();
		return *this;
	}
	catch (...) {
		tracer::unlock();
		throw;
	}
}


#ifdef WITH_FILTER
/**
 * @brief Register a filter
 *
 * @param[in] expr the filter expression
 *
 * @param[in] icase true to ignore case on filtering, false otherwise
 *
 * @param[in] mode either SYMBOL_FILTER or MODULE_FILTER
 *
 * @returns the new filter
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
filter* tracer::add_filter(const i8 *expr, bool icase, bool mode)
{
	filter *retval = NULL;

	try {
		retval = new filter(expr, icase, mode);
		m_filters->add(retval);
		return retval;
	}
	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Apply all module filters (in the order they were registered)
 *
 * @param[in] path the filtered module path
 *
 * @returns true if any filter matched, false otherwise
 */
bool tracer::apply_module_filters(const i8 *path)
{
	/* todo
				Use:
					mem_addr_t base = 0;
					const i8 *path = proc->ilookup(addr, base);
			to obtain path in caller
	*/
	if ( unlikely(path == NULL) ) {
		return false;
	}

	for (u32 i = 0, sz = filter_count(); likely(i < sz); i++) {
		const filter *f = get_filter(i);

		if ( likely(f->mode() == SYMBOL_FILTER) ) {
			continue;
		}

		if ( unlikely(f->apply(path)) ) {
			return true;
		}
	}

	return false;
}


/**
 * @brief Apply all symbol filters (in the order they were registered) to a name
 *
 * @param[in] nm the filtered name
 *
 * @returns true if any filter matched, false otherwise
 */
bool tracer::apply_symbol_filters(const i8 *nm)
{
	if ( unlikely(nm == NULL) ) {
		return false;
	}

	for (u32 i = 0, sz = filter_count(); likely(i < sz); i++) {
		const filter *f = get_filter(i);

		if ( likely(f->mode() == MODULE_FILTER) ) {
			continue;
		}

		if ( unlikely(f->apply(nm)) ) {
			return true;
		}
	}

	return false;
}


/**
 * @brief Get the number of registered filters
 *
 * @returns this->m_filters->size()
 */
inline u32 tracer::filter_count() const
{
	return m_filters->size();
}


/**
 * @brief Get a registered filter
 *
 * @param[in] i the filter registration index
 *
 * @returns this->m_filters->at(i)
 *
 * @throws instrument::exception
 */
inline filter* tracer::get_filter(u32 i) const
{
	return m_filters->at(i);
}


/**
 * @brief Unregister a filter
 *
 * @param[in] i the filter registration index
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
inline tracer& tracer::remove_filter(u32 i)
{
	m_filters->remove(i);
	return *this;
}
#endif


#ifdef WITH_PLUGIN
/**
 * @brief Register a plugin module (DSO)
 *
 * @param[in] path the path of the module file
 *
 * @param[in] scope
 *	the full scope of the plugin callbacks (NULL for C type linkage and ABI)
 *
 * @returns the new plugin
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
const plugin* tracer::add_plugin(const i8 *path, const i8 *scope)
{
	plugin *retval = NULL;

	try {
		tracer::lock();

		retval = new plugin(path, scope);
		m_plugins->add(retval);

		tracer::unlock();
		return retval;
	}
	catch (...) {
		delete retval;
		tracer::unlock();
		throw;
	}
}


/**
 * @brief Register a plugin
 *
 * @param[in] bgn the instrumentation starting callback
 *
 * @param[in] end the instrumentation ending callback
 *
 * @returns the new plugin
 *
 * @throws std::bad_alloc
 */
const plugin* tracer::add_plugin(modsym_t bgn, modsym_t end)
{
	plugin *retval = NULL;

	try {
		tracer::lock();

		retval = new plugin(bgn, end);
		m_plugins->add(retval);

		tracer::unlock();
		return retval;
	}
	catch (...) {
		delete retval;
		tracer::unlock();
		throw;
	}
}


/**
 * @brief Call all plugin enter functions in the order they were registered
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @returns *this
 */
tracer& tracer::begin_plugins(void *this_fn, void *call_site) const
{
	for (u32 i = 0, sz = plugin_count(); likely(i < sz); i++) {
		try {
			get_plugin(i)->begin(this_fn, call_site);
		}
		catch (exception &x) {
			std::cerr << x;
		}
		catch (std::exception &x) {
			std::cerr << x;
		}
		catch (...) {
			util::header(std::cerr, "x");
			std::cerr << "plugin "
								<< std::dec
								<< i
								<< ": unidentified exception\r\n";
		}
	}

	return const_cast<tracer&> (*this);
}


/**
 * @brief
 *	Call all plugin exit functions in the reverse order they were registered
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @returns *this
 */
tracer& tracer::end_plugins(void *this_fn, void *call_site) const
{
	for (i32 i = plugin_count() - 1; likely(i >= 0); i--) {
		try {
			get_plugin(i)->end(this_fn, call_site);
		}
		catch (exception &x) {
			std::cerr << x;
		}
		catch (std::exception &x) {
			std::cerr << x;
		}
		catch (...) {
			util::header(std::cerr, "x");
			std::cerr << "plugin "
								<< std::dec
								<< i
								<< ": unidentified exception\r\n";
		}
	}

	return const_cast<tracer&> (*this);
}


/**
 * @brief Get a registered plugin module (DSO)
 *
 * @param[in] path the path of the module file (can be NULL)
 *
 * @returns the plugin or NULL if no such plugin module is registered
 */
const plugin* tracer::get_plugin(const i8 *path) const
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) ) {
		return NULL;
	}

	tracer::lock();
	for (u32 i = 0, sz = m_plugins->size(); likely(i < sz); i++) {
		const plugin *plg = m_plugins->at(i);

		/* If this is an inline plugin */
		if ( unlikely(plg->path() == NULL) ) {
			continue;
		}

		if ( unlikely(strcmp(plg->path(), path) == 0) ) {
			tracer::unlock();
			return plg;
		}
	}

	tracer::unlock();
	return NULL;
}


/**
 * @brief Get a registered plugin
 *
 * @param[in] i the plugin registration index
 *
 * @returns this->m_plugins->at(i)
 *
 * @throws instrument::exception
 */
const plugin* tracer::get_plugin(u32 i) const
{
	try {
		tracer::lock();
		const plugin *retval = m_plugins->at(i);
		tracer::unlock();
		return retval;
	}
	catch (...) {
		tracer::unlock();
		throw;
	}
}


/**
 * @brief Get the number of registered plugins
 *
 * @returns this->m_plugins->size()
 */
inline u32 tracer::plugin_count() const
{
	return m_plugins->size();
}


/**
 * @brief Unregister all plugins
 *
 * @param[in] which plugin selector (one of ALL, DSO, INLINE)
 *
 * @returns *this
 */
tracer& tracer::remove_all_plugins(u32 which)
{
	tracer::lock();

	if ( likely(which == ALL) ) {
		m_plugins->clear();
		tracer::unlock();
		return *this;
	}

	for (u32 i = 0, sz = m_plugins->size(); likely(i < sz); i++) {
		const plugin *plg = m_plugins->at(i);

		if ( unlikely(plg->path() == NULL) ) {
			if ( unlikely(which == DSO) ) {
				continue;
			}
		}
		else if ( unlikely(which == INLINED) ) {
			continue;
		}

		m_plugins->remove(i--);
		sz--;
	}

	tracer::unlock();
	return *this;
}


/**
 * @brief Unregister a plugin module (DSO)
 *
 * @param[in] path the path of the module file (can be NULL)
 *
 * @returns *this
 */
tracer& tracer::remove_plugin(const i8 *path)
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) ) {
		return *this;
	}

	tracer::lock();
	for (u32 i = 0, sz = m_plugins->size(); likely(i < sz); i++) {
		const plugin *plg = m_plugins->at(i);

		/* If this is an inline plugin */
		if ( unlikely(plg->path() == NULL) ) {
			continue;
		}

		if ( unlikely(strcmp(plg->path(), path) == 0) ) {
			m_plugins->remove(i);
			break;
		}
	}

	tracer::unlock();
	return *this;
}


/**
 * @brief Unregister a plugin
 *
 * @param[in] i the plugin registration index
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
tracer& tracer::remove_plugin(u32 i)
{
	try {
		tracer::lock();
		m_plugins->remove(i);
		tracer::unlock();
		return *this;
	}
	catch (...) {
		tracer::unlock();
		throw;
	}
}
#endif

}
