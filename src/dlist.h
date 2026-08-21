#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Executes a display list from the given task.
 * @param task Task describing the display list to execute.
 */
// GRAPHICS - display lists
void dlist_execute(OSTask_t* task); // execute a display list
/**
 * Adds a dot to the framerate graph in debug mode.
 * @param y Vertical position of the dot.
 */
void dlist_addtestdot(int y); // add a dot to the framerate graph in debug mode
/**
 * Sets the camera movement offset for the current frame.
 * @param x X offset.
 * @param y Y offset.
 * @param z Z offset.
 */
void dlist_cammove(float x, float y, float z);
/**
 * Sets whether display list processing is skipped.
 * @param ignore Nonzero to ignore graphics.
 */
void dlist_ignoregraphics(int ignore);

#ifdef __cplusplus
};
#endif
