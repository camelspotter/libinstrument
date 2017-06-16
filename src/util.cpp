#include "../include/tracer.hpp"
#include "../include/util.hpp"

/**
	@file src/util.cpp

	@brief Class instrument::util method implementation
*/

namespace instrument {

/* Static member variable definition */

//chain<string> *util::s_config = NULL;

pthread_mutex_t util::s_global_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/**
 * @brief Library constructor
 *
 * @note If an exception occurs, the process exits
 */
void util::on_lib_load()
{
	try {
		//s_config = new chain<string>;
		return;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	exit(EXIT_FAILURE);
}


/**
 * @brief Library destructor
 */
void util::on_lib_unload()
{
	//delete s_config;
	//s_config = NULL;
	unlock();
}


/**
 * @brief Get the number of CLI arguments, related with libinstrument
 *
 * @returns util::s_config->size()
 */
u32 util::argc()
{
	return 0;//s_config->size();
}


/**
 * @brief Get a CLI argument, given its offset in util::s_config
 *
 * @param[in] i the offset (mandate E[0, util::argc() - 1])
 *
 * @returns util::s_config->at(i)
 *
 * @throws instrument::exception
 *
 * @see util::init
 /
const string* util::argv(u32 i)
{
	return s_config->at(i);
} */


/**
 * @brief Get the absolute path of the executable
 *
 * @returns the path (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
const i8* util::executable_path()
{
	/*
	 * The procfs filesystem maintains a directory for each process (/proc/pid)
	 * and a symlink therein (exe) that contains the absolute path of the process
	 * executable
	 */
	i8 path[PATH_MAX + 1];
	i32 len = snprintf(path, PATH_MAX + 1, "/proc/%d/exe", getpid());
	if ( unlikely(len < 0) ) {
		throw exception("snprintf failed with retval %d", len);
	}

	i8 *retval = new i8[PATH_MAX + 1];

	/* Read the contents of the symlink */
	len = readlink(path, retval, PATH_MAX);
	if ( unlikely(len < 0) ) {
		delete[] retval;

		throw exception(
			"failed to read symlink '%s' (errno %d - %s)",
			path,
			errno,
			strerror(errno));
	}

	retval[len] = '\0';
	return retval;
}


/**
 * @brief Parse a shell (environment) variable to its components
 *
 * @param[in] var the variable name (can be NULL)
 *
 * @returns the variable components, tokenized with ':' (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 /
chain<string>* util::getenv(const i8 *var)
{
	__D_ASSERT(var != NULL);
	if ( unlikely(var == NULL) ) {
		return NULL;
	}

	const i8 *val = ::getenv(var);
	if ( unlikely(val == NULL) ) {
		return NULL;
	}

	string tmp(val);
	return tmp.split(":+");
} */


/**
 * @brief
 *	Initialize the library runtime configuration. Seek command line arguments
 *	that are related with libinstrument (prefixed with --instrument-) and move
 *	them from the argument vector to the runtime configuration list
 *	(util::s_config)
 *
 * @param[in,out] argc the CLI argument count
 *
 * @param[in,out] argv the CLI argument vector
 *
 * @note If an exception occurs, the process exits
 */
void util::init(i32 &argc, i8 **argv)
{
	__D_ASSERT(argc > 1);
	__D_ASSERT(argv != NULL);
	if ( unlikely(argc <= 1 || argv == NULL) ) {
		return;
	}

	try {
		for (i32 i = 0; likely(i < argc); i++) {
			const i8 *arg = argv[i];

			/* If the argument is not libcsdbg-related */
			if ( likely(strstr(arg, "--instrument-") != arg) ) {
				continue;
			}

			if ( likely(strlen(arg) > 13) ) {
				//s_config->add(new string(arg + 13));
			}

			/* Remove it from the argument vector */
			for (i32 j = i; likely(j < argc); j++) {
				argv[j] = argv[j + 1];
			}

			i--;
			argc--;
		}

#if INSTRUMENT_DBG_LEVEL & INSTRUMENT_DBGL_INFO
		/* if ( unlikely(s_config->size() > 0) ) {
			util::dbg_info("libinstrument runtime configuration:");
		}

		for (u32 i = 0, sz = s_config->size(); likely(i < sz); i++) {
			util::dbg_info("  arg %d: --instrument-(%s)", i, s_config->at(i)->cstring());
		}*/
#endif

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
 * @brief Get the library installation prefix
 *
 * @returns instrument::g_prefix
 */
const i8* util::prefix()
{
	return g_prefix;
}


/**
 * @brief Get the demangled name of a type
 *
 * @param[in] inf the type info
 *
 * @returns the demangled name (heap allocated)
 *
 * @throws std::bad_alloc
 *
 * @note If demangling failed the decorated name is returned
 */
const i8* util::type_name(const std::type_info &inf)
{
	const i8 *nm = inf.name();

	/* The abi namespace is part of libstdc++ */
	i8 *retval = abi::__cxa_demangle(nm, NULL, NULL, NULL);
	if ( likely(retval != NULL) ) {
		return retval;
	}

	retval = new i8[strlen(nm) + 1];
	strcpy(retval, nm);
	return retval;
}


/**
 * @brief Get the library version numbers
 *
 * @param[out] major the major version number
 *
 * @param[out] minor the minor (and subminor) version number
 *
 * @note Omit either version number by passing NULL
 */
void util::version(u16 *major, u16 *minor)
{
	if ( likely(major != NULL) ) {
		*major = g_major;
	}

	if ( likely(minor != NULL) ) {
		*minor = g_minor;
	}
}


/**
 * @brief Lock the global access mutex
 *
 * @note Recursive locking is supported
 */
void util::lock()
{
	pthread_mutex_lock(&s_global_lock);
}


/**
 * @brief Compare two memory blocks
 *
 * @param[in] b1 the base address of the first block
 *
 * @param[in] b2 the base address of the second block
 *
 * @param[in] sz the block size compared
 *
 * @returns
 *	<0, 0, >0 if the first sz bytes of b1 are respectively, less than, equal, or
 *	greater than the first sz bytes of b2
 *
 * @throws csdbg::exception
 *
 * @note This method is used for portability (in place of BSD's bcmp)
 */
i32 util::memcmp(const void *b1, const void *b2, u32 sz)
{
	__D_ASSERT(b1 != NULL);
	__D_ASSERT(b2 != NULL);
	if ( unlikely(b1 == NULL || b2 == NULL) ) {
		throw exception("invalid argument: b1 (=%p) and/or b2 (=%p)", b1, b2);
	}

	const u8 *p1 = static_cast<const u8*> (b1);
	const u8 *p2 = static_cast<const u8*> (b2);
	while ( likely(sz-- > 0) ) {
		i8 diff = *(p1++) - *(p2++);

		if ( likely(diff != 0) ) {
			return diff;
		}
	}

	return 0;
}


/**
 * @brief Copy a memory block
 *
 * @param[out] dst the destination base address
 *
 * @param[in] src the source base address
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note This method is used for portability (in place of BSD's bcopy)
 */
void* util::memcpy(void *dst, const void *src, u32 sz)
{
	__D_ASSERT(dst != NULL);
	__D_ASSERT(src != NULL);
	if ( unlikely(dst == NULL || src == NULL) ) {
		return dst;
	}

	u8 *d = static_cast<u8*> (dst);
	const u8 *s = static_cast<const u8*> (src);
	while ( likely(sz-- > 0) ) {
		*(d++) = *(s++);
	}

	return dst;
}


/**
 * @brief Fill a memory block with a constant byte
 *
 * @param[out] mem the base address of the block
 *
 * @param[in] val the byte
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note This method is used for portability (in place of BSD's bzero)
 */
void* util::memset(void *mem, u8 val, u32 sz)
{
	__D_ASSERT(mem != NULL);
	if ( unlikely(mem == NULL) ) {
		return mem;
	}

	u8 *p = static_cast<u8*> (mem);
	while ( likely(sz-- > 0) ) {
		*(p++) = val;
	}

	return mem;
}


/**
 * @brief Reverse the byte order of a memory block
 *
 * @param[in,out] mem the base address of the block
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note Used to convert big endian data to little endian and vice versa
 */
void* util::memswap(void *mem, u32 sz)
{
	__D_ASSERT(mem != NULL);
	if ( unlikely(mem == NULL) ) {
		return mem;
	}

	u8 *l = static_cast<u8*> (mem);
	u8 *r = l + sz - 1;
	while ( likely(l < r) ) {
		u8 tmp = *l;
		*(l++) = *r;
		*(r--) = tmp;
	}

	return mem;
}


/**
 * @brief Zero format a memory block
 *
 * @param[out] mem the base address of the block
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 */
void* util::memzero(void *mem, u32 sz)
{
	return memset(mem, 0, sz);
}


/**
 * @brief Unlock the global access mutex
 */
void util::unlock()
{
	pthread_mutex_unlock(&s_global_lock);
}


/**
 * @brief Check if a file is a character device node
 *
 * @param[in] inf the file info
 *
 * @returns true if it is a character device node, false otherwise
 */
bool util::is_chardev(const fileinfo_t &inf)
{
	return S_ISCHR(inf.st_mode);
}


/**
 * @brief Check if the process has read access to a file
 *
 * @param[in] inf the file info
 *
 * @returns true if it is readable, false otherwise
 */
bool util::is_readable(const fileinfo_t &inf)
{
	if ( likely(geteuid() == inf.st_uid && (inf.st_mode & S_IRUSR)) ) {
		return true;
	}

	if ( likely(getegid() == inf.st_gid && (inf.st_mode & S_IRGRP)) ) {
		return true;
	}

	return inf.st_mode & S_IROTH;
}


/**
 * @brief Check if a file is a regular one
 *
 * @param[in] inf the file info
 *
 * @returns true if it is regular, false otherwise
 */
bool util::is_regular(const fileinfo_t &inf)
{
	return S_ISREG(inf.st_mode);
}


/**
 * @brief Check if the process has write access to a file
 *
 * @param[in] inf the file info
 *
 * @returns true if it is writable, false otherwise
 */
bool util::is_writable(const fileinfo_t &inf)
{
	if ( likely(geteuid() == inf.st_uid && (inf.st_mode & S_IWUSR)) ) {
		return true;
	}

	if ( likely(getegid() == inf.st_gid && (inf.st_mode & S_IWGRP)) ) {
		return true;
	}

	return inf.st_mode & S_IWOTH;
}


/**
 * @brief Print a tagged debug message on the standard error stream
 *
 * @param[in] tag the message tag
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @note Deprecated if debugging is disabled
 */
void util::dbg(console_tag_t tag, const i8 *fmt, va_list args)
{
#ifdef CSDBG_WITH_DEBUG
	__D_ASSERT(tag != NULL);
	__D_ASSERT(fmt != NULL);
	if ( unlikely(tag == NULL || fmt == NULL) ) {
		va_end(args);
		return;
	}

	const i8 *msg = NULL;
	try {
		msg = va_format(fmt, args);

		lock();
		if ( likely(!isspace(fmt[0])) ) {
			header(std::cerr, tag);
		}

		std::cerr << msg << "\r\n";
		delete[] msg;
		unlock();
	}
	catch (...) {
		__D_ASSERT(msg != NULL);
	}
#endif
}


/**
 * @brief Print an error debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debugging is disabled
 */
void util::dbg_error(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_ERROR
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return;
	}

	va_list args;
	va_start(args, fmt);
	dbg(ERROR_TAG, fmt, args);
#endif
}


/**
 * @brief Print an informational debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debug level is set lower than DBG_LEVEL_HIGH
 */
void util::dbg_info(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_INFO
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return;
	}

	va_list args;
	va_start(args, fmt);
	dbg(INFO_TAG, fmt, args);
#endif
}


/**
 * @brief Print a warning debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debug level is set lower than DBG_LEVEL_MEDIUM
 */
void util::dbg_warn(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_WARNING
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		return;
	}

	va_list args;
	va_start(args, fmt);
	dbg(WARNING_TAG, fmt, args);
#endif
}


/**
 * @brief Print a tagged message header on a (console) output stream
 *
 * @param[in] stream the output stream
 *
 * @param[in] tag the message tag
 */
void util::header(std::ostream &stream, console_tag_t tag)
{
	__D_ASSERT(tag != NULL);
	if ( unlikely(tag == NULL) ) {
		return;
	}

#ifdef CSDBG_WITH_COLOR_TERM
	u32 fg = EXCEPTION_TAG_FG;
	if ( likely(is_error(tag)) ) {
		fg = ERROR_TAG_FG;
	}
	else if ( likely(is_info(tag)) ) {
		fg = INFO_TAG_FG;
	}
	else if ( likely(is_warning(tag)) ) {
		fg = WARNING_TAG_FG;
	}

	stream << "\e[38;5;"
				 << std::dec
				 << fg
				 << "m"
				 << tag
				 << "\e[0m";
#else
	stream << tag;
#endif

	stream << " ["
				 << std::dec
				 << getpid()
				 << ", 0x"
				 << std::hex
				 << pthread_self();

	const i8 *name = NULL;
	const tracer *iface = tracer::interface();
	if ( likely(iface != NULL) ) {
		name =
			iface->proc()
					 ->current_thread()
					 ->name();
	}

	stream << " ("
				 << ((likely(name != NULL)) ? name : "anonymous")
				 << ")] ";
}


/**
 * @brief
 *	Format a buffer with a printf-style string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the formatted string (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
i8* util::va_format(const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	va_list cpargs;
	va_copy(cpargs, args);
	i8 *retval = NULL;

	try {
		i32 sz = va_size(fmt, cpargs);
		retval = new i8[sz + 1];

		i32 check = vsprintf(retval, fmt, args);
		if ( unlikely(check != sz) ) {
			throw exception("vsprintf failed with retval %d", check);
		}

		va_end(args);
		return retval;
	}
	catch (...) {
		delete[] retval;
		va_end(args);
		throw;
	}
}


/**
 * @brief
 *	Format a buffer with a printf-style string expanded with the values of a
 *	variable argument list
 *
 * @param[out] dst
 *	the buffer to be formatted. Use util::va_size to compute the suitable buffer
 *	size to avoid memory overflows. If NULL a proper buffer is allocated
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the formatted string (heap allocated if dst == NULL)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
i8* util::va_format(i8 *dst, const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	if ( unlikely(dst == NULL) ) {
		return va_format(fmt, args);
	}

	i32 sz = vsprintf(dst, fmt, args);
	va_end(args);

	if ( unlikely(sz < 0) ) {
		throw exception("vsprintf failed with retval %d", sz);
	}

	return dst;
}


/**
 * @brief
 *	Compute the size of a printf-style format string expanded with the values of
 *	a variable argument list
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the computed size (not including the trailing '\\0')
 *
 * @throws csdbg::exception
 */
i32 util::va_size(const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	i32 retval = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	if ( unlikely(retval < 0) ) {
		throw exception("vsnprintf failed with retval %d", retval);
	}

	return retval;
}

}
