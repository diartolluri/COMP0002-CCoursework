/*declares the maze-generation interface*/

#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include "maze_grid.h"

/*failed generation leaves the caller's current maze unchanged*/
/*the same seed and generation settings repeat the same layout*/
/*odd dimensions leave alternating cells for passages and separating walls*/
int mazeGenerate(Maze *maze, int width, int height, unsigned seed,
                 unsigned loopPercent, char *error, size_t capacity);

#endif
