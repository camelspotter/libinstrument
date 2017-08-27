#include "../include/tracer.hpp"

/**
	@file src/thread.cpp

	@brief Class instrument::thread method implementation
*/

namespace instrument {

/**
 * @brief Attach (register) this thread to the running process
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline thread& thread::attach_to_process()
{
	process::current()->register_thread(this);
	return *this;
}


/**
 * @brief Detach (remove) this thread from the running process
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
inline thread& thread::detach_from_process()
{
	process::current()->cleanup_thread(m_handle);
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
 * @returns the new thread
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
thread* thread::fork(	const i8 *nm, thread_main_t entry, thread_arg_t arg)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	if ( unlikely(entry == NULL) ) {
		throw exception("invalid argument: entry (=%p)", entry);
	}

	pthread_t id;
	i32 check = pthread_create(&id, NULL, entry, arg);
	if ( unlikely(check != 0) ) {
		throw exception(
			"failed to create thread '%s' (errno %d - %s)",
			nm,
			errno,
			strerror(errno));
	}

	thread *retval = NULL;
	try {
		retval = new thread(id, nm);
		retval->attach_to_process();
		return retval;
	}
	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Object constructor
 *
 * @param[in] nm the thread name (it can be NULL)
 *
 * @throws std::bad_alloc
 */
thread::thread(const i8 *nm)
try:
m_handle(pthread_self()),
m_lag(0),
m_name(NULL),
m_stack(NULL),
m_status(THREAD_INIT)
{
	if ( unlikely(nm != NULL) ) {
		m_name = new i8[strlen(nm) + 1];
		strcpy(m_name, nm);
	}

	m_stack = new stack<call>;
}
catch (...) {
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object constructor
 *
 * @param[in] id the real (pthread) ID
 *
 * @param[in] nm the thread name
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
thread::thread(pthread_t id, const i8 *nm)
try:
m_handle(id),
m_lag(0),
m_name(NULL),
m_stack(NULL),
m_status(THREAD_INIT)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	m_name = new i8[strlen(nm) + 1];
	strcpy(m_name, nm);
	m_stack = new stack<call>;
}
catch (...) {
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
thread::thread(const thread &src)
try:
m_handle(src.m_handle),
m_lag(src.m_lag),
m_name(NULL),
m_stack(NULL),
m_status(src.m_status)
{
	const i8 *nm = src.m_name;
	if ( unlikely(nm != NULL) ) {
		m_name = new i8[strlen(nm) + 1];
		strcpy(m_name, nm);
	}

	m_stack = src.m_stack->clone();
}

catch (...) {
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object destructor
 */
thread::~thread()
{
	delete[] m_name;
	delete m_stack;
	m_name = NULL;
	m_stack = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline thread* thread::clone() const
{
	return new thread(*this);
}


/**
 * @brief Get the thread handle
 *
 * @returns this->m_handle
 */
inline pthread_t thread::handle() const
{
	return m_handle;
}


/**
 * @brief
 *	Get the size (call depth) of the simulated call stack with respect to the
 *	real call stack, the number of calls that must be popped off the simulated
 *	stack to match the real one
 *
 * @returns this->m_lag
 */
inline i32 thread::lag() const
{
	return m_lag;
}


/**
 * @brief Get the thread name
 *
 * @returns this->m_name
 */
inline const i8* thread::name() const
{
	return m_name;
}


/**
 * @brief Set the thread name
 *
 * @param[in] nm the new name (NULL to unset)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
thread& thread::set_name(const i8 *nm)
{
	if ( unlikely(nm == NULL) ) {
		delete[] m_name;
		m_name = NULL;
		return *this;
	}

	u32 len = strlen(nm);
	if ( likely(m_name == NULL || len > strlen(m_name)) ) {
		delete[] m_name;
		m_name = NULL;
		m_name = new i8[len + 1];
	}

	strcpy(m_name, nm);
	return *this;
}


/**
 * @brief Get the thread status
 *
 * @returns this->m_status
 */
inline thread_status_t thread::status() const
{
	return m_status;
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
thread& thread::operator=(const thread &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	*m_stack = *rval.m_stack;
	m_handle = rval.m_handle;
	m_lag = rval.m_lag;
	m_status = rval.m_status;

	return set_name(rval.m_name);
}


/**
 * @brief Subscript operator
 *
 * @param[in] i the backtrace offset
 *
 * @returns the i-th function call
 *
 * @throws instrument::exception
 */
inline const call* thread::operator[](u32 i) const
{
	return backtrace(i);
}


/**
 * @brief Peek at the simulated call stack
 *
 * @param[in] i the offset
 *
 * @returns the i-th function call
 *
 * @throws instrument::exception
 */
inline const call* thread::backtrace(u32 i) const
{
	return m_stack->peek(i);
}


/**
 * @brief Get the size (call depth) of the simulated call stack
 *
 * @returns this->m_stack->size()
 */
inline u32 thread::call_depth() const
{
	return m_stack->size();
}


/**
 * @brief Simulate a function call
 *
 * @param[in] addr the function address
 *
 * @param[in] site the call site address
 *
 * @param[in] nm the function name (can be NULL)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
thread& thread::called(mem_addr_t addr, mem_addr_t site, const i8 *nm)
{
	/*
	 * If the function is called while an exception is unwinding the stack, keep
	 * track of the call depth difference between the simulated and the real call
	 * stack (the 'lag')
	 */
	if ( unlikely(std::uncaught_exception()) ) {
		m_lag--;
		return *this;
	}

	call *c = NULL;
	try {
		c = new call(addr, site, nm);
		m_stack->push(c);
		m_status = THREAD_START;
		return *this;
	}
	catch (...) {
		delete c;
		throw;
	}
}


/**
 * @brief Cancel execution of the thread
 *
 * @returns *this
 *
 * @note Failure in thread cancellation is silently ignored
 */
inline thread& thread::cancel()
{
	pthread_cancel(m_handle);
	return detach_from_process();
}


/**
 * @brief Traverse the simulated stack with a callback for each call
 *
 * @param[in] pfunc the callback (can be NULL for NO-OP)
 *
 * @returns *this
 */
inline thread& thread::each(const callback_t pfunc) const
{
	m_stack->each(pfunc);
	return const_cast<thread&> (*this);
}


/**
 * @brief Check if this thread ID matches the argument (pthread) ID
 *
 * @param[in] id the compared (pthread) ID
 *
 * @returns true if the IDs match, false otherwise
 */
inline bool thread::is(pthread_t id) const
{
	return pthread_equal(m_handle, id) != 0;
}


/**
 * @brief Check if this thread has a specific name
 *
 * @param[in] nm the name (can be NULL)
 *
 * @returns true if the names match, false otherwise
 */
inline bool thread::is(const i8 *nm) const
{
	if ( unlikely(nm == NULL) ) {
		return m_name == NULL;
	}

	if ( likely(m_name == NULL) ) {
		return false;
	}

	return strcmp(m_name, nm) == 0;
}


/**
 * @brief Check if this is the currently executing thread
 *
 * @returns true if this is the active thread, false otherwise
 */
inline bool thread::is_current() const
{
	return pthread_equal(m_handle, pthread_self()) != 0;
}


/**
 * @brief Join the thread and optionally obtain its exit status
 *
 * @param[out] exit_status
 *	a pointer to obtain the joined thread's exit status (ignored if NULL). Based
 *	on the specific thread implementation it can be heap allocated
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
thread& thread::join(void *exit_status)
{
	i32 retval;

	if ( likely(exit_status == NULL) ) {
		retval = pthread_join(m_handle, NULL);
	}
	else {
		retval = pthread_join(m_handle, &exit_status);
	}

	if ( unlikely(retval != 0) ) {
		const i8 *nm = m_name;
		if ( unlikely(nm == NULL) ) {
			nm = "anonymous";
		}

		throw exception(
			"failed to join thread '%s' (errno %d - %s)",
			nm,
			errno,
			strerror(errno));
	}

	return detach_from_process();
}


/**
 * @brief Simulate a function return
 *
 * @returns *this
 *
 * @throws Detect return from thread entry point (thread exit)
 */
thread& thread::returned()
{
	/*
	 * If the function returned because an exception is propagating, unwinding the
	 * stack, keep track of the call depth difference between the simulated and
	 * the real call stack (the 'lag')
	 */
	if ( unlikely(std::uncaught_exception()) ) {
		m_lag++;
	}
	else {
		m_stack->pop();
	}

	return *this;
}


/**
 * @brief Unwind the simulated call stack to meet the real call stack
 *
 * @returns *this
 */
thread& thread::unwind()
{
	while ( likely(m_lag > 0) ) {
		m_stack->pop();
		m_lag--;
	}

	return *this;
}

}
