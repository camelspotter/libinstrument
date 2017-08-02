CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

INCLUDE(include.cmake/util.cmake)

INCLUDE(include.cmake/eclipse.cmake)


# Environment variables

SET(CMAKE_C_COMPILER /usr/bin/gcc CACHE FILEPATH "C compiler" FORCE)

SET(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE FILEPATH "C++ compiler" FORCE)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "compiler invocations" FORCE)

SET(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "make output enable" FORCE)


# Project definition

PROJECT(instrument C CXX)

DESCRIBE_HOST()

DESCRIBE_TARGET()

IS_SUPPORTED()

SET(${PROJECT_NAME}_VERSION_MAJOR 1)

SET(${PROJECT_NAME}_VERSION_MINOR 0)

SET(CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "installation prefix" FORCE)


# Fixed primary option definitions

OPTION(_REENTRANT "thread safety" ON)

OPTION(WITH_COLOR_ASSERTIONS "color-coded assertions" ON)

OPTION(WITH_COLOR_TERM "support color terminals (VT100)" OFF)

OPTION(WITH_DEBUG "include debugging code" ON)

OPTION(WITH_UNRESOLVED "print unresolved symbols in traces" ON)


# Fixed secondary option definitions

OPTION(WITH_FILTER "support instrumentation filters" ON)

OPTION(WITH_HIGHLIGHT "support C++ trace syntax highlighting" ON)

OPTION(WITH_PLUGIN "support instrumentation plugins" ON)

OPTION(WITH_STREAM "support buffered output streams" ON)


# Dynamic option definitions

IF(WITH_DEBUG)

	OPTION(WITH_SYMBOL_ENUMERATION "enumerate loaded symbol tables" ON)

	SET(WITH_DEBUG_LEVEL DBGL_HIGH)

ENDIF(WITH_DEBUG)

IF(WITH_STREAM)

	OPTION(WITH_STREAM_FILE "support buffered file output streams" ON)

	OPTION(WITH_STREAM_STTY "support buffered serial tty output streams" ON)

	OPTION(WITH_STREAM_TCP "support buffered TCP/IP socket output streams" ON)

ENDIF(WITH_STREAM)


# Source and header directory listings

SET(SRC_ROOT src)

SET(HDR_ROOT include)

SET(HDR_IN_ROOT ${HDR_ROOT}.in)


# Source file listings

SET(SOURCES
	${SRC_ROOT}/call.cpp

	${SRC_ROOT}/chain.cpp

	${SRC_ROOT}/exception.cpp

	${SRC_ROOT}/list.cpp

	${SRC_ROOT}/node.cpp

	${SRC_ROOT}/object.cpp

	${SRC_ROOT}/process.cpp

	${SRC_ROOT}/stack.cpp

	${SRC_ROOT}/string.cpp

	${SRC_ROOT}/symbol.cpp

	${SRC_ROOT}/symtab.cpp

	${SRC_ROOT}/thread.cpp

	${SRC_ROOT}/tracer.cpp

	${SRC_ROOT}/util.cpp
)

IF(WITH_FILTER)

	SET(SOURCES ${SOURCES} ${SRC_ROOT}/filter.cpp)

ENDIF(WITH_FILTER)

IF(WITH_HIGHLIGHT)

	SET(SOURCES
			${SOURCES}

			${SRC_ROOT}/dictionary.cpp

			${SRC_ROOT}/parser.cpp

			${SRC_ROOT}/style.cpp
	)

ENDIF(WITH_HIGHLIGHT)

IF(WITH_PLUGIN)

	SET(SOURCES ${SOURCES} ${SRC_ROOT}/plugin.cpp)

ENDIF(WITH_PLUGIN)

IF(WITH_STREAM)

	SET(SOURCES ${SOURCES} ${SRC_ROOT}/stream.cpp)

	IF(WITH_STREAM_FILE)

		SET(SOURCES ${SOURCES} ${SRC_ROOT}/file.cpp)

	ENDIF(WITH_STREAM_FILE)

	IF(WITH_STREAM_STTY)

		SET(SOURCES ${SOURCES} ${SRC_ROOT}/stty.cpp)

	ENDIF(WITH_STREAM_STTY)

	IF(WITH_STREAM_TCP)

		SET(SOURCES ${SOURCES} ${SRC_ROOT}/tcp_socket.cpp)

	ENDIF(WITH_STREAM_TCP)

ENDIF(WITH_STREAM)


# Header file listings

SET(HEADERS
	${HDR_ROOT}/call.hpp

	${HDR_ROOT}/chain.hpp

	${HDR_ROOT}/config.hpp

	${HDR_ROOT}/config/config_definitions.hpp

	${HDR_ROOT}/config/config_globals.hpp

	${HDR_ROOT}/config/config_headers.hpp

	${HDR_ROOT}/config/config_macros.hpp

	${HDR_ROOT}/config/config_types.hpp

	${HDR_ROOT}/exception.hpp

	${HDR_ROOT}/list.hpp

	${HDR_ROOT}/node.hpp

	${HDR_ROOT}/object.hpp

	${HDR_ROOT}/process.hpp

	${HDR_ROOT}/stack.hpp

	${HDR_ROOT}/string.hpp

	${HDR_ROOT}/symbol.hpp

	${HDR_ROOT}/symtab.hpp

	${HDR_ROOT}/thread.hpp

	${HDR_ROOT}/tracer.hpp

	${HDR_ROOT}/util.hpp
)

IF(WITH_FILTER)

	SET(HEADERS ${HEADERS} ${HDR_ROOT}/filter.hpp)

ENDIF(WITH_FILTER)

IF(WITH_HIGHLIGHT)

	SET(HEADERS
			${HEADERS}

			${HDR_ROOT}/dictionary.hpp

			${HDR_ROOT}/parser.hpp

			${HDR_ROOT}/style.hpp
	)

ENDIF(WITH_HIGHLIGHT)

IF(WITH_PLUGIN)

	SET(HEADERS ${HEADERS} ${HDR_ROOT}/plugin.hpp)

ENDIF(WITH_PLUGIN)

IF(WITH_STREAM)

	SET(HEADERS ${HEADERS} ${HDR_ROOT}/stream.hpp)

	IF(WITH_STREAM_FILE)

		SET(HEADERS ${HEADERS} ${HDR_ROOT}/file.hpp)

	ENDIF(WITH_STREAM_FILE)

	IF(WITH_STREAM_STTY)

		SET(HEADERS ${HEADERS} ${HDR_ROOT}/stty.hpp)

	ENDIF(WITH_STREAM_STTY)

	IF(WITH_STREAM_TCP)

		SET(HEADERS ${HEADERS} ${HDR_ROOT}/tcp_socket.hpp)

	ENDIF(WITH_STREAM_TCP)

ENDIF(WITH_STREAM)


# Target definition

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)


# -D options (defines)

GET_PROPERTY(OPTS DIRECTORY ./ PROPERTY VARIABLES)

FOREACH(OPT ${OPTS})

	IF("${OPT}" MATCHES "WITH_" AND ${OPT})

		TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC ${OPT})

	ENDIF("${OPT}" MATCHES "WITH_" AND ${OPT})

ENDFOREACH(OPT)


# -f options

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC
	-fno-enforce-eh-specs

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


# Configuration rules

CONFIGURE_FILE(${HDR_IN_ROOT}/config_definitions.hpp.in ${PROJECT_SOURCE_DIR}/${HDR_ROOT}/config/config_definitions.hpp)

CONFIGURE_FILE(${HDR_IN_ROOT}/config_globals.hpp.in ${PROJECT_SOURCE_DIR}/${HDR_ROOT}/config/config_globals.hpp)

CONFIGURE_FILE(${HDR_IN_ROOT}/instrument.hpp.in ${HDR_IN_ROOT}/instrument.hpp)


# Installation rules

INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

INSTALL(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

INSTALL(PROGRAMS bin/vtcolors DESTINATION bin)

INSTALL(FILES ${PROJECT_BINARY_DIR}/${HDR_IN_ROOT}/instrument.hpp DESTINATION include)


LIST_FILES(etc/*.dict DICTIONARIES)

INSTALL(FILES ${DICTIONARIES} DESTINATION etc)


# Store final variable set

LIST_VARIABLES(${PROJECT_BINARY_DIR}/${PROJECT_NAME}_variables.txt)


# Install back in sources the installation manifests

INSTALL(FILES ${PROJECT_BINARY_DIR}/compile_commands.json DESTINATION ${PROJECT_SOURCE_DIR}/doc/)

# todo post-install
# INSTALL(FILES ${PROJECT_BINARY_DIR}/install_manifest.txt DESTINATION ${PROJECT_SOURCE_DIR}/doc/)

INSTALL(FILES ${PROJECT_BINARY_DIR}/instrument_variables.txt DESTINATION ${PROJECT_SOURCE_DIR}/doc/)
