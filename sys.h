#ifndef LIBCWD_THREAD_SAFE
// When using libcwd version 1 LIBCWD_THREAD_SAFE should be defined because it
// makes no sense to use the non-threading version of libcwd.
// While libcwd version 2 does not define LIBCWD_THREAD_SAFE anymore; it is
// always thread-safe anyway. Therefore we can use this macro to detect if
// we're using libcwd version 1 or 2.
#define LIBCWD_VERSION_2
#endif

#ifndef LIBCWD_VERSION_2
#ifdef CWDEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libcwd/sys.h>
#endif
#endif // LIBCWD_VERSION_2

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Used when building with cmake, because cmake isn't using a project-root config.h for its definitions.
// Currently only the submodules utils, threadpool and fastprimes have a config.h (todo: automate this too).
#ifdef HAVE_UTILS_CONFIG_H
#include <utils/config.h>
#endif
#ifdef HAVE_THREADPOOL_CONFIG_H
#include <threadpool/config.h>
#endif
#ifdef HAVE_FASTPRIMES_CONFIG_H
#include <fastprimes/config.h>
#endif
#ifdef HAVE_CAIROWINDOW_CONFIG_H
#include <cairowindow/config.h>
#endif

#define UNUSED_ARG(x)
