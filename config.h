#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include "solver.h"

typedef struct
{
    const char *file;
    const char *save;
    int generate;
    int width;
    int height;
    unsigned seed;
    unsigned loops;
    int delay;
    int graphics;
    int preview;
    size_t budget;
    SolverKind solver;
} Options;

int parseOptions(int argc, char **argv, Options *options);
void printUsage(const char *program);

#endif
