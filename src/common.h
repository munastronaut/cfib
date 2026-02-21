#if defined(__linux__) || defined(__FreeBSD__)
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#elif defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <getopt.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define STDOUT_FILENO _fileno(stdout)
#else
#include <time.h>
#include <unistd.h>
#endif

#define COUNT_LEADING_ZEROS __builtin_clzll

#define NS_IN_US 1000
#define NS_IN_MS 1000000
#define NS_IN_S 1000000000

#define LOG2_PHI 0.694242

#define USAGE "\x1b[4;1mUsage:\x1b[0m \x1b[1m%s\x1b[0m [-n | -r | -t] <index>\n"

extern char const *message;
extern char const *prompt_help;

typedef enum {
    OUTPUT_HELP = (1 << 0),
    OUTPUT_TIME = (1 << 1),
    NO_NEWLINE = (1 << 2),
    OUTPUT_NUM = (1 << 3),
    IS_TTY = (1 << 4),
} flags_t;

typedef enum {
    PARSE_SUCCESS,
    PARSE_ERROR,
    PARSE_HELP,
} status_t;

typedef struct {
    uint8_t flags;
    char *num_arg;
} ctx_t;

extern struct option const long_options[];

status_t parse_args(int argc, char *argv[], ctx_t *ctx);

uint64_t get_ns();

void print_calc_time(uint64_t ns);
