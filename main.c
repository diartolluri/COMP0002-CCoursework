#include <stdio.h>
#include <time.h>
#include "config.h"
#include "generator.h"
#include "maze.h"
#include "render.h"
#include "robot_internal.h"
#include "solver.h"
/* AI-assisted reference implementation: OpenAI Codex, November 2026. */

int main(int argc, char **argv)
{
    Options options;
    int parsed = parseOptions(argc, argv, &options);
    if (parsed == 2) return 0;
    if (parsed == 0) return 1;

    Maze maze = {0};
    char error[256] = "Could not allocate the default maze.";
    int created;
    if (options.file != NULL) created = mazeLoad(&maze, options.file, error, sizeof(error));
    else if (options.generate)
        created = mazeGenerate(&maze, options.width, options.height, options.seed,
                               options.loops, error, sizeof(error));
    else created = mazeCreateDefault(&maze);
    if (!created)
    {
        fprintf(stderr, "%s\n", error);
        return 1;
    }
    if (!mazeValidate(&maze, error, sizeof(error)) ||
        (options.save != NULL && !mazeSave(&maze, options.save, error, sizeof(error))))
    {
        fprintf(stderr, "%s\n", error);
        mazeDestroy(&maze);
        return 1;
    }

    if (options.preview)
    {
        renderPreview(&maze, NULL);
        mazeDestroy(&maze);
        return 0;
    }
    SearchStats stats = {0};
    RenderSettings settings = {
        .mode = options.graphics ? RENDER_DRAWAPP : RENDER_TEXT,
        .delay = options.delay, .solver = options.solver, .budget = options.budget,
        .stats = &stats, .seed = options.seed, .loops = options.loops,
        .generated = options.generate
    };
    Renderer renderer;
    if (!renderInit(&renderer, &maze, &settings))
    {
        mazeDestroy(&maze);
        return 1;
    }
    Robot *robot = robotCreate(&maze, renderRobot, &renderer);
    if (robot == NULL)
    {
        fputs("Could not allocate the robot.\n", stderr);
        renderDestroy(&renderer);
        mazeDestroy(&maze);
        return 1;
    }
    clock_t started = clock();
    SearchResult result = solve(robot, options.solver, options.budget, &stats);
    clock_t ended = clock();
    renderer.cpuSeconds = started == (clock_t)-1 || ended == (clock_t)-1 ? 0.0 :
                          (double)(ended - started) / CLOCKS_PER_SEC;
    int solved = result == SEARCH_SOLVED;
    if (!options.graphics)
    {
        RobotView view = robotView(robot);
        renderPreview(&maze, &view);
    }
    renderFinish(&renderer, solved);
    fprintf(stderr, "%s: %zu moves, %zu turns, %zu actions; %u iterations, depth %u.\n",
            searchResultName(result), robotMoves(robot), robotTurns(robot), stats.actions,
            stats.iterations, stats.depth);
    if (result == SEARCH_BUDGET)
        fputs("The budget is a resource guard, not evidence that the maze is unsolvable.\n", stderr);
    robotDestroy(robot);
    renderDestroy(&renderer);
    mazeDestroy(&maze);
    return solved ? 0 : 2;
}
