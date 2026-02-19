#include "common.h"

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
