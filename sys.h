// SPDX-FileCopyrightText: 2016-2019, 2023, 2025-2026 Carlo Wood
// SPDX-License-Identifier: MIT

// Mark that this header was included.
// A guard is not needed: this header should NEVER be included by another header.
#define __CWDS_DEBUG_H_INCLUDED

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
