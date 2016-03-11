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

#include <pthread.h>

#include <math.h>


/*
 * reef triggerfish headers
 */
#include "mos.h"
#include "mcolor.h"

#include "mtype.h"
#include "mtest.h"
#include "mtimer.h"

#include "merror.h"
#include "merror_macro.h"

#include "mtrace.h"
#include "mtrace_macro.h"

#include "mstring.h"

#include "mlist.h"
#include "mlist_macro.h"

#include "mhash.h"
#include "mhash_macro.h"

#include "mdf.h"
#include "mdf_macro.h"

#include "mcv.h"
#include "mcv_macro.h"

#endif
