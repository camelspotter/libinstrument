#ifndef _CONFIG_GLOBALS
#define _CONFIG_GLOBALS 1

/**
	@file include/config_globals.hpp

	@brief Library global variables
*/


namespace instrument {

/**
	@brief Supported instrument::string codepages

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
static const i8 g_libs_env[] = "INSTRUMENT_LIBS";

/**
	@brief Library version major
*/
static const u16 g_major = 1;

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


/*
	Syntax highlighter globals
*/

#ifdef WITH_HIGHLIGHT

/**
	@brief C++ stack trace syntax

	@see instrument::parser
*/
static const i8 g_trace_syntax[] = "[ \t\n\r\\{\\}\\(\\)\\*&,:<>]+";

#endif


/*
	TCP stream globals
*/

#ifdef WITH_STREAM_TCP

/**
	@brief LDP service port
*/
static const i32 g_ldp_port = 4242;

#endif

}

#endif
