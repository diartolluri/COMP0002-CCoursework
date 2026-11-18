/*declares search modes, results and statistics*/

#ifndef MAZE_SOLVER_H
#define MAZE_SOLVER_H

#include <stddef.h>
#include "maze_robot.h"

/*a budget stop is a resource limit, not proof that no route exists*/
typedef enum { SOLVER_DEEPENING, SOLVER_WALL } SolverKind;
typedef enum
{
    SEARCH_SOLVED, SEARCH_NO_ROUTE, SEARCH_BUDGET, SEARCH_MEMORY, SEARCH_DEPTH
} SearchResult;
typedef struct
{
    /*actions include inspection turns and physical backtracking*/
    size_t actions;
    unsigned iterations;
    unsigned depth;
    /*decision points to a fixed message and should not be freed*/
    const char *decision;
} SearchStats;

SearchResult solve(Robot *robot, SolverKind kind, size_t budget, SearchStats *stats);
const char *searchResultName(SearchResult result);

#endif
