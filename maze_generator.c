/*generates seeded mazes and optionally adds loops*/

#include "maze_generator.h"

#include <stdint.h>
#include <stdio.h>

/*recursive backtracking reference: Jamis Buck
   https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking*/
/*lcg parameters: Press et al., Numerical Recipes in C, 2nd ed., section 7.1*/

/*unsigned arithmetic wraps predictably, keeping seeded runs reproducible*/
static uint32_t nextRandom(uint32_t *state)
{
    *state = *state * UINT32_C(1664525) + UINT32_C(1013904223);
    return *state;
}

static int fail(char *error, size_t capacity, const char *message)
{
    if (error != NULL && capacity > 0)
        snprintf(error, capacity, "%s", message);
    return 0;
}

static int chooseDirection(unsigned candidates, uint32_t *state)
{
    unsigned count = 0;
    for (int direction = 0; direction < 4; direction++)
        count += (candidates & (1u << direction)) != 0;
    unsigned selected = (nextRandom(state) >> 16) % count;
    for (int direction = 0; direction < 4; direction++)
    {
        if ((candidates & (1u << direction)) != 0)
        {
            if (selected == 0)
                return direction;
            selected--;
        }
    }
    return 0;
}

static void carvePassages(Maze *maze, int currentX, int currentY, uint32_t *state)
{
    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};
    mazeSetTile(maze, currentX, currentY, TILE_EMPTY);

    /*each logical cell is entered once, recursion is bounded by 2500 cells*/
    for (;;)
    {
        /*two-square jumps leave room for a wall between neighbouring cells*/
        unsigned candidates = 0;
        for (int direction = 0; direction < 4; direction++)
        {
            int x = currentX + 2 * dx[direction];
            int y = currentY + 2 * dy[direction];
            if (x > 0 && x < maze->width - 1 && y > 0 && y < maze->height - 1 &&
                mazeTileAt(maze, x, y) == TILE_WALL)
                candidates |= 1u << direction;
        }
        if (candidates == 0)
            return;

        int direction = chooseDirection(candidates, state);
        mazeSetTile(maze, currentX + dx[direction], currentY + dy[direction], TILE_EMPTY);
        carvePassages(maze, currentX + 2 * dx[direction], currentY + 2 * dy[direction], state);
    }
}

/*only interior walls are opened, so the boundary openings stay unchanged*/
static void addLoops(Maze *maze, unsigned percent, uint32_t *state)
{
    for (int y = 1; y < maze->height - 1; y++)
    {
        for (int x = 1; x < maze->width - 1; x++)
        {
            if (mazeTileAt(maze, x, y) != TILE_WALL)
                continue;
            int horizontal = (x % 2 == 0 && y % 2 != 0) &&
                             mazeTileAt(maze, x - 1, y) == TILE_EMPTY &&
                             mazeTileAt(maze, x + 1, y) == TILE_EMPTY;
            int vertical = (x % 2 != 0 && y % 2 == 0) &&
                           mazeTileAt(maze, x, y - 1) == TILE_EMPTY &&
                           mazeTileAt(maze, x, y + 1) == TILE_EMPTY;
            if ((horizontal || vertical) && (nextRandom(state) >> 16) % 100u < percent)
                mazeSetTile(maze, x, y, TILE_EMPTY);
        }
    }
}

int mazeGenerate(Maze *maze, int width, int height, unsigned seed,
                 unsigned loopPercent, char *error, size_t capacity)
{
    if (maze == NULL || width < MAZE_MIN_SIZE || width > MAZE_MAX_SIZE ||
        height < MAZE_MIN_SIZE || height > MAZE_MAX_SIZE ||
        width % 2 == 0 || height % 2 == 0 || loopPercent > 100)
        return fail(error, capacity, "Generation needs odd dimensions 5..101 and loops 0..100.");

    Maze temporary = {0};
    if (!mazeCreate(&temporary, width, height))
        return fail(error, capacity, "Not enough memory to generate the maze.");
    uint32_t state = (uint32_t)seed;
    carvePassages(&temporary, 1, 1, &state);
    if (loopPercent > 0)
        addLoops(&temporary, loopPercent, &state);

    temporary.entranceX = 0;
    temporary.entranceY = 1;
    mazeSetTile(&temporary, 0, 1, TILE_EMPTY);
    mazeSetTile(&temporary, width - 1, height - 2, TILE_EXIT);
    if (!mazeValidate(&temporary, error, capacity))
    {
        mazeDestroy(&temporary);
        return 0;
    }
    mazeDestroy(maze);
    *maze = temporary;
    return 1;
}
