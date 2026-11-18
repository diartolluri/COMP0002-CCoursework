/*traverses mazes using wall following or iterative deepening*/

#include "maze_solver.h"
#include <limits.h>
#include <stdlib.h>

/*failed iddfs branches restore their entry pose, frames hold no maze map*/
/*iterative deepening reference: Russell/Norvig, AIMA, chapter 3
   https://aima.cs.berkeley.edu/4th-ed/pdfs/newchap03.pdf*/
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

typedef enum { STEP_CONTINUE, STEP_BUDGET, STEP_MEMORY } SearchStep;

static Frame *makeFrame(Frame *parent, unsigned remaining, unsigned rotation)
{
    Frame *frame = malloc(sizeof(*frame));
    if (frame != NULL) *frame = (Frame){parent, remaining, 0, rotation};
    return frame;
}

static int finishBranch(Search *search, Frame **top, int *cutoff)
{
    Frame *frame = *top;
    /*a depth cutoff tells the next pass to retry with a larger limit*/
    if (frame->remaining == 0) *cutoff = 1;
    /*walk back before dropping the frame so later choices start at the parent*/
    if (frame->parent != NULL && !returnToParent(search, frame)) return 0;
    *top = frame->parent;
    free(frame);
    return 1;
}

static SearchStep exploreChoice(Search *search, Frame **top)
{
    static const unsigned choices[] = {0, 1, 3, 2};
    Frame *frame = *top;
    unsigned rotation = choices[frame->nextChoice++];
    search->stats->decision = "Inspecting another passage";
    if (!turn(search, rotation)) return STEP_BUDGET;
    if (!canMoveForward(search->robot))
    {
        search->stats->decision = "Passage blocked; restoring heading";
        return turn(search, (4 - rotation) % 4) ? STEP_CONTINUE : STEP_BUDGET;
    }
    Frame *child = makeFrame(frame, frame->remaining - 1, rotation);
    if (child == NULL) return STEP_MEMORY;
    search->stats->decision = "Passage clear; moving forward";
    if (!perform(search, forward))
    {
        free(child);
        return STEP_BUDGET;
    }
    *top = child;
    return STEP_CONTINUE;
}

static SearchResult depthFirst(Search *search, unsigned limit, int *cutoff)
{
    Frame *top = makeFrame(NULL, limit, 0);
    if (top == NULL) return SEARCH_MEMORY;
    *cutoff = 0;
    while (top != NULL)
    {
        if (atMarker(search->robot))
        {
            releaseFrames(top);
            return SEARCH_SOLVED;
        }
        /*children skip the back edge because it leads straight to their parent*/
        unsigned choiceCount = top->parent == NULL ? 4 : 3;
        SearchStep step;
        if (top->remaining == 0 || top->nextChoice == choiceCount)
            step = finishBranch(search, &top, cutoff) ? STEP_CONTINUE : STEP_BUDGET;
        else step = exploreChoice(search, &top);
        if (step != STEP_CONTINUE)
        {
            releaseFrames(top);
            return step == STEP_MEMORY ? SEARCH_MEMORY : SEARCH_BUDGET;
        }
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

/*right-hand wall following: https://yozh.readthedocs.io/en/latest/projects/maze1.html
   boundary entrance and exit select the exterior face of the passage graph*/
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
