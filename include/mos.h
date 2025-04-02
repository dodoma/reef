/*
 * Copyright 2025 MaLiang <bigml@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __MOS_H__
#define __MOS_H__

#define IS_BIG_ENDIAN (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)
#ifndef htonll
#define htonll(x) ((1==htonl(1)) ? (x) :                                \
                   ((uint64_t)htonl((uint64_t)(x) & 0xFFFFFFFF) << 32) | htonl((uint64_t)(x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : \
                   ((uint64_t)ntohl((uint64_t)(x) & 0xFFFFFFFF) << 32) | ntohl((uint64_t)(x) >> 32))
#endif

/*
 * mos, operation system wrapper
 */
__BEGIN_DECLS

#define MOS_OFFSET_2_MEM(i)    ((char*)NULL + (i))
#define MOS_MEM_2_OFFSET(addr) ((char*)(addr) - (char*)NULL)

#define mos_free(p)                             \
    do {                                        \
        if ((p) != NULL) {                      \
            free((void*)(p));                   \
            (p) = NULL;                         \
        }                                       \
    } while (0)

void* mos_malloc(size_t size);
void* mos_calloc(size_t nmemb, size_t size);
void* mos_realloc(void *ptr, size_t size);

/*
 * return rand integer between [0, max)
 */
uint32_t mos_rand(uint32_t max);
bool mos_mkdir(char *path, mode_t mode);
bool mos_mkdirf(mode_t mode, char *fmt, ...);

/* return 0 on success, and -1 if an error occurs. */
int mos_rmrf(char *path);
int mos_rmrff(char *fmt, ...);

bool mos_copyfile(const char *src, const char *dest, mode_t mode);

double mos_timef();

static inline void mos_utc_time(struct timespec *ts)
{
#if defined(MOS_OSX)
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#elif defined(MOS_LINUX)
    clock_gettime(CLOCK_REALTIME, ts);
#endif
}


__END_DECLS
#endif
