#ifndef __MOS_H__
#define __MOS_H__

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

#define mos_free(p)                             \
    do {                                        \
        if ((p) != NULL) {                      \
            free(p);                            \
            (p) = NULL;                         \
        }                                       \
    } while (0)

void* mos_malloc(size_t size);
void* mos_calloc(size_t nmemb, size_t size);
void* mos_realloc(void *ptr, size_t size);

/*
 * return rand integer between [0, max)
 */
int mos_rand(int max);

double mos_timef();

static inline void mos_utc_time(struct timespec *ts)
{
#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif
}


__END_DECLS
#endif
