#include "maze.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Reachability uses breadth-first search, isolated from the robot solver. */
static int fail(char *error, size_t capacity, const char *message)
{
    if (error != NULL && capacity > 0)
        snprintf(error, capacity, "%s", message);
    return 0;
}

static int inside(const Maze *maze, int x, int y)
{
    return maze != NULL && maze->cells != NULL &&
           x >= 0 && x < maze->width && y >= 0 && y < maze->height;
}

static int boundary(const Maze *maze, int x, int y)
{
    return x == 0 || y == 0 || x == maze->width - 1 || y == maze->height - 1;
}

static int corner(const Maze *maze, int x, int y)
{
    return (x == 0 || x == maze->width - 1) &&
           (y == 0 || y == maze->height - 1);
}

int mazeCreate(Maze *maze, int width, int height)
{
    if (maze == NULL || width < MAZE_MIN_SIZE || height < MAZE_MIN_SIZE ||
        width > MAZE_MAX_SIZE || height > MAZE_MAX_SIZE)
        return 0;

    size_t count = (size_t)width * (size_t)height;
    if (count > SIZE_MAX / sizeof(Tile))
        return 0;
    Tile *cells = calloc(count, sizeof(*cells));
    if (cells == NULL)
        return 0;

    mazeDestroy(maze);
    *maze = (Maze){width, height, cells, -1, -1, NORTH};
    return 1;
}

void mazeDestroy(Maze *maze)
{
    if (maze != NULL)
    {
        free(maze->cells);
        *maze = (Maze){0};
    }
}

Tile mazeTileAt(const Maze *maze, int x, int y)
{
    if (!inside(maze, x, y))
        return TILE_WALL;
    return maze->cells[(size_t)y * (size_t)maze->width + (size_t)x];
}

void mazeSetTile(Maze *maze, int x, int y, Tile tile)
{
    if (inside(maze, x, y))
        maze->cells[(size_t)y * (size_t)maze->width + (size_t)x] = tile;
}

/* BFS reference: Russell/Norvig, AIMA, chapter 3 (breadth-first search).
   https://aima.cs.berkeley.edu/4th-ed/pdfs/newchap03.pdf */
static int exitReachable(const Maze *maze, char *error, size_t capacity)
{
    size_t count = (size_t)maze->width * (size_t)maze->height;
    unsigned char *seen = calloc(count, sizeof(*seen));
    size_t *queue = malloc(count * sizeof(*queue));
    if (seen == NULL || queue == NULL)
    {
        free(seen);
        free(queue);
        return fail(error, capacity, "Not enough memory to validate the maze.");
    }

    size_t first = 0;
    size_t last = 0;
    size_t start = (size_t)maze->entranceY * (size_t)maze->width +
                   (size_t)maze->entranceX;
    queue[last++] = start;
    seen[start] = 1;
    int found = 0;
    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};

    /* This full-world BFS validates input only; its data never reaches the solver. */
    while (first < last && !found)
    {
        size_t index = queue[first++];
        int x = (int)(index % (size_t)maze->width);
        int y = (int)(index / (size_t)maze->width);
        if (mazeTileAt(maze, x, y) == TILE_EXIT)
        {
            found = 1;
            break;
        }
        for (int direction = 0; direction < 4; direction++)
        {
            int nextX = x + dx[direction];
            int nextY = y + dy[direction];
            if (mazeTileAt(maze, nextX, nextY) == TILE_WALL)
                continue;
            size_t next = (size_t)nextY * (size_t)maze->width + (size_t)nextX;
            if (!seen[next])
            {
                seen[next] = 1;
                queue[last++] = next;
            }
        }
    }

    free(queue);
    free(seen);
    if (!found)
        return fail(error, capacity, "There is no path from S to E.");
    return 1;
}

int mazeValidate(Maze *maze, char *error, size_t capacity)
{
    if (maze == NULL || maze->cells == NULL ||
        maze->width < MAZE_MIN_SIZE || maze->width > MAZE_MAX_SIZE ||
        maze->height < MAZE_MIN_SIZE || maze->height > MAZE_MAX_SIZE)
        return fail(error, capacity, "Maze dimensions must each be between 5 and 101.");

    if (!inside(maze, maze->entranceX, maze->entranceY) ||
        !boundary(maze, maze->entranceX, maze->entranceY) ||
        corner(maze, maze->entranceX, maze->entranceY) ||
        mazeTileAt(maze, maze->entranceX, maze->entranceY) != TILE_EMPTY)
        return fail(error, capacity, "S must mark an empty non-corner boundary square.");

    int exits = 0;
    int openings = 0;
    for (int y = 0; y < maze->height; y++)
    {
        for (int x = 0; x < maze->width; x++)
        {
            Tile tile = mazeTileAt(maze, x, y);
            if (tile != TILE_WALL && tile != TILE_EMPTY && tile != TILE_EXIT)
                return fail(error, capacity, "The grid contains an invalid tile value.");
            if (tile == TILE_EXIT)
            {
                exits++;
                if (!boundary(maze, x, y) || corner(maze, x, y))
                    return fail(error, capacity, "E must mark a non-corner boundary square.");
            }
            if (boundary(maze, x, y) && tile != TILE_WALL)
            {
                openings++;
                if (tile != TILE_EXIT && (x != maze->entranceX || y != maze->entranceY))
                    return fail(error, capacity, "The outer wall has an opening other than S or E.");
            }
        }
    }
    if (exits != 1 || openings != 2)
        return fail(error, capacity, "The maze needs exactly one S and one E boundary opening.");

    if (!exitReachable(maze, error, capacity))
        return 0;
    if (maze->entranceY == 0)
        maze->startDirection = SOUTH;
    else if (maze->entranceX == maze->width - 1)
        maze->startDirection = WEST;
    else if (maze->entranceY == maze->height - 1)
        maze->startDirection = NORTH;
    else
        maze->startDirection = EAST;
    return 1;
}

int mazeCreateDefault(Maze *maze)
{
    static const char *layout[] = {
        "#########",
        "S   #   #",
        "### # # #",
        "#   # # #",
        "# ### # #",
        "#       E",
        "#########"
    };
    Maze temporary = {0};
    if (maze == NULL || !mazeCreate(&temporary, 9, 7))
        return 0;
    for (int y = 0; y < temporary.height; y++)
        for (int x = 0; x < temporary.width; x++)
            mazeSetTile(&temporary, x, y, layout[y][x] == '#' ? TILE_WALL :
                        layout[y][x] == 'E' ? TILE_EXIT : TILE_EMPTY);
    temporary.entranceX = 0;
    temporary.entranceY = 1;
    if (!mazeValidate(&temporary, NULL, 0))
    {
        mazeDestroy(&temporary);
        return 0;
    }
    mazeDestroy(maze);
    *maze = temporary;
    return 1;
}

static int readRows(FILE *file, char rows[MAZE_MAX_SIZE][MAZE_MAX_SIZE],
                    int *width, int *height, char *error, size_t capacity)
{
    int column = 0;
    int row = 0;
    int ch;
    *width = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\r')
        {
            if (fgetc(file) != '\n')
                return fail(error, capacity, "A carriage return must be followed by a newline.");
            ch = '\n';
        }
        if (ch == '\n')
        {
            if (column == 0)
                return fail(error, capacity, "Empty rows are not valid maze rows.");
            if (*width == 0)
                *width = column;
            if (column != *width)
                return fail(error, capacity, "Maze rows must all have the same width.");
            row++;
            column = 0;
            continue;
        }
        if (ch != '#' && ch != ' ' && ch != '.' && ch != 'S' && ch != 'E')
            return fail(error, capacity, "Only #, space, ., S and E are valid maze symbols.");
        if (row >= MAZE_MAX_SIZE || column >= MAZE_MAX_SIZE)
            return fail(error, capacity, "Maze dimensions cannot exceed 101 squares.");
        rows[row][column++] = (char)ch;
    }
    if (ferror(file))
        return fail(error, capacity, "An error occurred while reading the maze file.");
    if (column > 0)
    {
        if (*width == 0)
            *width = column;
        if (column != *width)
            return fail(error, capacity, "Maze rows must all have the same width.");
        row++;
    }
    *height = row;
    if (*width < MAZE_MIN_SIZE || row < MAZE_MIN_SIZE)
        return fail(error, capacity, "Maze dimensions must each be at least 5 squares.");
    return 1;
}

int mazeLoad(Maze *maze, const char *path, char *error, size_t capacity)
{
    if (maze == NULL || path == NULL)
        return fail(error, capacity, "A maze and input filename are required.");
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return fail(error, capacity, "Cannot open the maze input file.");

    char rows[MAZE_MAX_SIZE][MAZE_MAX_SIZE];
    int width = 0;
    int height = 0;
    int readOK = readRows(file, rows, &width, &height, error, capacity);
    int closeOK = fclose(file) == 0;
    if (!readOK)
        return 0;
    if (!closeOK)
        return fail(error, capacity, "Cannot close the maze input file.");

    Maze temporary = {0};
    if (!mazeCreate(&temporary, width, height))
        return fail(error, capacity, "Not enough memory to load the maze.");
    int entrances = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            char symbol = rows[y][x];
            mazeSetTile(&temporary, x, y, symbol == '#' ? TILE_WALL :
                        symbol == 'E' ? TILE_EXIT : TILE_EMPTY);
            if (symbol == 'S')
            {
                entrances++;
                temporary.entranceX = x;
                temporary.entranceY = y;
            }
        }
    }
    if (entrances != 1)
    {
        mazeDestroy(&temporary);
        return fail(error, capacity, "The maze file must contain exactly one S entrance.");
    }
    if (!mazeValidate(&temporary, error, capacity))
    {
        mazeDestroy(&temporary);
        return 0;
    }
    mazeDestroy(maze);
    *maze = temporary;
    return 1;
}

int mazeSave(const Maze *maze, const char *path, char *error, size_t capacity)
{
    if (maze == NULL || path == NULL)
        return fail(error, capacity, "A maze and output filename are required.");
    Maze check = *maze;
    if (!mazeValidate(&check, error, capacity))
        return 0;
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return fail(error, capacity, "Cannot open the maze output file.");
    int written = 1;
    for (int y = 0; y < maze->height && written; y++)
    {
        for (int x = 0; x < maze->width; x++)
        {
            Tile tile = mazeTileAt(maze, x, y);
            int symbol = x == maze->entranceX && y == maze->entranceY ? 'S' :
                         tile == TILE_WALL ? '#' : tile == TILE_EXIT ? 'E' : '.';
            if (fputc(symbol, file) == EOF)
            {
                written = 0;
                break;
            }
        }
        if (written && fputc('\n', file) == EOF)
            written = 0;
    }
    if (fclose(file) != 0)
        written = 0;
    if (!written)
        return fail(error, capacity, "Cannot write the complete maze output file.");
    return 1;
}
