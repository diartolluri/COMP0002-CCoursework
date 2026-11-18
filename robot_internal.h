#ifndef ROBOT_INTERNAL_H
#define ROBOT_INTERNAL_H

#include <stddef.h>
#include "maze.h"
#include "robot.h"

typedef struct { int x; int y; Direction direction; } RobotView;
typedef void (*RobotObserver)(const RobotView *view, void *context);

/* Setup/display access belongs to the world, never to the search module. */
Robot *robotCreate(const Maze *maze, RobotObserver observer, void *context);
void robotDestroy(Robot *robot);
RobotView robotView(const Robot *robot);
size_t robotMoves(const Robot *robot);
size_t robotTurns(const Robot *robot);

#endif
