/*keeps the robot state and implements its movement and sensors*/

#include "maze_robot_internal.h"
#include <stdlib.h>

struct Robot
{
    /*the grid belongs to main and must outlive this robot*/
    const Maze *maze;
    RobotView view;
    RobotObserver observer;
    void *context;
    size_t moves;
    size_t turns;
};

static const int stepX[] = {0, 1, 0, -1};
static const int stepY[] = {-1, 0, 1, 0};
/*the callback sends the starting pose and later actions to the renderer*/
static void notify(Robot *robot)
{
    if (robot->observer != NULL)
        robot->observer(&robot->view, robot->context);
}

Robot *robotCreate(const Maze *maze, RobotObserver observer, void *context)
{
    if (maze == NULL || maze->cells == NULL || maze->startDirection < NORTH ||
        maze->startDirection > WEST ||
        mazeTileAt(maze, maze->entranceX, maze->entranceY) == TILE_WALL)
        return NULL;
    Robot *robot = calloc(1, sizeof(*robot));
    if (robot != NULL)
    {
        robot->maze = maze;
        robot->view = (RobotView){maze->entranceX, maze->entranceY, maze->startDirection};
        robot->observer = observer;
        robot->context = context;
        notify(robot);
    }
    return robot;
}

void robotDestroy(Robot *robot) { free(robot); }

RobotView robotView(const Robot *robot) { return robot->view; }

size_t robotMoves(const Robot *robot) { return robot->moves; }

size_t robotTurns(const Robot *robot) { return robot->turns; }

/*the grid lookup also treats any position outside the maze as a wall*/
int canMoveForward(const Robot *robot)
{
    int direction = robot->view.direction;
    return mazeTileAt(robot->maze, robot->view.x + stepX[direction],
                      robot->view.y + stepY[direction]) != TILE_WALL;
}

/*blocked moves do not notify the renderer or increase the move count*/
void forward(Robot *robot)
{
    if (canMoveForward(robot))
    {
        robot->view.x += stepX[robot->view.direction];
        robot->view.y += stepY[robot->view.direction];
        robot->moves++;
        notify(robot);
    }
}

/*adding three wraps left around the compass without negative headings*/
void left(Robot *robot)
{
    robot->view.direction = (robot->view.direction + 3) % 4;
    robot->turns++;
    notify(robot);
}

void right(Robot *robot)
{
    robot->view.direction = (robot->view.direction + 1) % 4;
    robot->turns++;
    notify(robot);
}

int atMarker(const Robot *robot)
{
    return mazeTileAt(robot->maze, robot->view.x, robot->view.y) == TILE_EXIT;
}
