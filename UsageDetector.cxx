#include "sys.h"
#include "debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct usage_detector("USAGE");
NAMESPACE_DEBUG_CHANNELS_END
#endif
