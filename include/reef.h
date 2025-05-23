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
#ifdef USE_FCGI
#include <fcgi_stdio.h>
#endif
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
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include <ftw.h>
#include <math.h>

/*
 * reef triggerfish headers
 */
#include "mos_linux.h"
#include "mos_osx.h"
#include "mos_esp.h"
#include "mos.h"

#include "utf.h"

#include "mcolor.h"
#include "mbase64.h"

#include "mtype.h"
#include "mtest.h"
#include "mtimer.h"

#include "merror.h"
#include "merror_macro.h"

#include "mmime.h"
#include "mtrace.h"
#include "mtrace_macro.h"
#include "mtrace_multithread.h"
#include "mtrace_multithread_macro.h"
#include "mtrace_multimodule.h"
#include "mtrace_multimodule_macro.h"

#include "mstring.h"
#include "mbuf.h"

#include "mlist.h"
#include "mlist_macro.h"
#include "mslist.h"
#include "mdlist.h"
#include "mtree.h"
#include "mutree.h"
#include "mcheck.h"

#include "mhash.h"
#include "mhash_macro.h"

#include "mdf.h"
#include "mcs.h"
#include "mdf_data.h"
#include "mdf_aux.h"
#include "mdf_macro.h"

#include "mmemcache.h"

#include "mregexp.h"

#include "mhttp.h"
#include "mhttp_ext.h"
#include "mcgi.h"


/*
 * other useful software headers
 */

#endif
