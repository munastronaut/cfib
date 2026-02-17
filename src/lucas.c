#include "common.h"

char const *message =
    "\x1b[4;1mUsage:\x1b[0m\n"
    "\x1b[1m  %s\x1b[0m [OPTIONS] [NUMBER]\n"
    "\n"
    "\x1b[4;1mOptions:\x1b[0m\n"
    "\x1b[1m  -n, --number\x1b[0m\t Print number only\n"
    "\x1b[1m  -r, --raw\x1b[0m\t Print number only, without newline\n"
    "\x1b[1m  -t, --time\x1b[0m\t Print calculation time only\n"
    "\x1b[1m  -h, --help\x1b[0m\t Show help\n";

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
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + ts.tv_nsec;
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

int main(int argc, char *argv[]) {
    uint8_t flags = 0xa;
    char *num_arg = NULL;

    if (argc < 2) {
    usage:
        fprintf(flags & SHOW_HELP ? stdout : stderr, message, argv[0]);
        return flags & SHOW_HELP ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    for (size_t i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            flags |= SHOW_HELP;
            goto usage;
        } else if (strcmp(argv[i], "-n") == 0 ||
                   strcmp(argv[i], "--number") == 0) {
            flags &= ~OUTPUT_TIME;
        } else if (strcmp(argv[i], "-r") == 0 ||
                   strcmp(argv[i], "--raw") == 0) {
            flags &= ~OUTPUT_TIME;
            flags |= NO_NEWLINE;
        } else if (strcmp(argv[i], "-t") == 0 ||
                   strcmp(argv[i], "--time") == 0) {
            flags &= ~OUTPUT_NUM;
        } else if (argv[i][0] == '-' && !isdigit(argv[i][1])) {
            fprintf(stderr, "\x1b[1m%s:\x1b[0m unknown option %s\n", argv[0],
                    argv[i]);
            goto usage;
        } else {
            if (num_arg != NULL) {
                fprintf(stderr, "\x1b[1m%s:\x1b[0m multiple numbers passed\n",
                        argv[0]);
                goto usage;
            }
            num_arg = argv[i];
        }
    }

    if ((flags & OUTPUT_TIME) == (flags & OUTPUT_NUM) &&
        !(flags & OUTPUT_NUM)) {
        fputs("?\n", stderr);
        return EXIT_FAILURE;
    }

    if (!num_arg) {
        fprintf(stderr, "\x1b[1m%s:\x1b[0m input number not provided\n",
                argv[0]);
        goto usage;
    }

    char *p = num_arg;
    while (isspace(*p))
        ++p;

    if (*p == '-') {
        fprintf(stderr,
                "\x1b[1m%s:\x1b[0m input must be a nonnegative integer\n",
                argv[0]);
        goto usage;
    }

    errno = 0;
    char *endptr;

    uint64_t n = strtoull(num_arg, &endptr, 10);

    if (errno == ERANGE) {
        fprintf(stderr,
                "\x1b[1m%s:\x1b[0m value outside of unsigned 64-bit integer "
                "range\n",
                argv[0]);
        goto usage;
    }
    if (endptr == num_arg || *endptr != '\0') {
        fprintf(stderr, "\x1b[1m%s:\x1b[0m could not parse number input\n",
                argv[0]);
        goto usage;
    }

    size_t bits = (size_t)(n * LOG2_PHI) + 2;

    mpz_t a, b, t1, t2, t_a;
    mpz_init2(a, bits);
    mpz_init2(b, bits);
    mpz_init2(t1, bits);
    mpz_init2(t2, bits);
    mpz_init2(t_a, bits);
    mpz_set_ui(a, 2);
    mpz_set_ui(b, 1);

    uint64_t i = n == 0 ? 0 : 64 - COUNT_LEADING_ZEROS(n);

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

    if (flags & OUTPUT_NUM) {
        if (flags & OUTPUT_TIME)
            printf("\x1b[1mL_%" PRIu64 "\x1b[0m = ", n);

        mpz_out_str(stdout, 10, a);
        if (!(flags & NO_NEWLINE))
            putchar('\n');
    }

    if (flags & OUTPUT_TIME)
        print_calc_time(end - start);

    fflush(stdout);

    mpz_clears(a, b, t1, t2, t_a, NULL);

    return EXIT_SUCCESS;
}
