#ifndef _TRACER
#define _TRACER 1

/**
	@file include/tracer.hpp

	@brief Class instrument::tracer definition
*/

#include "./process.hpp"
#include "./string.hpp"
#ifdef WITH_FILTER
#include "./filter.hpp"
#endif
#ifdef WITH_PLUGIN
#include "./plugin.hpp"
#endif

namespace instrument {

#define ALL				0x01

#define INLINED		0x02

#define DSO				0x04


/**
	@brief
		A tracer object is the default libinstrument interface for instrumentation
		functions and for the library user

	The public interface of the class is used by the instrumentation functions to
	create a call stack simulation for each executing thread. The library user can
	use the interface to produce and output traces for exceptions or for threads.
	The constructors of the class are protected so there is no way for the library
	user to instantiate a tracer object. The library constructor (on_lib_load)
	creates a global static tracer object to be used as interface to the library
	facilities. All public methods are thread safe

	@todo Implement plugin discovery (in system, user and custom directories)
*/
class tracer: virtual public object
{
protected:

	/* Protected static variables */

	static tracer *s_iface;							/**< @brief Interface object */

	static pthread_mutex_t s_lock;			/**< @brief Access mutex */


	/* Protected variables */

#ifdef WITH_FILTER
	list<filter> *m_filters;						/**< @brief Instrumentation filters */
#endif

#ifdef WITH_PLUGIN
	list<plugin> *m_plugins;						/**< @brief Instrumentation plugins */
#endif

	process *m_proc;										/**< @brief Process handle */


	/* Protected static methods */

	static void __cyg_profile_func_enter(void*, void*);

	static void __cyg_profile_func_exit(void*, void*);

	static void __on_lib_load() __attribute((constructor));

	static void __on_lib_unload()	__attribute((destructor));

	static string& addr2line(string&, const i8*, mem_addr_t);

	static i32 on_dso_load(dl_phdr_info*, size_t, void*);


	/* Protected constructors, copy constructors and destructor */

	tracer();

	tracer(const tracer&);

	virtual	~tracer();

	virtual tracer* clone() const;


	/* Protected operator overloading methods */

	virtual tracer& operator=(const tracer&);


	/* Protected generic methods */

	virtual tracer& destroy();

public:

	/* Static methods */

	static tracer* interface();

	static void lock();

	static void unlock();


	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, tracer&);


	/* Accessor methods */

	virtual process* proc() const;


	/* Generic methods */

	/* Trace producing methods */

	virtual tracer& dump(string&) const;

	virtual tracer& trace(string&);

	virtual tracer& trace(string&, pthread_t) const;

	virtual tracer& unwind();


	/* Filter handling methods */

#ifdef WITH_FILTER
	virtual filter* add_filter(const i8*, bool, bool = SYMBOL_FILTER);

	virtual bool apply_module_filters(const i8*);

	virtual bool apply_symbol_filters(const i8*);

	virtual u32 filter_count() const;

	virtual filter* get_filter(u32) const;

	virtual tracer& remove_filter(u32);
#endif


	/* Plugin handling methods */

#ifdef WITH_PLUGIN
	virtual const plugin* add_plugin(const i8*, const i8* = NULL);

	virtual const plugin* add_plugin(modsym_t, modsym_t);

	virtual tracer& begin_plugins(void*, void*) const;

	virtual tracer& end_plugins(void*, void*) const;

	virtual const plugin* get_plugin(const i8*) const;

	virtual const plugin* get_plugin(u32) const;

	virtual u32 plugin_count() const;

	virtual tracer& remove_all_plugins(u32 which);

	virtual tracer& remove_plugin(const i8*);

	virtual tracer& remove_plugin(u32);
#endif
};

}

#endif
