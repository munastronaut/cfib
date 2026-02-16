#define _POSIX_C_SOURCE 199309L
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

double get_seconds() {
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    static int init = 0;
    if (!init) {
        QueryPerformanceFrequency(&frequency);
        init = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
#elif defined(__APPLE__)
    static mach_timebase_info_data_t info;
    static int init = 0;
    if (!init) {
        mach_timebase_info(&info);
        init = 1;
    }
    uint64_t now = mach_absolute_time();
    return ((double)now * info.numer / info.denom) / 1e9;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fputs("Usage: fib <number>\n", stderr);
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
        fputs("Error: Could not parse numeric input\n", stderr);
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

    double start = get_seconds();

    while (i > 0) {
        --i;

        mpz_mul_2exp(t1, b, 1);
        mpz_sub(t1, t1, a);
        mpz_mul(t_a, a, t1);

        mpz_mul(t2, b, t1);

        if ((n >> (i + 1)) & 1)
            mpz_add_ui(b, t2, 1);
        else
            mpz_sub_ui(b, t2, 1);

        mpz_swap(a, t_a);

        if ((n >> i) & 1) {
            mpz_add(t1, a, b);
            mpz_swap(a, b);
            mpz_swap(b, t1);
        }
    }

    double end = get_seconds();

    double elapsed_seconds = end - start;

    printf("F_%" PRIu64 " = ", n);
    mpz_out_str(stdout, 10, a);
    putchar('\n');
    printf("Calculation time: %lf seconds\n", elapsed_seconds);

    fflush(stdout);

    mpz_clears(a, b, t1, t2, t_a, NULL);

    return EXIT_SUCCESS;
}
