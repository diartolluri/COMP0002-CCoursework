/*reads command-line options and applies the selected defaults*/

#include "maze_config.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
         "  --cell-size N              Cell pixels: 0 automatic, or 4-44 for a larger view\n"
         "  --save PATH                Save the initial maze before searching\n"
         "  --help                     Show this help\n"
         "The default is a small fixed maze. Statistics/errors are written to stderr.");
}

/*strtoul accepts signs, so check the digits before converting*/
static bool number(const char *text, unsigned long maximum, unsigned long *value)
{
    if (*text == '\0') return false;
    for (const char *p = text; *p != '\0'; p++)
        if (*p < '0' || *p > '9') return false;
    errno = 0;
    char *end = NULL;
    unsigned long parsed = strtoul(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed > maximum) return false;
    *value = parsed;
    return true;
}

/*each bit records a setting that the preset must not replace*/
enum { SIZE_SET = 1u, SEED_SET = 2u, LOOPS_SET = 4u, DELAY_SET = 8u };

static bool applyPreset(const char *name, unsigned supplied, Options *options)
{
    if (name == NULL || strcmp(name, "small") == 0) return true;
    bool large = strcmp(name, "large") == 0;
    if (!large && strcmp(name, "loops") != 0)
    {
        fputs("--preset must be small, large or loops.\n", stderr);
        return false;
    }
    options->generate = true;
    if (!(supplied & SIZE_SET))
    {
        options->width = large ? 35 : 15;
        options->height = large ? 35 : 11;
    }
    if (!(supplied & SEED_SET)) options->seed = 42;
    if (!(supplied & LOOPS_SET)) options->loops = large ? 0 : 25;
    if (!(supplied & DELAY_SET)) options->delay = large ? 20 : 35;
    return true;
}

static bool readDimensions(int argc, char **argv, int *index, Options *options)
{
    unsigned long width, height;
    int i = *index;
    if (i + 2 >= argc || !number(argv[i + 1], 101, &width) ||
        !number(argv[i + 2], 101, &height) || width < 5 || height < 5 ||
        width % 2 == 0 || height % 2 == 0)
    {
        fputs("--generate requires two odd dimensions from 5 to 101.\n", stderr);
        return false;
    }
    options->generate = true;
    options->width = (int)width;
    options->height = (int)height;
    *index += 2;
    return true;
}

typedef enum
{
    OPTION_UNKNOWN, OPTION_HELP, OPTION_PREVIEW, OPTION_HEADLESS, OPTION_GRAPHICS,
    OPTION_GENERATE, OPTION_FILE, OPTION_SAVE, OPTION_PRESET, OPTION_SOLVER,
    OPTION_SEED, OPTION_LOOPS, OPTION_DELAY, OPTION_BUDGET, OPTION_CELL_SIZE
} OptionKind;

/*aliases share a kind so their validation follows the same path*/
static OptionKind findOption(const char *argument)
{
    static const struct { const char *name; OptionKind kind; } names[] = {
        {"--help", OPTION_HELP}, {"--preview", OPTION_PREVIEW},
        {"--headless", OPTION_HEADLESS}, {"--graphics", OPTION_GRAPHICS},
        {"--commands", OPTION_GRAPHICS}, {"--generate", OPTION_GENERATE},
        {"--file", OPTION_FILE}, {"-f", OPTION_FILE}, {"--save", OPTION_SAVE},
        {"--preset", OPTION_PRESET}, {"--solver", OPTION_SOLVER},
        {"--seed", OPTION_SEED}, {"--loops", OPTION_LOOPS},
        {"--delay", OPTION_DELAY}, {"--budget", OPTION_BUDGET},
        {"--cell-size", OPTION_CELL_SIZE}
    };
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        if (strcmp(argument, names[i].name) == 0) return names[i].kind;
    return OPTION_UNKNOWN;
}

static bool setPreset(const char *value, const char **preset)
{
    if (*preset != NULL)
    {
        fputs("Provide only one --preset.\n", stderr);
        return false;
    }
    *preset = value;
    return true;
}

static bool setSolver(const char *value, Options *options)
{
    if (strcmp(value, "iddfs") == 0) options->solver = SOLVER_DEEPENING;
    else if (strcmp(value, "wall") == 0) options->solver = SOLVER_WALL;
    else
    {
        fputs("--solver must be iddfs or wall.\n", stderr);
        return false;
    }
    return true;
}

static bool setNumber(OptionKind kind, const char *argument, const char *value,
                      Options *options, unsigned *supplied)
{
    unsigned long parsed;
    /*check the limit before narrowing the value to its stored type*/
    unsigned long maximum = kind == OPTION_SEED ? UINT_MAX :
                            kind == OPTION_LOOPS ? 100 :
                            kind == OPTION_DELAY ? 1000 :
                            kind == OPTION_CELL_SIZE ? 44 : 1000000000UL;
    if (!number(value, maximum, &parsed))
    {
        fprintf(stderr, "Invalid number for %s: %s.\n", argument, value);
        return false;
    }
    if (kind == OPTION_CELL_SIZE && parsed != 0 && parsed < 4)
    {
        fputs("--cell-size must be 0 (automatic) or 4-44 pixels.\n", stderr);
        return false;
    }
    switch (kind)
    {
        case OPTION_CELL_SIZE: options->cellSize = (int)parsed; break;
        case OPTION_SEED: options->seed = (unsigned)parsed; *supplied |= SEED_SET; break;
        case OPTION_LOOPS: options->loops = (unsigned)parsed; *supplied |= LOOPS_SET; break;
        case OPTION_DELAY: options->delay = (int)parsed; *supplied |= DELAY_SET; break;
        case OPTION_BUDGET: options->budget = (size_t)parsed; break;
        default: return false;
    }
    return true;
}

static bool readValue(OptionKind kind, const char *argument, const char *value,
                      Options *options, unsigned *supplied, const char **preset)
{
    switch (kind)
    {
        case OPTION_FILE: options->file = value; return true;
        case OPTION_SAVE: options->save = value; return true;
        case OPTION_PRESET: return setPreset(value, preset);
        case OPTION_SOLVER: return setSolver(value, options);
        default: return setNumber(kind, argument, value, options, supplied);
    }
}

static OptionsResult readArgument(int argc, char **argv, int *index, Options *options,
                                  unsigned *supplied, const char **preset)
{
    const char *argument = argv[*index];
    OptionKind kind = findOption(argument);
    switch (kind)
    {
        case OPTION_HELP: printUsage(argv[0]); return OPTIONS_HELP;
        case OPTION_PREVIEW: options->preview = true; return OPTIONS_READY;
        case OPTION_HEADLESS: options->graphics = false; return OPTIONS_READY;
        case OPTION_GRAPHICS: options->graphics = true; return OPTIONS_READY;
        case OPTION_GENERATE:
            if (!readDimensions(argc, argv, index, options)) return OPTIONS_ERROR;
            *supplied |= SIZE_SET;
            return OPTIONS_READY;
        case OPTION_UNKNOWN:
            fprintf(stderr, "Unknown option: %s. Use --help.\n", argument);
            return OPTIONS_ERROR;
        default: break;
    }
    /*consume the value so the outer loop does not treat it as another option*/
    if (++(*index) >= argc)
    {
        fprintf(stderr, "Missing value for %s.\n", argument);
        return OPTIONS_ERROR;
    }
    return readValue(kind, argument, argv[*index], options, supplied, preset) ?
           OPTIONS_READY : OPTIONS_ERROR;
}

static bool compatibleOptions(const Options *options, unsigned supplied)
{
    if ((options->file != NULL && options->generate) ||
        ((supplied & (SEED_SET | LOOPS_SET)) && !options->generate))
    {
        fputs("Use either --file or --generate; --seed/--loops require --generate.\n", stderr);
        return false;
    }
    return true;
}

OptionsResult parseOptions(int argc, char **argv, Options *options)
{
    *options = (Options){.width = 15, .height = 11, .seed = 1, .delay = 50,
                         .budget = 2000000, .solver = SOLVER_WALL};
#ifdef MAZE_DRAWAPP
    options->graphics = true;
#endif
    const char *preset = NULL;
    unsigned supplied = 0;
    for (int i = 1; i < argc; i++)
    {
        OptionsResult result = readArgument(argc, argv, &i, options, &supplied, &preset);
        if (result != OPTIONS_READY) return result;
    }
    /*explicit options win over preset defaults, whatever order they appear in*/
    return applyPreset(preset, supplied, options) && compatibleOptions(options, supplied) ?
           OPTIONS_READY : OPTIONS_ERROR;
}
