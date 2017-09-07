#include "../include/properties.hpp"
#include "../include/util.hpp"

/**
	@file src/properties.cpp

	@brief Class instrument::properties method implementation
*/

namespace instrument {

/*
	A chain<string> callback, to trim each entry
*/
void __trim_chain_callback(u32 index, string *item)
{
	item->trim();
}


/**
 * @brief Free all object resources
 *
 * @returns *this
 */
properties& properties::destroy()
{
	clear();

	delete[] m_data;
	delete[] m_path;

	m_data = NULL;
	m_path = NULL;

	return *this;
}


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
	destroy();
}


/**
 * @brief Object constructor
 *
 * @param[in] path the .properties file path (used as is)
 *
 * @throws std::bad_alloc
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
	destroy();
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
	destroy();
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
	if ( likely(len > strlen(m_path)) ) {
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

	u32 cnt = 0;
	property *current = NULL;
	chain<string> *lines = NULL;

	/* If an exception occurs, unmap/close the file, clean up and rethrow it */
	try {
		current = new property();

		string text(reinterpret_cast<i8*> (mmap_base));
		lines = text.split("\r?\n");

		for (u32 i = 0, linecnt = lines->size(); likely(i < linecnt); i++) {
			string *line = lines->at(i);
			line->trim();

			if ( unlikely(line->is_empty()) ) {
				continue;
			}

			i32 index = line->index_of("#");

			/* Line-spaning comment detection */
			if ( unlikely(index == 0) ) {
				line->at(0) = ' ';
				line->trim();

				if ( likely(!line->is_empty()) ) {
					current->m_comments->add(line);
				}

				if ( unlikely(line->index_of("=") >= 0) ) {
					cnt++;
					add(current);
					current = NULL;
					current = new property();
				}

				continue;
			}

			/* Inline comment detection */
			if ( unlikely(index > 0) ) {
				string *comment = line->substring(index, line->length());
				comment->at(0) = ' ';
				comment->trim();

				if ( likely(!comment->is_empty()) ) {
					current->m_inline_comment = comment;
				}

				line->crop(index);
				line->trim();
			}

			/* Parse token key and value */
			chain<string> *parts = line->split("=");
			parts->each(__trim_chain_callback);

			u32 partcnt = parts->size();
			if ( likely(partcnt >= 1) ) {
				current->m_name = parts->at(0);
			}

			if ( likely(partcnt >= 2) ) {
				current->m_value = parts->at(1);

				for (u32 j = 2; likely(j < partcnt); j++) {
					current->m_value->append("=");
					current->m_value->append(*parts->at(j));
				}
			}

			if ( likely(!current->is_empty()) ) {
				cnt++;
				add(current);
				current = NULL;
				current = new property();
			}
		}

		lines->detach_all();
		delete lines;
		delete current;
	}
	catch (...) {
		lines->detach_all();

		delete current;
		delete lines;

		munmap(mmap_base, sz);
		close(fd);

		throw;
	}

	munmap(mmap_base, sz);
	close(fd);

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

	return *this;
}


/**
 * @brief Serialize to the properties file
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 *
 * @todo Now serializes to console, implement print-to-file
 */
properties& properties::serialize()
{
	string buffer;

	for (u32 i = 0; likely(i < m_size); i++) {
		const property *p = m_data[i];

		/* Serialize multi-line comments */
		for (u32 j = 0, sz = p->m_comments->size(); likely(j < sz); j++) {
			buffer.append("#")
						.append(*p->m_comments->at(j))
						.append("\n");
		}

		if ( unlikely(p->m_name == NULL && p->m_value == NULL) ) {
			buffer.append("\n");
			continue;
		}

		/* Invalid token marker */
		if ( likely(!p->validate()) ) {
			buffer.append("## ");
		}

		/* Serialize token */
		string *s = p->m_name;
		if ( likely(s != NULL && !s->is_empty()) ) {
			buffer.append(*s);
		}

		buffer.append(" = ");

		s = p->m_value;
		if ( likely(s != NULL && !s->is_empty()) ) {
			buffer.append(*s);
		}

		s = p->m_inline_comment;
		if ( likely(s != NULL && !s->is_empty()) ) {
			buffer.append(" #")
						.append(*s);
		}

		buffer.append("\n\n");
	}

	/* Serialize to console */
	std::cout	<< buffer
						<< std::endl;

	return *this;
}

}
