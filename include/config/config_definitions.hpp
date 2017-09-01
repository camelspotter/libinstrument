#ifndef _CONFIG_DEFINITIONS
#define _CONFIG_DEFINITIONS 1

/**
	@file include/config_definitions.hpp

	@brief Library definitions
*/


namespace instrument {

/*
	String trimming
*/

/**
	@brief Trim leading whitespace only
*/
#define TRIM_LEADING				-1

/**
	@brief Trim trailing whitespace only
*/
#define TRIM_TRAILING				1

/**
	@brief Trim both leading and trailing whitespace
*/
#define TRIM_ALL						0


/*
	Thread running statuses
*/

/**
	@brief Thread initialized but not started
*/
#define THREAD_INIT					0x01

/**
	@brief Thread executing code before main
*/
#define THREAD_PREENTRY			0x02

/**
	@brief Thread started
*/
#define THREAD_START				0x04

/**
	@brief Thread finalized and exited
*/
#define THREAD_EXIT					0x08


/*
	Library output message tags
*/

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


/*
	Debug levels
*/

#ifdef WITH_DEBUG

/**
	@brief Error debug level
*/
#define DBGL_ERROR		0x01

/**
	@brief Warning debug level
*/
#define DBGL_WARNING	0x02

/**
	@brief Generic debug level
*/
#define DBGL_INFO			0x04

/**
	@brief Low debug level (only errors)
*/
#define DBGL_LOW			(DBGL_ERROR)

/**
	@brief Medium debug level (errors and warnings)
*/
#define DBGL_MEDIUM		(DBGL_LOW | DBGL_WARNING)

/**
	@brief High debug level (all messages)
*/
#define DBGL_HIGH			(DBGL_MEDIUM | DBGL_INFO)

/**
	@brief Selected debug level
*/
#define DBG_LEVEL			DBGL_HIGH

#endif


/*
	Use of color in terminals (RXVT, XTerm e.t.c) to color-code message tags
*/

#ifdef WITH_COLOR_TERM

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


/*
	Filter definitions
*/

#ifdef WITH_FILTER

/**
	@brief Filter whole modules (filter path)
*/
#define MODULE_FILTER				false

/**
	@brief Filter functions/methods
*/
#define SYMBOL_FILTER				true

#endif


/*
	Syntax highlighter definitions
*/

#ifdef WITH_HIGHLIGHT

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
#define HLT_FILE_FG					13

/**
	@brief Highlighter color for C++ ABI scopes
*/
#define HLT_SCOPE_FG				250

/**
	@brief Highlighter color for C++ function names
*/
#define HLT_FUNCTION_FG			214

/**
	@brief Dictionary lookup mode (regular expression)
*/
#define REGEXP_LOOKUP_MODE	true

/**
	@brief Dictionary lookup mode (plain)
*/
#define SIMPLE_LOOKUP_MODE	false

#endif


/*
	Stream related definitions
*/

#ifdef WITH_STREAM_FILE

/**
	@brief File creation default permission mask (rw-r--r--)
*/
#define DEFAULT_UMASK				0644

#endif

#ifdef WITH_STREAM_STTY

/**
	@brief Serial tty default baud rate
*/
#define DEFAULT_BAUD				9600

#endif

}

#endif
