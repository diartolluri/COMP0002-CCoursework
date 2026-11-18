/*sets up the maze, runs the robot search and releases the resources*/

#include <stdio.h>
#include <time.h>
#include "maze_config.h"
#include "maze_generator.h"
#include "maze_grid.h"
#include "maze_render.h"
#include "maze_robot_internal.h"
#include "maze_solver.h"

static int createMaze(Maze *maze, const Options *options, char *error, size_t capacity)
{
    if (options->file != NULL)
        return mazeLoad(maze, options->file, error, capacity);
    if (options->generate)
        return mazeGenerate(maze, options->width, options->height, options->seed,
                            options->loops, error, capacity);
    return mazeCreateDefault(maze);
}

static bool prepareMaze(Maze *maze, const Options *options)
{
    char error[256] = "Could not allocate the default maze.";
    /*short-circuit checks keep an invalid grid from being saved*/
    if (!createMaze(maze, options, error, sizeof(error)) ||
        !mazeValidate(maze, error, sizeof(error)) ||
        (options->save != NULL && !mazeSave(maze, options->save, error, sizeof(error))))
    {
        fprintf(stderr, "%s\n", error);
        return false;
    }
    return true;
}

/*the renderer borrows stats while animateMaze keeps them alive*/
static RenderSettings makeRenderSettings(const Options *options, const SearchStats *stats)
{
    return (RenderSettings){
        .mode = options->graphics ? RENDER_DRAWAPP : RENDER_TEXT,
        .delay = options->delay, .cellSize = options->cellSize, .solver = options->solver, .budget = options->budget,
        .stats = stats, .seed = options->seed, .loops = options->loops,
        .generated = options->generate
    };
}

/*stderr stays visible when stdout is piped into drawapp*/
static void reportSearch(SearchResult result, const Robot *robot, const SearchStats *stats)
{
    fprintf(stderr, "%s: %zu moves, %zu turns, %zu actions; %u iterations, depth %u.\n",
            searchResultName(result), robotMoves(robot), robotTurns(robot), stats->actions,
            stats->iterations, stats->depth);
    if (result == SEARCH_BUDGET)
        fputs("The budget is a resource guard, not evidence that the maze is unsolvable.\n", stderr);
}

static int runSearch(Robot *robot, Renderer *renderer, const Options *options, SearchStats *stats)
{
    /*clock measures c-side work, not the java viewer's playback time*/
    clock_t started = clock();
    SearchResult result = solve(robot, options->solver, options->budget, stats);
    clock_t ended = clock();
    renderer->cpuSeconds = started == (clock_t)-1 || ended == (clock_t)-1 ? 0.0 :
                           (double)(ended - started) / CLOCKS_PER_SEC;
    if (!options->graphics)
    {
        RobotView view = robotView(robot);
        renderPreview(renderer->maze, &view);
    }
    renderFinish(renderer, result == SEARCH_SOLVED);
    reportSearch(result, robot, stats);
    return result == SEARCH_SOLVED ? 0 : 2;
}

/*robot actions call the renderer, so the solver only has to choose moves*/
static int animateMaze(Maze *maze, const Options *options)
{
    SearchStats stats = {0};
    RenderSettings settings = makeRenderSettings(options, &stats);
    Renderer renderer;
    if (!renderInit(&renderer, maze, &settings)) return 1;
    Robot *robot = robotCreate(maze, renderRobot, &renderer);
    int status = 1;
    if (robot == NULL)
        fputs("Could not allocate the robot.\n", stderr);
    else
        status = runSearch(robot, &renderer, options, &stats);
    robotDestroy(robot);
    renderDestroy(&renderer);
    return status;
}

int main(int argc, char **argv)
{
    Options options;
    OptionsResult parsed = parseOptions(argc, argv, &options);
    if (parsed == OPTIONS_HELP) return 0;
    if (parsed == OPTIONS_ERROR) return 1;
    Maze maze = {0};
    int status = 1;
    if (prepareMaze(&maze, &options))
    {
        if (options.preview)
        {
            renderPreview(&maze, NULL);
            status = 0;
        }
        else status = animateMaze(&maze, &options);
    }
    /*the robot and renderer borrow the grid, so the maze is freed last*/
    mazeDestroy(&maze);
    return status;
}
