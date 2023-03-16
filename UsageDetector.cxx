#include "sys.h"
#include "debug.h"

// UsageDetector requires -std=c++2b or higher.
#if __cplusplus >= 202101L

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct usage_detector("USAGE");
NAMESPACE_DEBUG_CHANNELS_END
#endif

#endif
