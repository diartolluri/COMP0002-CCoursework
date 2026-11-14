#include "solver.h"
#include <limits.h>
#include <stdlib.h>

/* AI-assisted reference implementation: OpenAI Codex, November 2026.
   Failed IDDFS branches restore their entry pose; frames hold no maze map. */
/* Algorithm: Russell/Norvig, AIMA, chapter 3, iterative deepening.
   https://aima.cs.berkeley.edu/4th-ed/pdfs/newchap03.pdf */
typedef struct Frame
{
    struct Frame *parent;
    unsigned remaining;
    unsigned nextChoice;
    unsigned returnTurn;
} Frame;

typedef struct
{
    Robot *robot;
    size_t budget;
    SearchStats *stats;
} Search;

static int perform(Search *search, void (*action)(Robot *))
{
    if (search->budget != 0 && search->stats->actions >= search->budget)
        return 0;
    search->stats->actions++;
    action(search->robot);
    return 1;
}

static int turn(Search *search, unsigned clockwise)
{
    if (clockwise == 3) return perform(search, left);
    for (unsigned i = 0; i < clockwise; i++)
        if (!perform(search, right)) return 0;
    return 1;
}

static void releaseFrames(Frame *frame)
{
    while (frame != NULL)
    {
        Frame *parent = frame->parent;
        free(frame);
        frame = parent;
    }
}

static int returnToParent(Search *search, Frame *child)
{
    search->stats->decision = "Depth limit / branch done; returning";
    return turn(search, 2) && perform(search, forward) && turn(search, 2) &&
           turn(search, (4 - child->returnTurn) % 4);
}

static SearchResult depthFirst(Search *search, unsigned limit, int *cutoff)
{
    static const unsigned choices[] = {0, 1, 3, 2};
    Frame *top = malloc(sizeof(*top));
    if (top == NULL) return SEARCH_MEMORY;
    *top = (Frame){NULL, limit, 0, 0};
    *cutoff = 0;

    while (top != NULL)
    {
        if (atMarker(search->robot))
        {
            releaseFrames(top);
            return SEARCH_SOLVED;
        }
        unsigned choiceCount = top->parent == NULL ? 4 : 3;
        if (top->remaining == 0 || top->nextChoice == choiceCount)
        {
            if (top->remaining == 0) *cutoff = 1;
            Frame *parent = top->parent;
            if (parent != NULL && !returnToParent(search, top))
            {
                releaseFrames(top);
                return SEARCH_BUDGET;
            }
            free(top);
            top = parent;
            continue;
        }

        unsigned rotation = choices[top->nextChoice++];
        search->stats->decision = "Inspecting another passage";
        if (!turn(search, rotation))
        {
            releaseFrames(top);
            return SEARCH_BUDGET;
        }
        if (!canMoveForward(search->robot))
        {
            search->stats->decision = "Passage blocked; restoring heading";
            if (!turn(search, (4 - rotation) % 4))
            {
                releaseFrames(top);
                return SEARCH_BUDGET;
            }
            continue;
        }

        Frame *child = malloc(sizeof(*child));
        if (child == NULL)
        {
            releaseFrames(top);
            return SEARCH_MEMORY;
        }
        *child = (Frame){top, top->remaining - 1, 0, rotation};
        search->stats->decision = "Passage clear; moving forward";
        if (!perform(search, forward))
        {
            free(child);
            releaseFrames(top);
            return SEARCH_BUDGET;
        }
        top = child;
    }
    return SEARCH_NO_ROUTE;
}

static SearchResult deepen(Search *search)
{
    for (unsigned limit = 1; ; limit++)
    {
        int cutoff = 0;
        search->stats->depth = limit;
        search->stats->iterations++;
        SearchResult result = depthFirst(search, limit, &cutoff);
        if (result != SEARCH_NO_ROUTE || !cutoff) return result;
        if (limit == UINT_MAX) return SEARCH_DEPTH;
    }
}

/* Right-hand wall following: https://yozh.readthedocs.io/en/latest/projects/maze1.html
   Boundary entrance/exit select the same exterior face; see the design notes. */
static SearchResult followWall(Search *search)
{
    static const char *blocked[] = {
        "Right blocked; inspecting ahead",
        "Ahead blocked; inspecting left",
        "Dead end; turning back"
    };
    while (!atMarker(search->robot))
    {
        search->stats->decision = "Inspecting the right passage";
        if (!perform(search, right)) return SEARCH_BUDGET;
        for (unsigned choice = 0; choice < 4; choice++)
        {
            if (canMoveForward(search->robot))
            {
                search->stats->decision = "Passage clear; moving forward";
                if (!perform(search, forward)) return SEARCH_BUDGET;
                break;
            }
            if (choice == 3) return SEARCH_NO_ROUTE;
            search->stats->decision = blocked[choice];
            if (!perform(search, left)) return SEARCH_BUDGET;
        }
    }
    return SEARCH_SOLVED;
}

SearchResult solve(Robot *robot, SolverKind kind, size_t budget, SearchStats *stats)
{
    stats->actions = 0;
    stats->iterations = 0;
    stats->depth = 0;
    stats->decision = "Starting search";
    Search search = {robot, budget, stats};
    if (atMarker(robot)) return SEARCH_SOLVED;
    return kind == SOLVER_WALL ? followWall(&search) : deepen(&search);
}

const char *searchResultName(SearchResult result)
{
    switch (result)
    {
        case SEARCH_SOLVED: return "solved";
        case SEARCH_NO_ROUTE: return "no route";
        case SEARCH_BUDGET: return "action budget exhausted";
        case SEARCH_MEMORY: return "search allocation failed";
        case SEARCH_DEPTH: return "search depth exhausted";
    }
    return "unknown result";
}
