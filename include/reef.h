#ifndef __REEF_H__
#define __REEF_H__

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ > 6
#define ATTRIBUTE_PRINTF(a1,a2) __attribute__((__format__ (__printf__, a1, a2)))
#else
#define ATTRIBUTE_PRINTF(a1,a2)
#endif

#define	__F(func) (*func)

#ifndef PATH_MAX
#define PATH_MAX 256
#endif


/*
 * system headers
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

/*
 * reef triggerfish headers
 */
#include "mos_linux.h"
#include "mos_osx.h"
#include "mos_esp.h"
#include "mos.h"

#include "mcolor.h"
#include "mbase64.h"

#include "mtype.h"
#include "mtest.h"
#include "mtimer.h"

#include "merror.h"
#include "merror_macro.h"

#include "mtrace.h"
#include "mtrace_macro.h"
#include "mtrace_multithread.h"
#include "mtrace_multithread_macro.h"

#include "mstring.h"
#include "mbuf.h"
#include "mhttp.h"

#include "mlist.h"
#include "mlist_macro.h"

#include "mhash.h"
#include "mhash_macro.h"

#include "mdf.h"
#include "mdf_macro.h"
#include "mdf_data.h"

#include "mmemcache.h"

#include "mregexp.h"


/*
 * other useful software headers
 */

#endif
