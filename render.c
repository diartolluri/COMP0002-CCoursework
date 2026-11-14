#include "render.h"

#include <stdio.h>
#include <stdlib.h>

/* AI-assisted reference implementation: OpenAI Codex, November 2026.
   Exploration marks are display state only; the solver cannot access them. */

static size_t cellIndex(const Renderer *renderer, int x, int y)
{
    return (size_t)y * (size_t)renderer->maze->width + (size_t)x;
}

static void rememberVisit(Renderer *renderer, const RobotView *view)
{
    size_t index = cellIndex(renderer, view->x, view->y);
    if (!renderer->visits[index])
    {
        renderer->visitedCount++;
    }
    if (renderer->visits[index] < 4) renderer->visits[index]++;
}

#ifdef MAZE_DRAWAPP
#include "graphics.h"

enum { LEFT = 24, TOP = 100, CARD_GAP = 20, PANEL_WIDTH = 280 };
typedef struct { int r, g, b; } Colour;
static const Colour canvas = {241, 245, 249};
static const Colour paper = {255, 255, 255};
static const Colour ink = {24, 40, 59};
static const Colour muted = {104, 122, 141};
static const Colour line = {226, 234, 240};
static const Colour teal = {0, 143, 124};
static const Colour mint = {222, 244, 235};
static const Colour lavender = {231, 223, 248};
static const Colour rose = {250, 215, 220};
static const Colour entranceColour = {178, 214, 250};
static const Colour gold = {255, 204, 102};

static void useColour(Colour value) { setRGBColour(value.r, value.g, value.b); }

static void rectangle(int x, int y, int width, int height, Colour value)
{
    useColour(value);
    fillRect(x, y, width, height);
}

static void text(const char *value, int x, int y, int size, Colour valueColour)
{
    useColour(valueColour);
    setStringTextSize(size);
    /* The supplied function accepts char *, but its implementation only reads it. */
    drawString((char *)value, x, y);
}

static void card(int x, int y, int width, int height)
{
    rectangle(x + 2, y + 3, width, height, (Colour){230, 236, 242});
    rectangle(x, y, width, height, paper);
}

static void drawTile(const Renderer *renderer, int x, int y, int withVisit)
{
    const Maze *maze = renderer->maze;
    if (x < 0 || y < 0 || x >= maze->width || y >= maze->height) return;
    int cell = renderer->cellSize;
    int px = renderer->gridX + x * cell;
    int py = renderer->gridY + y * cell;
    Tile tile = mazeTileAt(maze, x, y);
    int entry = x == maze->entranceX && y == maze->entranceY;
    Colour fill = tile == TILE_WALL ? ink : paper;
    unsigned visits = withVisit ? renderer->visits[cellIndex(renderer, x, y)] : 0;
    if (visits > 0 && tile != TILE_WALL)
        fill = visits == 1 ? mint : visits < 4 ? lavender : rose;
    if (entry) fill = entranceColour;
    if (tile == TILE_EXIT) fill = gold;
    rectangle(px, py, cell, cell, fill);
    if (tile != TILE_WALL && cell >= 10)
    {
        useColour(line);
        drawRect(px, py, cell, cell);
    }
    if (visits > 0)
    {
        if (cell >= 20)
        {
            char count[3] = {(char)('0' + visits), '\0', '\0'};
            if (visits == 4) count[1] = '+';
            text(count, px + cell / 2 - (visits == 4 ? 6 : 3), py + cell / 2 + 4,
                 10, muted);
        }
        else
        {
            useColour(muted);
            fillOval(px + (cell - 2) / 2, py + (cell - 2) / 2, 2, 2);
        }
    }
}

static void drawLegend(const Renderer *renderer)
{
    int bottom = TOP + renderer->cardHeight;
    text("CELL KEY / visits count arrivals, not turns", LEFT + 24, bottom - 44, 9, muted);
    const char *labels[] = {"Wall", "Unvisited", "1 visit", "2-3 visits", "4+ visits",
                            "Entrance", "Exit", "Robot", "Clear", "Blocked"};
    const Colour colours[] = {ink, paper, mint, lavender, rose, entranceColour,
                              gold, teal, teal, {183, 109, 33}};
    for (int n = 0; n < 10; n++)
    {
        int x = LEFT + 24 + (n % 5) * 84;
        int y = bottom - 28 + (n / 5) * 16;
        if (n == 7)
        {
            int px[] = {x, x, x + 8};
            int py[] = {y - 9, y - 1, y - 5};
            useColour(teal);
            drawPolygon(3, px, py);
        }
        else if (n >= 8)
        {
            useColour(colours[n]);
            drawRect(x, y - 9, 8, 8);
        }
        else
        {
            rectangle(x, y - 9, 8, 8, colours[n]);
            if (n == 1) { useColour(line); drawRect(x, y - 9, 8, 8); }
        }
        text(labels[n], x + 13, y, 10, muted);
    }
}

static void drawDashboardStatic(const Renderer *renderer);

static void drawBackground(const Renderer *renderer)
{
    setWindowSize(renderer->windowWidth, renderer->windowHeight);
    background();
    clear();
    setLineWidth(1);
    rectangle(0, 0, renderer->windowWidth, renderer->windowHeight, canvas);
    text("ROBOT LAB  /  LIVE EXPLORATION", LEFT, 29, 11, teal);
    text("Maze Explorer", LEFT, 68, 32, ink);
    char source[128];
    if (renderer->settings.generated)
        snprintf(source, sizeof(source), "Generated maze  /  seed %u  /  loops %u%%",
                 renderer->settings.seed, renderer->settings.loops);
    else snprintf(source, sizeof(source), "Designed / loaded maze  /  %d x %d cells",
                  renderer->maze->width, renderer->maze->height);
    text(source, LEFT, 94, 13, muted);
    card(LEFT, TOP, renderer->cardWidth, renderer->cardHeight);
    text("THE MAZE", LEFT + 24, TOP + 28, 11, muted);
    text("FRONT SENSOR", LEFT + renderer->cardWidth - 151, TOP + 28, 10, muted);
    for (int y = 0; y < renderer->maze->height; y++)
        for (int x = 0; x < renderer->maze->width; x++) drawTile(renderer, x, y, 0);
    drawLegend(renderer);
    drawDashboardStatic(renderer);
    foreground();
    clear();
}

static void drawStatus(const Renderer *renderer)
{
    const char *status = renderer->finished ?
        (renderer->solved ? "EXIT FOUND" : "SEARCH STOPPED") : "EXPLORING";
    Colour statusColour = renderer->finished && !renderer->solved ?
        (Colour){193, 122, 36} : teal;
    rectangle(renderer->panelX, 38, PANEL_WIDTH, 39, paper);
    useColour(statusColour);
    fillOval(renderer->panelX + 17, 53, 8, 8);
    text(status, renderer->panelX + 39, 63, 12, statusColour);
}

static void progressBar(int x, int y, size_t value, size_t total, Colour fill)
{
    int width = PANEL_WIDTH - 40;
    rectangle(x, y, width, 6, line);
    double fraction = total == 0 ? 0.0 : (double)value / (double)total;
    if (fraction > 1.0) fraction = 1.0;
    if (fraction > 0.0) rectangle(x, y, (int)(fraction * width), 6, fill);
}

static void drawDashboardStatic(const Renderer *renderer)
{
    int x = renderer->panelX;
    card(x, 100, PANEL_WIDTH, 108);
    text("MOVES", x + 20, 124, 11, muted);
    text("TURNS", x + 154, 124, 11, muted);
    text("Quarter turns", x + 154, 191, 11, muted);
    card(x, 220, PANEL_WIDTH, 96);
    text("EXPLORATION", x + 20, 243, 11, muted);
    card(x, 328, PANEL_WIDTH, 110);
    text("SEARCH", x + 20, 351, 11, muted);
    text(renderer->settings.solver == SOLVER_DEEPENING ? "Iterative deepening" : "Wall following",
         x + 20, 376, 18, ink);
    if (renderer->settings.solver == SOLVER_DEEPENING)
    {
        text("PASS", x + 20, 398, 11, muted);
        text("DEPTH", x + 154, 398, 11, muted);
    }
    else
    {
        text("PRIORITY", x + 20, 398, 11, muted);
        text("Right > forward > left > back", x + 20, 423, 13, ink);
    }
    card(x, 450, PANEL_WIDTH, 148);
    text("RUN BUDGET", x + 20, 473, 11, muted);
    text("PLAYBACK TIME", x + 20, 538, 11, muted);
    char value[96];
    snprintf(value, sizeof(value), "Simulated / %d ms per action", renderer->settings.delay);
    text(value, x + 20, 587, 11, muted);
}

static void drawDashboard(Renderer *renderer)
{
    int x = renderer->panelX;
    SearchStats empty = {0};
    const SearchStats *stats = renderer->settings.stats != NULL ? renderer->settings.stats : &empty;
    int first = !renderer->hudInitialized;
    char value[128];
    if (first || renderer->hudFinished != renderer->finished) drawStatus(renderer);
    if (first || renderer->hudMoves != renderer->moves)
    {
        rectangle(x + 18, 134, 123, 43, paper);
        snprintf(value, sizeof(value), "%zu", renderer->moves);
        text(value, x + 20, 168, renderer->moves >= 100000 ? 22 : 32, ink);
        rectangle(x + 18, 179, 125, 17, paper);
        size_t revisits = renderer->moves + 1 - renderer->visitedCount;
        snprintf(value, sizeof(value), "%zu revisits", revisits);
        text(value, x + 20, 191, 11, muted);
    }
    if (first || renderer->hudTurns != renderer->turns)
    {
        rectangle(x + 152, 134, 118, 43, paper);
        snprintf(value, sizeof(value), "%zu", renderer->turns);
        text(value, x + 154, 168, renderer->turns >= 100000 ? 22 : 32, ink);
    }
    if (first || renderer->hudVisited != renderer->visitedCount)
    {
        rectangle(x + 18, 252, PANEL_WIDTH - 36, 27, paper);
        snprintf(value, sizeof(value), "%zu / %zu open cells", renderer->visitedCount,
                 renderer->passageCount);
        text(value, x + 20, 272, 19, ink);
        progressBar(x + 20, 284, renderer->visitedCount, renderer->passageCount, teal);
        rectangle(x + 18, 294, PANEL_WIDTH - 36, 17, paper);
        double coverage = 100.0 * (double)renderer->visitedCount / renderer->passageCount;
        snprintf(value, sizeof(value), "%.1f%% visited", coverage);
        text(value, x + 20, 306, 11, muted);
    }
    if (renderer->settings.solver == SOLVER_DEEPENING &&
        (first || renderer->hudIterations != stats->iterations || renderer->hudDepth != stats->depth))
    {
        rectangle(x + 18, 406, PANEL_WIDTH - 36, 25, paper);
        snprintf(value, sizeof(value), "%u", stats->iterations);
        text(value, x + 20, 426, 22, ink);
        snprintf(value, sizeof(value), "%u", stats->depth);
        text(value, x + 154, 426, 22, ink);
    }
    if (first || renderer->hudActions != stats->actions)
    {
        rectangle(x + 18, 482, PANEL_WIDTH - 36, 24, paper);
        if (renderer->settings.budget != 0)
            snprintf(value, sizeof(value), "%zu / %zu actions", stats->actions, renderer->settings.budget);
        else snprintf(value, sizeof(value), "%zu actions / unlimited", stats->actions);
        text(value, x + 20, 500, 15, ink);
        progressBar(x + 20, 512, stats->actions, renderer->settings.budget, (Colour){94, 137, 196});
        rectangle(x + 18, 544, PANEL_WIDTH - 36, 31, paper);
        double seconds = ((double)renderer->moves + renderer->turns) * renderer->settings.delay / 1000.0;
        snprintf(value, sizeof(value), "%.2f s", seconds);
        text(value, x + 20, 566, 25, ink);
    }
    renderer->hudMoves = renderer->moves;
    renderer->hudTurns = renderer->turns;
    renderer->hudVisited = renderer->visitedCount;
    renderer->hudActions = stats->actions;
    renderer->hudIterations = stats->iterations;
    renderer->hudDepth = stats->depth;
    renderer->hudFinished = renderer->finished;
    renderer->hudInitialized = 1;
}

static void drawFeedback(Renderer *renderer, const RobotView *view)
{
    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};
    int nx = view->x + dx[view->direction];
    int ny = view->y + dy[view->direction];
    int clearAhead = mazeTileAt(renderer->maze, nx, ny) != TILE_WALL;
    Colour sensor = clearAhead ? teal : (Colour){183, 109, 33};
    int cell = renderer->cellSize;
    if (nx >= 0 && ny >= 0 && nx < renderer->maze->width && ny < renderer->maze->height)
    {
        useColour(sensor);
        drawRect(renderer->gridX + nx * cell + 1, renderer->gridY + ny * cell + 1,
                 cell - 2, cell - 2);
    }
    int first = renderer->hudDecision == NULL;
    if (first || renderer->hudAhead != clearAhead)
    {
        int x = LEFT + renderer->cardWidth - 66;
        rectangle(x, TOP + 16, 54, 17, paper);
        text(clearAhead ? "CLEAR" : "BLOCKED", x, TOP + 28, 10, sensor);
        renderer->hudAhead = clearAhead;
    }
    const char *decision = renderer->settings.stats != NULL ? renderer->settings.stats->decision : NULL;
    if (decision == NULL) decision = "Starting search";
    if (renderer->finished) decision = renderer->solved ? "Exit marker detected; stopping" : "Search stopped: inspect the result message";
    if (first || renderer->hudDecision != decision)
    {
        int y = TOP + renderer->cardHeight - 62;
        rectangle(LEFT + 22, y - 13, renderer->cardWidth - 44, 18, paper);
        text(decision, LEFT + 24, y, 11, ink);
        renderer->hudDecision = decision;
    }
}

static void drawRobotAt(const Renderer *renderer, const RobotView *view, double gridX, double gridY)
{
    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};
    int cell = renderer->cellSize;
    int centerX = renderer->gridX + (int)((gridX + 0.5) * cell);
    int centerY = renderer->gridY + (int)((gridY + 0.5) * cell);
    int radius = cell * 3 / 8;
    if (radius < 1) radius = 1;
    int direction = view->direction;
    int x[] = {centerX + dx[direction] * radius,
               centerX - dx[direction] * radius / 2 - dy[direction] * radius,
               centerX - dx[direction] * radius / 2 + dy[direction] * radius};
    int y[] = {centerY + dy[direction] * radius,
               centerY - dy[direction] * radius / 2 + dx[direction] * radius,
               centerY - dy[direction] * radius / 2 - dx[direction] * radius};
    useColour(teal);
    fillPolygon(3, x, y);
    useColour((Colour){0, 93, 83});
    drawPolygon(3, x, y);
}

static void drawAction(Renderer *renderer, const RobotView *view, int moved)
{
    int frames = moved && renderer->cellSize >= 10 && renderer->settings.delay >= 12 ? 3 : 1;
    for (int frame = 1; frame <= frames; frame++)
    {
        double fraction = (double)frame / frames;
        double x = view->x;
        double y = view->y;
        if (renderer->hasPrevious)
        {
            static const int dx[] = {0, 1, 0, -1};
            static const int dy[] = {-1, 0, 1, 0};
            drawTile(renderer, renderer->previous.x + dx[renderer->previous.direction],
                     renderer->previous.y + dy[renderer->previous.direction], 1);
            drawTile(renderer, renderer->previous.x, renderer->previous.y, 1);
            if (moved) drawTile(renderer, view->x, view->y, 1);
            x = renderer->previous.x + (view->x - renderer->previous.x) * fraction;
            y = renderer->previous.y + (view->y - renderer->previous.y) * fraction;
        }
        else drawTile(renderer, view->x, view->y, 1);
        drawRobotAt(renderer, view, x, y);
        if (frame == frames)
        {
            drawFeedback(renderer, view);
            unsigned stride = renderer->settings.delay == 0 ? 32u :
                              (unsigned)(40 + renderer->settings.delay - 1) / renderer->settings.delay;
            if (!renderer->hudInitialized || renderer->moves + renderer->turns - renderer->hudActions >= stride)
                drawDashboard(renderer);
        }
        if (renderer->hasPrevious && renderer->settings.delay > 0)
        {
            int delay = renderer->settings.delay / frames;
            if (frame <= renderer->settings.delay % frames) delay++;
            if (delay > 0) sleep(delay);
        }
    }
}
#endif

int renderInit(Renderer *renderer, const Maze *maze, const RenderSettings *settings)
{
    if (renderer == NULL || maze == NULL || settings == NULL || maze->width <= 0 ||
        maze->height <= 0 || settings->delay < 0 || maze->cells == NULL ||
        (settings->mode != RENDER_TEXT && settings->mode != RENDER_DRAWAPP)) return 0;
    *renderer = (Renderer){.maze = maze, .settings = *settings};
#ifndef MAZE_DRAWAPP
    if (settings->mode == RENDER_DRAWAPP)
    {
        fputs("Graphics support is unavailable in this build. Build the DrawApp target.\n", stderr);
        return 0;
    }
#endif
    size_t count = (size_t)maze->width * (size_t)maze->height;
    renderer->visits = calloc(count, sizeof(*renderer->visits));
    if (renderer->visits == NULL)
    {
        fputs("Could not allocate exploration display state.\n", stderr);
        return 0;
    }
    for (int y = 0; y < maze->height; y++)
        for (int x = 0; x < maze->width; x++)
            renderer->passageCount += mazeTileAt(maze, x, y) != TILE_WALL;
#ifdef MAZE_DRAWAPP
    if (settings->mode == RENDER_DRAWAPP)
    {
        int longest = maze->width > maze->height ? maze->width : maze->height;
        renderer->cellSize = 420 / longest;
        if (renderer->cellSize > 44) renderer->cellSize = 44;
        if (renderer->cellSize < 3)
        {
            fputs("Maze is too large for the drawing dashboard.\n", stderr);
            renderDestroy(renderer);
            return 0;
        }
        int gridWidth = maze->width * renderer->cellSize;
        int gridHeight = maze->height * renderer->cellSize;
        int plotWidth = gridWidth > 420 ? gridWidth : 420;
        int plotHeight = gridHeight > 380 ? gridHeight : 380;
        renderer->cardWidth = plotWidth + 48;
        renderer->cardHeight = plotHeight + 120;
        renderer->gridX = LEFT + 24 + (plotWidth - gridWidth) / 2;
        renderer->gridY = TOP + 44 + (plotHeight - gridHeight) / 2;
        renderer->panelX = LEFT + renderer->cardWidth + CARD_GAP;
        renderer->windowWidth = renderer->panelX + PANEL_WIDTH + LEFT;
        int contentBottom = TOP + renderer->cardHeight;
        if (contentBottom < 598) contentBottom = 598;
        renderer->windowHeight = contentBottom + 24;
        drawBackground(renderer);
    }
#endif
    return 1;
}

void renderDestroy(Renderer *renderer)
{
    if (renderer != NULL)
    {
        free(renderer->visits);
        renderer->visits = NULL;
    }
}

void renderRobot(const RobotView *view, void *context)
{
    Renderer *renderer = context;
    int moved = renderer->hasPrevious &&
        (renderer->previous.x != view->x || renderer->previous.y != view->y);
    if (moved) renderer->moves++;
    else if (renderer->hasPrevious && renderer->previous.direction != view->direction)
        renderer->turns++;
    if (!renderer->hasPrevious || moved) rememberVisit(renderer, view);
#ifdef MAZE_DRAWAPP
    if (renderer->settings.mode == RENDER_DRAWAPP) drawAction(renderer, view, moved);
#endif
    renderer->previous = *view;
    renderer->hasPrevious = 1;
}

void renderPreview(const Maze *maze, const RobotView *view)
{
    static const char symbols[] = {'^', '>', 'v', '<'};
    for (int y = 0; y < maze->height; y++)
    {
        for (int x = 0; x < maze->width; x++)
        {
            if (view != NULL && view->x == x && view->y == y) putchar(symbols[view->direction]);
            else if (x == maze->entranceX && y == maze->entranceY) putchar('S');
            else
            {
                Tile tile = mazeTileAt(maze, x, y);
                putchar(tile == TILE_WALL ? '#' : tile == TILE_EXIT ? 'E' : ' ');
            }
        }
        putchar('\n');
    }
}

void renderFinish(Renderer *renderer, int solved)
{
    renderer->finished = 1;
    renderer->solved = solved;
#ifdef MAZE_DRAWAPP
    if (renderer->settings.mode == RENDER_DRAWAPP)
    {
        foreground();
        drawDashboard(renderer);
        if (renderer->hasPrevious) drawFeedback(renderer, &renderer->previous);
        char summary[256];
        double newCellRate = 100.0 * renderer->visitedCount / (renderer->moves + 1);
        snprintf(summary, sizeof(summary), "%s\nMoves: %zu | Turns: %zu | Revisits: %zu | New-cell rate: %.1f%%\nCPU (search + drawing generation): %.2f ms. Playback time is shown separately.",
                 solved ? "Exit found." : "Search stopped; a resource limit does not prove no route exists.",
                 renderer->moves, renderer->turns, renderer->moves + 1 - renderer->visitedCount,
                 newCellRate, renderer->cpuSeconds * 1000.0);
        message(summary);

    }
#endif
}
