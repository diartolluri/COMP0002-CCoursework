#include "config.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* AI-assisted reference implementation: OpenAI Codex, November 2026. */
void printUsage(const char *program)
{
    printf("Usage: %s [options]\n", program);
    puts("  --preset small|large|loops   Ready-to-run demonstration settings\n"
         "  --preview                  Show the maze without searching\n"
         "  --headless                 Solve without graphics\n"
         "  --graphics                 Animate using a DrawApp-enabled build\n"
         "  --commands                 Emit DrawApp commands without opening the viewer\n"
         "  -f, --file PATH            Load a rectangular ASCII maze (# . space S E)\n"
         "  --generate WIDTH HEIGHT    Generate odd dimensions from 5 to 101\n"
         "  --seed N                   Reproducible generator seed (default 1)\n"
         "  --loops N                  Percentage of eligible walls opened (0-100)\n"
         "  --solver iddfs|wall        Sensor-only search (default wall)\n"
         "  --budget N                 Maximum movement/turn actions (default 2000000)\n"
         "                             0 removes this guard; cyclic searches can be slow\n"
         "  --delay N                  DrawApp milliseconds per action (0-1000; default 50)\n"
         "  --save PATH                Save the initial maze before searching\n"
         "  --help                     Show this help\n"
         "The default is a small fixed maze. Statistics/errors are written to stderr.");
}

static int number(const char *text, unsigned long maximum, unsigned long *value)
{
    if (*text == '\0') return 0;
    for (const char *p = text; *p != '\0'; p++)
        if (*p < '0' || *p > '9') return 0;
    errno = 0;
    char *end = NULL;
    unsigned long parsed = strtoul(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed > maximum) return 0;
    *value = parsed;
    return 1;
}

enum { SIZE_SET = 1u, SEED_SET = 2u, LOOPS_SET = 4u, DELAY_SET = 8u };

static int applyPreset(const char *name, unsigned supplied, Options *options)
{
    if (name == NULL || strcmp(name, "small") == 0) return 1;
    int large = strcmp(name, "large") == 0;
    if (!large && strcmp(name, "loops") != 0)
    {
        fputs("--preset must be small, large or loops.\n", stderr);
        return 0;
    }
    options->generate = 1;
    if (!(supplied & SIZE_SET))
    {
        options->width = large ? 35 : 15;
        options->height = large ? 35 : 11;
    }
    if (!(supplied & SEED_SET)) options->seed = 42;
    if (!(supplied & LOOPS_SET)) options->loops = large ? 0 : 25;
    if (!(supplied & DELAY_SET)) options->delay = large ? 20 : 35;
    return 1;
}

static int readDimensions(int argc, char **argv, int *index, Options *options)
{
    unsigned long width, height;
    int i = *index;
    if (i + 2 >= argc || !number(argv[i + 1], 101, &width) ||
        !number(argv[i + 2], 101, &height) || width < 5 || height < 5 ||
        width % 2 == 0 || height % 2 == 0)
    {
        fputs("--generate requires two odd dimensions from 5 to 101.\n", stderr);
        return 0;
    }
    options->generate = 1;
    options->width = (int)width;
    options->height = (int)height;
    *index += 2;
    return 1;
}

int parseOptions(int argc, char **argv, Options *options)
{
    *options = (Options){.width = 15, .height = 11, .seed = 1, .delay = 50,
                         .budget = 2000000, .solver = SOLVER_WALL};
#ifdef MAZE_DRAWAPP
    options->graphics = 1;
#endif
    const char *preset = NULL;
    unsigned supplied = 0;
    for (int i = 1; i < argc; i++)
    {
        const char *argument = argv[i];
        if (strcmp(argument, "--help") == 0)
        {
            printUsage(argv[0]);
            return 2;
        }
        if (strcmp(argument, "--preview") == 0) { options->preview = 1; continue; }
        if (strcmp(argument, "--headless") == 0) { options->graphics = 0; continue; }
        if (strcmp(argument, "--graphics") == 0) { options->graphics = 1; continue; }
        if (strcmp(argument, "--commands") == 0) { options->graphics = 1; continue; }
        if (strcmp(argument, "--generate") == 0)
        {
            if (!readDimensions(argc, argv, &i, options)) return 0;
            supplied |= SIZE_SET;
            continue;
        }
        int isFile = strcmp(argument, "--file") == 0 || strcmp(argument, "-f") == 0;
        int isSave = strcmp(argument, "--save") == 0;
        int isSolver = strcmp(argument, "--solver") == 0;
        int isSeed = strcmp(argument, "--seed") == 0;
        int isLoops = strcmp(argument, "--loops") == 0;
        int isDelay = strcmp(argument, "--delay") == 0;
        int isBudget = strcmp(argument, "--budget") == 0;
        int isPreset = strcmp(argument, "--preset") == 0;
        if (!(isFile || isSave || isSolver || isSeed || isLoops || isDelay || isBudget || isPreset))
        {
            fprintf(stderr, "Unknown option: %s. Use --help.\n", argument);
            return 0;
        }
        if (++i >= argc)
        {
            fprintf(stderr, "Missing value for %s.\n", argument);
            return 0;
        }
        const char *value = argv[i];
        if (isFile) options->file = value;
        else if (isSave) options->save = value;
        else if (isPreset)
        {
            if (preset != NULL)
            {
                fputs("Provide only one --preset.\n", stderr);
                return 0;
            }
            preset = value;
        }
        else if (isSolver)
        {
            if (strcmp(value, "iddfs") == 0) options->solver = SOLVER_DEEPENING;
            else if (strcmp(value, "wall") == 0) options->solver = SOLVER_WALL;
            else { fputs("--solver must be iddfs or wall.\n", stderr); return 0; }
        }
        else
        {
            unsigned long parsed = 0;
            unsigned long maximum = isSeed ? UINT_MAX : isLoops ? 100 :
                                    isDelay ? 1000 : 1000000000UL;
            if (!number(value, maximum, &parsed))
            {
                fprintf(stderr, "Invalid number for %s: %s.\n", argument, value);
                return 0;
            }
            if (isSeed) { options->seed = (unsigned)parsed; supplied |= SEED_SET; }
            if (isLoops) { options->loops = (unsigned)parsed; supplied |= LOOPS_SET; }
            if (isDelay) { options->delay = (int)parsed; supplied |= DELAY_SET; }
            if (isBudget) options->budget = (size_t)parsed;
        }
    }
    /* Apply only defaults: explicit values win regardless of argument order. */
    if (!applyPreset(preset, supplied, options)) return 0;
    if ((options->file != NULL && options->generate) ||
        ((supplied & (SEED_SET | LOOPS_SET)) && !options->generate))
    {
        fputs("Use either --file or --generate; --seed/--loops require --generate.\n", stderr);
        return 0;
    }
    return 1;
}
