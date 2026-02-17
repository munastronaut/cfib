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
#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#pragma intrinsic(__lzcnt)
#else
#include <time.h>
#endif

#if defined(_WIN32)
#define COUNT_LEADING_ZEROS(x) __lzcnt64((x))
#else
#define COUNT_LEADING_ZEROS(x) __builtin_clzll((x))
#endif

#define NS_IN_US 1000
#define NS_IN_MS 1000000
#define NS_IN_S 1000000000

#define LOG2_PHI 0.694242

#define OUTPUT_NUM (1 << 3)
#define NO_NEWLINE (1 << 2)
#define OUTPUT_TIME (1 << 1)
#define SHOW_HELP (1)

#define USAGE "\x1b[4;1mUsage:\x1b[0m \x1b[1m%s\x1b[0m [OPTION] <index>\n"

uint64_t get_ns();

void print_calc_time(uint64_t ns);
