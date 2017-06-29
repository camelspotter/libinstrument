CMAKE_MINIMUM_REQUIRED(VERSION 3.8)


# Project definition

PROJECT(instrument C CXX)

SET(${PROJECT_NAME}_VERSION_MAJOR 1)

SET(${PROJECT_NAME}_VERSION_MINOR 0)

SET(CMAKE_INSTALL_PREFIX	/usr/local)

IF(LINUX) 
  OPTION(_REENTRANT) 
	
# Source and header files listings

SET(SRC_ROOT src)

SET(HDR_ROOT include)

SET(SOURCES
	${SRC_ROOT}/call.cpp
	${SRC_ROOT}/chain.cpp
	${SRC_ROOT}/dictionary.cpp
	${SRC_ROOT}/exception.cpp
	${SRC_ROOT}/file.cpp
	${SRC_ROOT}/filter.cpp
	${SRC_ROOT}/list.cpp
	${SRC_ROOT}/node.cpp
	${SRC_ROOT}/object.cpp
	${SRC_ROOT}/parser.cpp
	${SRC_ROOT}/plugin.cpp
	${SRC_ROOT}/process.cpp
	${SRC_ROOT}/stack.cpp
	${SRC_ROOT}/stream.cpp
	${SRC_ROOT}/string.cpp
	${SRC_ROOT}/stty.cpp
	${SRC_ROOT}/style.cpp
	${SRC_ROOT}/symbol.cpp
	${SRC_ROOT}/symtab.cpp
	${SRC_ROOT}/tcp_socket.cpp
	${SRC_ROOT}/thread.cpp
	${SRC_ROOT}/tracer.cpp
	${SRC_ROOT}/util.cpp
)

SET(HEADERS
	${HDR_ROOT}/call.hpp
	${HDR_ROOT}/chain.hpp
	${HDR_ROOT}/config.hpp
	${HDR_ROOT}/config/config_definitions.hpp
	${HDR_ROOT}/config/config_globals.hpp
	${HDR_ROOT}/config/config_headers.hpp
	${HDR_ROOT}/config/config_macros.hpp
	${HDR_ROOT}/config/config_types.hpp
	${HDR_ROOT}/dictionary.hpp
	${HDR_ROOT}/exception.hpp
	${HDR_ROOT}/file.hpp
	${HDR_ROOT}/filter.hpp
	${HDR_ROOT}/list.hpp
	${HDR_ROOT}/node.hpp
	${HDR_ROOT}/object.hpp
	${HDR_ROOT}/parser.hpp
	${HDR_ROOT}/plugin.hpp
	${HDR_ROOT}/process.hpp
	${HDR_ROOT}/stack.hpp
	${HDR_ROOT}/stream.hpp
	${HDR_ROOT}/string.hpp
	${HDR_ROOT}/stty.hpp
	${HDR_ROOT}/style.hpp
	${HDR_ROOT}/symbol.hpp
	${HDR_ROOT}/symtab.hpp
	${HDR_ROOT}/tcp_socket.hpp
	${HDR_ROOT}/thread.hpp
	${HDR_ROOT}/tracer.hpp
	${HDR_ROOT}/util.hpp
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)


# Fixed -D options

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC
	-D_REENTRANT							# thread safety
	-DWITH_COLOR_ASSERTIONS		# color-coded assertions
	-DWITH_COLOR_TERM					# support color terminals (VT100)
	-DWITH_DEBUG							# include debugging code
	-DWITH_FILTER							# support instrumentation filters
	-DWITH_HIGHLIGHT					# support C++ trace syntax highlighting
	-DWITH_PLUGIN							# support instrumentation plugins
	-DWITH_STREAM							# support buffered output streams
	-DWITH_UNRESOLVED					# print unresolved symbols in traces

	-DWITH_SYMBOL_ENUMERATION
	-DWITH_STREAM_FILE
	-DWITH_STREAM_STTY
	-DWITH_STREAM_TCP
)


# -f options

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC
	-fno-enforce-eh-specs
	-fPIC
	-fstrict-aliasing
)


# Generic options

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC
	-O2
	-march=native
	-rdynamic
	-std=gnu++0x
)


# -W options

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC
	-Wall
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
	-Wtype-limits
)


# Installation rules

INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

INSTALL(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

install(PROGRAMS extra/vtcolors DESTINATION bin)

install(FILES extra/extensions.dict extra/keywords.dict extra/types.dict DESTINATION etc)


GET_PROPERTY(DEFS TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS)
MESSAGE(STATUS "Definitions " ${DEFS})

GET_PROPERTY(FLAGS TARGET ${PROJECT_NAME} PROPERTY COMPILE_OPTIONS)
MESSAGE(STATUS "Flags " ${FLAGS})
