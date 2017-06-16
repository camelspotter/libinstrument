#ifndef _CSDBG_CONFIG
#define _CSDBG_CONFIG 1

/**
	@file include/config.hpp

	@brief Library configuration, type, macro and global variable definition
*/


#include <iostream>
#include <typeinfo>

#include <climits>
#include <cstdarg>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

#include <cxxabi.h>
#include <bfd.h>
#include <link.h>
#include <pthread.h>
#include <regex.h>
#include <sys/stat.h>

#ifdef CSDBG_WITH_HIGHLIGHT
#include <fcntl.h>
#include <sys/mman.h>
#endif

#ifdef CSDBG_WITH_STREAM
#include <sys/file.h>
#include <sys/time.h>

#ifdef CSDBG_WITH_STREAM_STTY
#include <termios.h>
#endif

#ifdef CSDBG_WITH_STREAM_TCP
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
#endif

#ifdef __cplusplus
}
#endif


/**
	@brief Complete library namespace

	This namespace consists of the libcsdbg classes/templates and global variables
	(csdbg::g_*) that provide the default library configuration

	@todo Implement syscall wrappers for proper retval handling (replays)
	@todo Restructure config.hpp
*/
namespace csdbg {

/**
	@brief 8-bit signed integer
*/
typedef char								i8;

/**
	@brief 16-bit signed integer
*/
typedef short								i16;

/**
	@brief 32-bit signed integer
*/
typedef int									i32;

/**
	@brief 64-bit signed integer
*/
typedef long long						i64;

/**
	@brief 8-bit unsigned integer
*/
typedef unsigned char				u8;

/**
	@brief 16-bit unsigned integer
*/
typedef unsigned short			u16;

/**
	@brief 32-bit unsigned integer
*/
typedef unsigned int				u32;

/**
	@brief 64-bit unsigned integer
*/
typedef unsigned long long	u64;


/**
	@brief Locale codepage
*/
typedef const i8*						codepage_t;

/**
	@brief Console message header tag
*/
typedef const i8*						console_tag_t;

/**
	@brief Thread running status
*/
typedef u8									thread_status_t;


/*
	Address bus size type definition
*/
#if defined __x86_64__ || defined __ppc64__

/**
	@brief 64-bit memory address
*/
typedef unsigned long long	mem_addr_t;

#else

/**
	@brief 32-bit memory address
*/
typedef unsigned int				mem_addr_t;

#endif


/*
	Syntax highlighter type definitions
*/
#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief VT100 attribute bitmask

	This type could be defined as a u8, it is defined as u16 to reserve space. The
	sizeof(csdbg::style) remains 8 bytes due to 4 byte alignment
*/
typedef u16									attrset_t;

/**
	@brief VT100 terminal color
*/
typedef u8									color_t;

/**
	@brief File metadata
*/
typedef struct stat					fileinfo_t;

#endif


/*
	Plugin system type definitions
*/
#ifdef CSDBG_WITH_PLUGIN

/**
	@brief DSO handle
*/
typedef void*								dso_t;

/**
	@brief Plugin callback
*/
typedef void (*							modsym_t)(void*, void*);

#endif


/*
	TCP streaming type definitions
*/
#ifdef CSDBG_WITH_STREAM_TCP

/**
	@brief TCP IPv4 address
*/
typedef struct sockaddr_in	tcp_addr_t;

/**
	@brief IP address
*/
typedef struct sockaddr			ip_addr_t;

#endif

}


/*
	Static library globals
*/
namespace csdbg {

/**
	@brief Supported csdbg::string codepages

	@see string::m_locale
*/
static const codepage_t g_codepages[] = {

	"iso-8859-1",

	"iso-8859-7"

};

/**
	@brief DSO filtering shell variable

	@see tracer::on_dso_load
*/
static const i8 g_libs_env[] = "CSDBG_LIBS";

/**
	@brief Library version major
*/
static const u16 g_major = 2;

/**
	@brief Block size (allocation alignment)

	@see string::memalign
*/
static const u16 g_memblock_sz = 64;

/**
	@brief Library version minor
*/
static const u16 g_minor = 0;

/**
	@brief Preallocation block size

	@see list::memalign
*/
static const u16 g_prealloc_sz = 128;

/**
	@brief Library installation prefix
*/
static const i8 g_prefix[] = "/usr/local";


#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief C++ stack trace syntax

	@see csdbg::parser
*/
static const i8 g_trace_syntax[] = "[ \t\n\r\\{\\}\\(\\)\\*&,:<>]+";

#endif


#ifdef CSDBG_WITH_STREAM_TCP

/**
	@brief LDP service port
*/
static const i32 g_ldp_port = 4242;

#endif

}


/*
	Library output message tags and macros
*/
namespace csdbg {

/**
	@brief Tag for error console messages
*/
#define	ERROR_TAG						"[e]"

/**
	@brief Tag for exception console messages
*/
#define	EXCEPTION_TAG				"[x]"

/**
	@brief Tag for informational console messages
*/
#define	INFO_TAG						"[i]"

/**
	@brief Tag for warning console messages
*/
#define	WARNING_TAG					"[w]"

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

}


/*
	Debug levels and assertions
*/
#ifdef CSDBG_WITH_DEBUG

/**
	@brief Error debug level
*/
#define CSDBG_DBGL_ERROR		0x01

/**
	@brief Warning debug level
*/
#define CSDBG_DBGL_WARNING	0x02

/**
	@brief Generic debug level
*/
#define CSDBG_DBGL_INFO			0x04

/**
	@brief Low debug level (only errors)
*/
#define CSDBG_DBGL_LOW			(CSDBG_DBGL_ERROR)

/**
	@brief Medium debug level (errors and warnings)
*/
#define CSDBG_DBGL_MEDIUM		(CSDBG_DBGL_LOW | CSDBG_DBGL_WARNING)

/**
	@brief High debug level (all messages)
*/
#define CSDBG_DBGL_HIGH			(CSDBG_DBGL_MEDIUM | CSDBG_DBGL_INFO)

/**
	@brief Selected debug level
*/
#define CSDBG_DBG_LEVEL			CSDBG_DBGL_HIGH

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


/*
	Use of color in terminals (RXVT, XTerm e.t.c) to color-code message tags and
	assertions
*/
#ifdef CSDBG_WITH_COLOR_TERM

/**
	@brief Tag color for error and exception messages
*/
#define ERROR_TAG_FG				9

/**
	@brief Tag color for error and exception messages
*/
#define EXCEPTION_TAG_FG		196

/**
	@brief Tag color for informational messages
*/
#define INFO_TAG_FG					61

/**
	@brief Tag color for warning messages
*/
#define WARNING_TAG_FG			202

#endif


#ifdef CSDBG_WITH_COLOR_ASSERTIONS

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
	Syntax highlighter definitions
*/
#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief Highlighter color for numbers (any base)
*/
#define HLT_NUMBER_FG				208

/**
	@brief Highlighter color for C++ keywords
*/
#define HLT_KEYWORD_FG			61

/**
	@brief Highlighter color for C++ intrinsic types
*/
#define HLT_TYPE_FG					105

/**
	@brief Highlighter color for C++ files
*/
#define HLT_FILE_FG					250

/**
	@brief Highlighter color for C++ ABI scopes
*/
#define HLT_SCOPE_FG				250

/**
	@brief Highlighter color for C++ function names
*/
#define HLT_FUNCTION_FG			214

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
