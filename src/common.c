#include "common.h"

char const *message =
    "A program that calculates %s numbers\n"
    "\n" USAGE "\n"
    "\x1b[4;1mArguments:\x1b[0m\n"
    "  <index>\t\tthe index of the Fibonacci number\n"
    "\n"
    "\x1b[4;1mOptions:\x1b[0m\n"
    "\x1b[1m  -n, --num-only\x1b[0m\tPrint number only, with newline\n"
    "\x1b[1m  -r, --raw-only\x1b[0m\tPrint number only, without newline (default when piping)\n"
    "\x1b[1m  -t, --time-only\x1b[0m\tPrint calculation time only\n"
    "\x1b[1m  -h, --help\x1b[0m\t\tPrint this help and exit\n";

char const *prompt_help = "\n" USAGE "Try '\x1b[1m%s --help\x1b[0m' for more information.\n";

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

void print_calc_time(uint64_t ns) {
    printf("\x1b[1mcalculation time:\x1b[0m ");

    if (ns < NS_IN_US) {
        printf("%" PRIu64 "ns\n", ns);
        return;
    }

    double val;
    char const *unit;

    if (ns < NS_IN_MS) {
        val = ns / 1000.0;
        unit = "us";
    } else if (ns < NS_IN_S) {
        val = ns / 1000000.0;
        unit = "ms";
    } else {
        val = ns / 1000000000.0;
        unit = "s";
    }

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%.3f", val);

    char *p = buf + len - 1;

    while (p > buf && *p == '0')
        *p-- = '\0';

    if (p > buf && *p == '.')
        *p-- = '\0';

    printf("%s%s\n", buf, unit);
}
