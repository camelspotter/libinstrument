#include "../include/process.hpp"

/**
	@file src/process.cpp

	@brief Class instrument::process method implementation
*/

namespace instrument {

/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
process::process()
try:
m_pid(getpid()),
m_symtabs(NULL),
m_threads(NULL)
{
	m_symtabs = new list<symtab>;
	m_threads = new list<thread>;
}
catch (...) {
	delete m_symtabs;
	m_symtabs = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
process::process(const process &src)
try:
m_pid(src.m_pid),
m_symtabs(NULL),
m_threads(NULL)
{
	m_symtabs = src.m_symtabs->clone();
	m_threads = src.m_threads->clone();
}
catch (...) {
	delete m_symtabs;
	m_symtabs = NULL;
}


/**
 * @brief Object destructor
 */
process::~process()
{
	delete m_symtabs;
	delete m_threads;
	m_symtabs = NULL;
	m_threads = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline process* process::clone() const
{
	return new process(*this);
}


/**
 * @brief Get the process ID
 *
 * @returns this->m_pid
 */
inline pid_t process::pid() const
{
	return m_pid;
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
process& process::operator=(const process &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	m_pid = rval.m_pid;
	*m_symtabs = *rval.m_symtabs;
	*m_threads = *rval.m_threads;

	return *this;
}


/**
 * @brief Spawn a new, instrumented and named thread
 *
 * @param[in] nm the new thread's name
 *
 * @param[in] entry the new thread's entry point
 *
 * @param[in] arg the new thread's entry point single argument (can be NULL)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @todo Return the newly created thread
 */
process& process::fork_thread(const i8 *nm,
															 thread_main_t entry,
															 thread_arg_t arg)
{
	__D_ASSERT(arg != NULL);

	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	if ( unlikely(entry == NULL) ) {
		throw exception("invalid argument: entry (=%p)", entry);
	}

	pthread_t id;
	i32 retval = pthread_create(&id, NULL, entry, arg);
	if ( unlikely(retval != 0) ) {
		throw exception(
			"failed to create thread '%s' (errno %d - %s)",
			nm,
			errno,
			strerror(errno));
	}

	thread *t = NULL;
	try {
		t = new thread(id, nm);
		m_threads->add(t);
		return *this;
	}
	catch (...) {
		delete t;
		throw;
	}
}


/**
 * @brief Cancel the execution of a thread
 *
 * @param[in] t the thread (can be NULL for NO-OP)
 *
 * @returns *this
 *
 * @note Failure in thread cancellation is silently ignored
 */
process& process::thread_cancel(thread *t)
{
	__D_ASSERT(t != NULL);
	if ( likely(t == NULL) ) {
		return *this;
	}

	pthread_t id = t->handle();
	pthread_cancel(id);

	return cleanup_thread(id);
}


/**
 * @brief Join a thread and optionally obtain its exit status
 *
 * @param[in] t the thread (can be NULL for NO-OP)
 *
 * @param[out] exit_status
 *	a pointer to obtain the joined thread's exit status (ignored if NULL). Based
 *	on the specific thread implementation it can be heap allocated
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
process& process::thread_join(thread *t, void *exit_status)
{
	__D_ASSERT(t != NULL);
	if ( unlikely(t == NULL) ) {
		return *this;
	}

	pthread_t id = t->handle();
	i32 retval;

	if ( likely(exit_status == NULL) ) {
		retval = pthread_join(id, NULL);
	}
	else {
		retval = pthread_join(id, &exit_status);
	}

	if ( unlikely(retval != 0) ) {
		const i8 *nm = t->name();
		if ( likely(nm == NULL) ) {
			nm = "anonymous";
		}

		throw exception(
			"failed to join thread '%s' (errno %d - %s)",
			nm,
			errno,
			strerror(errno));
	}

	return cleanup_thread(id);
}


/**
 * @brief
 *	Add a symbol table to the namespace. The symbol table is loaded from a non
 *	stripped objective code file (executable or DSO library)
 *
 * @param[in] path the path of the objective code file
 *
 * @param[in] base the load base address
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
process& process::add_module(const i8 *path, mem_addr_t base)
{
	symtab *table = NULL;
	try {
		table = new symtab(path, base);
		m_symtabs->add(table);
		return *this;
	}
	catch (...) {
		delete table;
		throw;
	}
}


/**
 * @brief
 *	Inverse lookup. Find the module (executable or DSO library) that defines a
 *	symbol and return its path and load base address
 *
 * @param[in] addr the symbol address
 *
 * @param[out] base the load base address of the module
 *
 * @returns the path of the module or NULL if the address is unresolved
 *
 * @see tracer::addr2line
 */
const i8* process::ilookup(mem_addr_t addr, mem_addr_t &base) const
{
	for (u32 i = 0, sz = m_symtabs->size(); likely(i < sz); i++) {
		const symtab *table = m_symtabs->at(i);

		if ( unlikely(table->exists(addr)) ) {
			base = table->base();
			return table->path();
		}
	}

	base = 0;
	return NULL;
}


/**
 * @brief Lookup an address to resolve a symbol name
 *
 * @param[in] addr the address
 *
 * @returns the demangled symbol name or NULL if the address is unresolved
 *
 * @note
 *	If demangling failed upon symbol table loading/parsing the decorated symbol
 *	is returned
 */
const i8* process::lookup(mem_addr_t addr) const
{
	for (u32 i = 0, sz = m_symtabs->size(); likely(i < sz); i++) {
		const i8* retval =
			m_symtabs->at(i)
							 ->addr2name(addr);

		if ( unlikely(retval != NULL) ) {
			return retval;
		}
	}

	/* The address was not resolved */
	return NULL;
}


/**
 * @brief Get the number of modules
 *
 * @returns this->m_symtabs->size()
 */
inline u32 process::module_count() const
{
	return m_symtabs->size();
}


/**
 * @brief Get the number of symbols
 *
 * @returns the sum of the loaded symbol table sizes
 */
u32 process::symbol_count() const
{
	u32 cnt = 0;
	for (i32 i = m_symtabs->size() - 1; likely(i >= 0); i--) {
		cnt +=
			m_symtabs->at(i)
							 ->size();
	}

	return cnt;
}


/**
 * @brief Cleanup libinstrument-related thread resources upon thread exit
 *
 * @param[in] id the thread ID
 *
 * @returns *this
 *
 * @attention
 *	The library detects when a thread is exiting and will call this method to
 *	do the appropriate cleanup. Neverthelss there can be cases when the thread
 *	exits abnormally (e.g cancelled without calling process::thread_cancel), so
 *	as a good practise, it should be called from thread cancellation handlers to
 *	release resources. If you don't cleanup the thread handle though it becomes
 *	useless when the actual thread has exited, it continues to occupy memory and
 *	will also inject junk, empty traces in dumps or in explicit trace requests
 *
 * @see man pthread_cleanup_push, pthread_cleanup_pop
 */
process& process::cleanup_thread(pthread_t id)
{
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		const thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(id)) ) {
			m_threads->remove(i);
			break;
		}
	}

	return *this;
}


/**
 * @brief Cleanup all libinstrument-related zombie thread resources
 *
 * @returns *this
 *
 * @see instrument::thread::m_status and related macros
 */
process& process::cleanup_zombie_threads()
{
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		const thread *thr = m_threads->at(i);

		if ( likely(thr->call_depth() > 0) ) {
			continue;
		}

		thread_status_t status = thr->status();
		if ( unlikely(is_thread_started(status) || is_thread_finished(status)) ) {
			m_threads->remove(i--);
			sz--;
		}
	}

	return *this;
}


/**
 * @brief Get the currently executing thread
 *
 * @returns the instrument::thread object that tracks the actual current thread
 *
 * @throws std::bad_alloc
 *
 * @note
 *	When an actual thread is created the m_threads chain is populated with an
 *	entry for the equivalent instrument::thread object when the thread executes
 *	its first <b>instrumented</b> function, unless process::fork_thread is used
 *	for its creation
 */
thread* process::current_thread() const
{
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is_current()) ) {
			return thr;
		}
	}

	thread *retval = NULL;
	try {
		retval = new thread;
		m_threads->add(retval);
		return retval;
	}

	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Get a thread by (pthread) ID
 *
 * @param[in] id the thread ID
 *
 * @returns
 *	the instrument::thread object that tracks the actual thread (with the given
 *	ID) or NULL if no such thread is found
 */
thread* process::get_thread(pthread_t id) const
{
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(id)) ) {
			return thr;
		}
	}

	return NULL;
}


/**
 * @brief Get a thread by name
 *
 * @param[in] nm the name (can be NULL)
 *
 * @returns
 *	the instrument::thread object that tracks the actual thread (with the given
 *	name) or NULL if no such thread is found
 */
thread* process::get_thread(const i8 *nm) const
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) ) {
		return NULL;
	}

	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(nm)) ) {
			return thr;
		}
	}

	return NULL;
}


/**
 * @brief Get a thread by its offset in the active thread enumerator
 *
 * @param[in] i the offset
 *
 * @returns this->m_threads->at(i)
 *
 * @throws instrument::exception
 */
inline thread* process::get_thread(u32 i) const
{
	return m_threads->at(i);
}


/**
 * @brief Get the active thread count
 *
 * @returns this->m_threads->size()
 */
inline u32 process::thread_count() const
{
	return m_threads->size();
}

}
