#include "../include/tracer.hpp"

/**
	@file src/process.cpp

	@brief Class instrument::process method implementation
*/

namespace instrument {

/**
 * @brief Return the currently running process
 *
 * @returns tracer::interface()->proc()
 *
 * @throws instrument::exception
 */
process* process::current()
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		throw exception("Tracer interface not initialized");
	}

	process *retval = iface->proc();
	if ( unlikely(retval == NULL) ) {
		throw exception("No running process handle initialized");
	}

	return retval;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
process::process()
try:
m_lock(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP),
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
m_lock(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP),
m_pid(src.m_pid),
m_symtabs(NULL),
m_threads(NULL)
{
	src.lock();
	m_symtabs = src.m_symtabs->clone();
	m_threads = src.m_threads->clone();
	src.unlock();
}
catch (...) {
	src.unlock();
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

	unlock();
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
	rval.lock();
	lock();

	if ( unlikely(this == &rval) ) {
		rval.unlock();
		return unlock();
	}

	m_pid = rval.m_pid;
	*m_symtabs = *rval.m_symtabs;
	*m_threads = *rval.m_threads;

	rval.unlock();
	return unlock();
}


/**
 * @brief Obtain object access
 *
 * @returns *this
 */
inline process& process::lock() const
{
	pthread_mutex_lock(const_cast<pthread_mutex_t*> (&m_lock));
	return const_cast<process&> (*this);
}


/**
 * @brief Yield object access
 *
 * @returns *this
 */
inline process& process::unlock() const
{
	pthread_mutex_unlock(const_cast<pthread_mutex_t*> (&m_lock));
	return const_cast<process&> (*this);
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
	lock();

	symtab *table = NULL;
	try {
		table = new symtab(path, base);
		m_symtabs->add(table);
		return unlock();
	}
	catch (...) {
		unlock();
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
 */
const i8* process::inverse_lookup(mem_addr_t addr, mem_addr_t &base) const
{
	lock();

	for (u32 i = 0, sz = m_symtabs->size(); likely(i < sz); i++) {
		const symtab *table = m_symtabs->at(i);

		if ( unlikely(table->exists(addr)) ) {
			base = table->base();
			const i8 *retval = table->path();
			unlock();
			return retval;
		}
	}

	unlock();
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
	lock();
	for (u32 i = 0, sz = m_symtabs->size(); likely(i < sz); i++) {
		const i8* retval =
			m_symtabs	->at(i)
							 	->addr2name(addr);

		if ( unlikely(retval != NULL) ) {
			unlock();
			return retval;
		}
	}

	/* The address was not resolved */
	unlock();
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
	lock();

	u32 cnt = 0;
	for (i32 i = m_symtabs->size() - 1; likely(i >= 0); i--) {
		cnt +=
			m_symtabs	->at(i)
							 	->size();
	}

	unlock();
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
	lock();

	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		const thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(id)) ) {
			m_threads->remove(i);
			break;
		}
	}

	return unlock();
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
	lock();

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

	return unlock();
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
 *	its first <b>instrumented</b> function, unless thread::fork is used for its
 *	creation
 */
thread* process::current_thread() const
{
	lock();

	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is_current()) ) {
			unlock();
			return thr;
		}
	}

	thread *retval = NULL;
	try {
		retval = new thread;
		m_threads->add(retval);
		unlock();
		return retval;
	}

	catch (...) {
		unlock();
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
	lock();

	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(id)) ) {
			unlock();
			return thr;
		}
	}

	unlock();
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

	lock();

	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is(nm)) ) {
			unlock();
			return thr;
		}
	}

	unlock();
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
thread* process::get_thread(u32 i) const
{
	lock();

	try {
		thread *retval = m_threads->at(i);
		unlock();
		return retval;
	}
	catch (...) {
		unlock();
		throw;
	}
}


/**
 * @brief Register a thread to the process
 *
 * @param[in] t the registered thread
 *
 * @returns *this;
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
process& process::register_thread(thread *t)
{
	lock();

	if ( unlikely(get_thread(t->handle()) != NULL) ) {
		unlock();
		throw new exception(	"Process %d already has thread 0x%x registered",
													m_pid,
													t->handle());
	}

	try {
		m_threads->add(t);
		return unlock();
	}
	catch (...) {
		unlock();
		throw;
	}
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
