CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(libinstrument C CXX)

#set(CMAKE_CXX_COMPILER		/usr/bin/g++)
#set(CMAKE_C_COMPILER		/usr/bin/gcc)

set(test_SOURCES test.cpp)

SET(sources	src/object.cpp
		src/util.cpp)

SET(headers	include/config/config_headers.hpp
		include/config.hpp
		include/object.hpp
		include/util.hpp)

ADD_EXECUTABLE(test ${test_SOURCES})

ADD_LIBRARY(libinstrument.so SHARED ${sources} ${headers})

# include()
#find_package(Boost REQUIRED)

#add_library(boost INTERFACE IMPORTED)
# set_property(TARGET boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
#find_package(MyOtherLibrary REQUIRED)

#enable_testing()
#add_subdirectory(liblib)
#add_subdirectory(app)