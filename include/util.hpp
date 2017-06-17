#ifndef _INSTRUMENT_UTIL
#define _INSTRUMENT_UTIL 1

/**
	@file include/Util.hpp

	@brief Class instrument::util definition
*/

//#include "./string.hpp"
#include "./object.hpp"

namespace instrument {

/**
	@brief This class provides various low level utility and portability methods
*/
class util: virtual public object
{
protected:

	/* Protected static variables */

	//static chain<string> *s_config;						/**< @brief Runtime configuration */

	static pthread_mutex_t s_global_lock;		/**< @brief Global access mutex */


	/* Protected static methods */

	static void on_lib_load()	__attribute((constructor));

	static void on_lib_unload()	__attribute((destructor));

public:

	/* Generic methods */

	/* Library compile time and runtime configuration handling methods */

	static u32 argc();

	//static const string* argv(u32);

	static const i8* executable_path();

	//static chain<string>* getenv(const i8*);

	static void init(i32&, i8**);

	static const i8* prefix();

	static const i8* type_name(const std::type_info&);

	static void version(u16*, u16*);


	/* Various methods (with portability issues) */

	static void lock();

	static i32 memcmp(const void*, const void*, u32);

	static void* memcpy(void*, const void*, u32);

	static void* memset(void*, u8, u32);

	static void* memswap(void*, u32);

	static void* memzero(void*, u32);

	static void unlock();


	/* Utility methods */

	//static bool is_chardev(const fileinfo_t&);

	//static bool is_readable(const fileinfo_t&);

	//static bool is_regular(const fileinfo_t&);

	//static bool is_writable(const fileinfo_t&);


	/* Output and debug methods */

	static void dbg(console_tag_t, const i8*, va_list);

	static void dbg_error(const i8*, ...);

	static void dbg_info(const i8*, ...);

	static void dbg_warn(const i8*, ...);

	static void header(std::ostream&, console_tag_t);

	static i8* va_format(const i8*, va_list);

	static i8* va_format(i8*, const i8*, va_list);

	static i32 va_size(const i8*, va_list);
};

}

#endif
