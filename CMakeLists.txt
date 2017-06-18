CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(project_name instrument)
SET(CMAKE_CXX_COMPILER	/usr/bin/g++)
SET(CMAKE_C_COMPILER	/usr/bin/gcc)

PROJECT(${project_name}			C CXX)
SET(${project_name}_VERSION_MAJOR	1)
SET(${project_name}_VERSION_MINOR	0)

SET(CMAKE_BINARY_DIR		${CMAKE_SOURCE_DIR}/.build)
SET(CMAKE_INSTALL_PREFIX	/usr/local)

# Fixed -D options
ADD_DEFINITIONS(-D_REENTRANT			# thread safety
		-DWITH_COLOR_ASSERTIONS		# color-coded assertions
		-DWITH_COLOR_TERM		# support color terminals (VT100)
		-DWITH_DEBUG			# include debugging code
		-DWITH_FILTER			# support instrumentation filters
		-DWITH_HIGHLIGHT		# support C++ trace syntax highlighting
		-DWITH_PLUGIN			# support instrumentation plugins
		-DWITH_STREAM			# support buffered output streams
		-DWITH_UNRESOLVED)		# print unresolved symbols in traces

# -f options
ADD_DEFINITIONS(-fno-enforce-eh-specs
		-fPIC
		-fstrict-aliasing)

# Generic options
ADD_DEFINITIONS(-O2
		-march=native
		-rdynamic
		-std=gnu++0x)

# -W options
ADD_DEFINITIONS(-Wall
		-Wabi
		-Wcast-align
		-Wcast-qual
		-Wclobbered
		-Wctor-dtor-privacy
		-Wdisabled-optimization
		-Wempty-body
		-Wformat-security
		-Winit-self
		-Wlogical-op
		-Wmissing-field-initializers
		-Wmissing-include-dirs
		-Wmissing-noreturn
		-Wnon-virtual-dtor
		-Woverlength-strings
		-Wpacked
		-Wredundant-decls
		-Wsign-compare
		-Wswitch-enum
		-Wtype-limits)

SET(src_root	src)
SET(hdr_root	include)

SET(sources	${src_root}/call.cpp
		${src_root}/chain.cpp
		${src_root}/exception.cpp
		${src_root}/list.cpp
		${src_root}/node.cpp
		${src_root}/object.cpp
		${src_root}/stack.cpp
		${src_root}/string.cpp
		${src_root}/symbol.cpp
		${src_root}/util.cpp)

SET(headers	${hdr_root}/call.hpp
		${hdr_root}/chain.hpp
		${hdr_root}/config.hpp
		${hdr_root}/config/config_headers.hpp
		${hdr_root}/exception.hpp
		${hdr_root}/list.hpp
		${hdr_root}/node.hpp
		${hdr_root}/object.hpp
		${hdr_root}/stack.hpp
		${hdr_root}/string.hpp
		${hdr_root}/symbol.hpp
		${hdr_root}/util.hpp)
                  
ADD_LIBRARY(${project_name} SHARED ${sources} ${headers})

MESSAGE(STATUS "Build system generated")
		