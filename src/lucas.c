#include "common.h"

int main(int argc, char *argv[]) {
    uint8_t flags = IS_TTY | OUTPUT_NUM | OUTPUT_TIME;
    char *num_arg = NULL;

    if (!isatty(STDOUT_FILENO))
        flags = (flags & ~(IS_TTY | OUTPUT_TIME)) | NO_NEWLINE;

    for (size_t i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (arg[0] == '-' && arg[1] != '\0') {
            if (arg[1] == '-') {
                if (strcmp(arg, "--help") == 0) {
                    flags |= OUTPUT_HELP;
                    goto usage;
                }
                if (strcmp(arg, "--num-only") == 0) {
                    flags &= ~OUTPUT_TIME;
                    continue;
                }
                if (strcmp(arg, "--raw-only") == 0) {
                    flags = (flags & ~OUTPUT_TIME) | NO_NEWLINE;
                    continue;
                }
                if (strcmp(arg, "--time-only") == 0) {
                    flags &= ~OUTPUT_NUM;
                    continue;
                }
                fprintf(stderr, "\x1b[1m%s:\x1b[0m unrecognized option '\x1b[1m%s\x1b[0m'\n", argv[0],
                        arg);
                goto error;
            }
            for (size_t j = 1; arg[j] != '\0'; ++j) {
                switch (arg[j]) {
                    case 'h':
                        flags |= OUTPUT_HELP;
                        goto usage;
                    case 'n':
                        flags &= ~OUTPUT_TIME;
                        break;
                    case 'r':
                        flags = (flags & ~OUTPUT_TIME) | NO_NEWLINE;
                        break;
                    case 't':
                        flags &= ~OUTPUT_NUM;
                        break;
                    default:
                        if (isdigit(arg[j]) && j == 1) goto multiple;
                        fprintf(stderr, "\x1b[1m%s:\x1b[0m unrecognized option '\x1b[1m%s\x1b[0m'\n", argv[0],
                                arg);
                        goto error;
                }
            }
        } else {
            multiple:
            if (num_arg != NULL) {
                fprintf(stderr, "\x1b[1m%s:\x1b[0m multiple indices passed\n", argv[0]);
                goto error;
            }
            num_arg = arg;
        }
    }

    if ((flags & IS_TTY) && !(flags & OUTPUT_TIME) && !(flags & OUTPUT_NUM)) {
        fputs("?\n", stderr);
        return EXIT_FAILURE;
    }

    if (!num_arg) {
        static char buf[64];
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\r\n")] = 0;
            if (buf[0] == '\0') {
                fprintf(stderr, "\x1b[1m%s:\x1b[0m index not provided\n", argv[0]);
                goto error;
            }
            num_arg = buf;
        }
    }

    char *p = num_arg;
    while (isspace(*p))
        ++p;

    if (*p == '-') {
        fprintf(stderr, "\x1b[1m%s:\x1b[0m index must be a nonnegative integer\n", argv[0]);
        goto error;
    }

    errno = 0;
    char *endptr;

    uint64_t n = strtoull(num_arg, &endptr, 10);

    if (errno == ERANGE) {
        fprintf(stderr,
                "\x1b[1m%s:\x1b[0m index outside of unsigned 64-bit integer "
                "range\n",
                argv[0]);
        goto error;
    }
    if (endptr == num_arg || *endptr != '\0') {
        fprintf(stderr, "\x1b[1m%s:\x1b[0m could not parse index\n", argv[0]);
        goto error;
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

usage:
    fprintf(flags & OUTPUT_HELP ? stdout : stderr, message, "Lucas", argv[0]);
    return flags & OUTPUT_HELP ? EXIT_SUCCESS : EXIT_FAILURE;
error:
    fprintf(stderr, prompt_help, argv[0], argv[0]);
}
