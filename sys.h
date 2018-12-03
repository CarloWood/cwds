#ifdef CWDEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libcwd/sys.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define UNUSED_ARG(x)

#include <bits/c++config.h>
#ifndef __always_inline
#error "__always_inline not defined by bits/c++config.h with this compiler."
#endif
