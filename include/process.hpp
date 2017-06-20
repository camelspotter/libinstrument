#ifndef _PROCESS
#define _PROCESS 1

/**
	@file include/process.hpp

	@brief Class instrument::process definition
*/

#include "./symtab.hpp"
#include "./thread.hpp"

namespace instrument {

/**
	@brief This class represents a process, its entire namespace and thread group

	An object of this class is an abstraction of the actual running process. It
	stores the whole instrumented namespace and the details of all the simulated
	threads and their stacks. The namespace consists of a number of symbol tables,
	one for each objective code module (executable and selected DSO libraries). A
	process object offers methods to perform batch symbol lookups, inverse lookups
	(given a resolved symbol find the module that defines it) and thread handling.
	Access to the process object <b>is not thread safe</b>
*/
class process: virtual public object
{
protected:

	/* Protected variables */

	pid_t m_pid;												/**< @brief Process ID */

	list<symtab> *m_symtabs;						/**< @brief Symbol table list */

	list<thread> *m_threads;						/**< @brief Instrumented thread list */

public:

	typedef void*	thread_arg_t;

	typedef void* (*thread_main_t)(thread_arg_t);


	/* Constructors, copy constructors and destructor */

	process();

	process(const process&);

	virtual	~process();

	virtual process* clone() const;


	/* Accessor methods */

	virtual pid_t pid() const;


	/* Operator overloading methods */

	virtual process& operator=(const process&);


	/* Generic methods */

	virtual process& fork_thread(const i8*, thread_main_t, thread_arg_t);

	virtual process& thread_cancel(thread*);

	virtual process& thread_join(thread*, void* = NULL);


	/* Module (symtab) handling methods */

	virtual process& add_module(const i8*, mem_addr_t);

	virtual const i8* ilookup(mem_addr_t, mem_addr_t&) const;

	virtual const i8* lookup(mem_addr_t) const;

	virtual u32 module_count() const;

	virtual u32 symbol_count() const;


	/* Thread handling methods */

	virtual process& cleanup_thread(pthread_t);

	virtual process& cleanup_zombie_threads();

	virtual thread* current_thread() const;

	virtual thread* get_thread(pthread_t) const;

	virtual thread* get_thread(const i8*) const;

	virtual thread* get_thread(u32) const;

	virtual u32 thread_count() const;
};

}

#endif
