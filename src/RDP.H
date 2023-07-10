
#define MAXRDPVX 64

typedef struct
{
    // (24) set by dlist
    float   pos[3]; // clip coordinates
    dword   icol;   // original color/normal
    float   tex[2];
    // (16) set by dlist at first usage
    float   col[4]; // shade color, range 0..255, [r,g,b,a]
    // (24) set by rdp at drawprims
    float   ct[2];  // combined texture coords
    float   cc[4];  // combined color
} Vertex; // 64 bytes

extern Vertex *rdpvx[MAXRDPVX];
extern char    rdpvxflag[MAXRDPVX];

#define VX_CLIPX1    0x01
#define VX_CLIPX2    0x02
#define VX_CLIPY1    0x04
#define VX_CLIPY2    0x08
#define VX_CLIPZ     0x10
#define VX_CLIPALL   0x1f
#define VX_INITDONE  0x40

// all primitives are now triangles
typedef struct
{
    Vertex *c[3];
    int     wirecolor; // 0=don't draw, 1-7=color (set in flush)
} Primitive; // 16 bytes
// viewport changes saved as:
// c[0]=NULL
// c[1]=NULL
// c[2]=vertex where pos[0,1]=x0/y0 and tex[0,1]=x1/y1

typedef struct
{
    // this struct used for both texrect and fillrect
    // s0,t0,tile,flip only used on texrect
    float   x0,y0,s0,t0; // NOTE: 1.0=pixel, not 4.0 as in hw
    float   x1,y1,s1,t1;
    int     flip,tile;
} TexRect;

// segments not really handled in rdp, but this rdp supports them too
void rdp_segment(int seg,dword base);

// main execute command. Returns 0 if command processed, -1 if unknown,
// >=0 if that many more following opcodes needed
// (regardless of their command code, used for texrect)
int  rdp_cmd(dword *cmd);

// drawing (draw commands C0..CF,E4,E5 *not* interpreted with rdp_cmd)
void rdp_fillrect(TexRect *tr);
void rdp_texrect(TexRect *tr);
void rdp_newvtx(int first,int num);
void rdp_tri(int *vind);
void rdp_fogrange(float min,float max); // set fog range (fogcolor used)
void rdp_viewport(float xm,float ym,float xa,float ya);
void rdp_flat(int flat);

// frame level control
void rdp_framestart(void);
void rdp_frameend(void);
void rdp_opendisplay(void);
void rdp_closedisplay(void);
void rdp_screenshot(char *file); // TGA 24bit
void rdp_addtestdot(int y);
void rdp_grabscreen(void);
void rdp_swap(void);
void rdp_copybackground(dword base,int wid,int hig);

extern int showwire;
extern int showinfo;
extern int showtest;
extern int showtest2;

void rdp_togglefullscreen(void);

void rdp_texture(int on,int tile,int level);

int  rdp_gfxactive(void);

