// SPDX-FileCopyrightText: 2023, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#define LIBCWD_VERSION_2
#include "sys.h"
#include "debug.h"

// UsageDetector requires -std=c++2b or higher.
#if __cplusplus >= 202101L

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
Channel usage_detector("USAGE");
NAMESPACE_DEBUG_CHANNELS_END
#endif

#endif
