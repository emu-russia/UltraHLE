#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// GRAPHICS - display lists
void dlist_execute(OSTask_t* task); // execute a display list
void dlist_addtestdot(int y); // add a dot to the framerate graph in debug mode
void dlist_cammove(float x, float y, float z);
void dlist_ignoregraphics(int ignore);

#ifdef __cplusplus
};
#endif
