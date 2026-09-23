/*
** SPDX-License-Identifier: BSD-3-Clause
** Copyright Contributors to the OpenEXR Project.
*/

#ifndef OPENEXR_PRIVATE_THREAD_H
#define OPENEXR_PRIVATE_THREAD_H

#include "openexr_config.h"

#if ILMTHREAD_THREADING_ENABLED
#    ifdef _WIN32
/*
 * On Windows, use native InitOnceExecuteOnce, which avoids MSVC's
 * problematic <threads.h>
 */
#        include <windows.h>
#        define ONCE_FLAG_INIT 0
typedef LONG volatile once_flag;
static inline void
call_once (once_flag* flag, void (*func) (void))
{
    static const LONG ONCE_FLAG_CALLING = ONCE_FLAG_INIT + 1;
    static const LONG ONCE_FLAG_FINISHED = ONCE_FLAG_INIT + 2;
    const LONG res = InterlockedCompareExchange (flag, ONCE_FLAG_CALLING, ONCE_FLAG_INIT);
    if (res == ONCE_FLAG_INIT) {
        func ();
        InterlockedCompareExchange (flag, ONCE_FLAG_FINISHED, ONCE_FLAG_CALLING);
    } else if (res != ONCE_FLAG_FINISHED) {
        do {
            Sleep (1);
        } while (InterlockedCompareExchange (flag, ONCE_FLAG_FINISHED, ONCE_FLAG_FINISHED) != ONCE_FLAG_FINISHED);
    }
}
#    elif __has_include(<threads.h>) && !defined(__FreeBSD__)
/*
 * On Linux (glibc 2.28+), use standard <threads.h>.
 * FreeBSD requires -lstdthreads for <threads.h>; use pthread fallback instead.
 */
#        include <threads.h>

#    else
/*
 * No <threads.h> on macOS and older Linux distros: fall back to pthreads
 */
#        include <pthread.h>
#        define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
typedef pthread_once_t once_flag;
static inline void
call_once (once_flag* flag, void (*func) (void))
{
    (void) pthread_once (flag, func);
}

#    endif

#endif /* ILMTHREAD_THREADING_ENABLED */

/*
 * If threading is disabled, or call_once/ONCE_FLAG_INIT wasn't declared
 * above, declare a default implementation.
 */
#ifndef ONCE_FLAG_INIT
#  define ONCE_FLAG_INIT 0
typedef int once_flag;
static inline void
call_once (once_flag* flag, void (*func) (void))
{
    if (!*flag) {
        *flag = 1;
        func ();
    }
}
#endif


#endif /* OPENEXR_PRIVATE_THREAD_H */
