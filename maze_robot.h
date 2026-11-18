/*exposes the five robot controls to the search code*/

#ifndef MAZE_ROBOT_H
#define MAZE_ROBOT_H

/*the solver gets these controls, not the stored pose or maze*/
typedef struct Robot Robot;
/*among these controls, only forward changes position, turns only change heading*/
void forward(Robot *robot);
void left(Robot *robot);
void right(Robot *robot);
/*the two sensor calls leave the pose and action counters unchanged*/
int atMarker(const Robot *robot);
int canMoveForward(const Robot *robot);

#endif
