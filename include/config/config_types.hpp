#ifndef _CONFIG_TYPES
#define _CONFIG_TYPES 1

/**
	@file include/config_types.hpp

	@brief Library type definitions
*/


/**
	@brief Complete library namespace

	This namespace consists of the libinstrument classes/templates and global
	variables (instrument::g_*) that provide the default library configuration

	@todo Implement syscall wrappers for proper retval handling (replays)
*/
namespace instrument {

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


/*
	Secondary types
*/

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

/**
	@brief Thread entry function argument type
*/
typedef void*								thread_arg_t;

/**
	@brief Thread entry function
*/
typedef void* (*						thread_main_t)(thread_arg_t);


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

#ifdef WITH_HIGHLIGHT

/**
	@brief VT100 attribute bitmask

	This type could be defined as a u8, it is defined as u16 to reserve space. The
	sizeof(instrument::style) remains 8 bytes due to 4 byte alignment
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

#ifdef WITH_PLUGIN

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

#ifdef WITH_STREAM_TCP

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

#endif
