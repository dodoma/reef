#ifndef __MOS_MACH_H__
#define __MOS_MACH_H__

#ifdef MOS_OSX
#include <pthread.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>            /* getifaddrs */
#include <netdb.h>              /* getnameinfo */

#include <mach/clock.h>
#include <mach/mach.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#define MSG_NOSIGNAL SO_NOSIGPIPE

#define FMT_INT64  "%lld"
#define FMT_UINT64 "%llu"
#define FMT_SIZE   "%zu"
#endif

#endif
