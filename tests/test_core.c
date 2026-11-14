#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generator.h"
#include "maze.h"
#include "robot_internal.h"
#include "solver.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    exit(EXIT_FAILURE); } } while (0)

typedef struct
{
    int distance;
    int passable;
    int reachable;
    int edges;
    int openings;
    int exits;
} Facts;

typedef struct
{
    const Maze *maze;
    RobotView previous;
    size_t moves;
    size_t turns;
    int started;
    int sawExit;
    int requireStop;
} Trace;

static const int dx[] = {0, 1, 0, -1};
static const int dy[] = {-1, 0, 1, 0};

/* Independent BFS is a test oracle; the robot solver receives no grid access. */
static Facts inspect(const Maze *maze)
{
    int count = maze->width * maze->height;
    int *distance = malloc((size_t)count * sizeof(*distance));
    int *queue = malloc((size_t)count * sizeof(*queue));
    CHECK(distance != NULL && queue != NULL);
    for (int i = 0; i < count; i++) distance[i] = -1;
    Facts result = {.distance = -1};
    for (int y = 0; y < maze->height; y++)
    {
        for (int x = 0; x < maze->width; x++)
        {
            Tile tile = mazeTileAt(maze, x, y);
            if (tile == TILE_WALL) continue;
            result.passable++;
            result.exits += tile == TILE_EXIT;
            result.openings += x == 0 || y == 0 ||
                               x == maze->width - 1 || y == maze->height - 1;
            result.edges += mazeTileAt(maze, x + 1, y) != TILE_WALL;
            result.edges += mazeTileAt(maze, x, y + 1) != TILE_WALL;
        }
    }
    int start = maze->entranceY * maze->width + maze->entranceX;
    CHECK(start >= 0 && start < count);
    int head = 0;
    int tail = 0;
    distance[start] = 0;
    queue[tail++] = start;
    while (head < tail)
    {
        int index = queue[head++];
        int x = index % maze->width;
        int y = index / maze->width;
        result.reachable++;
        if (mazeTileAt(maze, x, y) == TILE_EXIT) result.distance = distance[index];
        for (int direction = 0; direction < 4; direction++)
        {
            int nx = x + dx[direction];
            int ny = y + dy[direction];
            if (nx < 0 || ny < 0 || nx >= maze->width || ny >= maze->height ||
                mazeTileAt(maze, nx, ny) == TILE_WALL) continue;
            int next = ny * maze->width + nx;
            if (distance[next] >= 0) continue;
            distance[next] = distance[index] + 1;
            queue[tail++] = next;
        }
    }
    free(queue);
    free(distance);
    return result;
}

static void observe(const RobotView *view, void *context)
{
    Trace *trace = context;
    CHECK(view->direction >= NORTH && view->direction <= WEST);
    CHECK(mazeTileAt(trace->maze, view->x, view->y) != TILE_WALL);
    if (trace->started)
    {
        CHECK(!trace->requireStop || !trace->sawExit);
        RobotView previous = trace->previous;
        if (view->x == previous.x && view->y == previous.y)
        {
            int rotation = (view->direction - previous.direction + 4) % 4;
            CHECK(rotation == 1 || rotation == 3);
            trace->turns++;
        }
        else
        {
            CHECK(view->direction == previous.direction);
            CHECK(view->x == previous.x + dx[previous.direction]);
            CHECK(view->y == previous.y + dy[previous.direction]);
            trace->moves++;
        }
    }
    trace->sawExit |= mazeTileAt(trace->maze, view->x, view->y) == TILE_EXIT;
    trace->previous = *view;
    trace->started = 1;
}

static int samePose(RobotView a, RobotView b)
{
    return a.x == b.x && a.y == b.y && a.direction == b.direction;
}

static void checkSolved(const Maze *maze, SolverKind kind, size_t budget)
{
    Facts facts = inspect(maze);
    CHECK(facts.distance >= 0);
    Trace trace = {.maze = maze, .requireStop = 1};
    Robot *robot = robotCreate(maze, observe, &trace);
    CHECK(robot != NULL);
    SearchStats stats;
    CHECK(solve(robot, kind, budget, &stats) == SEARCH_SOLVED);
    CHECK(atMarker(robot) && trace.sawExit);
    CHECK(trace.moves == robotMoves(robot) && trace.turns == robotTurns(robot));
    CHECK(stats.actions == trace.moves + trace.turns);
    if (kind == SOLVER_DEEPENING) CHECK(stats.depth == (unsigned)facts.distance);
    robotDestroy(robot);
}

static void testRobot(void)
{
    const int entryX[] = {2, 4, 2, 0};
    const int entryY[] = {0, 2, 4, 2};
    const int facing[] = {SOUTH, WEST, NORTH, EAST};
    for (int side = 0; side < 4; side++)
    {
        Maze maze = {0};
        CHECK(mazeCreate(&maze, 5, 5));
        for (int y = 1; y < 4; y++)
            for (int x = 1; x < 4; x++) mazeSetTile(&maze, x, y, TILE_EMPTY);
        maze.entranceX = entryX[side];
        maze.entranceY = entryY[side];
        maze.startDirection = facing[side];
        mazeSetTile(&maze, maze.entranceX, maze.entranceY, TILE_EMPTY);
        mazeSetTile(&maze, 4 - maze.entranceX, 4 - maze.entranceY, TILE_EXIT);
        Trace trace = {.maze = &maze};
        Robot *robot = robotCreate(&maze, observe, &trace);
        CHECK(robot != NULL && !atMarker(robot));
        RobotView initial = robotView(robot);
        for (int i = 0; i < 4; i++) left(robot);
        CHECK(samePose(initial, robotView(robot)));
        for (int i = 0; i < 4; i++) right(robot);
        CHECK(samePose(initial, robotView(robot)));
        left(robot);
        right(robot);
        CHECK(samePose(initial, robotView(robot)));
        right(robot);
        right(robot);
        CHECK(!canMoveForward(robot));
        RobotView outsideFacing = robotView(robot);
        size_t moves = robotMoves(robot);
        forward(robot);
        CHECK(samePose(outsideFacing, robotView(robot)) && robotMoves(robot) == moves);
        right(robot);
        right(robot);
        for (int i = 0; i < 4; i++)
        {
            CHECK(canMoveForward(robot));
            forward(robot);
        }
        CHECK(atMarker(robot) && !canMoveForward(robot));
        RobotView atExit = robotView(robot);
        forward(robot);
        CHECK(samePose(atExit, robotView(robot)));
        CHECK(robotMoves(robot) == 4 && trace.moves == 4);
        robotDestroy(robot);
        maze.startDirection = 99;
        CHECK(robotCreate(&maze, NULL, NULL) == NULL);
        mazeDestroy(&maze);
    }
    CHECK(robotCreate(NULL, NULL, NULL) == NULL);
}

static Maze corridor(void)
{
    Maze maze = {0};
    CHECK(mazeCreate(&maze, 5, 5));
    maze.entranceX = 0;
    maze.entranceY = 2;
    for (int x = 0; x < 5; x++) mazeSetTile(&maze, x, 2, TILE_EMPTY);
    mazeSetTile(&maze, 4, 2, TILE_EXIT);
    return maze;
}

static void testSearch(void)
{
    Maze maze = corridor();
    for (int direction = 0; direction < 4; direction++)
    {
        maze.startDirection = direction;
        checkSolved(&maze, SOLVER_DEEPENING, 10000);
    }
    maze.startDirection = EAST;
    checkSolved(&maze, SOLVER_WALL, 10000);
    for (size_t budget = 1; budget <= 80; budget++)
    {
        Trace trace = {.maze = &maze, .requireStop = 1};
        Robot *robot = robotCreate(&maze, observe, &trace);
        SearchStats stats;
        CHECK(robot != NULL);
        SearchResult result = solve(robot, SOLVER_DEEPENING, budget, &stats);
        CHECK(result == SEARCH_BUDGET || result == SEARCH_SOLVED);
        CHECK(stats.actions <= budget);
        if (result == SEARCH_BUDGET) CHECK(stats.actions == budget);
        CHECK(stats.actions == robotMoves(robot) + robotTurns(robot));
        robotDestroy(robot);
    }
    mazeDestroy(&maze);

    CHECK(mazeCreate(&maze, 5, 5));
    maze.entranceX = 0;
    maze.entranceY = 2;
    for (int x = 0; x < 4; x++) mazeSetTile(&maze, x, 2, TILE_EMPTY);
    mazeSetTile(&maze, 1, 1, TILE_EMPTY);
    mazeSetTile(&maze, 2, 3, TILE_EMPTY);
    mazeSetTile(&maze, 4, 4, TILE_EXIT);
    CHECK(inspect(&maze).distance == -1);
    for (int direction = 0; direction < 4; direction++)
    {
        maze.startDirection = direction;
        Robot *robot = robotCreate(&maze, NULL, NULL);
        SearchStats stats;
        CHECK(robot != NULL);
        RobotView initial = robotView(robot);
        CHECK(solve(robot, SOLVER_DEEPENING, 10000, &stats) == SEARCH_NO_ROUTE);
        CHECK(samePose(initial, robotView(robot)));
        robotDestroy(robot);
    }
    mazeDestroy(&maze);

    CHECK(mazeCreate(&maze, 5, 5));
    maze.entranceX = 0;
    maze.entranceY = 1;
    maze.startDirection = EAST;
    mazeSetTile(&maze, 0, 1, TILE_EMPTY);
    for (int y = 1; y < 4; y++)
        for (int x = 1; x < 4; x++)
            if (x != 2 || y != 2) mazeSetTile(&maze, x, y, TILE_EMPTY);
    mazeSetTile(&maze, 4, 4, TILE_EXIT);
    Trace trace = {.maze = &maze, .requireStop = 1};
    Robot *robot = robotCreate(&maze, observe, &trace);
    SearchStats stats;
    CHECK(robot != NULL);
    CHECK(solve(robot, SOLVER_DEEPENING, 5000, &stats) == SEARCH_BUDGET);
    CHECK(stats.actions == 5000 && !trace.sawExit);
    robotDestroy(robot);
    mazeDestroy(&maze);
}

static void testGenerated(void)
{
    char error[256];
    for (unsigned seed = 0; seed < 20; seed++)
    {
        for (unsigned loops = 0; loops <= 100; loops += 50)
        {
            Maze maze = {0};
            CHECK(mazeGenerate(&maze, 7, 7, seed, loops, error, sizeof(error)));
            Facts facts = inspect(&maze);
            CHECK(facts.openings == 2 && facts.exits == 1 && facts.distance > 0);
            CHECK(facts.passable == facts.reachable);
            if (loops == 0)
            {
                CHECK(facts.edges == facts.passable - 1);

            }
            if (loops == 100) CHECK(facts.edges > facts.passable - 1);
            checkSolved(&maze, SOLVER_WALL, 10000);
            checkSolved(&maze, SOLVER_DEEPENING, 5000000);
            Maze copy = {0};
            CHECK(mazeGenerate(&copy, 7, 7, seed, loops, error, sizeof(error)));
            CHECK(copy.entranceX == maze.entranceX && copy.entranceY == maze.entranceY);
            CHECK(memcmp(copy.cells, maze.cells, 49 * sizeof(*maze.cells)) == 0);
            mazeDestroy(&copy);
            mazeDestroy(&maze);
        }
    }
    const int dimensions[][2] = {{5, 5}, {5, 101}, {101, 5}, {101, 101}};
    for (size_t i = 0; i < sizeof(dimensions) / sizeof(dimensions[0]); i++)
    {
        Maze maze = {0};
        CHECK(mazeGenerate(&maze, dimensions[i][0], dimensions[i][1], 314, 0,
                           error, sizeof(error)));
        Facts facts = inspect(&maze);
        CHECK(facts.openings == 2 && facts.exits == 1 && facts.distance > 0);
        CHECK(facts.passable == facts.reachable && facts.edges == facts.passable - 1);
        checkSolved(&maze, SOLVER_WALL, 20u * (size_t)maze.width * (size_t)maze.height);
        mazeDestroy(&maze);
    }
}

static void writeBytes(const char *path, const char *data, size_t length)
{
    FILE *file = fopen(path, "wb");
    CHECK(file != NULL);
    CHECK(fwrite(data, 1, length, file) == length);
    CHECK(fclose(file) == 0);
}

static void testFiles(int argc, char **argv)
{
    char error[256];
    Maze maze = {0};
    CHECK(mazeCreateDefault(&maze));
    checkSolved(&maze, SOLVER_DEEPENING, 100000);
    CHECK(mazeSave(&maze, "build/test-roundtrip.txt", error, sizeof(error)));
    Maze loaded = {0};
    CHECK(mazeLoad(&loaded, "build/test-roundtrip.txt", error, sizeof(error)));
    CHECK(maze.width == loaded.width && maze.height == loaded.height);
    CHECK(maze.entranceX == loaded.entranceX && maze.entranceY == loaded.entranceY);
    CHECK(memcmp(maze.cells, loaded.cells,
                 (size_t)maze.width * (size_t)maze.height * sizeof(*maze.cells)) == 0);
    mazeDestroy(&loaded);
    CHECK(remove("build/test-roundtrip.txt") == 0);

    const char *valid = "#####\r\nS...#\r\n###.#\r\n#...E\r\n#####";
    writeBytes("build/test-input.txt", valid, strlen(valid));
    CHECK(mazeLoad(&loaded, "build/test-input.txt", error, sizeof(error)));
    checkSolved(&loaded, SOLVER_DEEPENING, 10000);
    mazeDestroy(&loaded);
    const char *invalid[] = {
        "", "#####\nS...#\n###.#\n#..E\n#####\n",
        "#####\nS.X.#\n###.#\n#...E\n#####\n",
        "#####\nS...#\n###.#\n#....\n#####\n",
        "#####\nS...#\n#####\n#...E\n#####\n",
        "#####\nS...#\nS##.#\n#...E\n#####\n",
        "#####\nS...#\n###.#\nE...E\n#####\n"
    };
    Tile *originalCells = maze.cells;
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
    {
        writeBytes("build/test-input.txt", invalid[i], strlen(invalid[i]));
        error[0] = '\0';
        CHECK(!mazeLoad(&maze, "build/test-input.txt", error, sizeof(error)));
        CHECK(error[0] != '\0' && maze.cells == originalCells);
    }
    char nulInput[] = "#####\nS...#\n###.#\n#...E\n#####\n";
    nulInput[8] = '\0';
    writeBytes("build/test-input.txt", nulInput, sizeof(nulInput) - 1);
    CHECK(!mazeLoad(&maze, "build/test-input.txt", error, sizeof(error)));
    CHECK(maze.cells == originalCells);
    CHECK(remove("build/test-input.txt") == 0);
    CHECK(!mazeGenerate(&maze, 6, 7, 1, 0, error, sizeof(error)));
    CHECK(!mazeGenerate(&maze, 7, 7, 1, 101, error, sizeof(error)));
    CHECK(maze.cells == originalCells);
    mazeDestroy(&maze);
    for (int i = 1; i < argc; i++)
    {
        CHECK(mazeLoad(&maze, argv[i], error, sizeof(error)));
        checkSolved(&maze, SOLVER_DEEPENING, 5000000);
        checkSolved(&maze, SOLVER_WALL, 100000);
        mazeDestroy(&maze);
    }
}

/* Exhaust all 5x5 interiors and distinct ordered boundary openings. */
static void testBoundaryWallFollowing(void)
{
    const int bx[] = {1,2,3,4,4,4,3,2,1,0,0,0};
    const int by[] = {0,0,0,1,2,3,4,4,4,3,2,1};
    unsigned valid = 0;
    for (unsigned pattern = 0; pattern < 512; pattern++)
    {
        for (int entrance = 0; entrance < 12; entrance++)
        {
            for (int exitIndex = 0; exitIndex < 12; exitIndex++)
            {
                if (entrance == exitIndex) continue;
                Maze maze = {0};
                CHECK(mazeCreate(&maze, 5, 5));
                for (int i = 0; i < 9; i++)
                    if (pattern & (1u << i))
                        mazeSetTile(&maze, i % 3 + 1, i / 3 + 1, TILE_EMPTY);
                maze.entranceX = bx[entrance];
                maze.entranceY = by[entrance];
                mazeSetTile(&maze, bx[entrance], by[entrance], TILE_EMPTY);
                mazeSetTile(&maze, bx[exitIndex], by[exitIndex], TILE_EXIT);
                Facts facts = inspect(&maze);
                CHECK(mazeValidate(&maze, NULL, 0) == (facts.distance >= 0));
                if (facts.distance >= 0)
                {
                    valid++;
                    checkSolved(&maze, SOLVER_WALL, 1000);
                }
                mazeDestroy(&maze);
            }
        }
    }
    CHECK(valid == 18676);
    printf("Exhaustive boundary test: all %u reachable 5x5 cases solved.\n", valid);
}

int main(int argc, char **argv)
{
    testRobot();
    testSearch();
    testGenerated();
    testFiles(argc, argv);
    testBoundaryWallFollowing();
    puts("Core checks passed: robot contracts, search traces/BFS oracle, budgets, generation, files.");
    return EXIT_SUCCESS;
}
