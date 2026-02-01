/*
** SPDX-License-Identifier: BSD-3-Clause
** Copyright Contributors to the OpenEXR Project.
*/

#ifndef OPENEXR_PRIVATE_THREAD_H
#define OPENEXR_PRIVATE_THREAD_H

#include "openexr_config.h"

// Thread-safe single initiatization, using InitOnceExecuteOnce on Windows,
// pthread_once elsewhere, or a simple variable if threading is completely disabled.
#if ILMTHREAD_THREADING_ENABLED
#    ifdef _WIN32
#        include <windows.h>
#        define ONCE_FLAG_INIT 0
typedef LONG volatile once_flag;
static inline void
call_once (once_flag* flag, void (*func) (void))
{
    static const LONG ONCE_FLAG_CALLING = ONCE_FLAG_INIT + 1;
    static const LONG ONCE_FLAG_FINISHED = ONCE_FLAG_CALLING + 1;
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
#    else
#        include <pthread.h>
#        define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
typedef pthread_once_t once_flag;
static inline void
call_once (once_flag* flag, void (*func) (void))
{
    (void) pthread_once (flag, func);
}
#    endif
#else
#    define ONCE_FLAG_INIT 0
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
