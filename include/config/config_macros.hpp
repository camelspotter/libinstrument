#ifndef _CONFIG_MACROS
#define _CONFIG_MACROS 1

/**
	@file include/config_macros.hpp

	@brief Library macro definitions
*/


/**
	@brief Check if a tag is for an error console message
*/
#define is_error(x)								(strcmp(x, ERROR_TAG) == 0)

/**
	@brief Check if a tag is for an exception console message
*/
#define is_exception(x)						(strcmp(x, EXCEPTION_TAG) == 0)

/**
	@brief Check if a tag is for an informational console message
*/
#define is_info(x)								(strcmp(x, INFO_TAG) == 0)

/**
	@brief Check if a tag is for a warning console message
*/
#define is_warning(x)							(strcmp(x, WARNING_TAG) == 0)


/*
	Thread status check macros
*/

/**
	@brief Has thread initialized but not started
*/
#define is_thread_initialized(x)	(x == THREAD_INIT)

/**
	@brief Has thread initialized and started
*/
#define is_thread_started(x)			(x == THREAD_START)

/**
	@brief Has thread finalized and exited
*/
#define is_thread_finished(x)			(x == THREAD_EXIT)


/*
	Debug assertions (with optional color coding)
*/

#ifdef WITH_COLOR_ASSERTIONS

#define ASSERT_COLOR_DEF					"\e[1m\e[38;5;9m"

#define ASSERT_COLOR_UNDEF				"\e[0m"

#else

#define ASSERT_COLOR_DEF		""

#define ASSERT_COLOR_UNDEF	""

#endif


#ifdef WITH_DEBUG

/**
	@brief Assertion macro
*/
#define __D_ASSERT(x)								\
if (!(x)) {													\
	std::cerr	<< ASSERT_COLOR_DEF			\
						<< std::endl						\
						<< "assertion '"				\
						<< #x										\
						<< "' failed"						\
						<< std::endl						\
						<< "on line "						\
						<< std::dec							\
						<< __LINE__							\
						<< std::endl						\
						<< "in file '"					\
						<< __FILE__							\
						<< "'"									\
						<< std::endl						\
						<< "in function "				\
						<< __PRETTY_FUNCTION__	\
						<< std::endl						\
						<< std::endl						\
						<< ASSERT_COLOR_UNDEF;	\
}

#else

#define __D_ASSERT(x)

#endif


/*
	Use of gcc built-ins for branch prediction and cache optimization
*/

#ifdef __GNUC__

/**
	@brief Offer a hint (positive) to the pipeline branch predictor
*/
#define likely(expr)							__builtin_expect((expr), true)

/**
	@brief Offer a hint (negative) to the pipeline branch predictor
*/
#define unlikely(expr)						__builtin_expect((expr), false)

/**
	@brief Prefetch a block from memory to the cache (for read)
*/
#define precache_r(addr)					__builtin_prefetch((addr), 0, 3)

/**
	@brief Prefetch a block from memory to the cache (for write)
*/
#define precache_w(addr)					__builtin_prefetch((addr), 1, 3)

#else

#define likely(expr)							(expr)

#define unlikely(expr)						(expr)

#define precache_r(addr)

#define precache_w(addr)

#endif

#endif
