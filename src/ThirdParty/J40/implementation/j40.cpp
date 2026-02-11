#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(_POSIX_C_SOURCE)
#undef _POSIX_C_SOURCE
#endif
#if defined(_XOPEN_SOURCE)
#undef _XOPEN_SOURCE
#endif
#if defined(_ISOC11_SOURCE)
#undef _ISOC11_SOURCE
#endif

#if defined(_MSC_VER)
#if (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#if (_MSC_VER < 1800)
#include <float.h>
#include <math.h>
static inline double cbrt(double arg) { return pow(arg, 1.0 / 3.0); }
static inline float cbrtf(float arg) { return powf(arg, 1.0f / 3.0f); }
static inline bool isfinite(double num) { return !!_finite(num); }
#endif
#endif

#define J40_CONFIRM_THAT_THIS_IS_EXPERIMENTAL_AND_POTENTIALLY_UNSAFE
#define J40_IMPLEMENTATION
#include <j40.h>
