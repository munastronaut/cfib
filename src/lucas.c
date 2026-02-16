#define _DEFAULT_SOURCE
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <stdlib.h>
#define LOG2_PHI 0.694242
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

#define NS_IN_US 1000
#define NS_IN_MS 1000000
#define NS_IN_S 1000000000

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
    return (uint64_t)((counter.QuadPart * INT64_C(1000000000)) /
                      frequency.QuadPart);
#elif defined(__APPLE__)
    static mach_timebase_info_data_t info;
    static int init = 0;
    if (!init) {
        mach_timebase_info(&info);
        init = 1;
    }
    uint64_t now = mach_absolute_time();
    return now * info.numer / info.denom;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + ts.tv_nsec;
#endif
}

void print_calc_time(uint64_t ns) {
    printf("Calculation time: ");

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *p = argv[1];
    while (isspace(*p))
        ++p;

    if (*p == '-') {
        fputs("Error: Input must be a nonnegative integer\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    char *endptr;

    uint64_t n = strtoull(argv[1], &endptr, 10);

    if (errno == ERANGE) {
        fputs("Error: Value outside of unsigned 64-bit integer range\n",
              stderr);
        return EXIT_FAILURE;
    }
    if (endptr == argv[1] || *endptr != '\0') {
        fputs("Error: Could not parse number input\n", stderr);
        return EXIT_FAILURE;
    }

    size_t bits = (size_t)(n * LOG2_PHI) + 2;

    mpz_t a, b, t1, t2, t_a;
    mpz_init2(a, bits);
    mpz_init2(b, bits);
    mpz_init2(t1, bits);
    mpz_init2(t2, bits);
    mpz_init2(t_a, bits);
    mpz_set_ui(a, 0);
    mpz_set_ui(b, 1);

    uint64_t i = n == 0 ? 0 : 64 - __builtin_clzll(n);

    uint64_t start = get_ns();

    while (i > 0) {
        --i;

        mpz_mul(t1, a, a);
        mpz_mul(t2, a, b);

        if ((n >> (i + 1)) & 1) {
            mpz_add_ui(t_a, t1, 2);
            mpz_add_ui(b, t2, 1);
        } else {
            mpz_sub_ui(t_a, t1, 2);
            mpz_sub_ui(b, t2, 1);
        }

        mpz_swap(a, t_a);

        if ((n >> i) & 1) {
            mpz_add(t1, a, b);
            mpz_swap(a, b);
            mpz_swap(b, t1);
        }
    }

    uint64_t end = get_ns();

    uint64_t elapsed_ns = end - start;

    printf("L_%" PRIu64 " = ", n);
    mpz_out_str(stdout, 10, a);
    putchar('\n');
    print_calc_time(elapsed_ns);

    fflush(stdout);

    mpz_clears(a, b, t1, t2, t_a, NULL);

    return EXIT_SUCCESS;
}
