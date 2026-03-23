#include "common.h"

int main(int argc, char *argv[]) {
    ctx_t ctx;
    status_t status = parse_args(argc, argv, &ctx);

    if (status == PARSE_HELP) {
        print_help(&ctx, argv[0], "Fibonacci");
        return EXIT_SUCCESS;
    }

    if (status == PARSE_ERROR)
        goto error;

    if (!ctx.num_arg) {
        static char buf[64];
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\r\n")] = 0;
            if (buf[0] == '\0') {
                print_err(&ctx, argv[0], "index not provided");
                goto error;
            }
            ctx.num_arg = buf;
        }
    }

    errno = 0;
    char *endptr;

    uint64_t n = strtoull(ctx.num_arg, &endptr, 10);

    if (errno == ERANGE) {
        print_err(&ctx, argv[0], "index outside of unsigned 64-bit integer range");
        goto error;
    }
    if (endptr == ctx.num_arg || *endptr != '\0') {
        print_err(&ctx, argv[0], "could not parse index");
        goto error;
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

    uint64_t i = n == 0 ? 0 : 64 - COUNT_LEADING_ZEROS(n);

    uint64_t start = get_ns();
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
    uint64_t end = get_ns();

    if (ctx.flags & OUTPUT_NUM) {
        if (ctx.flags & OUTPUT_TIME && !(ctx.flags & NO_NEWLINE)) {
            style_t const *s = (ctx.flags & USE_COLOR) ? &with_ansi : &no_ansi;
            printf("%sF_%" PRIu64 "%s = ", s->bold, n, s->reset);
        }

        mpz_out_str(stdout, 10, a);
        if (!(ctx.flags & NO_NEWLINE))
            putchar('\n');
    }

    fflush(stdout);

    if (ctx.flags & OUTPUT_TIME) {
        if (ctx.flags & NO_NEWLINE)
            putchar('\n');

        FILE *stream = (ctx.flags & NO_NEWLINE) ? stderr : stdout;
        print_calc_time(end - start, stream, &ctx);
    }

    mpz_clears(a, b, t1, t2, t_a, NULL);

    return EXIT_SUCCESS;

error:
    print_prompt_help(&ctx, argv[0]);
    return EXIT_FAILURE;
}
