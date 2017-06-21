#ifndef _PLUGIN
#define _PLUGIN 1

/**
	@file include/plugin.hpp

	@brief Class instrument::plugin definition
*/

#include "./object.hpp"

namespace instrument {

/**
	@brief Function instrumentation plugin

	A plugin object is the way to declare a pair of instrumentation functions and
	register them with libinstrument to be run upon function call and return. A
	plugin can be created by passing it the addresses of the instrumentation
	functions or by loading a plugin <b>DSO</b> module that implements and exports
	these two functions. A plugin invokes the system dynamic linker to find, load
	and link the module, it must reside in one of the linker search directories.

	The names of the instrumentation functions (<b>__cyg_profile_func_enter</b>
	and <b>__cyg_profile_func_exit</b>) are <b>hard-coded into g++</b>, so only a
	unique implementation of these can exist at linkage time, therefore only one
	function profiler can be used at a time. The plugin class is the fix to this
	<b>g++ shortcoming</b>.

	The plugin class supports both <b>C and C++ ABIs</b>. To resolve C++ functions
	the user must pass the symbol name and its full scope (as a separate argument
	in the form namespace::class). To resolve C functions omit the scope argument
	altogether. The module callback functions must be named <b>mod_enter</b> and
	<b>mod_exit</b>, take two <b>void*</b> arguments and return <b>void</b> for
	the plugin object to resolve them correctly. All plugin functions (as all
	libinstrument functions) must <b>NOT</b> be instrumented by libinstrument, as
	this will result in a recurse and a stack overflow

	@see tracer::register_plugin, tracer::unregister_plugin
	@see
		<a href="index.html#sec5_6">
			<b>5.6 Using the instrumentation plugin API</b>
		</a>
*/
class plugin: virtual public object
{
protected:

	/* Protected variables */

	modsym_t m_begin;					/**< @brief Instrumentation starting callback */

	modsym_t m_end;						/**< @brief Instrumentation ending callback */

	dso_t m_handle;						/**< @brief DSO handle (as provided by dlopen) */

	i8 *m_path;								/**< @brief Module file path */


	/* Protected generic methods */

	virtual plugin& destroy();

public:

	/* Constructors, copy constructors and destructor */

	explicit plugin(const i8*, const i8* = NULL);

	plugin(modsym_t, modsym_t);

	plugin(const plugin&);

	virtual	~plugin();

	virtual plugin* clone() const;


	/* Accessor methods */

	virtual const i8* path() const;


	/* Operator overloading methods */

	virtual plugin& operator=(const plugin&);


	/* Generic methods */

	virtual plugin& begin(void*, void*) const;

	virtual plugin& end(void*, void*) const;

	virtual modsym_t resolve(const i8*, const i8* = NULL) const;
};

}

#endif
