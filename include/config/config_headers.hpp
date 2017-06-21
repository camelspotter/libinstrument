#ifndef _CONFIG_HEADERS
#define _CONFIG_HEADERS 1

/**
	@file include/config_headers.hpp

	@brief Library header includes
*/


#include <iostream>
#include <typeinfo>

#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

#include <cxxabi.h>
#include <bfd.h>
#include <link.h>
#include <pthread.h>
#include <regex.h>
#include <sys/stat.h>

#ifdef WITH_HIGHLIGHT
#include <fcntl.h>
#include <sys/mman.h>
#endif

#ifdef WITH_STREAM
#include <sys/file.h>
#include <sys/time.h>

#ifdef WITH_STREAM_STTY
#include <termios.h>
#endif

#ifdef WITH_STREAM_TCP
//#include <arpa/inet.h>
//#include <sys/socket.h>
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
