/*declares robot setup, display snapshots and observer callbacks*/

#ifndef MAZE_ROBOT_INTERNAL_H
#define MAZE_ROBOT_INTERNAL_H

#include <stddef.h>
#include "maze_grid.h"
#include "maze_robot.h"

typedef struct { int x; int y; Direction direction; } RobotView;
/*observers copy the pose if they need to compare it with a later action*/
/*context is borrowed callback data, robotDestroy does not release it*/
typedef void (*RobotObserver)(const RobotView *view, void *context);

/*setup and display can read the pose, the search module cannot*/
Robot *robotCreate(const Maze *maze, RobotObserver observer, void *context);
void robotDestroy(Robot *robot);
RobotView robotView(const Robot *robot);
size_t robotMoves(const Robot *robot);
size_t robotTurns(const Robot *robot);

#endif
