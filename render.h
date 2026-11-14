#ifndef RENDER_H
#define RENDER_H

/* AI-assisted reference interfaces: OpenAI Codex, November 2026. */

#include <stddef.h>
#include "maze.h"
#include "robot_internal.h"
#include "solver.h"

typedef enum { RENDER_TEXT, RENDER_DRAWAPP } RenderMode;

typedef struct
{
    RenderMode mode;
    int delay;
    SolverKind solver;
    size_t budget;
    const SearchStats *stats;
    unsigned seed;
    unsigned loops;
    int generated;
} RenderSettings;

typedef struct
{
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
    /* Display arrival counts, capped at four for the colour bands. */
    unsigned char *visits;
    RobotView previous;
    int hasPrevious;
    int finished;
    int solved;
    double cpuSeconds;
    size_t hudActions;
    size_t hudMoves;
    size_t hudTurns;
    size_t hudVisited;
    unsigned hudIterations;
    unsigned hudDepth;
    int hudInitialized;
    int hudFinished;
    int hudAhead;
    const char *hudDecision;
} Renderer;

int renderInit(Renderer *renderer, const Maze *maze, const RenderSettings *settings);
void renderDestroy(Renderer *renderer);
void renderRobot(const RobotView *view, void *context);
void renderPreview(const Maze *maze, const RobotView *view);
void renderFinish(Renderer *renderer, int solved);

#endif
