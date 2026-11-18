/*defines maze tiles, directions and the grid interface*/

#ifndef MAZE_GRID_H
#define MAZE_GRID_H

#include <stddef.h>

enum { MAZE_MIN_SIZE = 5, MAZE_MAX_SIZE = 101 };
typedef enum { NORTH, EAST, SOUTH, WEST } Direction;

typedef enum
{
    TILE_WALL,
    TILE_EMPTY,
    TILE_EXIT
} Tile;

typedef struct
{
    int width;
    int height;
    /*copying this struct does not duplicate the allocated grid*/
    Tile *cells;
    int entranceX;
    int entranceY;
    Direction startDirection;
} Maze;

/*start owning maze values at {0}, then destroy them when finished*/
int mazeCreate(Maze *maze, int width, int height);
int mazeCreateDefault(Maze *maze);
void mazeDestroy(Maze *maze);
/*out-of-grid reads return a wall, so callers need no separate edge check*/
Tile mazeTileAt(const Maze *maze, int x, int y);
void mazeSetTile(Maze *maze, int x, int y, Tile tile);
/*validation also sets the direction that faces into the entrance*/
int mazeValidate(Maze *maze, char *error, size_t capacity);
int mazeLoad(Maze *maze, const char *path, char *error, size_t capacity);
int mazeSave(const Maze *maze, const char *path, char *error, size_t capacity);

#endif
