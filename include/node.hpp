#ifndef _NODE
#define _NODE 1

/**
	@file include/node.hpp

	@brief Class instrument::node definition and method implementation
*/

#include "./object.hpp"

namespace instrument {

/**
	@brief
		A node in a templated chain (doubly-linked list) or stack (singly-linked
		LIFO queue)

	A node object can be linked to a single node (<b>direct addressing</b>), or to
	two nodes (<b>XOR linking</b>). Class instrument::stack uses singly-linked
	nodes, class instrument::chain is a doubly-linked list. A node can be
	instantiated only through the public methods of a chain or a stack object. A
	node can point to data of any non-scalar type (intrinsic or user defined).
	When a node is released it also calls delete (not delete[]) on its data
	pointer, unless it's previously detached. When a node is copied or assigned,
	only its data are copied, using T(const T&) or T::operator=(const T&),
	exceptions thrown from these methods are not handled, they are propagated up
	the call stack

	@see instrument::chain
	@see instrument::stack
*/
template <class T>
class node: virtual public object
{
protected:

	/* Protected variables */

	T	*m_data;						/**< @brief Node data */

	node *m_link;					/**< @brief Next node link (direct or XOR link) */


	/* Friend classes and functions */

	template <class F> friend class chain;

	template <class F> friend class stack;


	/* Protected constructors, copy constructors and destructor */

	explicit node(T*);

	node(const node&);

	virtual ~node();

	virtual node* clone() const;


	/* Protected accessor methods */

	virtual T* detach();

	virtual node* link(const node* = NULL) const;

	virtual node& link_to(const node*);

	virtual node& unlink_from(const node*);


	/* Protected operator overloading methods */

	virtual node& operator=(const node&);

	virtual node* operator^(const node&) const;
};


/**
 * @brief Object constructor
 *
 * @param[in] d the node data pointer
 *
 * @note The object is not added to a container
 */
template <class T>
inline node<T>::node(T *d):
m_data(d),
m_link(NULL)
{
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 *
 * @note The object is not added to a container
 */
template <class T>
inline node<T>::node(const node &src):
m_data(NULL),
m_link(NULL)
{
	if ( likely(src.m_data != NULL) ) {
		m_data = new T(*src.m_data);
	}
}


/**
 * @brief Object destructor
 *
 * @note
 *	When the object is released memory pointed by m_data is released also, using
 *	operator delete
 */
template <class T>
inline node<T>::~node()
{
	delete m_data;
	m_data = NULL;
	m_link = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
template <class T>
inline node<T>* node<T>::clone() const
{
	return new node(*this);
}


/**
 * @brief Detach the data pointer from the node
 *
 * @returns the detached data pointer
 */
template <class T>
inline T* node<T>::detach()
{
	T *retval = m_data;
	m_data = NULL;
	return retval;
}


/**
 * @brief Get the next node (using direct or XOR linking)
 *
 * @param[in] prev
 *	the previous node, from either direction (NULL for direct linking)
 *
 * @returns the next node
 */
template <class T>
inline node<T>* node<T>::link(const node<T> *prev) const
{
	mem_addr_t addr = reinterpret_cast<mem_addr_t> (m_link);
	mem_addr_t mask = reinterpret_cast<mem_addr_t> (prev);
	return reinterpret_cast<node<T>*> (addr ^ mask);
}


/**
 * @brief Link with a node (for XOR linking)
 *
 * @param[in] n the node to link to (no-op if NULL)
 *
 * @returns *this
 */
template <class T>
inline node<T>& node<T>::link_to(const node<T> *n)
{
	m_link = link(n);
	return *this;
}


/**
 * @brief Unlink from a node (for XOR linking)
 *
 * @param[in] n the node to unlink from (no-op if NULL)
 *
 * @returns *this
 */
template <class T>
inline node<T>& node<T>::unlink_from(const node<T> *n)
{
	m_link = link(n);
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
 *
 * @note Only the node data are copied
 */
template <class T>
node<T>& node<T>::operator=(const node &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	const T *data = rval.m_data;
	if ( unlikely(data == NULL) ) {
		delete m_data;
		m_data = NULL;
	}
	else if ( unlikely(m_data == NULL) ) {
		m_data = new T(*data);
	}
	else {
		*m_data = *data;
	}

	return *this;
}


/**
 * @brief Get the next node (XOR linking)
 *
 * @param[in] prev the previous node, from either direction
 *
 * @returns the next node
 */
template <class T>
inline node<T>* node<T>::operator^(const node<T> &prev) const
{
	return link(&prev);
}

}

#endif
