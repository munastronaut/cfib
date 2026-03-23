#include "common.h"

struct option const long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"num", no_argument, NULL, 'n'},
    {"raw", no_argument, NULL, 'r'},
    {"time", no_argument, NULL, 't'},
    {NULL, 0, NULL, 0},
};

status_t parse_args(int argc, char *argv[], ctx_t *ctx) {
    int opt;
    opterr = 0;

    ctx->flags = argc > 2 ? IS_TTY : IS_TTY | OUTPUT_TIME | OUTPUT_NUM;
    ctx->num_arg = NULL;

    int disable_color = (getenv("NO_COLOR") != NULL);

    if (!isatty(STDOUT_FILENO))
        ctx->flags = (ctx->flags & ~(IS_TTY | OUTPUT_TIME)) | OUTPUT_NUM | NO_NEWLINE;

    if (isatty(STDOUT_FILENO) && !disable_color)
        ctx->flags |= USE_COLOR;

    optind = 1;

    while ((opt = getopt_long(argc, argv, "hnrt", long_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            ctx->flags |= OUTPUT_HELP;
            return PARSE_HELP;
        case 'n':
            ctx->flags |= OUTPUT_NUM;
            break;
        case 'r':
            ctx->flags |= (OUTPUT_NUM | NO_NEWLINE);
            break;
        case 't':
            ctx->flags |= OUTPUT_TIME;
            break;
        case '?':
            if (optopt >= '0' && optopt <= '9') {
                print_err(ctx, argv[0], "index must be a nonnegative integer");
                return PARSE_ERROR;
            }

            style_t const *s = (ctx->flags & USE_COLOR) ? &with_ansi : &no_ansi;
            if (optopt)
                fprintf(stderr, "%s%s:%s invalid option -- '%s%c%s'\n", s->bold, argv[0], s->reset,
                        s->bold, optopt, s->reset);
            else
                fprintf(stderr, "%s%s:%s unrecognized option '%s%s%s'\n", s->bold, argv[0],
                        s->reset, s->bold, argv[optind - 1], s->reset);
            return PARSE_ERROR;
        default:
            return PARSE_ERROR;
        }
    }
    if (optind < argc) {
        ctx->num_arg = argv[optind];
        if (ctx->num_arg[0] == '-') {
            print_err(ctx, argv[0], "index must be a nonnegative integer");
            return PARSE_ERROR;
        }
        if (optind + 1 < argc) {
            print_err(ctx, argv[0], "multiple indices passed");
            return PARSE_ERROR;
        }
    }

    return PARSE_SUCCESS;
}

uint64_t get_ns() {
#ifdef _WIN32
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

style_t const with_ansi = {"\x1b[1m", "\x1b[4m", "\x1b[0m"};
style_t const no_ansi = {"", "", ""};

void print_calc_time(uint64_t ns, FILE *stream, ctx_t *ctx) {
    style_t const *s = (ctx->flags & USE_COLOR) ? &with_ansi : &no_ansi;
    fprintf(stream, "%scalculation time:%s ", s->bold, s->reset);

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

    fprintf(stream, "%s%s\n", buf, unit);
}

// I know how messy these functions look.
void print_help(ctx_t *ctx, char const *name, char const *type) {
    style_t const *s = (ctx->flags & USE_COLOR) ? &with_ansi : &no_ansi;
    printf(HELP, type, s->bold, s->uline, s->reset, "\n  ", s->bold, name, s->reset, s->bold,
           s->uline, s->reset, type, s->bold, s->uline, s->reset, s->bold, s->reset, s->bold,
           s->reset, s->bold, s->reset, s->bold, s->reset);
}

void print_prompt_help(ctx_t *ctx, char const *name) {
    style_t const *s = (ctx->flags & USE_COLOR) ? &with_ansi : &no_ansi;
    fprintf(stderr, PROMPT_HELP, s->bold, s->uline, s->reset, " ", s->bold, name, s->reset, s->bold,
            name, s->reset);
}

void print_err(ctx_t *ctx, char const *name, char const *msg) {
    style_t const *s = (ctx->flags & USE_COLOR) ? &with_ansi : &no_ansi;
    fprintf(stderr, "%s%s:%s %s\n", s->bold, name, s->reset, msg);
}
