#ifndef __MOS_LINUX_H__
#define __MOS_LINUX_H__

#ifdef MOS_LINUX
#include <pthread.h>
#include <syscall.h>

#define FMT_INT64  "%jd"
#define FMT_UINT64 "%ju"
#endif

#endif
