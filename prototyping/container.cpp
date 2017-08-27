#include "../include/tracer.hpp"
#include "../include/util.hpp"

/**
	@file src/container.cpp

	@brief Class instrument::container method implementation
*/

namespace instrument {

/* Static member variable definition */

container *container::s_context = NULL;

pthread_mutex_t container::s_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/* Link the instrumentation functions with C-style linking */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the beginning of instrumented functions. By implementing this
 *	function (and __cyg_profile_func_exit), libinstrument injects all AOP constructs
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	container *context = container::context();

	__D_ASSERT(this_fn != NULL);
	__D_ASSERT(call_site != NULL);
	__D_ASSERT(context != NULL);
	if ( unlikely(context == NULL) ) {
		return;
	}

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);
		mem_addr_t site = reinterpret_cast<mem_addr_t> (call_site);

		context::lock();

    /* @todo Implement AOP core */

  	context::unlock();
		return;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	context::unlock();
	exit(EXIT_FAILURE);
}


/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the end of instrumented functions. By implementing this function
 *	(and __cyg_profile_func_enter), libinstrument injects all AOP constructs
 *
 * @param[in] this_fn the address of the returning function
 *
 * @param[in] call_site the address that the program counter will return to
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	container *context = container::context();

	__D_ASSERT(context != NULL);
	if ( unlikely(context == NULL) ) {
		return;
	}

	try {
		context::lock();

		/* @todo Implement AOP core */

		context::unlock();
		return;
	}
	catch (exception x) {
		std::cerr << x;
	}
	catch (std::exception x) {
		std::cerr << x;
	}

	context::unlock();
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
void context::__on_lib_load()
{
	try {
		s_context = new container;

		/* @todo Group such calls -> util::dbg_info("libinstrument.so.%d.%d initialized", g_major, g_minor); */
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
	delete s_context;
	s_context = NULL;
	/* util::dbg_info("libinstrument.so.%d.%d finalized", g_major, g_minor); */
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
container::container()
try:
{
}
catch (...) {
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 */
container::container(const container &src)
try:
{
}
catch (...) {
	destroy();
}


/**
 * @brief Object destructor
 */
container::~container()
{
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline container* container::clone() const
{
	return new container(*this);
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
inline container& container::operator=(const container &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	return *this;
}


/**
 * @brief Get the interface object
 *
 * @returns container::s_context if the interface object is enabled, NULL otherwise
 */
container* container::context()
{
	/* The interface is not yet initialized */
	if ( unlikely(s_context == NULL) ) {
		return NULL;
	}

	return s_context;
}


/**
 * @brief Lock the access mutex
 *
 * @note Recursive locking is supported
 */
void container::lock()
{
	pthread_mutex_lock(&s_lock);
}


/**
 * @brief Unlock the access mutex
 */
void container::unlock()
{
	pthread_mutex_unlock(&s_lock);
}

}
