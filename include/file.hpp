#ifndef _FILE
#define _FILE 1

/**
	@file include/file.hpp

	@brief Class instrument::file definition
*/

#include "./stream.hpp"

namespace instrument {

/**
	@brief A buffered file output stream

	A file object is a buffered output stream used to output LDP and generic data
	to a file. Based on the unique identifiers of the instrumented process, a file
	object can assign file names in an unambiguous way. This class is not thread
	safe, the caller must implement thread synchronization, nevertheless basic
	file locking methods are inherited from instrument::stream

	@note Methods seek_to and resize are not const in case mmap is used

	@see
		<a href="index.html#sec5_4">
			<b>5.4 LDP (Libinstrument Debug Protocol)</b>
		</a>
	@see <a href="index.html#sec5_5_1"><b>5.5.1 Using instrument::file</b></a>

	@todo Recode unique_id to return a new file object
	@todo Add a specifier for a fixed length random string to unique_id
*/
class file: virtual public stream
{
protected:

	/* Protected variables */

	i8 *m_path;										/**< @brief Output file path */

public:

	/* Static methods */

	static string* unique_id(const i8*);


	/* Constructors, copy constructors and destructor */

	explicit file(const i8*);

	file(const file&);

	virtual ~file();

	virtual file* clone() const;


	/* Accessor methods */

	virtual const i8* path() const;


	/* Operator overloading methods */

	virtual file& operator=(const file&);


	/* Generic methods */

	virtual file& flush();

	virtual file& open();

	virtual file& open(u32, u32);

	virtual file& resize(u32);

	virtual file& seek_to(i32, bool = false);

	virtual file& sync() const;

	virtual file& sync(bool) const;
};

}

#endif
