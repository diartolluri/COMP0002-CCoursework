#ifndef GENERATOR_H
#define GENERATOR_H

#include "maze.h"

int mazeGenerate(Maze *maze, int width, int height, unsigned seed,
                 unsigned loopPercent, char *error, size_t capacity);

#endif
