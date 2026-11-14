#ifndef ROBOT_H
#define ROBOT_H

/* AI-assisted reference interfaces: OpenAI Codex, November 2026. */

typedef struct Robot Robot;
void forward(Robot *robot);
void left(Robot *robot);
void right(Robot *robot);
int atMarker(const Robot *robot);
int canMoveForward(const Robot *robot);

#endif
