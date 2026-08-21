/**
 * \file rdp.h
 * RDP (Reality Display Processor) emulation: vertices, primitives and display-list commands.
 */

// rdp emulation.

#ifdef __cplusplus
extern "C" {
#endif

#define MAXRDPVX 64

/**
 * One RDP vertex: clip-space position, original color/normal, texture coordinates and combined color.
 */
typedef struct
{
    // (24) set by dlist
    float   pos[3]; // clip coordinates
    uint32_t   icol;   // original color/normal
    /** Texture coordinates in tile space. */
    float   tex[2];
    // (16) set by dlist at first usage
    float   col[4]; // shade color, range 0..255, [r,g,b,a]
    // (24) set by rdp at drawprims
    float   ct[2];  // combined texture coords
    float   cc[4];  // combined color
} Vertex; // 64 bytes

/** Pointers to the active rdp vertices. */
extern Vertex *rdpvx[MAXRDPVX];
/** Per-vertex flags (clip and init state). */
extern char    rdpvxflag[MAXRDPVX];

#define VX_CLIPX1    0x01
#define VX_CLIPX2    0x02
#define VX_CLIPY1    0x04
#define VX_CLIPY2    0x08
#define VX_CLIPZ     0x10
#define VX_CLIPALL   0x1f
#define VX_INITDONE  0x40

// all primitives are now triangles
/**
 * One triangle primitive referencing three vertices, 16 bytes.
 * @note A viewport change is stored as a primitive with c[0]=c[1]=NULL and the rectangle in c[2]->pos/tex.
 */
typedef struct
{
    /** Pointers to the three vertices of the triangle. */
    Vertex *c[3];
    int     wirecolor; // 0=don't draw, 1-7=color (set in flush)
} Primitive; // 16 bytes
// viewport changes saved as:
// c[0]=NULL
// c[1]=NULL
// c[2]=vertex where pos[0,1]=x0/y0 and tex[0,1]=x1/y1

/**
 * Rectangle parameters used by both the texrect and fillrect display-list commands.
 */
typedef struct
{
    // this struct used for both texrect and fillrect
    // s0,t0,tile,flip only used on texrect
    float   x0,y0,s0,t0; // NOTE: 1.0=pixel, not 4.0 as in hw
    /** End x/y and end texture s/t coordinates (texrect only). */
    float   x1,y1,s1,t1;
    /** Texture flip flag and tile number (texrect only). */
    int     flip,tile;
} TexRect;

/**
 * Sets the base address of an RDP memory segment.
 * @param seg Segment number (0-15).
 * @param base Physical base address of the segment.
 */
// segments not really handled in rdp, but this rdp supports them too
void rdp_segment(int seg,uint32_t base);

/**
 * Executes one RDP display-list command.
 * @param cmd Pointer to the command words.
 * @return 0 if the command was processed, -1 if unknown, >=0 if that many more opcodes are needed.
 */
// main execute command. Returns 0 if command processed, -1 if unknown,
// >=0 if that many more following opcodes needed
// (regardless of their command code, used for texrect)
int  rdp_cmd(uint32_t *cmd);

/**
 * Draws a filled rectangle.
 * @param tr Rectangle parameters.
 */
// drawing (draw commands C0..CF,E4,E5 *not* interpreted with rdp_cmd)
void rdp_fillrect(TexRect *tr);
/**
 * Draws a textured rectangle.
 * @param tr Rectangle and texture parameters.
 */
void rdp_texrect(TexRect *tr);
/**
 * Assigns num new vertices to rdp vertices first..first+num-1.
 * @param first First rdp vertex index.
 * @param num Number of vertices to assign.
 */
void rdp_newvtx(int first,int num);
/**
 * Adds a triangle primitive referencing three vertices.
 * @param vind Array of three vertex indices.
 */
void rdp_tri(int *vind);
/**
 * Sets the fog range and enables or disables fog.
 * @param min Minimum fog distance.
 * @param max Maximum fog distance.
 */
void rdp_fogrange(float min,float max); // set fog range (fogcolor used)
/**
 * Sets the viewport rectangle.
 * @param xm Half viewport width.
 * @param ym Half viewport height.
 * @param xa Viewport center x.
 * @param ya Viewport center y.
 */
void rdp_viewport(float xm,float ym,float xa,float ya);
/**
 * Sets flat shading mode.
 * @param flat Non-zero to enable flat shading.
 */
void rdp_flat(int flat);

/**
 * Starts a new rendering frame.
 */
// frame level control
void rdp_framestart(void);
/**
 * Ends the current frame and shows the rendered buffer.
 */
void rdp_frameend(void);
/**
 * Opens the graphics display if not already open.
 */
void rdp_opendisplay(void);
/**
 * Closes the graphics display if open.
 */
void rdp_closedisplay(void);
/**
 * Takes a screenshot of the current framebuffer to a 24-bit TGA file.
 * @param file Output filename, or NULL to auto-generate one.
 */
void rdp_screenshot(char *file); // TGA 24bit
/**
 * Adds a debug dot to the framerate graph.
 * @param y Screen y position of the dot.
 */
void rdp_addtestdot(int y);
/**
 * Grabs the current screen contents for framebuffer texturing (currently disabled).
 */
void rdp_grabscreen(void);
/**
 * Requests a buffer swap at the next frame end.
 */
void rdp_swap(void);
/**
 * Copies a background image from RDRAM to the screen as two 256-pixel textures.
 * @param base RDRAM address of the background image.
 * @param wid Width of the background in pixels.
 * @param hig Height of the background in pixels.
 */
void rdp_copybackground(uint32_t base,int wid,int hig);

/** Enables wireframe debug drawing. */
extern int showwire;
/** Enables the debug info overlay. */
extern int showinfo;
/** Debug test mode selector. */
extern int showtest;
/** Secondary debug test mode selector. */
extern int showtest2;

/**
 * Toggles between fullscreen and windowed display.
 */
void rdp_togglefullscreen(void);

/**
 * Selects the texture tile used for the next primitives.
 * @param on Texture enable flag.
 * @param tile Tile number to select.
 * @param level Texture level.
 */
void rdp_texture(int on,int tile,int level);

/**
 * Returns whether the fullscreen graphics display is active.
 * @return Non-zero if the graphics display is open and fullscreen.
 */
int  rdp_gfxactive(void);

#ifdef __cplusplus
};
#endif
