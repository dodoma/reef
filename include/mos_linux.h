#ifndef __MOS_LINUX_H__
#define __MOS_LINUX_H__

#ifdef MOS_LINUX
#include <pthread.h>
#include <fcntl.h>
#include <syscall.h>

#include <net/if.h>             /* struct ifreq */
#include <sys/ioctl.h>          /* ioctl */

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>            /* getifaddrs */
#include <netdb.h>              /* getnameinfo */

#define FMT_INT64  "%jd"
#define FMT_UINT64 "%ju"
#define FMT_SIZE   "%zu"
#endif

#endif
