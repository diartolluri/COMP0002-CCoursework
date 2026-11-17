#ifndef MAZE_H
#define MAZE_H

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
    Tile *cells;
    int entranceX;
    int entranceY;
    Direction startDirection;
} Maze;

/* Initialise owning Maze values to {0}; destroy them once when finished. */
int mazeCreate(Maze *maze, int width, int height);
int mazeCreateDefault(Maze *maze);
void mazeDestroy(Maze *maze);
Tile mazeTileAt(const Maze *maze, int x, int y);
void mazeSetTile(Maze *maze, int x, int y, Tile tile);
int mazeValidate(Maze *maze, char *error, size_t capacity);
int mazeLoad(Maze *maze, const char *path, char *error, size_t capacity);
int mazeSave(const Maze *maze, const char *path, char *error, size_t capacity);

#endif
