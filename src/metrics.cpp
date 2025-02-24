#include "metrics.h"
#include <cstddef>

thread_local size_t LOCAL_INSCT_CNTR = 0;
std::atomic<size_t> INSCT_CNTR = 0;
