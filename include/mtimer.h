#ifndef __MTIMER_H__
#define __MTIMER_H__

/*
 * ***attention*** NOT TRHEAD SAFE
 *
 * mtimer, A simple timer for measuring delays.
 * Alberto Bertogli (albertito@blitiri.com.ar) - September/2006
 *
 * Nested timers call are supported (max 64 nests). The result is in usecs.
 *
 * 1000000 usecs == 1 sec
 */
__BEGIN_DECLS

void mtimer_start(void);
unsigned long mtimer_stop(const char *fmt, ...) ATTRIBUTE_PRINTF(1, 2);

__END_DECLS
#endif
