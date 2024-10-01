#include <chrono>

struct ms_timer
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    [[nodiscard]] std::chrono::milliseconds clock()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    }
};
