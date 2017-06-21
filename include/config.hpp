#ifndef _CONFIG
#define _CONFIG 1

/**
	@file include/config.hpp

	@brief Library configuration, type, macro and global variable definition
*/

#include "config/config_headers.hpp"

#include "config/config_types.hpp"

#include "config/config_definitions.hpp"

#include "config/config_globals.hpp"


/**
	@brief Check if a tag is for an error console message
*/
#define is_error(x)					(strcmp(x, ERROR_TAG) == 0)

/**
	@brief Check if a tag is for an exception console message
*/
#define is_exception(x)			(strcmp(x, EXCEPTION_TAG) == 0)

/**
	@brief Check if a tag is for an informational console message
*/
#define is_info(x)					(strcmp(x, INFO_TAG) == 0)

/**
	@brief Check if a tag is for a warning console message
*/
#define is_warning(x)				(strcmp(x, WARNING_TAG) == 0)


/*
	Debug levels and assertions
*/
#ifdef WITH_DEBUG

/**
	@brief Assertion macro
*/
#define __ASSERT(x)																						\
if (!(x)) {																										\
	std::cerr << "\r\nassertion '" << #x << "' failed";				\
	std::cerr << "\r\non line " << std::dec << __LINE__;				\
	std::cerr << "\r\nin file '" << __FILE__ << "'";						\
	std::cerr << "\r\nin function " << __PRETTY_FUNCTION__;		\
	std::cerr << "\r\n\r\n";																		\
}

#else

#define __ASSERT(x)

#endif


#ifdef WITH_COLOR_ASSERTIONS

/**
	@brief Color coded assertion macro
*/
#define __D_ASSERT(x)																					\
	std::cerr << "\e[1m\e[38;5;9m";															\
	__ASSERT(x);																								\
	std::cerr << "\e[0m"

#else

#define __D_ASSERT(x)				__ASSERT(x)

#endif


/*
	Use of gcc built-ins for branch prediction and cache optimization
*/
#ifdef __GNUC__

/**
	@brief Offer a hint (positive) to the pipeline branch predictor
*/
#define likely(expr)				__builtin_expect((expr), true)

/**
	@brief Offer a hint (negative) to the pipeline branch predictor
*/
#define unlikely(expr)			__builtin_expect((expr), false)

/**
	@brief Prefetch a block from memory to the cache (for read)
*/
#define precache_r(addr)		__builtin_prefetch((addr), 0, 3)

/**
	@brief Prefetch a block from memory to the cache (for write)
*/
#define precache_w(addr)		__builtin_prefetch((addr), 1, 3)

#else

#define likely(expr)				(expr)

#define unlikely(expr)			(expr)

#define precache_r(addr)

#define precache_w(addr)

#endif

#endif
