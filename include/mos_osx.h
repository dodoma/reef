#ifndef __MOS_MACH_H__
#define __MOS_MACH_H__

#ifdef MOS_OSX
#include <pthread.h>

#include <mach/clock.h>
#include <mach/mach.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#endif
