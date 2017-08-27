#ifndef _THREAD
#define _THREAD 1

/**
	@file include/thread.hpp

	@brief Class instrument::thread definition
*/

#include "./call.hpp"
#include "./stack.hpp"

namespace instrument {

/**
	@brief This class represents a thread of execution in the instrumented process

	An object of this class is an abstraction of the actual threading system. It
	stores the simulated call stack and other thread specific data and it is used
	to track a thread execution. The simulated call stack can be traversed using
	simple callbacks and method thread::each. Currently only POSIX threads are
	supported. As each thread object manipulates thread specific data, it can be
	considered thread safe

	@todo Use std::thread (C++11) class for portability
	@todo Store the entry method (to detect thread exit)
*/
class thread: virtual public object
{
protected:

	/* Protected variables */

	pthread_t m_handle;					/**< @brief Thread handle */

	i32 m_lag;									/**< @brief
																	 The number of calls that must be popped off
																	 the simulated stack for it to match the real
																	 one */

	i8 *m_name;									/**< @brief Thread name */

	stack<call> *m_stack;				/**< @brief Simulated call stack */

	thread_status_t m_status;		/**< @brief Running status */


	/* Protected generic methods */

	virtual thread& attach_to_process();

	virtual thread& detach_from_process();

public:

	typedef void (*callback_t)(u32, call*);


	/* Static methods */

	static thread* fork(const i8*, thread_main_t, thread_arg_t);


	/* Constructors, copy constructors and destructor */

	explicit thread(const i8* = NULL);

	thread(pthread_t, const i8*);

	thread(const thread&);

	virtual	~thread();

	virtual thread* clone() const;


	/* Accessor methods */

	virtual	pthread_t handle() const;

	virtual i32 lag() const;

	virtual const i8* name() const;

	virtual thread& set_name(const i8*);

	virtual thread_status_t status() const;


	/* Operator overloading methods */

	virtual thread& operator=(const thread&);

	virtual const call* operator[](u32) const;


	/* Generic methods */

	virtual const call* backtrace(u32) const;

	virtual u32 call_depth() const;

	virtual thread& called(mem_addr_t, mem_addr_t, const i8* = NULL);

	virtual thread& cancel();

	virtual thread& each(const callback_t) const;

	virtual bool is(pthread_t) const;

	virtual bool is(const i8*) const;

	virtual bool is_current() const;

	virtual thread& join(void* = NULL);

	virtual thread& returned();

	virtual thread& unwind();
};

}

#endif
