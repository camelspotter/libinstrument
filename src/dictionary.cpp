#include "../include/dictionary.hpp"
#include "../include/util.hpp"

/**
	@file src/dictionary.cpp

	@brief Class instrument::dictionary method implementation
*/

namespace instrument {

/**
 * @brief Object constructor
 *
 * @param[in] nm the dictionary name
 *
 * @param[in] path the path of the data file (can be NULL to start empty)
 *
 * @param[in] mode the lookup mode (REGEXP_LOOKUP_MODE or SIMPLE_LOOKUP_MODE)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
dictionary::dictionary(const i8 *nm, const i8 *path, bool mode)
try:
list<string>(),
m_mode(mode),
m_name(NULL)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	m_name = new i8[strlen(nm) + 1];
	strcpy(m_name, nm);

	__D_ASSERT(path != NULL);
	if ( likely(path != NULL) ) {
		load_file(path);
	}
}
catch (...) {
	clear();
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
dictionary::dictionary(const dictionary &src)
try:
list<string>(src),
m_mode(src.m_mode),
m_name(NULL)
{
	m_name = new i8[strlen(src.m_name) + 1];
	strcpy(m_name, src.m_name);
}
catch (...) {
	clear();
}


/**
 * @brief Object destructor
 */
dictionary::~dictionary()
{
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline dictionary* dictionary::clone() const
{
	return new dictionary(*this);
}


/**
 * @brief Get the lookup mode
 *
 * @returns this->m_mode
 */
inline bool dictionary::mode() const
{
	return m_mode;
}


/**
 * @brief Get the dictionary name
 *
 * @returns this->m_name
 */
inline const i8* dictionary::name() const
{
	return m_name;
}


/**
 * @brief Set the lookup mode
 *
 * @param[in] mode the new mode
 *
 * @returns *this
 */
inline dictionary& dictionary::set_mode(bool mode)
{
	m_mode = mode;
	return *this;
}


/**
 * @brief Set the dictionary name
 *
 * @param[in] nm the new name
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
dictionary& dictionary::set_name(const i8 *nm)
{
	if ( unlikely(nm == NULL) ) {
		throw exception("invalid argument: nm (=%p)", nm);
	}

	u32 len = strlen(nm);
	if (len > strlen(m_name)) {
		delete[] m_name;
		m_name = NULL;
		m_name = new i8[len + 1];
	}

	strcpy(m_name, nm);
	return *this;
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
dictionary& dictionary::operator=(const dictionary &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Copy words */
	list<string>::operator=(rval);

	m_mode = rval.m_mode;
	return set_name(rval.m_name);
}


/**
 * @brief Load words from a dictionary file
 *
 * @param[in] path the path of the data file (can be NULL for NO-OP)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @note
 *	Each non-empty line in the file is translated as a single token. A line with
 *	only whitespace characters is considered an empty line. The tokens are
 *	trimmed to remove leading and trailing whitespace characters. If the file is
 *	empty no tokens are loaded, but the object remains valid
 *
 * @attention The file is <b>memory mapped</b>
 */
dictionary& dictionary::load_file(const i8 *path)
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) ) {
		return *this;
	}

	/* Stat the dictionary file path and make some preliminary checks */
	fileinfo_t inf;
	i32 retval = stat(path, &inf);

	/* File doesn't exist */
	if ( unlikely(errno == ENOENT) ) {
		throw exception("file '%s' does not exist", path);
	}

	/* Stat failed */
	else if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to stat path '%s' (errno %d - %s)",
			path,
			errno,
			strerror(errno)
		);
	}

	/* Not a regular file */
	else if ( unlikely(!util::is_regular(inf)) ) {
		throw exception("'%s' is not a regular file", path);
	}

	/* Not a readable file */
	else if ( unlikely(!util::is_readable(inf)) ) {
		throw exception("file '%s' is not readable", path);
	}

	i32 sz = inf.st_size;
	if ( unlikely(sz == 0) ) {
		util::dbg_warn("dictionary file '%s' is empty", path);
		return *this;
	}

	/* Open the file */
	i32 fd;
	do {
		fd = open(path, O_RDONLY);
	}
	while ( unlikely(fd < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(fd < 0) ) {
		throw exception(
			"failed to open file '%s' for reading (errno %d - %s)",
			path,
			errno,
			strerror(errno)
		);
	}

	/* Memory map the file */
	void *mmap_base = mmap(NULL, sz, PROT_READ, MAP_SHARED, fd, 0);
	if ( unlikely(mmap_base == MAP_FAILED) ) {
		close(fd);

		throw exception(
			"failed to memory map file '%s' (errno %d - %s)",
			path,
			errno,
			strerror(errno)
		);
	}

	string *word = NULL;
	i32 cnt = 0;

	/* If an exception occurs, unmap/close the file, clean up and rethrow it */
	try {
		i32 bytes = sz;
		i8 *offset, *cur;
		offset = cur = static_cast<i8*> (mmap_base);

		/* Load the dictionary words */
		while ( likely(bytes-- > 0) ) {
			if ( unlikely(*cur == '\n') ) {
				if ( likely(cur != offset) ) {
					word = new string("%.*s", cur - offset, offset);
					word->trim();

					if ( unlikely(word->length() == 0) ) {
						delete word;
					}
					else {
						cnt++;
						add(word);
					}

					word = NULL;
				}

				offset = ++cur;
			}
			else {
				++cur;
			}
		}
	}
	catch (...) {
		delete word;
		munmap(mmap_base, sz);
		close(fd);
		throw;
	}

	munmap(mmap_base, sz);
	close(fd);

#if DBG_LEVEL & DBGL_INFO
	if ( likely(cnt > 0) ) {
		util::dbg_info(
			"file '%s' (%d word%s) loaded on dictionary '%s'",
			path,
			cnt,
			(cnt != 1) ? "s" : "",
			m_name
		);
	}
	else {
		util::dbg_info("dictionary file '%s' is empty", path);
	}
#endif

	return *this;
}


/**
 * @brief Dictionary lookup
 *
 * @param[in] exp the expression to lookup
 *
 * @param[in] icase true to ignore case in comparing/matching
 *
 * @returns the matched dictionary word, NULL if no match is found
 *
 * @throws instrument::exception
 */
const string* dictionary::lookup(const string &exp, bool icase) const
{
	for (u32 i = 0; likely(i < m_size); i++) {
		const string *word = at(i);

		if ( likely(m_mode == SIMPLE_LOOKUP_MODE) ) {
			if ( unlikely(exp.compare(*word, icase) == 0) ) {
				return word;
			}
		}

		else if ( unlikely(exp.match(*word, icase)) ) {
			return word;
		}
	}

	return NULL;
}

}
