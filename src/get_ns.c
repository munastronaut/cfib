#include "common.h"

uint64_t get_ns() {
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    static int init = 0;
    if (!init) {
        QueryPerformanceFrequency(&frequency);
        init = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)((counter.QuadPart * (int64_t)NS_IN_S) / frequency.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * NS_IN_S + ts.tv_nsec;
#endif
}
