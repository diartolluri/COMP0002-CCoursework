/*declares the run settings and option-parsing interface*/

#ifndef MAZE_CONFIG_H
#define MAZE_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include "maze_solver.h"

/*file paths point into argv and are not owned by this struct*/
typedef struct
{
    const char *file;
    const char *save;
    bool generate;
    int width;
    int height;
    unsigned seed;
    unsigned loops;
    int delay;
    int cellSize;
    bool graphics;
    /*preview validates and prints the maze without starting a search*/
    bool preview;
    size_t budget;
    SolverKind solver;
} Options;

typedef enum { OPTIONS_ERROR, OPTIONS_READY, OPTIONS_HELP } OptionsResult;

/*help is a normal early exit, distinct from an invalid command*/
OptionsResult parseOptions(int argc, char **argv, Options *options);
void printUsage(const char *program);

#endif
