/*defines rendererer state and the drawing interface*/

#ifndef MAZE_RENDER_H
#define MAZE_RENDER_H

#include <stdbool.h>
#include <stddef.h>
#include "maze_grid.h"
#include "maze_robot_internal.h"
#include "maze_solver.h"

typedef enum { RENDER_TEXT, RENDER_DRAWAPP } RenderMode;

typedef struct
{
    RenderMode mode;
    int delay;
    /*zero keeps automatic sizing, explicit sizes are checked against window limits*/
    int cellSize;
    SolverKind solver;
    size_t budget;
    const SearchStats *stats;
    unsigned seed;
    unsigned loops;
    bool generated;
} RenderSettings;

typedef struct
{
    /*the maze and search stats are borrowed, only visits is owned here*/
    const Maze *maze;
    RenderSettings settings;
    int cellSize;
    int windowWidth;
    int windowHeight;
    int gridX;
    int gridY;
    int cardWidth;
    int cardHeight;
    int panelX;
    size_t moves;
    size_t turns;
    size_t visitedCount;
    size_t passageCount;
    /*arrival counts stop at four, matching the last colour band*/
    unsigned char *visits;
    RobotView previous;
    bool hasPrevious;
    bool finished;
    bool solved;
    /*cpu time excludes the viewer, playback time comes from the chosen delay*/
    double cpuSeconds;
    /*cached values avoid sending unchanged dashboard text on every action*/
    size_t hudActions;
    size_t hudMoves;
    size_t hudTurns;
    size_t hudVisited;
    unsigned hudIterations;
    unsigned hudDepth;
    bool hudInitialized;
    bool hudFinished;
    bool hudAhead;
    const char *hudDecision;
} Renderer;

int renderInit(Renderer *renderer, const Maze *maze, const RenderSettings *settings);
void renderDestroy(Renderer *renderer);
void renderRobot(const RobotView *view, void *context);
void renderPreview(const Maze *maze, const RobotView *view);
void renderFinish(Renderer *renderer, int solved);

#endif
