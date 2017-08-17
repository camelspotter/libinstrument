#ifndef _LIST
#define _LIST 1

/**
	@file include/list.hpp

	@brief Class instrument::list definition and method implementation
*/

#include "./exception.hpp"

namespace instrument {

/**
	@brief Lightweight, templated, array list (random access list)

	The list supports shared data (multiple lists can point to the same data) but
	it's not thread safe, callers should synchronize thread access. Duplicate and
	NULL pointers aren't allowed (within a single list). When an item is detached
	it is just removed from the list and not deleted. A list can be traversed
	using callbacks	and method list::each.

	A list object has vastly better performance in access times, compared to a
	chain (doubly linked list), O(1) vs O(n), but item addition, insertion and
	removal is more expensive. To get the best of both, memory preallocation (in
	blocks) is supported (transparently)
*/
template <class T>
class list: virtual public object
{
protected:

	/* Protected variables */

	T **m_data;												/**< @brief Data array */

	bool m_ordered;										/**< @brief Maintain ordering */

	u32 m_size;												/**< @brief Item count */

	u32 m_slots;											/**< @brief Allocated item slots */


	/* Protected generic methods */

	virtual list& memalign(u32 = 1, bool = true);

public:

	typedef void (*callback_t)(u32, T*);


	/* Constructors, copy constructors and destructor */

	list(u32 = 1, bool = false);

	list(const list&);

	virtual	~list();

	virtual list* clone() const;


	/* Accessor methods */

	virtual T* at(u32) const;

	virtual bool ordered() const;

	virtual list& set_ordered(bool);

	virtual	u32 size() const;

	virtual	u32 slots() const;


	/* Operator overloading methods */

	virtual list& operator=(const list&);

	virtual T* operator[](u32) const;


	/* Generic methods */

	virtual list& add(T*);

	virtual u32 available() const;

	virtual list& clear();

	virtual T* detach(u32);

	virtual list& each(const callback_t) const;

	virtual list& remove(u32);

	virtual i32 search(const T*) const;
};


/**
 * @brief Allocate aligned memory, mandate a minimum number of slots
 *
 * @param[in] slots the mandatory slot count
 *
 * @param[in] keep true to keep the current data
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
template <class T>
list<T>& list<T>::memalign(u32 slots, bool keep)
{
	if ( unlikely(slots <= m_slots && m_slots != 0) ) {
		return (likely(keep)) ? *this : clear();
	}

	/* Aligned size */
	m_slots = (slots + g_memblock_sz - 1) / g_memblock_sz;
	m_slots *= g_memblock_sz;

	T **aligned = new T*[m_slots];
	if ( likely(keep) ) {
		for (u32 i = 0; likely(i < m_size); i++) {
			aligned[i] = m_data[i];
		}
	}
	else {
		clear();
	}

	delete[] m_data;
	m_data = aligned;
	return *this;
}


/**
 * @brief Object constructor
 *
 * @param[in] slots the minimum slot count
 *
 * @param[in] ordered true to keep the list ordered
 *
 * @throws std::bad_alloc
 */
template <class T>
inline list<T>::list(u32 slots, bool ordered):
m_data(NULL),
m_ordered(ordered),
m_size(0),
m_slots(0)
{
	memalign(slots);
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
template <class T>
inline list<T>::list(const list &src)
try:
m_data(NULL),
m_ordered(src.m_ordered),
m_size(0),
m_slots(0)
{
	*this = src;
}
catch(...) {
	clear();
	delete[] m_data;
	m_data = NULL;
}


/**
 * @brief Object destructor
 */
template <class T>
inline list<T>::~list()
{
	clear();
	delete[] m_data;
	m_data = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
template <class T>
inline list<T>* list<T>::clone() const
{
	return new list(*this);
}


/**
 * @brief Get the item at a list offset
 *
 * @param[in] i the offset
 *
 * @returns the i-th item
 *
 * @throws instrument::exception
 */
template <class T>
inline T* list<T>::at(u32 i) const
{
	if ( unlikely(i >= m_size) ) {
		throw exception("offset out of list bounds (%d >= %d)", i, m_size);
	}

	return m_data[i];
}


/**
 * @brief Check if the list maintains ordering
 *
 * @returns this->m_ordered
 */
template <class T>
inline bool list<T>::ordered() const
{
	return m_ordered;
}


/**
 * @brief Enable/disable list ordering
 *
 * @param[in] ordered
 *	true to keep the list ordered (when items are detached or removed)
 *
 * @returns *this
 *
 * @note
 *	When list ordering is not maintained, item removal/detaching is a lot faster
 */
template <class T>
inline list<T>& list<T>::set_ordered(bool ordered)
{
	m_ordered = ordered;
	return *this;
}


/**
 * @brief Get the list size (item count)
 *
 * @returns this->m_size
 */
template <class T>
inline u32 list<T>::size() const
{
	return m_size;
}


/**
 * @brief Get the list allocated size (slot count)
 *
 * @returns this->m_slots
 */
template <class T>
inline u32 list<T>::slots() const
{
	return m_slots;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 *
 * @note
 *	Automatically resolves collisions when the lists overlap (when they both
 *	contain any identical pointers)
 */
template <class T>
list<T>& list<T>::operator=(const list &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Check if the lists overlap and detach shared pointers */
	for (u32 i = 0; likely(i < m_size); i++) {
		if ( unlikely(rval.search(m_data[i]) >= 0) ) {
			m_data[i] = NULL;
		}
	}

	/* Mandate size and clear */
	memalign(rval.m_size, false);

	for (u32 i = 0; likely(i < rval.m_size); i++) {
		m_data[i] = new T(*rval.m_data[i]);
		m_size++;
	}

	m_ordered = rval.m_ordered;
	return *this;
}


/**
 * @brief Subscript operator
 *
 * @param[in] i the index
 *
 * @returns the i-th item
 *
 * @throws instrument::exception
 */
template <class T>
inline T* list<T>::operator[](u32 i) const
{
	return at(i);
}


/**
 * @brief Add an item to the list
 *
 * @param[in] d the new item pointer
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
template <class T>
list<T>& list<T>::add(T *d)
{
	if ( unlikely(d == NULL) ) {
		throw exception("invalid argument: d (=%p)", d);
	}

	/* If the pointer already exists in the list */
	i32 i;
	if ( unlikely((i = search(d)) >= 0) ) {
		throw exception("list @ %p already has an item @ %p (at %d)", this, d, i);
	}

	/* Mandate size if preallocation is needed */
	if ( unlikely(m_size == m_slots) ) {
		memalign(m_size + 1);
	}

	m_data[m_size++] = d;
	return *this;
}


/**
 * @brief Get the list available slots
 *
 * @returns the number of free slots
 */
template <class T>
inline u32 list<T>::available() const
{
	return m_slots - m_size;
}


/**
 * @brief Empty the list
 *
 * @returns *this
 */
template <class T>
list<T>& list<T>::clear()
{
	for (u32 i = 0; likely(i < m_size); i++) {
		delete m_data[i];
		m_data[i] = NULL;
	}

	m_size = 0;
	return *this;
}


/**
 * @brief Detach the item at a list offset
 *
 * @param[in] i the offset
 *
 * @returns the detached i-th item
 *
 * @throws instrument::exception
 */
template <class T>
T* list<T>::detach(u32 i)
{
	T *d = at(i);

	/* If it's the last list item */
	if ( unlikely(i == m_size - 1) ) {
		m_size--;
		return d;
	}

	/* Shift left all items after the i-th, to preserve ordering */
	if ( unlikely(m_ordered) ) {
		for (u32 j = i + 1; likely(j < m_size);) {
			m_data[i++] = m_data[j++];
		}
	}

	/* Fill the gap with the last item, if the list is not ordered */
	else {
		m_data[i] = m_data[m_size - 1];
	}

	m_size--;
	return d;
}


/**
 * @brief Traverse the list with a callback for each item
 *
 * @param[in] pfunc the callback (can be NULL for NO-OP)
 *
 * @returns *this
 */
template <class T>
list<T>& list<T>::each(const callback_t pfunc) const
{
	__D_ASSERT(pfunc != NULL);
	if ( unlikely(pfunc == NULL) ) {
		return const_cast<list<T>&> (*this);
	}

	for (u32 i = 0; likely(i < m_size); i++) {
		pfunc(i, m_data[i]);
	}

	return const_cast<list<T>&> (*this);
}


/**
 * @brief Dispose the item at a list offset
 *
 * @param[in] i the offset
 *
 * @returns *this
 *
 * @throws instrument::exception
 */
template <class T>
inline list<T>& list<T>::remove(u32 i)
{
	delete detach(i);
	return *this;
}


/**
 * @brief Search for an item
 *
 * @param[in] d the searched item address (can be NULL)
 *
 * @returns the item offset in the list, -1 if not item is found
 */
template <class T>
i32 list<T>::search(const T *d) const
{
	__D_ASSERT(d != NULL);
	if ( unlikely(d == NULL) ) {
		return -1;
	}

	for (u32 i = 0; likely(i < m_size); i++) {
		if ( unlikely(m_data[i] == d) ) {
			return i;
		}
	}

	return -1;
}

}

#endif
