
#pragma once

// Initialization / display management (fx.c, OpenGL backend)
/**
 * Returns the name of the OpenGL backend.
 * @return Backend name string.
 */
char* init_name();
/**
 * Handles the fullscreen request for the OpenGL backend.
 * @param fullscreen Non-zero to request fullscreen.
 * @return 0 (always; the backend renders into the window the emulator provides).
 */
int init_fullscreen(int fullscreen);
/**
 * Queries whether the graphics device is available.
 * @return 0 (always).
 */
int init_query();
/**
 * Reinitializes the graphics device by shutting it down and starting it again.
 */
void init_reinit();
/**
 * Initializes the graphics device and creates the OpenGL context.
 * @return 0 on success, -1 on failure.
 */
int init_init();
/**
 * Shuts down the graphics device and releases the OpenGL context.
 */
void init_deinit();
/**
 * Handles activation of the display.
 */
void init_activate();
/**
 * Marks the display as resized so the viewport is reapplied on the next frame.
 * @param xs New client width.
 * @param ys New client height.
 */
void init_resize(int xs, int ys);
/**
 * Swaps the front and back buffers of the display.
 */
void init_bufferswap();
/**
 * Clears the color and depth buffers and applies the scissor region for the game viewport.
 * @param writecolor Non-zero to clear the color buffer.
 * @param writedepth Non-zero to clear the depth buffer.
 * @param cr Red component of the clear color.
 * @param cg Green component of the clear color.
 * @param cb Blue component of the clear color.
 */
void init_clear(int writecolor, int writedepth, float cr, float cg, float cb);
/**
 * Reads a rectangle of pixels from the framebuffer into a buffer.
 * @param fb Buffer format (X_FB_RGB565 or X_FB_RGBA8888), optionally ORed with X_FB_FRONT or X_FB_BACK.
 * @param x Left edge in game coordinates.
 * @param y Top edge in game coordinates.
 * @param xs Width of the rectangle in game pixels.
 * @param ys Height of the rectangle in game pixels.
 * @param buffer Destination buffer.
 * @param bufrowlen Row length of the destination buffer in bytes.
 * @return 0 on success, 1 on failure.
 */
int init_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);
/**
 * Writes a rectangle of pixels from a buffer into the framebuffer.
 * @param fb Buffer format (X_FB_RGB565 or X_FB_RGBA8888), optionally ORed with X_FB_FRONT or X_FB_BACK.
 * @param x Left edge in game coordinates.
 * @param y Top edge in game coordinates.
 * @param xs Width of the rectangle in game pixels.
 * @param ys Height of the rectangle in game pixels.
 * @param buffer Source buffer.
 * @param bufrowlen Row length of the source buffer in bytes.
 * @return 1 (not implemented).
 */
int init_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);

// Letterboxed viewport geometry (the game image xs x ys inside the window)
/**
 * Computes the letterboxed viewport of the game image inside the window.
 * @param lx Receives the left edge of the viewport in window pixels.
 * @param ly Receives the top edge of the viewport in window pixels.
 * @param vw Receives the viewport width in window pixels.
 * @param vh Receives the viewport height in window pixels.
 */
void gl_viewport_geom(int* lx, int* ly, int* vw, int* vh);

// Pixel pipeline modes (fx.c, OpenGL backend)
/**
 * Resets the cached pixel pipeline mode state.
 */
void mode_init();
/**
 * Sets the filtering and wrap modes of a texture unit.
 * @param tmu Texture unit index (0 or 1).
 * @param format X format flags controlling filtering and clamping.
 * @param trilin Non-zero to enable trilinear filtering.
 */
void mode_texturemode(int tmu, int format, int trilin);
/**
 * Loads a texture into texture unit 0.
 * @param txtind Index of the texture to load.
 */
void mode_loadtexture(int txtind);
/**
 * Loads two textures into texture units 0 and 1.
 * @param txtind1 Index of the first texture.
 * @param txtind2 Index of the second texture.
 */
void mode_loadmultitexture(int txtind1, int txtind2);
/**
 * Applies the pending pixel pipeline mode changes to the OpenGL state.
 */
void mode_change();
