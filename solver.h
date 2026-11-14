#ifndef SOLVER_H
#define SOLVER_H

/* AI-assisted reference interfaces: OpenAI Codex, November 2026. */

#include <stddef.h>
#include "robot.h"

typedef enum { SOLVER_DEEPENING, SOLVER_WALL } SolverKind;
typedef enum
{
    SEARCH_SOLVED, SEARCH_NO_ROUTE, SEARCH_BUDGET, SEARCH_MEMORY, SEARCH_DEPTH
} SearchResult;
typedef struct
{
    size_t actions;
    unsigned iterations;
    unsigned depth;
    const char *decision;
} SearchStats;

SearchResult solve(Robot *robot, SolverKind kind, size_t budget, SearchStats *stats);
const char *searchResultName(SearchResult result);

#endif
