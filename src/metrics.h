#include <atomic>
#include <cstddef>

extern thread_local size_t LOCAL_INSCT_CNTR;
extern std::atomic<size_t> INSCT_CNTR;
