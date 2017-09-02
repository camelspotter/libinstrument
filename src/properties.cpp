#include "../include/properties.hpp"
#include "../include/util.hpp"

/**
	@file src/properties.cpp

	@brief Class instrument::properties method implementation
*/

namespace instrument {

/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
properties::properties()
try:
list<property>(1, true),
m_path(NULL)
{
	u32 prefix_len = strlen(util::prefix()); 
	m_path = new i8[prefix_len + strlen(g_properties_path) + 2];
	
	strcpy(m_path, util::prefix());
	strcpy(m_path + prefix_len, "/");
	strcpy(m_path + prefix_len + 1, g_properties_path);
}
catch (...) {
	clear();
	
	delete[] m_data;
	delete[] m_path;
	
	m_data = NULL;
	m_path = NULL;
}


/**
 * @brief Object constructor
 *
 * @throws std::bad_alloc
 * @todo Detect relative paths and use the installation prefix 
 */
properties::properties(const i8 *path)
try:
list<property>(1, true),
m_path(NULL)
{
	m_path = new i8[strlen(path) + 1];
	strcpy(m_path, path);
}
catch (...) {
	clear();
	
	delete[] m_data;
	delete[] m_path;
	
	m_data = NULL;
	m_path = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
properties::properties(const properties &src)
try:
list<property>(src),
m_path(NULL)
{
	m_path = new i8[strlen(src.m_path) + 1];
	strcpy(m_path, src.m_path);
}
catch (...) {
	clear();
	
	delete[] m_data;
	delete[] m_path;
	
	m_data = NULL;
	m_path = NULL;
}


/**
 * @brief Object destructor
 */
properties::~properties()
{
	delete[] m_path;
	m_path = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline properties* properties::clone() const
{
	return new properties(*this);
}


/**
 * @brief Get the properties file path
 *
 * @returns this->m_path
 */
inline const i8* properties::path() const
{
	return m_path;
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
properties& properties::operator=(const properties &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Copy data */
	list::operator=(rval);

	u32 len = strlen(rval.m_path);
	if ( likely (len > strlen(m_path)) ) {
		delete[] m_path;
		m_path = NULL;
		m_path = new i8[len + 1];
	}

	strcpy(m_path, rval.m_path);
	return *this;
}


/**
 * @brief Deserialize the properties file
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
properties& properties::deserialize()
{
	/* Stat the properties file path and make some preliminary checks */
	fileinfo_t inf;
	i32 retval = stat(m_path, &inf);

	/* File doesn't exist */
	if ( unlikely(errno == ENOENT) ) {
		throw exception("properties file '%s' does not exist", m_path);
	}

	/* Stat failed */
	else if ( unlikely(retval < 0) ) {
		throw exception(
			"failed to stat path '%s' (errno %d - %s)",
			m_path,
			errno,
			strerror(errno)
		);
	}

	/* Not a regular file */
	else if ( unlikely(!util::is_regular(inf)) ) {
		throw exception("'%s' is not a regular file", m_path);
	}

	/* Not a readable file */
	else if ( unlikely(!util::is_readable(inf)) ) {
		throw exception("file '%s' is not readable", m_path);
	}

	i32 sz = inf.st_size;
	if ( unlikely(sz == 0) ) {
		util::dbg_warn("properties file '%s' is empty", m_path);
		return *this;
	}

	/* Open the file */
	i32 fd;
	do {
		fd = open(m_path, O_RDONLY);
	}
	while ( unlikely(fd < 0 && (errno == EINTR || errno == EAGAIN)) );

	if ( unlikely(fd < 0) ) {
		throw exception(
			"failed to open properties file '%s' for reading (errno %d - %s)",
			m_path,
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
			m_path,
			errno,
			strerror(errno)
		);
	}

	chain<string> *lines = NULL;
	i32 cnt = 0;

	/* If an exception occurs, unmap/close the file, clean up and rethrow it */
	try {
		string text(reinterpret_cast<i8*> (mmap_base));		
		lines = text.split("\n");
		
		for (u32 i = 0, sz = lines->size(); likely(i < sz); i++) {
			string *line = lines->at(i);
			line->trim();
			
			
			
			cnt++;
			std::cout << *line << "\n";
		}
		
		delete lines;

		/* i32 bytes = sz;
		i8 *offset, *cur;
		offset = cur = static_cast<i8*> (mmap_base);

		/ Load the dictionary words /
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
						//add(word);
					}

					word = NULL;
				}

				offset = ++cur;
			}
			else {
				++cur;
			}
		} */
	}
	catch (...) {
		delete lines;
		munmap(mmap_base, sz);
		close(fd);
		throw;
	}

	munmap(mmap_base, sz);
	close(fd);

#if DBG_LEVEL & DBGL_INFO
	if ( likely(cnt > 0) ) {
		util::dbg_info(
			"properties file '%s' loaded, %d token%s",
			m_path,
			cnt,
			(cnt != 1) ? "s" : ""
		);
	}
	else {
		util::dbg_info("properties file '%s' is empty", m_path);
	}
#endif

	return *this;
}


properties& properties::serialize()
{
	return *this;
}

}
