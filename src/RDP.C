#include "ultra.h"
#include "x.h"

// whole frame collected, drawn in bursts
#define MAXVX       16384
#define MAXPR       8192
#define MAXTXT      1000  // max textures in a frame
#define MAXLOAD     2048  // max textures loaded
#define MAXCOMB     256   // max different combinemodes

#define GEOMFLAGS   0 //X_DUMPDATA

//#define VERTEXARRAY

// globals
int     showwire;
int     showinfo;
int     showtest;
int     showtest2;

const float colscale=(1.0/256.0);

static char *fmt[]={"RGBA","YUV","CI","IA","I","?5","?6","?7"};
static char *bpp[]={"4b","8b","16b","32b"};
static char *cm []={"WRAP","MIRROR","CLAMP","CLAMP3"};

typedef struct
{
    int     tmembase;
    int     tmemrl;
    int     fmt;
    int     bpp;
    int     xs,ys;
    int     cmt,maskt,shiftt;
    int     cms,masks,shifts;
    int     x0,y0;
    int     x1,y1;
    int     x0full,y0full;
    int     palette;
    // from preceding settimg
    dword   membase;
    int     memrl;
    dword   memx0;
    dword   memy0;
    dword   memfmt;
    dword   membpp;
    dword   crc;
    // mapped to where
    int     texture;
    int     fromfb;
    //
    int     settilemark;
} Tile;

typedef struct
{
    // info on what loaded
    int     fromfb;
    dword   membase;
    dword   memx0;
    dword   memy0;
    dword   mempal;
    dword   memfmt;
    dword   membpp;
    dword   memcrc;
    int     memxs,memys;
    // when loaded/used
    int     creation_vidframe;
    int     used_vidframe;
    int     txtslot; // rst.loadtxt slot
    // handle
    int     xhandle;
    int     xs,ys; // scaled size
    int     cms,cmt;
} Texture;

// bufs
#define RDP_BUF_TXT 0
#define RDP_BUF_Z   1
#define RDP_BUF_C   2

// bpps
#define RDP_BPP_4   0
#define RDP_BPP_8   1
#define RDP_BPP_16  2
#define RDP_BPP_32  3

// formats
#define RDP_FMT_RGBA 0
#define RDP_FMT_YUV  1
#define RDP_FMT_CI   2
#define RDP_FMT_IA   3
#define RDP_FMT_I    4

// blending
#define RDP_BLEND_DUNNO      0
#define RDP_BLEND_MULALPHA   1 // 0055
#define RDP_BLEND_AA_OPAQUE  2 // 0044
#define RDP_BLEND_NORMAL     3 // 0F0A or 0A0F or 0000
#define RDP_BLEND_ALPHAMIX   4 // 0050
#define RDP_BLEND_ALPHAMIX2  5 // 0040
#define RDP_BLEND_DARKEN     6 // 4C40

// combine indices
#define RDP_X     0
#define RDP_Y     1
#define RDP_M     2
#define RDP_A     3
#define RDP_TYPE  4

static char *cname[32]={
"0","1","SHADE","PRIM","ENV",
"SHAA","PRIA","ENVA","PLODF",
"BLEND","FOG","FILL",
"?","?","?","?",
"TEX0","TEX1","TEX0A","TEX1A",
"COMB","COMBA","LODF","NOISE","K4","K5","CENTER","SCALE","DUNNO",
"?","?","?"};

// colors
#define C_ZERO   0
#define C_ONE    1
#define C_SHADE  2
#define C_PRIM   3
#define C_ENV    4
// color alphas
#define C_SHAA   5
#define C_PRIA   6
#define C_ENVA   7
#define C_PLODF  8
// nonblendable colors
#define C_BLEND  9
#define C_FOG    10
#define C_FILL   11
// texture sources (not usable as colors really)
#define C_TEX0   16 // gray in rst.col
#define C_TEX1   17
#define C_TEX0A  18
#define C_TEX1A  19
// misc sources (not usable as colors really)
#define C_COMB   20 // combined cycle0
#define C_COMBA  21 // combined cycle0
#define C_LODF   22 // combined cycle0
#define C_NOISE  23 // combined cycle0
#define C_K4     24 // combined cycle0
#define C_K5     25 // combined cycle0
#define C_CENTER 26 // combined cycle0
#define C_SCALE  27 // combined cycle0
#define C_DUNNO  28 // combined cycle0
// overlayed on unused elements
#define C_ALP50  24 // 255,255,255,128
#define C_ALL50  25 // 128,128,128,128
#define C_ALL25  26 //  64, 64, 64, 64
#define C_COL50  27 // 128,128,128,255
#define C_ALP75  28 // 255,255,255,192
// total num
#define C_FULL   31 // special handlin
#define C_NUM    32
#define C_INDEX  0x7f
#define C_NEGATE 0x80

#define C_SPECIALCASE (65536*2)
#define C_MULCASE     (65536*1) // x*y
#define C_SUBCASE     (65536*4) // x-y

#define ISCOLOR(x)    (!((x)&16))
#define C_MUL(x,y)    ((x)+((y)<<8)+C_MULCASE) // NOTE: C_NEGATE only allowed for x
#define C_SUB(x,y)    ((x)+((y)<<8)+C_SUBCASE) // NOTE: C_NEGATE only allowed for x

char *stylename[8]={"Ignore","Const","Add","Mul","MulAdd","Blend","Full","Comb"};

// combine styles
#define STYLE_IGNORE   0
#define STYLE_CONST    1  // A
#define STYLE_ADD      2  // X+A
#define STYLE_MUL      3  // X*M
#define STYLE_MULADD   4  // X*M+A
#define STYLE_BLEND    5  // Y->X,M
#define STYLE_FULL     6  // (X-Y)*M+A
#define STYLE_COMB     7  // CONST, but also COMB

// Combine mode drawing method
typedef struct
{
    // original mode for this description
    dword   combine0,combine1;
    dword   other0,other1;
    int     dualtxt;     // second X_MODE, txt[1] used.
    // parsed combine state
    int     passes;      // 1 or 2
    int     texenable;   // 0=notxt, +1=text0used, +2=text1used
    int     env;         // envcolor C_*
    int     forcezcmp[4]; // >0=force, cmp and update forced then too
    int     forcezupd[4];
    int     forceatst[4]; // -1=force off, +1=force 0.05, +2=0.25, +3=0.5
    // parsed combine gouraurd color mixing [cycle][color,alpha]
    int     blend1[4];    // x_blend(1,_)
    int     blend2[4];    // x_blend(_,2)
    int     txt   [4];    // which tile to use for texture on passes
    int     col   [4][2]; // source + mul*256 + negate *65536
    int     com   [4][2]; // x_combine()
} Combine;

typedef struct
{
    // large internal tables (at start to preserve alignment better)
    Vertex    vxtab[MAXVX];
    int       vxtabinited[MAXVX];
    Primitive prtab[MAXPR];
    xt_pos    vxpos[MAXVX];
    dword     tmemsrc[4096/8*2]; // where textures loaded to tmem
    dword     tmemrl [4096/8*2]; // where textures loaded to tmem
    dword     tmemx0 [4096/8*2]; // where textures loaded to tmem
    dword     tmemy0 [4096/8*2]; // where textures loaded to tmem
    int       txtload[MAXLOAD];
    Texture   txt[MAXTXT]; // 0 not used
    Tile      tile[8];

    int       vxtabinitedcnt;

    int       lastloadb;

    int       dualtmu;

    int       nexttexturetile;
    int       texturetile;

    int       rectmode;

    dword     rawfillcolor;

    // initdone?
    int       opened;
    int       fullscreen;

    // current frame
    int       myframe;
    int       txtrefreshcnt;
    int       starttimeus;
    int       fillrectcnt;

    int       swapflag;
    int       swapcnt;

    int       testcnt;

    int       tris;

    int       firstfillrect;

    int       geyemode;

    // big data
    ushort    palette[256];
    int       txtloads;

    // tables for whole frame
    int       vxtabi;
    int       prtabi;
    // fog settings
    float     fogmin,fogmax;
    int       fogenable;
    int       fogcolor;
    int       foglasttype;

    // framebuffer texturing detection
    dword     lastcbufs[8];
    int       lastcbufi;
    char     *framegrab;

    // viewport
    int       view_x0;
    int       view_x1;
    int       view_y0;
    int       view_y1;

    // active colors
    byte      col[C_NUM][4];
    float     colf[C_NUM][4];
    // memory segmenets
    dword     segment[16];
    // buffers [txt,z,c]
    dword     bufbase[3];
    dword     buffmt[3];
    dword     bufbpp[3];
    dword     bufwid[3];
    // last tlut load
    dword     tlut_base;
    int       tlut_palbase;
    int       tlut_num;
    dword     tlut_lastbase;
    int       tlut_lastpalbase;
    int       tlut_lastnum;

    // combined (and actually used) s,t transform
    float     txt_uadd;
    float     txt_vadd;
    float     txt_uscale;
    float     txt_vscale;
    // for second texture in dual mode
    float     txt_uadd2;
    float     txt_vadd2;
    float     txt_uscale2;
    float     txt_vscale2;

    // mode (active texture considered part of mode)
    // [active,last,backup used by fillrect]
    int       modechange;             // 1=set changed, 2=set all
    int       firstmodechange;
    int       txtchange;
    int       lastusedtile;
    int       last_prtabi;            // the last mode spans from this to current tritabi
    dword     texture1[3];
    dword     texture2[3];
    dword     other0[3];
    dword     other1[3];
    dword     combine0[3];
    dword     combine1[3];

    int       flat; // from dlist.c
    int       setflat; // from dlist.c

    // temporary stuff for determining combine modes
    // [c0_color,c0_alpha,c1_color,c1_alpha] [x,m,a,y] -> (x-y)*m+a
    int       s_combx[4][4];
    int       s_combxbak[4][4]; // for dumping only
    int       s_combinecycles;
    // basic results
    int       s_combinestyle[4];
    int       s_combinetex[4];    // texture present (+1,+2)
    int       s_combinetexboth;   // texture present (+1,+2)
    int       s_combinetexbothbak;   // texture present (+1,+2)

    // parsed state
    int       s_cycles;
    int       s_txtfilt; // 0=pointsample
    int       s_zcmp;
    int       s_zupd;
    int       s_noz;
    int       s_zmode; // decal/overlay modes
    int       s_cvgmode; // decal/overlay modes
    int       s_tluttype;
    int       s_zsrc;
    int       s_alphatst;
    int       s_forcebl;
    int       s_blendbits;
    int       s_blend1,s_blend2;
    Combine  *s_c;
    int       lastalphatst;
    int       lastzmode;
    // combine cache (also takes other into account)
    Combine   combcache[MAXCOMB];
    Combine   combdummy;
    int       combcacheused;
    // temp parameters for texrect
    TexRect   texrect;
    // multiword commands (texrect)
    int       wordcmd;
    int       wordsleft;

    // debugging
    int       debugwirecolor; // 0=don't draw wire
    // debugging counts
    int       cnt_setting;
    int       cnt_texture;
    int       cnt_texturegen;

    int       frameopen;
} RendState;

static Vertex rdpdummyvx[MAXRDPVX];

RendState rst;

// publics (used by dlist.c)
Vertex   *rdpvx[MAXRDPVX]; // ptrs to active vertices
char      rdpvxflag[MAXRDPVX];

#define COM (*rst.s_c)

void newmode(void);
void newtextures(void);
void flushprims(void);

/****************************************************************************
/* DEBUG screenshot
*/

unsigned char tgaheader[]={
0,0,2,0,0,0,0,24,0,0,0,0,
0,0,
0,0,
24,0};

void rdp_screenshot(char *name0)
{
    FILE *f1;
    char  name[256];
    char *buf;
    int   x,y;

    if(!rst.opened)
    {
        exception("graphics not active, cannot take screenshot");
        return;
    }

    if(name0)
    {
        strcpy(name,name0);
        if(!strchr(name,'.')) strcat(name,".tga");
    }
    else
    {
        for(x=1;x<1000;x++)
        {
            sprintf(name,"%02i.tga",x);
            f1=fopen(name,"rb");
            if(!f1) break;
            fclose(f1);
            // next
        }
    }
    print("note: took screenshot %s\n",name);

    tgaheader[12]=init.gfxwid&255;
    tgaheader[13]=init.gfxwid/256;
    tgaheader[14]=init.gfxhig&255;
    tgaheader[15]=init.gfxhig/256;

    x=tgaheader[12]+tgaheader[13]*256;
    y=tgaheader[14]+tgaheader[15]*256;
    print("Took screenshot: %s (%ix%ix24bit)\n",name,x,y);

    buf=malloc(init.gfxwid*init.gfxhig*4);
    x_readfb(X_FB_FRONT|X_FB_RGBA8888,0,0,init.gfxwid,init.gfxhig,buf,init.gfxwid*4);

    f1=fopen(name,"wb");
    if(!f1)
    {
        print("Error opening file.\n");
        return;
    }
    fwrite(tgaheader,1,sizeof(tgaheader),f1);
    for(y=init.gfxhig-1;y>=0;y--)
    {
        for(x=0;x<init.gfxwid;x++)
        {
            putc(buf[x*4+y*init.gfxwid*4+2],f1);
            putc(buf[x*4+y*init.gfxwid*4+1],f1);
            putc(buf[x*4+y*init.gfxwid*4+0],f1);
        }
    }
    fclose(f1);

    free(buf);
}

/****************************************************************************
/* Helper routines for setting state
*/

static __inline dword address(dword address)
{ // segment convert to physical address
    int seg=(address>>24)&0x3f;
    if(seg>15)
    {
        error("rdp: segment > 15\n");
        seg=0;
    }
    return( (address&0xffffff) + rst.segment[seg] );
}

static void setbuffer(int i,dword c0,dword c1)
{
    dword addr=address(c1);

    rst.bufbase[i]=addr;
    rst.buffmt [i]=FIELD(c0,21,3);
    rst.bufbpp [i]=FIELD(c0,19,2);
    rst.bufwid [i]=FIELD(c0,0,12)+1;
    if(i==RDP_BUF_C)
    {
        mem_write32(addr+0 ,0xefefffef);
        mem_write32(addr+4 ,0xefefffef);
        /*
        if(cart.iszelda)
        {
            for(j=0;j<8;j++)
            {
                if(rst.lastcbufs[j]==addr) return;
            }
        }
        if(addr!=rst.lastcbufs[rst.lastcbufi])
        {
            rst.lastcbufi++;
            rst.lastcbufi&=7;
            rst.lastcbufs[rst.lastcbufi]=addr;
        }
        */
    }
}

Vertex *newvx(void)
{
    int i;
    if(rst.vxtabi>=MAXVX)
    {
        error("rdp: too many vertices in frame (max %i)",MAXVX);
        i=MAXVX-1;
    }
    i=rst.vxtabi++;
    rst.vxtab[i].col[0]=1.0f;
    rst.vxtab[i].col[1]=0.0f;
    rst.vxtab[i].col[2]=1.0f;
    rst.vxtab[i].col[3]=1.0f;
    return(rst.vxtab+i);
}

Primitive *newpr(void)
{
    int i;
    if(rst.prtabi>=MAXVX)
    {
        error("rdp: too many primitives in frame (max %i)",MAXPR);
        i=MAXVX-1;
    }
    i=rst.prtabi++;
    return(rst.prtab+i);
}

// returns 1 if address is contained in a lately used color buffer
static int isoldcbuf(dword addr)
{
    int cnt=0;
    if(mem_read32(addr+0)==0xefefffef) cnt++;
    if(mem_read32(addr+4)==0xefefffef) cnt++;
    return(cnt==2);
    /*
    for(j=0;j<8;j++)
    {
        b=rst.lastcbufs[j];
        if(addr>=b && addr<=b+320*240*2)
        {
            return(1);
        }
    }
    return(0);
    */
}

void setintensityfromalpha(int di,int si)
{
    byte *d=rst.col[di];
    byte *s=rst.col[si];
    float *df=rst.colf[di];
    float *sf=rst.colf[si];
    d[0]=s[3];
    d[1]=s[3];
    d[2]=s[3];
    d[3]=s[3];
    df[0]=sf[3];
    df[1]=sf[3];
    df[2]=sf[3];
    df[3]=sf[3];
}

static __inline void setcolor(int ci,dword a)
{
    byte *col=rst.col[ci];
    float *colf=rst.colf[ci];
    col[3]=a; a>>=8;
    col[2]=a; a>>=8;
    col[1]=a; a>>=8;
    col[0]=a;
    colf[0]=colscale*col[0];
    colf[1]=colscale*col[1];
    colf[2]=colscale*col[2];
    colf[3]=colscale*col[3];
}

static void setfillcolor(int ci,dword c)
{
    byte col[4];
    int a;
    a=FIELD(c,11,5); col[3]=8*a+(a>>2);
    a=FIELD(c, 6,5); col[2]=8*a+(a>>2);
    a=FIELD(c, 1,5); col[1]=8*a+(a>>2);
    a=FIELD(c, 0,1); col[0]=255;
    setcolor(ci,*(dword *)col);
}

static void setcolorintensity(int ci,int a)
{
    byte col[4];
    col[0]=a;
    col[1]=a;
    col[2]=a;
    col[3]=a;
    setcolor(ci,*(dword *)col);
}

void rdp_freetexmem(void)
{
    int i;
    for(i=1;i<MAXTXT;i++)
    {
        if(rst.txt[i].xhandle)
        {
            x_freetexture(rst.txt[i].xhandle);
        }
        memset(&rst.txt[i],0,sizeof(Texture));
    }
    x_cleartexmem();
}

/****************************************************************************
/* DEBUG Info overlays and other debug stuff
*/

struct
{
    int x,y;
} testdot[1024];

static float dxm=+1.0/318;
static float dym=-1.0/238;
static float dxa=-1.0;
static float dya=+1.0;

static float oxm=+1.0/319*2;
static float oym=-1.0/238*2;
static float oxa=-1.0;
static float oya=+1.0;

void rdp_addtestdot(int y)
{
    static int i=0;
    testdot[i].x=639;
    testdot[i].y=y;
    i++;
    i&=1023;
}

void viewport(int rectmode)
{
    float x0,y0,x1,y1;

    if(rst.rectmode==rectmode) return;
    rst.rectmode=rectmode;

    if(rst.prtabi!=rst.last_prtabi)
    {
        // flush prims before viewport changes
        flushprims();
    }

    if(rectmode)
    {
        x0=0;
        y0=0;
        x1=init.gfxwid-1;
        y1=init.gfxhig-1;
    }
    else
    {
        x0=rst.view_x0;
        y0=rst.view_y0;
        x1=rst.view_x1;
        y1=rst.view_y1;
    }

    if(showinfo)
    {
        x0=x0*0.48+320*0.01;
        y0=y0*0.48+240*1.01;
        x1=x1*0.48+320*0.01;
        y1=y1*0.48+240*1.01;
    }
    x_viewport(x0,y0,x1,y1);

    // add to wirelist
    {
        Primitive *p;
        Vertex    *v;
        p=newpr();
        p->c[0]=NULL;
        p->c[1]=NULL;
        p->c[2]=v=newvx();
        v->pos[0]=x0;
        v->pos[1]=y0;
        v->tex[0]=x1;
        v->tex[1]=y1;
        // skip this in drawing
        rst.last_prtabi=rst.prtabi;
    }
}

static void realdrawmode(void)
{
    x_projection(90,0.9,32768.0);
    viewport(0);
    x_projmatrix(NULL);
    x_matrix(NULL);
    x_reset();
    x_geometry(GEOMFLAGS);
    x_flush();
}

static void debugdrawmode(void)
{
    x_projection(90,0.9,32768.0);
    x_viewport(0,0,init.gfxwid-1,init.gfxhig-1);
    x_projmatrix(NULL);
    x_matrix(NULL);
    x_reset();
    x_geometry(GEOMFLAGS);
    x_flush();
}

__inline static void orthovxpos(float x,float y)
{
    x_vxpos(x*dxm+dxa,y*dym+dya,1.0);
}

static void drawdot(int x,int y)
{
    x_vxcolor(0.9,0.5,0.1);
    x_begin(X_QUADS);
    orthovxpos(x+0,y+0);
    orthovxpos(x+0,y+2);
    orthovxpos(x+2,y+2);
    orthovxpos(x+2,y+0);
    x_end();
}

void debugdrawtexture(int tilepos,int txti,int size)
{
    int xhandle=rst.txt[txti].xhandle;
    int xn,s1,s2,x,y;
    float a,b;

    xn=640/size;
    xn&=~1;
    s1=1;
    s2=size-1;

    x=size*(tilepos%xn);
    y=size*(tilepos/xn);

    a=0.5/rst.txt[txti].xs;
    b=0.5/rst.txt[txti].ys;

    x_texture(rst.txt[txti].xhandle);

    x_begin(X_QUADS);
    x_vxtex(0.0+a,0.0+b); orthovxpos(x+s1,y+s1);
    x_vxtex(0.0+a,1.0-b); orthovxpos(x+s1,y+s2);
    x_vxtex(1.0-a,1.0-b); orthovxpos(x+s2,y+s2);
    x_vxtex(1.0-a,0.0+b); orthovxpos(x+s2,y+s1);
    x_end();
}

static void drawmarkers(void)
{ // framecount marker
    int i;

    for(i=0;i<1024;i++)
    {
        if(testdot[i].x>0)
        {
            drawdot(testdot[i].x,478-testdot[i].y);
            testdot[i].x-=2;
        }
    }
}

void drawtextures(void)
{
    int i,size;
    if(rst.txtloads<=0) return;

    for(size=64;size>8;size-=4)
    {
        if((rst.txtloads/(640/size)+2)*size<=240) break;
    }

    x_blend(X_ALPHA,X_INVOTHERALPHA);
    x_combine(X_TEXTURE);
    for(i=0;i<rst.txtloads;i++)
    {
        debugdrawtexture(i,rst.txtload[i],size);
    }
}

char *colortext(byte *c)
{
    static char buf[16];
    sprintf(buf,"%02X%02X%02X%02X",c[0],c[1],c[2],c[3]);
    return(buf);
}

/****************************************************************************
** Textures
*/

static byte buf1[256*256*4];
static byte buf2[256*256*4];
static byte mbuf[1024*4];

int txt_findstart(Tile *t)
{
    dword i;
    i=(((t->membase+(t->y0<<8))&0xffffff)/17)%(MAXTXT-4)+1;
    return((int)i);
}

int txt_findempty(Tile *t)
{
    int i;
    int j,besti,bestframe;

    besti=i=txt_findstart(t);
    bestframe=0x7fffffff;

    // find oldest nearby texture
    for(j=0;j<MAXTXT/8;j++)
    {
        i++; if(i>=MAXTXT) i=1;
        if(rst.txt[i].used_vidframe<bestframe)
        {
            bestframe=rst.txt[i].used_vidframe;
            besti=i;
        }
    }

    return(besti);
}

int txt_findmatch(Tile *t)
{
    Texture *txt;
    int i,j;

    i=txt_findstart(t);

    // find texture matching parameters
    for(j=0;j<MAXTXT/8;j++)
    {
        i++; if(i>=MAXTXT) i=1;
        txt=rst.txt+i;
        if(txt->membase==t->membase &&
           txt->memy0  ==t->memy0   &&
           txt->memx0  ==t->memx0   &&
           txt->cms    ==t->cms     &&
           txt->cmt    ==t->cmt     &&
           txt->memxs  ==t->xs      &&
           txt->memys  ==t->ys      &&
           txt->mempal ==t->palette &&
           txt->membpp ==t->membpp  &&
           txt->memfmt ==t->memfmt  && i)
        {
            if(txt->memcrc==t->crc)
            {
                return(i);
            }
            else
            {
                return(-i);
            }
        }
    }
    return(0);
}

void txt_scale(byte *dst,int dx,int dy,int drl,
               byte *src,int sx,int sy,int srl)
{
    int x,y,x1,y1;
    int xmul=16384*sx/dx;
    int ymul=16384*sy/dy;
    dword *dw=(dword *)dst;
    dword *sw=(dword *)src;

    if(st.dumpgfx) logd("\n+tile scale (%i,%i,%i)->(%i,%i,%i) mul (%04X,%04X) "
        ,sx,sy,srl,dx,dy,drl,xmul,ymul);

    if(sx<1 || sy<1 || sx>512 || sy>256)
    {
        logd("\n+tile INVALID SRC SIZE!");
        memset(dst,0,dy*drl*4);
        return;
    }
    if(dx<1 || dy<1 || dx>512 || dy>256)
    {
        logd("\n+tile INVALID DST SIZE!");
        memset(dst,0,dy*drl*4);
        return;
    }

    for(y=0;y<dy;y++) for(x=0;x<dx;x++)
    {
        x1=(x*xmul) >> 14;
        y1=(y*ymul) >> 14;
        dw[x+y*drl]=sw[x1+y1*srl];
    }

    /*
    xmul=16384*dx/sx;
    ymul=16384*dy/sy;
    memset(dw,0,dy*drl*4);
    for(y=0;y<sy;y++) for(x=0;x<sx;x++)
    {
        x1=(x*xmul) >> 14;
        y1=(y*ymul) >> 14;
        dw[x1+y1*drl]=sw[x+y*srl];
    }
    */
}

void txt_fill(byte *dst,int dx,int dy,dword col)
{
    dword *dw=(dword *)dst;
    int    x,y;
    for(y=0;y<dy;y++) for(x=0;x<dx;x++)
    {
        dw[x+y*dx]=col;
    }
}

int txt_checkalpha(byte *dst,int dx,int dy)
{
    dword *dw=(dword *)dst;
    int    x,y;
    dword  a;
    for(y=0;y<dy;y++) for(x=0;x<dx;x++)
    {
        a=dw[x+y*dx];
        if(a<0xf0000000) return(1);
    }
    return(0);
}

void txt_border(byte *dst,int dx,int dy,dword col)
{
    dword *dw=(dword *)dst;
    int    x,y;
    for(y=0;y<dy;y++)
    {
        x=0;
        dw[x+y*dx]=col;
        x=dx-1;
        dw[x+y*dx]=col;
    }
    for(x=0;x<dx;x++)
    {
        y=0;
        dw[x+y*dx]=col;
        y=dy-1;
        dw[x+y*dx]=col;
    }
}

void txt_mirrorx(byte *dst,int dx,int dy,byte *src)
{
    dword *dw=(dword *)dst;
    dword *ds=(dword *)src;
    int    rl=dx*2;
    int    x,y;
    for(y=dy-1;y>=0;y--)
    {
        for(x=0;x<dx;x++) dw[x+y*rl]=ds[x+y*dx];
        for(x=0;x<dx;x++) dw[dx+x+y*rl]=ds[(dx-1-x)+y*dx];
    }
}

void txt_mirrory(byte *dst,int dx,int dy,byte *src)
{
    dword *dw=(dword *)dst;
    dword *ds=(dword *)src;
    int    y;
    for(y=0;y<dy;y++)
    {
        memcpy(dw+y*dx,ds+y*dx,dx*4);
    }
    for(y=0;y<dy;y++)
    {
        memcpy(dw+(dy+y)*dx,ds+(dy-1-y)*dx,dx*4);
    }
}

dword crcmem(dword addr,int size,int samples)
{
    int i,ia;
    dword crc=0;

    if(samples<=0) ia=4;
    else
    {
        ia=size/samples;
        ia&=~3;
        if(!ia) ia=4;
    }
    for(i=0;i<size;i+=ia)
    {
        crc^=mem_read32p(addr+i);
        crc =(crc<<7)|(crc>>(32-7));
    }

    return(crc);
}

int txt_rl(Tile *t)
{
    int rl;
//    if(t->memrl<=1) rl=t->xs*realbpp/8;
    if(t->memrl<=1) rl=t->tmemrl;
    else            rl=t->memrl;
    return(rl);
}

dword txt_calccrc(Tile *t)
{
    int    size,rl;
    dword  crc,madd,addr;
    int    realbpp=4<<t->bpp;

    if(isoldcbuf(t->membase))
    {
        t->fromfb=1;
        return(-2);
    }
    else t->fromfb=0;

    rl=txt_rl(t);

    madd=t->memx0;//*realbpp/8;
    addr=t->membase+t->memy0*rl+madd;
    size=t->ys*rl;

    st2.gfx_txtbytes+=size;

    crc=0;
    // include palette? (5 samples)
    if(t->fmt==2) crc+=crcmem(rst.tlut_base,rst.tlut_num*2,5);
    // memory image crc (7 samples)
    crc+=crcmem(addr,size,7);

    if(st.dumpgfx) logd("\n+tile calccrc %08X,%i -> %08X",addr,size,crc);

    return(crc);
}

byte *txt_loadline(byte *mbuf,dword addr,int y,int flip,int rl2)
{
    int x,rl;
    dword a;
    addr+=y*rl2/2;
    rl=((rl2>>1)+3)/4;
    if(!(y&1)) flip=0;
    for(x=0;x<rl;x++)
    {
        if(addr&3)
        {
            a =mem_read8(addr+3)<<24;
            a|=mem_read8(addr+2)<<16;
            a|=mem_read8(addr+1)<<8;
            a|=mem_read8(addr+0)<<0;
        }
        else
        {
            a=mem_read32p(addr);
            a=FLIP32(a);
        }
        *(dword *)(mbuf+(x^flip)*4)=a;
        addr+=4;
    }
    return(mbuf);
}

void txt_loadtlut(int pal,int num,dword addr)
{
    int i;
    dword x;

    pal*=16;
    logd("\n+tile loadtlut %08X pal=%i num=%i ",
        addr,pal,num);
    if(pal+num>256)
    {
        logd("!palette overflow\n");
        warning("rdp: palette tlut load overflow (pal=%i num=%i addr=%X)",pal,num,addr);
        num=256-pal;
    }
    for(i=0;i<num;i+=2)
    {
        x=mem_read32p(addr);
        addr+=4;
        rst.palette[pal++]=(x>>16);
        rst.palette[pal++]=x;
    }
}

void txt_paletteread(byte *dst,int ind)
{
    int a,c,i;

    a=2048+ind*2;
    c=rst.palette[ind];

    if(rst.s_tluttype==3)
    { // IA
        i=(c>>8);
        a=c&255;
        dst[0]=i;
        dst[1]=i;
        dst[2]=i;
        dst[3]=a;
    }
    else
    { // RGBA
        a=FIELD(c,11,5); dst[0]=8*a+(a>>2);
        a=FIELD(c, 6,5); dst[1]=8*a+(a>>2);
        a=FIELD(c, 1,5); dst[2]=8*a+(a>>2);
        a=FIELD(c, 0,1); dst[3]=255*a;
    }
}

void txt_showpal(byte *dst,int xs,int ys,int palbase,int palnum)
{
    dword *d=(dword *)dst;
    int x,y;
    if(0)
    {
        for(y=0;y<xs;y++)
        {
            for(x=0;x<ys;x++)
            {
                d[x+y*xs]=0xff0000ff;
            }
        }
    }
    if(palnum==256)
    {
        for(y=0;y<16;y++)
        {
            for(x=0;x<16;x++)
            {
                txt_paletteread((byte *)(d+x+y*xs),palbase+x+y*16);
            }
        }
    }
    else
    {
        for(y=0;y<16;y++)
        {
            for(x=0;x<16;x++)
            {
                txt_paletteread((byte *)(d+x+y*xs),palbase+(x>>2)+(y>>2)*16);
            }
        }
    }
}

void txt_convert(byte *dst0,Tile *t)
{ // convert from texmem to 32 bit RGBA
    int    palbase=16*t->palette;
    int    bpp=t->bpp;
    int    fmt=t->fmt;
    int    xs=t->xs;
    int    ys=t->ys;
    int    x,y,rl2,i,j,a;
    dword  c,addr;
    byte  *m,*dst=dst0;
    int    realbpp;
    int    madd,flip;

    realbpp=4<<t->bpp;

    if(t->memrl<0) flip=1; else flip=0;
    rl2=txt_rl(t)*2;

    if(rl2>512*4*2)
    {
        print("rl=%08X xs=%i realbpp=%i t->memrl=%i\n",rl2/2,xs,realbpp,t->memrl);
        exception("rdp: invalid texture memory width");
        return;
    }

    madd=t->memx0;//*realbpp/8;
    addr=t->membase+t->memy0*rl2/2;

    if(st.dumpgfx) logd("\n+tile convert from %08X bpp=%i rl=%i (%i,%i) base(%ib,%il) ",
                          addr,realbpp,rl2/2,xs,ys,t->memx0,t->memy0);

    if(0)
    {
        char name[32];
        FILE *f1;
        sprintf(name,"ti%06X.out",t->membase&0xffffff);
        f1=fopen(name,"wb");
        for(y=0;y<ys;y++)
        {
            m=txt_loadline(mbuf,addr,y,0,rl2);
            fwrite(m,1,rl2/2,f1);
        }
        fclose(f1);
    }

    //   fmt: 0    1   2   3   4
    // bits:  RGBA YUV CI  IA  I    ok=supported
    //  4bpp  ok   -   ok  ok  ok   pa=partial
    //  8bpp  ?    -   ok  ok  ok    +=seen, not supported
    // 16bpp  ok   -   -   ok  -     -=not seen
    // 32bpp  ok   -   -   -   -     -=not seen
if(bpp==1 && fmt==0) fmt=2;

    if(fmt==2)
    {
        if(1)
        { // always
            txt_loadtlut(rst.tlut_palbase,rst.tlut_num,rst.tlut_base);
        }
        else
        { // if base changed (doesn't always work, crc needed)
            if(rst.tlut_base   !=rst.tlut_lastbase    ||
               rst.tlut_palbase!=rst.tlut_lastpalbase ||
               rst.tlut_num    !=rst.tlut_lastnum     )
            {
                rst.tlut_lastbase   =rst.tlut_base   ;
                rst.tlut_lastpalbase=rst.tlut_palbase;
                rst.tlut_lastnum    =rst.tlut_num    ;
                txt_loadtlut(rst.tlut_palbase,rst.tlut_num,rst.tlut_base);
            }
        }
    }

//-------------------------------- 32bpp
    if(bpp==3 && fmt==0)
    { // RGBA 32bit 8-8-8-8
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                *dst++=(m[0]);
                *dst++=(m[1]);
                *dst++=(m[2]);
                *dst++=(m[3]);
                m+=4;
            }
        }
    }
//-------------------------------- 16bpp
    else if(bpp==2 && fmt==0)
    { // RGBA 16bit 1-5-5-5
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                { // 5551 rgba
                    c=(m[0]<<8)+m[1];
                    a=FIELD(c,11,5); *dst++=8*a+(a>>2);
                    a=FIELD(c, 6,5); *dst++=8*a+(a>>2);
                    a=FIELD(c, 1,5); *dst++=8*a+(a>>2);
                    a=FIELD(c, 0,1); *dst++=255*a;
                    //dst[-4]=dst[-1]; // show alpha as red
                }
                m+=2;
            }
        }
    }
    else if(bpp==2 && fmt==3)
    { // IA 16bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                int i,a;
                a=m[1];
                i=m[0];
                *dst++=i;
                *dst++=i;
                *dst++=i;
                *dst++=a;
                m+=2;
            }
        }
    }
//-------------------------------- 8bpp
    else if(bpp==1 && fmt==3)
    { // IA 8bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                i=((m[0]>>4)&15)*17;
                a=( m[0]    &15)*17;
                *dst++=i;
                *dst++=i;
                *dst++=i;
                *dst++=a;
                m++;
            }
        }
    }
    else if(bpp==1 && fmt==0)
    { // RGBA 8bit [NEVER USER, converted to CI 8 bit currently]
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                a=(m[0]>>6)&3; a|=(a<<2); a|=(a<<4); *dst++=a;
                a=(m[0]>>4)&3; a|=(a<<2); a|=(a<<4); *dst++=a;
                a=(m[0]>>2)&3; a|=(a<<2); a|=(a<<4); *dst++=a;
                a=(m[0]>>0)&3; a|=(a<<2); a|=(a<<4); *dst++=a;
                m++;
            }
        }
    }
    else if(bpp==1 && fmt==4)
    { // I 8bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                i=m[0];
                dst[0]=i;
                dst[1]=i;
                dst[2]=i;
                dst[3]=i;
                dst+=4;
                m++;
            }
        }
    }
    else if(bpp==1 && fmt==2)
    { // CI 8bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x++)
            {
                i=m[0];
                txt_paletteread(dst,i+palbase);
                dst+=4;
                m++;
            }
        }
//        txt_showpal(dst0,xs,ys,palbase,256);
    }
//-------------------------------- 4bpp
    else if(bpp==0 && fmt==0)
    { // RGBA 4bit 1-1-1-1
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x+=2)
            {
                a=m[0];
                *dst++=(m[0]&0x80)?255:0;
                *dst++=(m[0]&0x40)?255:0;
                *dst++=(m[0]&0x20)?255:0;
                *dst++=(m[0]&0x10)?255:0;
                a<<=4;
                *dst++=(m[0]&0x80)?255:0;
                *dst++=(m[0]&0x40)?255:0;
                *dst++=(m[0]&0x20)?255:0;
                *dst++=(m[0]&0x10)?255:0;
                m++;
            }
        }
    }
    else if(bpp==0 && fmt==3)
    { // IA 4bit 3-1
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x+=2)
            {
                for(j=4;j>=0;j-=4)
                {
                    i=((m[0]>>(j+1))&7)*36;
                    a=((m[0]>>(j+0))&1)*255;
                    *dst++=i;
                    *dst++=i;
                    *dst++=i;
                    *dst++=a;
                }
                m++;
            }
        }
    }
    else if(bpp==0 && fmt==4)
    { // I 4bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x+=2)
            {
                for(j=4;j>=0;j-=4)
                {
                    i=(m[0]>>j)&15;
                    dst[0]=i*17;
                    dst[1]=i*17;
                    dst[2]=i*17;
                    dst[3]=i*17;
                    dst+=4;
                }
                m++;
            }
        }
    }
    else if(bpp==0 && fmt==2)
    { // CI 4bit
        for(y=0;y<ys;y++)
        {
            m=madd+txt_loadline(mbuf,addr,y,flip,rl2);
            for(x=0;x<xs;x+=2)
            {
                for(j=4;j>=0;j-=4)
                {
                    i=(m[0]>>j)&15;
                    txt_paletteread(dst,i+palbase);
                    dst+=4;
                }
                m++;
            }
        }
//        txt_showpal(dst0,xs,ys,palbase,16);
    }
//-------------------------------- unknown
    else
    {
        logd("\n+tile t_converttexture bpp=%i fmt=%i ???\n",bpp,fmt);
        warning("dlist: unsupported texture format bpp=%i fmt=%i",bpp,fmt);
        for(y=0;y<ys;y++)
        {
            for(x=0;x<xs;x++)
            {
                c=((x^y)<<4)^128;
                *dst++=c;
                *dst++=c;
                *dst++=c;
                *dst++=255;
            }
        }
    }

    logd("\n+tile convert ends %08X (flip=%i)",addr+ys*rl2/2,flip);

    if(0 && flip)
    {
        dword *d=(dword *)dst0,a;
        for(y=0;y<ys;y++)
        {
            if(!(y&1))
            {
                d+=xs;
                continue;
            }
            for(x=0;x<xs;x+=2)
            {
                a=d[0];
                d[0]=d[1];
                d[1]=a;
                d+=2;
            }
        }
    }

    if(0)
    {
        char name[32];
        FILE *f1;
        sprintf(name,"ti%06X.out",addr&0xffffff);
        f1=fopen(name,"wb");
        fwrite(dst0,4,xs*ys,f1);
        fclose(f1);
        print("convert %08X %ix%i\n",addr,xs,ys);
    }
}

void rdp_grabscreen(void)
{
    #if 0
    if(rst.myframe>rst.lastframegrabbed+8 || rst.myframe<rst.lastframegrabbed)
    {
        int i;
        print("rdp_grabscreen\n");
        if(!rst.framegrab)
        {
            rst.framegrab=malloc(init.gfxwid*init.gfxhig*4);
            if(!rst.framegrab) return;
        }
        rst.lastframegrabbed=rst.myframe;
        x_readfb(X_FB_FRONT|X_FB_RGBA8888,0,0,init.gfxwid,init.gfxhig,rst.framegrab,init.gfxwid*4);
        // invalidate framegrabs in cache
        for(i=0;i<MAXTXT;i++)
        {
            if(rst.txt[i].fromfb)
            {
                rst.txt[i].membase=0xffffffff;
                rst.txt[i].memfmt=999;
            }
        }
    }
    else
    {
        //print("rdp_grabscreen ignored\n");
    }
    #endif
}

void txt_fromfb(byte *buf,Tile *t)
{
    int x,y,y0,xa,xx;
    dword *s,*d;

    if(!rst.framegrab)
    {
        d=(dword *)buf;
        for(y=0;y<t->ys;y++)
        {
            for(x=0;x<t->xs;x++)
            {
               d[x+y*t->xs]=0;
            }
        }
        return;
    }

    d=(dword *)buf;
    xa=4096*init.gfxwid/t->xs;
    y0=t->memy0;

    if(y0+t->ys>240 || t->ys>240 || t->xs>320) return;

//print("fromfb size %i,%i basey %i\n",t->xs,t->ys,t->memy0);

    for(y=0;y<t->ys;y++)
    {
        s=(dword *)rst.framegrab;
        s+=init.gfxwid*((y+y0)*init.gfxhig/240);
        xx=0;
        for(x=0;x<t->xs;x++)
        {
            d[x+y*t->xs]=s[xx>>12];
            xx+=xa;
        }
        //d[0+y*t->xs]=y*77;
    }
}

void txt_fromcbuf(byte *buf,Tile *t)
{
    int x,y;
    dword *d;
    dword cols[4]={0xff000000,0xff101010,
                   0xff101010,0xff000000};

    d=(dword *)buf;
    for(y=0;y<t->ys;y++)
    {
        for(x=0;x<t->xs;x++)
        {
            d[x+y*t->xs]=cols[(y&1)*2+(x&1)];
        }
    }
}

#define SWAP(s,d) sd=s,s=d,d=sd

void txt_loaddata(Texture *txt,Tile *t)
{
    int sx,sy;
    int limit,flags;
    int hasalpha;
    byte *s,*d,*sd;

    if((t->xs&1) && t->bpp==0) t->xs--;

    // calc sizes (allow 4 pixel smaller than original)
    sx=8; while(sx+4<t->xs) sx*=2;
    sy=8; while(sy+4<t->ys) sy*=2;

    if(t->cms || t->cmt)
    {
        limit=4;
        if(sx>128) sx=128;
        if(sy>128) sy=128;
    }
    else
    {
        limit=8;
        if(sx>256) sx=256;
        if(sy>256) sy=256;
    }
    if(sx<sy)
    {
        if(sx*limit<sy) sx=sy/limit;
    }
    else
    {
        if(sy*limit<sx) sy=sx/limit;
    }

    s=buf1;
    d=buf2;
    // convert to RGBA
    if(st.dumpgfx) logd("\n+tile txt_convert %08X (%i,%i) rl=%ip fmt=%s bpp=%s ",
        t->membase,t->xs,t->ys,t->memrl,fmt[t->fmt],bpp[t->bpp]);

    /*
    if(cart.iszelda && t->membase==rst.bufbase[RDP_BUF_Z])
    { // reusing old screen buffer
        logd(" FRAMEBUFCOPY ");
        txt_fromfb(s,t);
        txt->fromfb=1;
    }
    */
    if(t->fromfb)
    {
        txt->fromfb=0;
        logd(" CBUFSOURCE ");
        txt_fromcbuf(s,t);
    }
    else
    {
        // convert to RGBA
        txt_convert(s,t);
    }

    // check for alpha
    hasalpha=txt_checkalpha(s,t->xs,t->ys);

    // scale to power of 2
    if(sx!=t->xs || sy!=t->ys)
    {
        txt_scale(d,sx,sy,sx,s,t->xs,t->ys,t->xs);
        SWAP(d,s);
    }

    // mirror
    if(t->cms&1)
    {
        if(st.dumpgfx) logd(" mirrorx ");
        txt_mirrorx(d,sx,sy,s);
        SWAP(d,s);
        sx*=2;
    }
    if(t->cmt&1)
    {
        if(st.dumpgfx) logd(" mirrory ");
        txt_mirrory(d,sx,sy,s);
        SWAP(d,s);
        sy*=2;
    }

    txt->xs=sx;
    txt->ys=sy;
    txt->cms=t->cms;
    txt->cmt=t->cmt;

    txt->creation_vidframe=rst.myframe;

    if(hasalpha) flags=X_RGBA4444;
    else flags=X_RGBA5551;

    if(t->cmt>=2 || t->cms>=2)
    {
        flags|=X_CLAMP;
        if(t->cms<2) flags|=X_CLAMPNOX;
        if(t->cmt<2) flags|=X_CLAMPNOY;
    }

//    if(t->membase==0x8028b2b0) fill(s,sx,sy,0xff0000ff);
//    if((t->membase&0xffff0000)==0x00340000) txt_fill(s,sx,sy,0xff0000ff);

//    if(rst.s_txtfilt==0) flags|=X_NOBILIN;

    txt->xhandle=x_createtexture(flags,txt->xs,txt->ys);
    if(txt->xhandle<0) exception("too many textures\n");
    x_loadtexturelevel(txt->xhandle,0,s);

    //if(st.dumpgfx) logd("\n+tile x_create %04X size %ix%i\n",flags,txt->xs,txt->ys);
}

void txt_setscales(int tile)
{
    Tile    *t=rst.tile+tile+rst.texturetile;
    Texture *txt=rst.txt+t->texture;
    float    xd,yd,xdm,ydm;

    if(!t->texture)
    {
        logd("\n+tile ERROR??? tile %i setscale with no texture\n",tile);
        rst.txt_uscale=1/32.0/32.0;
        rst.txt_vscale=1/32.0/32.0;
        rst.txt_uadd=0;
        rst.txt_vadd=0;
        return;
    }

    xd=(1.0/32.0)/(t->xs);
    yd=(1.0/32.0)/(t->ys);

    if(t->shifts>0 && t->shifts<10) xdm=1.0/(1 <<     t->shifts );
    else if(t->shifts>=10)          xdm=    (1 << (16-t->shifts));
    else                            xdm=1.0;
    if(t->shiftt>0 && t->shiftt<10) ydm=1.0/(1 <<     t->shiftt );
    else if(t->shiftt>=10)          ydm=    (1 << (16-t->shiftt));
    else                            ydm=1.0;

    if(1)
    {
        rst.txt_uadd=16.0-8.0*t->x0full/xdm;
        rst.txt_vadd=16.0-8.0*t->y0full/ydm;
    }
    else
    {
        rst.txt_uadd=16.0-8.0*t->x0full;
        rst.txt_vadd=16.0-8.0*t->y0full;
    }

    xd*=xdm;
    yd*=ydm;

    if(t->cms&1) xd*=0.5; // mirror
    if(t->cmt&1) yd*=0.5; // mirror

    rst.txt_uscale=xd;
    rst.txt_vscale=yd;

    if(st.dumpgfx) logd("\n+tile setscales tile %i txt %i scale %.4f,%.4f add %.4f,%.4f base %i,%i",
        tile,t->texture,rst.txt_uscale,rst.txt_vscale,rst.txt_uadd,rst.txt_vadd,t->memx0,t->memy0);
}

void txt_select(int tile)
{
    Tile    *t=rst.tile+tile+rst.texturetile;
    Texture *txt=rst.txt+t->texture;

    if(st.dumpgfx) logd("\n+tile SELECT tile %i txt %i",tile,t->texture);
    x_texture(txt->xhandle);
    txt_setscales(tile);
}

void txt_select2(int tile1,int tile2)
{
    Tile    *t;
    Texture *txt;
    int      xh1,xh2;

    t=rst.tile+tile2+rst.texturetile;
    txt=rst.txt+t->texture;
    if(st.dumpgfx) logd("\n+tile SELECT-MT1 tile %i txt %i",tile1,t->texture);
    xh2=txt->xhandle;
    txt_setscales(tile2);

    rst.txt_uscale2=rst.txt_uscale;
    rst.txt_vscale2=rst.txt_vscale;
    rst.txt_uadd2=rst.txt_uadd;
    rst.txt_vadd2=rst.txt_vadd;

    t=rst.tile+tile1+rst.texturetile;
    txt=rst.txt+t->texture;
    if(st.dumpgfx) logd("\n+tile SELECT-MT2 tile %i txt %i",tile2,t->texture);
    xh1=txt->xhandle;
    txt_setscales(tile1);

    x_texture2(xh1,xh2);
}

void txt_prepare(int tile)
{
    Tile    *t=rst.tile+tile+rst.texturetile;
    Texture *txt;
    int      match=0,crcerror=0,sizeok;

    sizeok=0;
    if((unsigned)t->xs<=256 && (unsigned)t->ys<=256) sizeok=1;
    if((unsigned)t->xs<=512 && (unsigned)t->ys<=128) sizeok=1;

    if((unsigned)t->bpp>3 || (unsigned)t->fmt>7 || !sizeok)
    {
        logd("\n+tile %i ILLEGAL??? texture size (%ix%i)/format (%i/%i)",
            tile,t->xs,t->ys,t->bpp,t->fmt);
        t->texture=0;
        return;
    }

    // update tile membase
    t->membase=rst.tmemsrc[t->tmembase>>3];
    t->memrl  =rst.tmemrl [t->tmembase>>3];
    t->memx0  =rst.tmemx0 [t->tmembase>>3];
    t->memy0  =rst.tmemy0 [t->tmembase>>3];
    t->crc    =txt_calccrc(t);

    match=txt_findmatch(t);

    if(match>0)
    {
        t->texture=match;
        txt=rst.txt+t->texture;
        if(st.dumpgfx) logd("\n+tile %i cache hit (texture %i)",tile,t->texture);
    }
    else
    {
        if(match<0) t->texture=-match;
        else t->texture=txt_findempty(t);

        txt=rst.txt+t->texture;

        // free slot
        if(txt->xhandle) x_freetexture(txt->xhandle);
        memset(txt,0,sizeof(Texture));

        txt->membpp =t->membpp;
        txt->memfmt =t->memfmt;
        txt->membase=t->membase;
        txt->mempal =t->palette;
        txt->memx0  =t->memx0;
        txt->memy0  =t->memy0;
        txt->memxs  =t->xs;
        txt->memys  =t->ys;
        txt->memcrc =t->crc;
        txt->used_vidframe=-1;

        txt_loaddata(rst.txt+t->texture,t);

        if(st.dumpgfx)
        {
            if(crcerror) logd("\n+tile %i cache crc miss (loading to texture %i)",tile,t->texture);
            else         logd("\n+tile %i cache address miss (loading to texture %i)",tile,t->texture);
        }

        rst.cnt_texturegen++;
    }

    rst.cnt_texture++;

    if(txt->used_vidframe!=rst.myframe)
    {
        txt->txtslot=rst.txtloads;
        if(rst.txtloads<MAXLOAD)
        {
            rst.txtload[rst.txtloads++]=t->texture;
        }
        else
        {
            warning("rdp: too many textureloads in frame!");
        }
        txt->used_vidframe=rst.myframe;
    }

    logd(" [txtload %i]",txt->txtslot);
}

/****************************************************************************
** Drawing
*/

static void setfog(int pass)
{
    float cr,cg,cb;
    int type;

    if(rst.fogenable)
    {
        if(pass) type=X_LINEARADD;
        else type=X_LINEAR;
    }
    else type=X_DISABLE;

    if(type==rst.foglasttype) return;

    rst.foglasttype=type;

    // send to x-engine
    if(type==X_DISABLE)
    {
        x_fog(X_DISABLE,0,0,1.0,0.5,0.1);
    }
    else
    {
        cr=rst.colf[C_FOG][0];
        cg=rst.colf[C_FOG][1];
        cb=rst.colf[C_FOG][2];
        // max*1.2... looks better, not sure what is right
        x_fog(type,rst.fogmin,rst.fogmax*1.2,cr,cg,cb);
    }
}

float getcolor(int j,int r,int i)
{
    int a;
    float x;
    a=rst.s_combx[j][r];
    x=rst.colf[a&C_INDEX][i];
    if(a&C_NEGATE) x=1.0-x;
    if(a&C_MULCASE) x*=rst.colf[(a>>8)&C_INDEX][i];
    return(x);
}

void mixcolor(float *d,float *shade,int col,int alp)
{
    int   c,c1,c2;
    float cr,cg,cb,ca;

    rst.colf[C_SHADE][0]=shade[0];
    rst.colf[C_SHADE][1]=shade[1];
    rst.colf[C_SHADE][2]=shade[2];
    rst.colf[C_SHADE][3]=shade[3];
    rst.colf[C_SHAA][0]=shade[3];
    rst.colf[C_SHAA][1]=shade[3];
    rst.colf[C_SHAA][2]=shade[3];
    rst.colf[C_SHAA][3]=shade[3];

    // color
    if(col==C_FULL || alp==C_FULL)
    {
        // full blend based on rst.s_combx
        int i,j;
        float x;

        for(i=0;i<4;i++)
        {
            if(i==3) j=1; else j=0;
            x =getcolor(j,0,i);
            x-=getcolor(j,1,i);
            x*=getcolor(j,2,i);
            x+=getcolor(j,3,i);
            if(x<0) x=0;
            if(x>1) x=1;
            d[i]=x;
        }
        /*
        print("direct %.2f %.2f %.2f %.2f shadeg %.2f  mix %i %i %i %i \n",
            d[0],d[1],d[2],d[3],
            rst.colf[C_SHADE][1],
            rst.s_combx[j][0],
            rst.s_combx[j][1],
            rst.s_combx[j][2],
            rst.s_combx[j][3]);
        */
    }

    c=col;
    if(c!=C_FULL)
    {
        c1=c&C_INDEX;
        cr=rst.colf[c1][0];
        cg=rst.colf[c1][1];
        cb=rst.colf[c1][2];
        if(c&C_NEGATE)
        {
            cr=1.0f-cr;
            cg=1.0f-cg;
            cb=1.0f-cb;
        }

        if(c&C_MULCASE)
        {
            c2=(c>>8)&C_INDEX;
            cr*=rst.colf[c2][0];
            cg*=rst.colf[c2][1];
            cb*=rst.colf[c2][2];
        }

        d[0]=cr;
        d[1]=cg;
        d[2]=cb;
    }

    // alpha
    c=alp;
    if(c!=C_FULL)
    {
        c1=c&C_INDEX;
        ca=rst.colf[c1][3];
        if(c&C_NEGATE)
        {
            ca=1.0-ca;
        }

        if(c&C_MULCASE)
        {
            c2=(c>>8)&C_INDEX;
            ca*=rst.colf[c2][3];
        }

        d[3]=ca;
    }
}

void drawprims(int p,int i0,int i1)
{
    Primitive *pr;
    Vertex *vx;
    int i,j,ji;

    rst.vxtabinitedcnt++; // forces a new init to all vertices used

    if(st.dumpgfx)
    {
        logd("\n+FLUSH pass:%i/%i txttile:%i blend:%04X/%04X comb:%04X/%04X col:%05X/%05X envc:%i",
            p+1,COM.passes,
            COM.txt[p],
            COM.blend1[p],COM.blend2[p],
            COM.com[p][0],COM.com[p][1],
            COM.col[p][0],COM.col[p][1],
            COM.env);
    }

    // draw
    if(COM.dualtxt)
    {
        if(st.dumpgfx) logd("DUALTXT! ");
        x_begin(X_TRIANGLES);
        for(i=i0;i<i1;i++)
        {
            pr=rst.prtab+i;
            for(j=0;j<3;j++)
            {
                ji=(j+2); if(ji>=3) ji-=3;
                vx=pr->c[ji];

                if(rst.vxtabinited[vx-rst.vxtab]!=rst.vxtabinitedcnt)
                {
                    // vertex not yet inited on this pass
                    rst.vxtabinited[vx-rst.vxtab]=rst.vxtabinitedcnt;

                    // color
                    mixcolor(vx->cc,vx->col,COM.col[p][0],COM.col[p][1]);

                    // texture
                    vx->ct[0]=-1;
                    vx->ct[1]=-1;
                }

                if(!j || !rst.flat)
                {
                    x_vxcolor4(vx->cc[0],vx->cc[1],vx->cc[2],vx->cc[3]);
                }
                /*
                x_vxtex ((vx->tex[0]+rst.txt_uadd)*rst.txt_uscale,
                         (vx->tex[1]+rst.txt_vadd)*rst.txt_vscale);
                x_vxtex2((vx->tex[0]+rst.txt_uadd2)*rst.txt_uscale2,
                         (vx->tex[1]+rst.txt_vadd2)*rst.txt_vscale2);
                */
                x_vxtex ((vx->tex[0]+rst.txt_uadd )*rst.txt_uscale,
                         (vx->tex[1]+rst.txt_vadd )*rst.txt_vscale);
                x_vxtex2((vx->tex[0]+rst.txt_uadd2)*rst.txt_uscale,
                         (vx->tex[1]+rst.txt_vadd2)*rst.txt_vscale);
                x_vxposv((xt_pos *)vx->pos);
            }
        }
        x_end();
    }
    else
    {
        x_begin(X_TRIANGLES);
        for(i=i0;i<i1;i++)
        {
            pr=rst.prtab+i;
            for(j=0;j<3;j++)
            {
                ji=(j+2); if(ji>=3) ji-=3;
                vx=pr->c[ji];

                if(rst.vxtabinited[vx-rst.vxtab]!=rst.vxtabinitedcnt)
                {
                    // vertex not yet inited on this pass
                    rst.vxtabinited[vx-rst.vxtab]=rst.vxtabinitedcnt;

                    // color
                    mixcolor(vx->cc,vx->col,COM.col[p][0],COM.col[p][1]);

                    // texture
                    vx->ct[0]=(vx->tex[0]+rst.txt_uadd)*rst.txt_uscale;
                    vx->ct[1]=(vx->tex[1]+rst.txt_vadd)*rst.txt_vscale;
                }

                if(!j || !rst.flat)
                {
                    x_vxcolor4(vx->cc[0],vx->cc[1],vx->cc[2],vx->cc[3]);
                }
                x_vxtex   (vx->ct[0],vx->ct[1]);
                x_vxposv  ((xt_pos *)vx->pos);
            }
        }
        x_end();
    }

    // dump
    if(st.dumpgfx)
    {
        for(i=i0;i<i1;i++)
        {
            pr=rst.prtab+i;
            logd("\ndrawprim");
            for(j=0;j<3;j++)
            {
                vx=pr->c[j];
                logd("\ndvx[%04i: %9.2f %9.2f %9.2f uv %5.2f %5.2f rgb %.2f %.2f %.2f %.2f ]",
                    vx-rst.vxtab,
                    vx->pos[0],vx->pos[1],vx->pos[2],
                    vx->ct[0],vx->ct[1],
                    vx->cc[0],vx->cc[1],vx->cc[2],vx->cc[3]);
                if(rst.flat && j==2) logd(" (flat)");
            }
        }
    }
}

// draw primitives collected since last modechange
// update rst.last_prtabi
void flushprims(void)
{
    int i,i0,i1;

    if(rst.prtabi==rst.last_prtabi) return;

    if(st.dumpgfx) logd("\n+FLUSH (%i prims) atst=%i (blenda=%02X)",
        rst.prtabi-rst.last_prtabi,rst.s_cycles,
        rst.s_alphatst,rst.col[C_BLEND][3]);

    i0=rst.last_prtabi;
    i1=rst.prtabi;
    rst.last_prtabi=rst.prtabi;

    x_geometry(GEOMFLAGS);

//    if(COM.blend1[0]==X_ONE) rst.s_alphatst=0;

    if(1)
    {
        rst.lastalphatst=rst.s_alphatst;
        if(rst.s_alphatst==4 ||
          (rst.s_alphatst==1))// && rst.colf[C_BLEND][3]>0.1))
        {
            if(0)
            {
                float f;
                f=rst.colf[C_BLEND][3];
                if(f<0.1) f=0.1;
                logd("\n+alphatst %i %.3f",rst.s_alphatst,f);
                if(rst.s_alphatst!=1) x_alphatest(0.6);
                else x_alphatest(f);
            }
            else
            {
                x_alphatest(0.25);
            }
        }
        else
        {
            if(COM.blend1[0]==X_ALPHA) x_alphatest(0.05);
            else x_alphatest(1.0);
        }
    }

    {
        rst.lastzmode=rst.s_zmode;

        if(rst.s_zmode>0) x_zdecal(1.01);
        else x_zdecal(1.0);
    }

    { // draw
        for(i=0;i<COM.passes;i++)
        {
            float c[4];
            setfog(i);
            mixcolor(c,rst.colf[C_ONE],COM.env,COM.env);
            x_envcolor(c[0],c[1],c[2],c[3]);
            if(st.dumpgfx) logd(" ENV=%.2f %.2f %.2f %.2f ",c[0],c[1],c[2],c[3]);
            x_blend(COM.blend1[i],COM.blend2[i]);
            if(COM.dualtxt)
            {
                x_procombine2(COM.com[i][0],COM.com[i][1],COM.dualtxt,0);
            }
            else
            {
                x_procombine(COM.com[i][0],COM.com[i][1]);
            }
            if(COM.forceatst[i]) switch(COM.forceatst[i])
            {
                case -1: x_alphatest(1.00); break;
                case  1: x_alphatest(0.05); break;
                case  2: x_alphatest(0.25); break;
                case  3: x_alphatest(0.50); break;
                case  4: x_alphatest(0.75); break;
                case  9:
                    {
                        static int cnt=0;
                        cnt++;
                        switch(cnt&3)
                        {
                        case 0: x_alphatest(0.05); break;
                        case 1: x_alphatest(0.25); break;
                        case 2: x_alphatest(0.15); break;
                        case 3: x_alphatest(0.35); break;
                        }
                    }
                    break;
            }
            if(COM.forcezcmp[i]>0)
            {
                x_mask(X_ENABLE,COM.forcezupd[i],COM.forcezcmp[i]);
            }
            else if(i==0)
            {
                x_mask(X_ENABLE,rst.s_zupd?X_ENABLE:X_DISABLE,
                                rst.s_zcmp?X_ENABLE:X_DISABLE);
            }
            if(COM.texenable)
            {
                if(COM.dualtxt)
                {
                    txt_select2(COM.txt[0],COM.txt[1]);
                }
                else
                {
                    if(i==0 || COM.txt[0]!=COM.txt[i]) txt_select(COM.txt[i]);
                }
            }
            drawprims(i,i0,i1);
        }
    }
}

void clearprims(void)
{
    rst.vxtabi=0;
    rst.prtabi=0;
}

void wireprims(void)
{
    static int rcr,rcg,rcb;
    Primitive *pr;
    int prcnt,cnt;
    int j;

    if(!showinfo && !showwire) return;

    rcr+=77; rcr&=255;
    rcg+=37; rcg&=255;
    rcb+=97; rcb&=255;

    x_flush();

    debugdrawmode();

    x_reset();
    x_geometry(X_WIRE);
    x_mask(X_ENABLE,X_DISABLE,X_DISABLE);

    pr=rst.prtab;
    prcnt=rst.prtabi;
    cnt=0;

    while(prcnt-->0)
    {
        if(!pr->c[0])
        {
            float x0,y0,x1,y1;
            // viewport change
            x0=pr->c[2]->pos[0];
            y0=pr->c[2]->pos[1];
            x1=pr->c[2]->tex[0];
            y1=pr->c[2]->tex[1];
            if(showinfo)
            {
                x0+=320.0;
                x1+=320.0;
            }
            //print("prcnt %-5i: (%.0f,%.0f)-(%.0f,%.0f)\n",prcnt,x0,y0,x1,y1);
            x_viewport(x0,y0,x1,y1);
            pr++;
            continue;
        }
        x_begin(X_TRIANGLES);
        {
            cnt++;
            switch(pr->wirecolor)
            { // wirecolor
                case 1:
                    x_vxcolor(0.0,0.0,0.6);
                    break;
                case 2:
                    x_vxcolor(0.0,0.6,0.0);
                    break;
                case 3:
                    x_vxcolor(0.0,0.6,0.6);
                    break;
                case 4:
                    x_vxcolor(0.6,0.0,0.0);
                    break;
                case 5:
                    x_vxcolor(0.6,0.0,0.6);
                    break;
                case 6:
                    x_vxcolor(0.6,0.6,0.0);
                    break;
                case 7:
                    x_vxcolor(0.5,0.5,0.5);
                    break;
                case 8:
                    x_vxcolor(0.2,0.2,0.2);
                    break;
                case 9:
                    x_vxcolor(0.0,0.0,1.0);
                    break;
                case 10:
                    x_vxcolor(0.0,1.0,0.0);
                    break;
                case 11:
                    x_vxcolor(0.0,1.0,1.0);
                    break;
                case 12:
                    x_vxcolor(1.0,0.0,0.0);
                    break;
                case 13:
                    x_vxcolor(1.0,0.0,1.0);
                    break;
                case 14:
                    x_vxcolor(1.0,1.0,0.0);
                    break;
                case 15:
                    x_vxcolor(1.0,1.0,1.0);
                    break;
                default: // flash
                    x_vxcolor(rcr/256.0,rcg/256.0,rcb/256.0);
                    break;
            }
            for(j=0;j<3;j++)
            {
                x_vxposv((xt_pos *)pr->c[j]->pos);
            }
        }
        x_end();
        pr++;
    }

    x_flush();

    debugdrawmode();
}

void rdp_fillrect(TexRect *tr)
{
    Primitive *p,*p2;
    Vertex *v;

    viewport(1);
    if(rst.modechange) newmode();

    if(rst.firstfillrect)
    {
        if(tr->x0-tr->x1>250 && tr->y0-tr->y1>150)
        {
            int x=tr->x0+1;
            int y=tr->y0+1;
            if(x>295 && x<325)
            {
                x=320;
                y=240;
            }
            if(x!=init.viewportwid || y!=init.viewporthig)
            {
                init.viewportwid=x;
                init.viewporthig=y;
                print("rdp: new screen resolution %ix%i\n",x,y);
            }
        }
        rst.firstfillrect=0;
    }

    if(rst.s_cycles>2)
    {
        tr->x0+=1.0;
        tr->y0+=1.0;
    }

    if(rst.s_zsrc==1)
    {
        return;
        // prim color src, hexen uses to clear zbuf
        if(tr->x0-tr->x1>250 && tr->y0-tr->y1>150)
        {
            x_clear(0,1,0,0,0);
        }
        else return;
    }

    if(rst.bufbase[RDP_BUF_Z]==rst.bufbase[RDP_BUF_C] || rst.rawfillcolor==0xFFFCFFFC)
    {
        // z-buffer clear
        return;
    }

    rst.fillrectcnt++;

    if(st.dumpgfx)
    {
        logd("\n+fillrect %i (%.1f,%.1f)-(%.1f,%.1f) screen (%.2f,%.2f)-(%.2f,%.2f) ",
            rst.fillrectcnt,
            tr->x1,tr->y1,
            tr->x0,tr->y0,
            tr->x1*oxm+oxa,tr->y1*oym+oya,
            tr->x0*oxm+oxa,tr->y0*oym+oya);
    }

//    if(rst.fillrectcnt!=3) return;

    // generate primitive
    p=newpr();
    p->wirecolor=15;

    // generate vertices
    v=p->c[0]=newvx();
    v->pos[0]=tr->x0*oxm+oxa;
    v->pos[1]=tr->y0*oym+oya;
    v->pos[2]=1.0;

    v=p->c[1]=newvx();
    v->pos[0]=tr->x0*oxm+oxa;
    v->pos[1]=tr->y1*oym+oya;
    v->pos[2]=1.0;

    v=p->c[2]=newvx();
    v->pos[0]=tr->x1*oxm+oxa;
    v->pos[1]=tr->y1*oym+oya;
    v->pos[2]=1.0;

    // generate second primitive
    p2=newpr();
    p2->wirecolor=15;
    p2->c[0]=p->c[0];
    p2->c[1]=p->c[2];

    v=p2->c[2]=newvx();
    v->pos[0]=tr->x1*oxm+oxa;
    v->pos[1]=tr->y0*oym+oya;
    v->pos[2]=1.0;

    st2.gfx_tris+=2;
}

void rdp_texrect(TexRect *tr)
{
    Primitive *p,*p2;
    Vertex *v;
    float u0,u1,v0,v1;

    rst.tris++;

    viewport(1);
    if(rst.modechange) newmode();

    if(rst.s_cycles>2)
    {
        u0=tr->s0;
        v0=tr->t0;
        u1=tr->s1*(tr->x0-tr->x1)/32.0;
        v1=tr->t1*(tr->y0-tr->y1)/32.0;
        if(rst.s_cycles>2) u1*=0.25;
        u1+=u0;
        v1+=v0;
        tr->x0+=1.0;
        tr->y0+=1.0;
    }
    else
    {
        u0=tr->s0;
        v0=tr->t0;
        u1=tr->s1*(tr->x0-tr->x1-1)/32.0;
        v1=tr->t1*(tr->y0-tr->y1-1)/32.0;
        if(rst.s_cycles>2) u1*=0.25;
        u1+=u0;
        v1+=v0;
    }

    if(tr->y0<=tr->y1+1.1)
    {
        tr->y0++;
    }

    if(tr->flip)
    {
        float t;
        t=v1;
        v1=v0;
        v0=t;
    }

    // generate primitive
    p=newpr();
    p->wirecolor=15;

    v=p->c[0]=newvx();
    v->pos[0]=tr->x0*oxm+oxa;
    v->pos[1]=tr->y0*oym+oya;
    v->pos[2]=1.0;
    v->tex[0]=u1;
    v->tex[1]=v1;

    v=p->c[1]=newvx();
    v->pos[0]=tr->x0*oxm+oxa;
    v->pos[1]=tr->y1*oym+oya;
    v->pos[2]=1.0;
    v->tex[0]=u1;
    v->tex[1]=v0;

    v=p->c[2]=newvx();
    v->pos[0]=tr->x1*oxm+oxa;
    v->pos[1]=tr->y1*oym+oya;
    v->pos[2]=1.0;
    v->tex[0]=u0;
    v->tex[1]=v0;

    // generate second primitive
    p2=newpr();
    p2->wirecolor=15;
    p2->c[0]=p->c[0];
    p2->c[1]=p->c[2];

    v=p2->c[2]=newvx();
    v->pos[0]=tr->x1*oxm+oxa;
    v->pos[1]=tr->y0*oym+oya;
    v->pos[2]=1.0;
    v->tex[0]=u0;
    v->tex[1]=v1;

    st2.gfx_tris+=2;
}

void rdp_viewport(float xm,float ym,float xa,float ya)
{
    int xs=320;
    int ys=240;

    if(init.viewportwid)
    {
        xs=init.viewportwid;
        ys=init.viewporthig;
    }

    oxm=+2.0/(xs-1);
    oym=-2.0/(ys-1);

    xm*=(float)init.gfxwid/xs;
    ym*=(float)init.gfxhig/ys;
    xa*=(float)init.gfxwid/xs;
    ya*=(float)init.gfxhig/ys;
    rst.view_x0=xa-xm;
    rst.view_y0=ya-ym;
    rst.view_x1=xa+xm-1;
    rst.view_y1=ya+ym-1;
    if(rst.view_x0<0) rst.view_x0=0;
    if(rst.view_x1<0) rst.view_x1=0;
    if(rst.view_y0<0) rst.view_y0=0;
    if(rst.view_y1<0) rst.view_y1=0;
    if(rst.view_x0>=init.gfxwid) rst.view_x0=init.gfxwid-1;
    if(rst.view_x1>=init.gfxwid) rst.view_x1=init.gfxwid-1;
    if(rst.view_y0>=init.gfxhig) rst.view_y0=init.gfxhig-1;
    if(rst.view_y1>=init.gfxhig) rst.view_y1=init.gfxhig-1;
    logd("\n!rdp_viewport (%i,%i)-(%i,%i)",
        rst.view_x0,rst.view_y0,
        rst.view_x1,rst.view_y1);
    realdrawmode();
}

void rdp_texture(int on,int tile,int level)
{
    if(tile<6) rst.nexttexturetile=tile;
}

void rdp_fogrange(float min,float max)
{
    rst.fogmin=min;
    rst.fogmax=max;
    if(rst.fogmin==0.0 && rst.fogmax==0.0) rst.fogenable=0;
    else rst.fogenable=1;
    rst.foglasttype=-1;
}

void fillcbuf(void)
{
    int   xs,ys,rl;
    dword addr;

    addr=rst.bufbase[RDP_BUF_C];
    rl  =rst.bufwid [RDP_BUF_C]*2;
    xs  =rst.view_x1-rst.view_x0;
    ys  =rst.view_y1-rst.view_y0;
    addr+=rst.view_x0*2;
    addr+=rst.view_y0*2*rl;

    print("FillCBUF %08X: rl=%i (%i,%i)-(%i,%i) sz(%i,%i)\n",
        addr,rl,
        rst.view_x0,rst.view_y0,
        rst.view_x1,rst.view_y1,
        xs,ys);
}

void rdp_tri(int *vind)
{
    Primitive *p;

    rst.tris++;

    viewport(0);
    if(rst.modechange) newmode();

    p=newpr();

    if(rst.vxtabi>MAXVX || rst.prtabi>MAXPR)
    {
        exception("dlist: vx/pr-tab corrupted!\n");
        return;
    }

    p->c[0]=rdpvx[vind[0]];
    p->c[1]=rdpvx[vind[1]];
    p->c[2]=rdpvx[vind[2]];
    p->wirecolor=rst.debugwirecolor;

    st2.gfx_tris++;
}

// assign new vertices to rdp vertices first..(first+num-1)
void rdp_newvtx(int first,int num)
{
    int i;
    if(st.dumpgfx)
    {
        logd("\nrdp_newvtx(base=%i,first=%i,num=%i) ",rst.vxtabi,first,num);
    }
    if(num+rst.vxtabi>=MAXVX)
    {
        error("rdp: loadvx too many vertices in frame (max %i)",MAXVX);
        rst.vxtabi=0;
    }

    num+=first;
    for(i=first;i<num;i++)
    {
        rdpvx[i]=rst.vxtab+rst.vxtabi;
        rst.vxtabi++;
    }
}

/****************************************************************************
/* Rendering State conversion - Combine
*/

int combmap[32][8]={ // X,Y,M,A, AlphaXYA, AlphaM
C_COMB  ,C_COMB  ,C_COMB  ,C_COMB  ,C_COMB  ,C_LODF  ,0,0,
C_TEX0  ,C_TEX0  ,C_TEX0  ,C_TEX0  ,C_TEX0  ,C_TEX0  ,0,0,
C_TEX1  ,C_TEX1  ,C_TEX1  ,C_TEX1  ,C_TEX1  ,C_TEX1  ,0,0,
C_PRIM  ,C_PRIM  ,C_PRIM  ,C_PRIM  ,C_PRIM  ,C_PRIM  ,0,0,
C_SHADE ,C_SHADE ,C_SHADE ,C_SHADE ,C_SHADE ,C_SHADE ,0,0,
C_ENV   ,C_ENV   ,C_ENV   ,C_ENV   ,C_ENV   ,C_ENV   ,0,0,
C_ONE   ,C_CENTER,C_SCALE ,C_DUNNO ,C_ONE   ,C_PLODF ,0,0,
C_NOISE ,C_K4    ,C_COMBA ,C_ZERO  ,C_ZERO  ,C_ZERO  ,0,0,
//
C_TEX0A ,C_TEX0A ,C_TEX0A ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_TEX1A ,C_TEX1A ,C_TEX1A ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_PRIA  ,C_PRIA  ,C_PRIA  ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_SHAA  ,C_SHAA  ,C_SHAA  ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_ENVA  ,C_ENVA  ,C_ENVA  ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_LODF  ,C_LODF  ,C_LODF  ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_PLODF ,C_PLODF ,C_PLODF ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0,
C_ZERO  ,C_ZERO  ,C_K5    ,C_DUNNO ,C_DUNNO ,C_DUNNO ,0,0};

int combtab[16][8]={ // x0/1,pos,num,map,c2,c1,-,-
0,20,0x0f,0,RDP_X,0,0,0,  //saC0
1,28,0x0f,1,RDP_Y,0,0,0,  //sbC0
0,15,0x1f,2,RDP_M,0,0,0,  // mC0
1,15,0x07,3,RDP_A,0,0,0,  // aC0
0,12,0x07,4,RDP_X,1,0,0,  //saA0
1,12,0x07,4,RDP_Y,1,0,0,  //sbA0
0, 9,0x07,5,RDP_M,1,0,0,  // mA0
1, 9,0x07,4,RDP_A,1,0,0,  // aA0
0, 5,0x0f,0,RDP_X,2,0,0,  //saC1
1,24,0x0f,1,RDP_Y,2,0,0,  //sbC1
0, 0,0x1f,2,RDP_M,2,0,0,  // mC1
1, 6,0x07,3,RDP_A,2,0,0,  // aC1
1,21,0x07,4,RDP_X,3,0,0,  //saA1
1, 3,0x07,4,RDP_Y,3,0,0,  //sbA1
1,18,0x07,5,RDP_M,3,0,0,  // mA1
1, 0,0x07,4,RDP_A,3,0,0}; // aA1

static Combine comfill;
static Combine comcopy;
static int     dump; // for com routines

char *dump_combinenm(char *buf0,int c)
{
    char *buf=buf0;
    *buf=0;
    if(c&C_MULCASE)
    {
        *buf++='(';
        strcpy(buf,cname[c&C_INDEX]); buf+=strlen(buf);
        *buf++='*';
        strcpy(buf,cname[(c>>8)&C_INDEX]); buf+=strlen(buf);
        *buf++=')';
    }
    else
    {
        strcpy(buf,cname[c&C_INDEX]); buf+=strlen(buf);
    }

    if(c&C_SPECIALCASE)
    {
        *buf++='!';
        *buf++='S';
    }
    *buf++=0;

    return(buf0);
}

char *dump_combineeq(char *buf0,int *cx,int style,int recurse)
{
    char *x,*y,*m,*a;
    char *buf=buf0;
    int   ri;
    static char tmp[16][80];

    if(recurse>0) ri=1; else ri=0;

    if(recurse>0 && cx[RDP_X]==C_COMB) x=dump_combineeq(tmp[ri+ 0],cx-8,STYLE_IGNORE,-1);
    else                               x=dump_combinenm(tmp[ri+ 2],cx[RDP_X]);
    if(recurse>0 && cx[RDP_Y]==C_COMB) y=dump_combineeq(tmp[ri+ 4],cx-8,STYLE_IGNORE,-1);
    else                               y=dump_combinenm(tmp[ri+ 6],cx[RDP_Y]);
    if(recurse>0 && cx[RDP_M]==C_COMB) m=dump_combineeq(tmp[ri+ 8],cx-8,STYLE_IGNORE,-1);
    else                               m=dump_combinenm(tmp[ri+10],cx[RDP_M]);
    if(recurse>0 && cx[RDP_A]==C_COMB) a=dump_combineeq(tmp[ri+12],cx-8,STYLE_IGNORE,-1);
    else                               a=dump_combinenm(tmp[ri+14],cx[RDP_A]);

    if(recurse==-1)
    {
        *buf++='[';
        *buf++=' ';
    }

    if(style==STYLE_IGNORE)
    {
        if(*x=='0' && *y=='0' && *m=='0') style=STYLE_CONST;
        else if(*y=='0' && *a=='0') style=STYLE_MUL;
        else if(*y=='0') style=STYLE_MULADD;
        else if(cx[RDP_Y]==cx[RDP_A]) style=STYLE_BLEND;
        else style=STYLE_FULL;
    }

    switch(style)
    {
    case STYLE_COMB:
    case STYLE_CONST:
        sprintf(buf,"%s",a);
        break;
    case STYLE_MUL:
        sprintf(buf,"%s*%s",x,m);
        break;
    case STYLE_MULADD:
        sprintf(buf,"%s*%s+%s",x,m,a);
        break;
    case STYLE_BLEND:
        sprintf(buf,"%s->%s,%s",y,x,m);
        break;
    default:
        strcpy(buf,"??:");
        buf+=3;
    case STYLE_FULL:
        sprintf(buf,"(%s-%s)*%s+%s",x,y,m,a);
        break;
    }

    if(recurse==-1)
    {
        buf+=strlen(buf);
        *buf++=' ';
        *buf++=']';
        *buf++=0;
    }

    return(buf0);
}

void dumpscombine(char *txt)
{
    int alp=0,alp0,alp1;
    char buf1[80];
    if(*txt=='*')
    {
        alp0=0;
        alp1=1;
        txt++;
    }
    else if(*txt=='!')
    {
        alp0=1;
        alp1=1;
        txt++;
    }
    else
    {
        alp0=0;
        alp1=0;
    }
    for(alp=alp0;alp<=alp1;alp++)
    {
        dump_combineeq(buf1,rst.s_combx[2+alp],rst.s_combinestyle[2+alp],1);
        logd("\n%s%c: %-38s",txt,alp?'A':'C',buf1);
        logd( "cs0=%-6s",stylename[rst.s_combinestyle[alp+0]]);
        logd(" cs1=%-6s",stylename[rst.s_combinestyle[alp+2]]);
        logd(" tx0=%i",rst.s_combinetex[alp+0]);
        logd(" tx1=%i",rst.s_combinetex[alp+2]);
        logd(" cyc=%i",rst.s_combinecycles);
    }
}

void dump_combineresult(void)
{
    int i;
    logd("\n+mode combine-^^^: texenable=%i ",COM.texenable);
    if(COM.passes>1 && COM.col[0][1]!=C_ONE)
    {
        logd(" Transparent-Multipass! ");
    }
    logd("ENV=%s ",colortext(rst.col[C_ENV]));
    logd("PRIA=%s ",colortext(rst.col[C_PRIA]));
    logd("PRIM=%s ",colortext(rst.col[C_PRIM]));
    logd("PLODF=%02X ",rst.col[C_PLODF][3]);
    logd("passes=%i: ",COM.passes);
    for(i=0;i<COM.passes;i++)
    {
        logd("\n+mode combine-^^%i: ",i);
        logd("Col:%05X/%05X,Com:%04X/%04X,Bl:%04X/%04X,Env:%04X,T%i ",
            COM.col[i][0],COM.col[i][1],
            COM.com[i][0],COM.com[i][1],
            COM.blend1[i],COM.blend2[i],
            COM.env,
            COM.txt[i]);
    }
}

void dump_combineeqs(int alp)
{
    char buf1[100];
    char buf2[100];
    dump_combineeq(buf1,rst.s_combx[0+alp],STYLE_FULL,0);
    dump_combineeq(buf2,rst.s_combx[2+alp],STYLE_FULL,0);
    logd("[ Cycle1: %s Cycle2: %s ]",buf1,buf2);
}

void dump_combine(dword x0,dword x1)
{
    int  cn=rst.s_cycles;
    char buf1[100];
    char buf2[100];

    if(cn==3)
    {
        logd("\n+mode combine-all: COPY");
        return;
    }
    if(cn==4)
    {
        logd("\n+mode combine-all: FILL");
        return;
    }

    if(0)
    {
        logd("\n+mode combine-col: ");
        dump_combineeq(buf1,rst.s_combx[0],STYLE_IGNORE,0);
        dump_combineeq(buf2,rst.s_combx[2],STYLE_IGNORE,0);
        if(cn==2) logd("%-38s Cycle2: %s",buf1,buf2);
        else      logd("%s",buf1);

        logd("\n+mode combine-alp: ");
        dump_combineeq(buf1,rst.s_combx[1],STYLE_IGNORE,0);
        dump_combineeq(buf2,rst.s_combx[3],STYLE_IGNORE,0);
        if(cn==2) logd("%-38s Cycle2: %s",buf1,buf2);
        else      logd("%s",buf1);
    }
    else
    {
        int alp;
        for(alp=0;alp<=1;alp++)
        {
            dump_combineeq(buf1,rst.s_combx[2+alp],rst.s_combinestyle[2+alp],1);
            logd("\n+mode combine-%s: %-38s",alp?"alp":"col",buf1);
            dump_combineeq(buf1,rst.s_combxbak[0+alp],0,0);
            dump_combineeq(buf2,rst.s_combxbak[2+alp],0,0);
            if(cn==2)  logd("<= Cycle1: %s Cycle2: %s",buf1,buf2);
            else       logd("<= Cycle1: %s",buf1);
        }
        dump_combineresult();
    }
}

int mode_findcache(dword x0,dword x1,dword o0,dword o1)
{
    int ci;
    for(ci=0;ci<rst.combcacheused;ci++)
    {
        if(rst.combcache[ci].combine0==x0 &&
           rst.combcache[ci].combine1==x1 &&
           rst.combcache[ci].other0  ==o0 &&
           rst.combcache[ci].other1  ==o1)
        {
            rst.s_c=&rst.combcache[ci];
            return(ci);
        }
    }
    return(-1);
}

int mode_newcache(dword x0,dword x1,dword o0,dword o1)
{
    int ci,a;

    ci=rst.combcacheused++;
    if(ci>=MAXCOMB) a=0;
    rst.combcacheused=ci+1;

    rst.s_c=&rst.combcache[ci];
    memset(rst.s_c,0,sizeof(Combine));
    COM.combine0=x0;
    COM.combine1=x1;
    COM.other0  =o0;
    COM.other1  =o1;

    return(ci);
}

// set combx[i] to passthrough
void com_setcomb(int i)
{
    rst.s_combinestyle[i]=STYLE_COMB;
    rst.s_combx[i][RDP_X]=0;
    rst.s_combx[i][RDP_Y]=0;
    rst.s_combx[i][RDP_M]=0;
    rst.s_combx[i][RDP_A]=C_COMB;
}

// set combx[i] to passthrough
void com_setzero(int i)
{
    rst.s_combinestyle[i]=STYLE_CONST;
    rst.s_combx[i][RDP_X]=0;
    rst.s_combx[i][RDP_Y]=0;
    rst.s_combx[i][RDP_M]=0;
    rst.s_combx[i][RDP_A]=0;
}

// set combx[i] to passthrough
void com_setcopy(int d,int s)
{
    rst.s_combinestyle[d]=rst.s_combinestyle[s];
    rst.s_combx[d][RDP_X]=rst.s_combx[s][RDP_X];
    rst.s_combx[d][RDP_Y]=rst.s_combx[s][RDP_Y];
    rst.s_combx[d][RDP_M]=rst.s_combx[s][RDP_M];
    rst.s_combx[d][RDP_A]=rst.s_combx[s][RDP_A];
}

// convert x0,x1 => rst.combine[]/combinebak[] array
void com_convertcombine(dword x0,dword x1)
{
    int c,i,i1,i2,a,v,x,pos,msk,map,c1,c2;
    for(c=0;c<2;c++)
    {
        i1=8*c;
        i2=i1+8;
        for(i=i1;i<i2;i++)
        {
            if (combtab[i][0]) x=x1; else x=x0;
            pos=combtab[i][1];
            msk=combtab[i][2];
            map=combtab[i][3];
            c2 =combtab[i][4];
            c1 =combtab[i][5];

            a=( x>>pos ) & msk;
            if(a>15) v=C_ZERO;
            else     v=combmap[a][map];

            rst.s_combx[c1][c2]=v;
            rst.s_combxbak[c1][c2]=v;
        }
    }
    rst.s_combinecycles=rst.s_cycles;
}

// calculate rst.combinestyle,rst.combinetex,rst.combinetexboth
void com_calculatestyles(void)
{
    int i,j,a,s;
    // determine styles
    for(i=0;i<rst.s_combinecycles*2;i++)
    {
        if(rst.s_combx[i][RDP_Y]!=C_ZERO)
        {
            if(rst.s_combx[i][RDP_Y]!=rst.s_combx[i][RDP_A])
                 s=STYLE_FULL;
            else s=STYLE_BLEND;
        }
        else
        {
            if(rst.s_combx[i][RDP_A]!=C_ZERO)
            {
                if(rst.s_combx[i][RDP_X]==C_ZERO &&
                   rst.s_combx[i][RDP_M]==C_ZERO )
                {
                    if(rst.s_combx[i][RDP_A]==C_COMB)
                         s=STYLE_COMB;
                    else s=STYLE_CONST;
                }
                else
                {
                    if(rst.s_combx[i][RDP_M]==C_ONE) s=STYLE_ADD;
                    else s=STYLE_MULADD;
                }
            }
            else if(rst.s_combx[i][RDP_X]==C_ZERO && rst.s_combx[i][RDP_M]==C_ZERO)
            {
                s=STYLE_CONST;
            }
            else
            {
                s=STYLE_MUL;
            }
        }
        rst.s_combinestyle[i]=s;
    }
    // clear first cycle if no C_COMB in second cycle
    if(rst.s_combinecycles==2)
    {
        for(i=2;i<4;i++)
        {
            for(j=0;j<4;j++)
            {
                if(rst.s_combx[i][j]==C_COMB) break;
            }
            if(j==4)
            {
                if(dump) logd(" #setzero%i ",i-2);
                com_setzero(i-2);
            }
            if(rst.s_combx[i][RDP_Y]==C_COMB) break;
            if(rst.s_combx[i][RDP_M]==C_COMB) break;
            if(rst.s_combx[i][RDP_A]==C_COMB) break;
        }
    }
    // determine texture presense
    rst.s_combinetexboth=0;
    for(i=0;i<4;i++)
    {
        s=0;
        for(j=0;j<4;j++)
        {
            a=rst.s_combx[i][j];
            if(a==C_TEX0 || a==C_TEX0A) s|=1;
            if(a==C_TEX1 || a==C_TEX1A) s|=2;
        }
        rst.s_combinetex[i]=s;
        rst.s_combinetexboth|=s;
    }
}

// reorder noncolors to X parameter in muls/adds
// also remove trivial *1
void com_reorder(void)
{
    int i,s,sd;
    for(i=0;i<rst.s_combinecycles*2;i++)
    {
        s=rst.s_combinestyle[i];
        if(s==STYLE_MUL || s==STYLE_MULADD)
        { // swap texture to X in mul case
            if(rst.s_combx[i][RDP_X]<C_TEX0)
            {
                SWAP(rst.s_combx[i][RDP_X],rst.s_combx[i][RDP_M]);
            }
            // check for *1, remove it
            if(rst.s_combx[i][RDP_M]==C_ONE && s==STYLE_MUL)
            {
                rst.s_combinestyle[i]=STYLE_CONST;
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_X];
                rst.s_combx[i][RDP_X]=C_ZERO;
                rst.s_combx[i][RDP_Y]=C_ZERO;
                rst.s_combx[i][RDP_M]=C_ZERO;
            }
        }
        else if(s==STYLE_ADD)
        { // swap texture to X in add case
            if(rst.s_combx[i][RDP_X]<C_TEX0)
            {
                SWAP(rst.s_combx[i][RDP_X],rst.s_combx[i][RDP_A]);
            }
        }
    }
}

// convert PRIM to PRIM*SHADE (flatmode)
void com_flat(void)
{
    int i,j;
    for(i=0;i<rst.s_combinecycles*2;i++)
    {
        for(j=0;j<4;j++)
        {
            if(rst.s_combx[i][j]==C_PRIM)
            {
                rst.s_combx[i][j]=C_MUL(C_SHADE,C_PRIM);
            }
        }
    }
}

// remove all states with (X-Y)*LOD+A, convert to A
void com_removelod(void)
{
    int i;
    for(i=0;i<rst.s_combinecycles*2;i++)
    {
        /* // test to remove a zelda specific case
        if((rst.s_combx[i][RDP_M]==C_SHADE ||
           rst.s_combx[i][RDP_M]==C_PRIM) && i>=2)
        {
            com_setcomb(i);
            continue;
        }
        */
        // blend(TEX,TEX,lod)
        if(rst.s_combx[i][RDP_M]==C_LODF || rst.s_combx[i][RDP_M]==C_PLODF)
        {
            rst.s_combinestyle[i]=STYLE_CONST;
            if(rst.s_combx[i][RDP_X]==C_TEX0) rst.s_combx[i][RDP_A]=C_TEX0;
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #LodRemove%i ",i);
            continue;
        }
        if(rst.s_combinestyle[i]==STYLE_ADD &&
           rst.s_combx[i][RDP_X]==C_PLODF &&
           rst.s_combx[i][RDP_M]==C_ONE)
        {
            rst.s_combinestyle[i]=STYLE_CONST;
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #Lod*1-Remove%i ",i);
            continue;
        }
        if(rst.s_combinestyle[i]==STYLE_MULADD &&
           rst.s_combx[i][RDP_X]==C_PLODF &&
           rst.s_combx[i][RDP_M]==C_ONE)
        {
            rst.s_combinestyle[i]=STYLE_CONST;
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #MulAddPLODF%i ",i);
            continue;
        }
    }
}

void com_removemuladd(void)
{
    int i;
    for(i=0;i<rst.s_combinecycles*2;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_MULADD)
        {
            rst.s_combinestyle[i]=STYLE_MUL;
            rst.s_combx[i][RDP_A]=0;
            if(dump) logd(" #MulAddRemove ",i);
        }
    }
}

// remove first cycle [tex0->tex1,const] if dual cycle mode
void com_removetextex2(void)
{
    int i,a;
    if(rst.s_combinecycles<2) return;
    for(i=0;i<2;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_BLEND &&
           rst.s_combinestyle[i+2]!=STYLE_CONST &&
//           rst.s_combinestyle[i+2]!=STYLE_MUL &&
           !ISCOLOR(rst.s_combx[i][RDP_X]) &&
           !ISCOLOR(rst.s_combx[i][RDP_Y]) &&
            ISCOLOR(rst.s_combx[i][RDP_M]) &&
            rst.s_combx[i][RDP_M]!=C_SHADE &&
            rst.s_combx[i][RDP_M]!=C_PRIM )
        {
            // Tex->Tex,Color - color not SHADE or PRIM
            if(i==1) a=4*rst.col[rst.s_combx[i][RDP_M]][3];
            else
            {
                a=rst.col[rst.s_combx[i][RDP_M]][0]
                 +rst.col[rst.s_combx[i][RDP_M]][1]
                 +rst.col[rst.s_combx[i][RDP_M]][2]
                 +rst.col[rst.s_combx[i][RDP_M]][3];
            }
            /*
            if(rst.s_combx[i][RDP_X]==C_TEX0 ||
               rst.s_combx[i][RDP_Y]==C_TEX0)
            {
                rst.s_combx[i][RDP_A]=C_TEX0;
            }
            else
            */
            if(a<0x10*4)
            {
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_Y];
                rst.s_combx[i][RDP_X]=0;
                rst.s_combx[i][RDP_Y]=0;
                rst.s_combx[i][RDP_M]=0;
                rst.s_combinestyle[i]=STYLE_CONST;
                if(dump) logd(" #Simplify[TexTex]%i ",i);
            }
            else if(a>0xf0*4)
            {
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_X];
                rst.s_combx[i][RDP_X]=0;
                rst.s_combx[i][RDP_Y]=0;
                rst.s_combx[i][RDP_M]=0;
                rst.s_combinestyle[i]=STYLE_CONST;
                if(dump) logd(" #Simplify[TexTex]%i ",i);
            }
        }
        if(rst.s_combinestyle[i]==STYLE_FULL &&
           ( rst.s_combx[i][RDP_A]==C_TEX0 ||
             rst.s_combx[i][RDP_A]==C_TEX1 ||
             rst.s_combx[i][RDP_A]==C_TEX0A ||
             rst.s_combx[i][RDP_A]==C_TEX1A ) )
        {
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            rst.s_combinestyle[i]=STYLE_CONST;

            if(dump) logd(" #Simplify[Full]%i ",i);
        }
    }
}

int avgcolor(int c,int i)
{
    int a;
    if(i&1) a=rst.col[c][3];
    else
    {
        a=rst.col[c][0]
         +rst.col[c][1]
         +rst.col[c][2]
         +rst.col[c][3];
        a>>=2;
    }
    return(a);
}

// remove blend where blender is PRIM/ENV and it's fully to one way
// remove also multiplys where multiplier is 1 or 0
void com_removefullblend(void)
{
    int i,a;
    if(rst.s_cycles==2) for(i=0;i<4;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_MUL &&
           ISCOLOR(rst.s_combx[i][RDP_M]) &&
           rst.s_combx[i][RDP_M]!=C_SHADE &&
           rst.s_combx[i][RDP_M]!=C_SHAA &&
           rst.s_combx[i][RDP_M]!=C_PRIM )   // prim often changes
        {
            a=avgcolor(rst.s_combx[i][RDP_M],i&1);
            if(a==255)
            {
                rst.s_combinestyle[i]=STYLE_CONST;
                rst.s_combx[i][RDP_M]=0;
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_X];
                rst.s_combx[i][RDP_X]=0;
                rst.s_combx[i][RDP_Y]=0;
                if(dump) logd(" #Simplify[*FF]%i ",i);
            }
            else if(a==0)
            {
                rst.s_combinestyle[i]=STYLE_CONST;
                rst.s_combx[i][RDP_M]=0;
                rst.s_combx[i][RDP_A]=C_ZERO;
                rst.s_combx[i][RDP_X]=0;
                rst.s_combx[i][RDP_Y]=0;
                if(dump) logd(" #Simplify[*00]%i ",i);
            }
        }
    }
    for(i=0;i<4;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_BLEND &&
           (!ISCOLOR(rst.s_combx[i][RDP_X]) ||
            !ISCOLOR(rst.s_combx[i][RDP_Y]) ) &&
            ISCOLOR(rst.s_combx[i][RDP_M]) &&
            rst.s_combx[i][RDP_M]!=C_SHADE &&
            rst.s_combx[i][RDP_M]!=C_SHAA )
        {
            a=avgcolor(rst.s_combx[i][RDP_M],i&1);
            if(a==0)
            {
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_Y];
                if(dump) logd(" #Simplify[<-100%%]%i ",i);
            }
            else if(a==255)
            {
                rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_X];
                if(dump) logd(" #Simplify[->100%%]%i ",i);
            }
            else continue;

            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            rst.s_combinestyle[i]=STYLE_CONST;
        }
    }
}

// combine second cycle *COLOR to first cycle colors if possible (set second to passthrough)
void com_combinemul2(void)
{
    int i;
    if(rst.s_combinecycles<2) return;
    for(i=0;i<2;i++)
    {
        if(rst.s_combinestyle[i+2]==STYLE_MUL &&
           rst.s_combx[i+2][RDP_X]==C_COMB &&
           ISCOLOR(rst.s_combx[i+2][RDP_M]) )
        { // second cycle is *COLOR
            if(rst.s_combinestyle[i]==STYLE_MUL &&
               ISCOLOR(rst.s_combx[i][RDP_X]) )
            { // first is COLOR*?
                rst.s_combx[i][RDP_X]=C_MUL(rst.s_combx[i][RDP_X],
                                            rst.s_combx[i+2][RDP_M]);
            }
            else if(rst.s_combinestyle[i]==STYLE_MUL &&
               ISCOLOR(rst.s_combx[i][RDP_M]) )
            { // first is ?*COLOR
                rst.s_combx[i][RDP_M]=C_MUL(rst.s_combx[i][RDP_M],
                                            rst.s_combx[i+2][RDP_M]);
            }
            else if(rst.s_combinestyle[i]==STYLE_BLEND &&
               ISCOLOR(rst.s_combx[i][RDP_X]) &&
               ISCOLOR(rst.s_combx[i][RDP_Y]) &&
               !ISCOLOR(rst.s_combx[i][RDP_M]))
            { // first is COLOR->COLOR,?
                rst.s_combx[i][RDP_X]=C_MUL(rst.s_combx[i][RDP_X],
                                            rst.s_combx[i+2][RDP_M]);
                rst.s_combx[i][RDP_Y]=C_MUL(rst.s_combx[i][RDP_Y],
                                            rst.s_combx[i+2][RDP_M]);
                rst.s_combx[i][RDP_A]=C_MUL(rst.s_combx[i][RDP_A],
                                            rst.s_combx[i+2][RDP_M]);
            }
            else if(rst.s_combinestyle[i]==STYLE_BLEND &&
               !ISCOLOR(rst.s_combx[i][RDP_X]) &&
               !ISCOLOR(rst.s_combx[i][RDP_Y]) &&
               ISCOLOR(rst.s_combx[i][RDP_M]) )
            { // first is TEX?->TEX?,COLORf:
                rst.s_combx[i][RDP_M]=C_MUL(rst.s_combx[i][RDP_M],
                                            rst.s_combx[i+2][RDP_M])|C_SPECIALCASE;
                // special case handling for this
            }
            else if(rst.s_combinestyle[i]==STYLE_FULL &&
               ISCOLOR(rst.s_combx[i][RDP_X]) &&
               ISCOLOR(rst.s_combx[i][RDP_Y]) &&
               ISCOLOR(rst.s_combx[i][RDP_A]) )
            { // first is (COLOR-COLOR)*?+COLOR
                rst.s_combx[i][RDP_X]=C_MUL(rst.s_combx[i][RDP_X],
                                            rst.s_combx[i+2][RDP_M]);
                rst.s_combx[i][RDP_Y]=C_MUL(rst.s_combx[i][RDP_Y],
                                            rst.s_combx[i+2][RDP_M]);
                rst.s_combx[i][RDP_A]=C_MUL(rst.s_combx[i][RDP_A],
                                            rst.s_combx[i+2][RDP_M]);
            }
            else continue;

            com_setcomb(i+2);
            if(dump) logd(" #C_MUL ",i);
        }
    }
}

// combine single cycle COLOR*COLOR
void com_combinemul1(void)
{
    int i;
    for(i=0;i<2;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_MUL &&
           ISCOLOR(rst.s_combx[i][RDP_X]) &&
           ISCOLOR(rst.s_combx[i][RDP_M]) )
        { // second cycle is *COLOR
            rst.s_combx[i][RDP_A]=C_MUL(rst.s_combx[i][RDP_X],
                                        rst.s_combx[i][RDP_M]);
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #SimplifyA%i ",i);
        }
    }
}

// combine single 1*X or X*1
void com_combinemulone(void)
{
    int i;
    for(i=0;i<4;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_MUL &&
           rst.s_combx[i][RDP_X]==C_ONE)
        { // second cycle is *COLOR
            rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_M];
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #*1 ");
        }
        else if(rst.s_combinestyle[i]==STYLE_MUL &&
                rst.s_combx[i][RDP_M]==C_ONE)
        { // second cycle is *COLOR
            rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_X];
            rst.s_combx[i][RDP_X]=0;
            rst.s_combx[i][RDP_Y]=0;
            rst.s_combx[i][RDP_M]=0;
            if(dump) logd(" #*1 ");
        }
    }
}

// convert dual cycle to single cycle if possible (set second to passthrough)
void com_joincycles(void)
{
    int i,j;
    if(rst.s_combinecycles<2) return;
    for(i=0;i<2;i++)
    {
        if(rst.s_combinestyle[i]==STYLE_CONST)
        { // first cycle = const, replace COMB -> const
            for(j=0;j<4;j++)
            {
                if(rst.s_combx[i+2][j]==C_COMB)
                {
                    rst.s_combx[i+2][j]=rst.s_combx[i][RDP_A];
                }
            }
            // copy second cycle to first
            memcpy(rst.s_combx[i],rst.s_combx[i+2],4*4);
            com_setcomb(i+2);
        }
    }
}

int  com_issingle(int i)
{
    if(rst.s_combinecycles<2) return(1);
    if(rst.s_combinestyle[i+2]==STYLE_COMB) return(1);
    return(0);
}

void com_forcesingle(int i)
{
    // need to remove second cycle, for now let's just drop it
    com_setcomb(i+2);
    if(dump) logd(" #droppedC2! ");
}

void com_forcesimple(int i)
{
    // need to simplyfy, just leave A
    if(rst.s_combinestyle[i]==STYLE_BLEND &&
       rst.s_combx[i][RDP_M]&C_SPECIALCASE)
    {
        rst.s_combx[i][RDP_Y]=0;
        rst.s_combx[i][RDP_X]=rst.s_combx[i][RDP_A];
        rst.s_combx[i][RDP_M]=(rst.s_combx[i][RDP_M]>>8)&255;
        rst.s_combx[i][RDP_A]=0;
        rst.s_combinestyle[i]=STYLE_MUL;
        if(dump) logd(" #droppedBLENDS! ");
    }
    /*
    else if(rst.s_combinestyle[i]==STYLE_BLEND &&
            ISCOLOR(rst.s_combx[i][RDP_Y]) &&
            rst.s_combx[i][RDP_X]==C_TEX0)
    {
        rst.s_combx[i][RDP_X]=rst.s_combx[i][RDP_X];
        rst.s_combx[i][RDP_M]=rst.s_combx[i][RDP_M];
        rst.s_combx[i][RDP_A]=0;
        rst.s_combx[i][RDP_Y]=0;
        rst.s_combinestyle[i]=STYLE_MUL;
        if(dump) logd(" #droppedCOL->TXT! ");
    }
    */
    else
    {
        if(rst.s_combinestyle[i]==STYLE_FULL)
        {
            if((rst.s_combx[i][RDP_M]==C_TEX0 ||
                rst.s_combx[i][RDP_M]==C_TEX1) &&
                ISCOLOR(rst.s_combx[i][RDP_A]))
            {
               rst.s_combx[i][RDP_X]=rst.s_combx[i][RDP_M];
               rst.s_combx[i][RDP_Y]=0;
               rst.s_combx[i][RDP_M]=C_ONE;
               rst.s_combinestyle[i]=STYLE_ADD;
               if(dump) logd(" #droppedXYM+! ");
               return;
            }
        }

        if(rst.s_combx[i][RDP_M]==C_TEX0 ||
           rst.s_combx[i][RDP_M]==C_TEX1)
        {
           rst.s_combx[i][RDP_A]=rst.s_combx[i][RDP_M];
        }
        rst.s_combx[i][RDP_X]=0;
        rst.s_combx[i][RDP_Y]=0;
        rst.s_combx[i][RDP_M]=0;
        rst.s_combinestyle[i]=STYLE_CONST;
        if(dump) logd(" #droppedXYM! ");
    }
}

void com_unknown(int unknown)
{
    // no supported
    COM.passes=1;
    COM.col[0][0]=C_DUNNO;
    COM.com[0][0]=X_COLOR;
    COM.col[0][1]=C_ONE;
    COM.com[0][1]=X_COLOR;
    COM.txt[0]=0;
    COM.txt[1]=0;
    logd("\n+mode combine-???: u%i",unknown);
    if(unknown&2) logd(",A?");
    if(unknown&1) logd(",C?");

    if(showwire || showinfo || showtest)
    {
        // put a flashing color in these modes
        logd(" (used flashing)");
    }
    else
    {
        // come up with something
        if(rst.s_combinetexboth&1)
        {
            rst.col[0][0]=0;
            COM.com[0][0]=X_TEXTURE;
            COM.texenable=1;
            logd(" (used tex0)");
        }
        else
        {
            rst.col[0][0]=rst.s_combx[0][RDP_A];
            if(rst.col[0][0]>C_TEX0) rst.col[0][0]=C_ONE;
            COM.com[0][0]=X_COLOR;
            COM.texenable=0;
            logd(" (used color %s)",cname[rst.col[0][0]]);
        }
    }
}

// clear 3DFX mapping
void com_clearmapping(int a)
{
    int i;
    for(i=0;i<4;i++)
    {
        COM.col[i][a]=0;
        COM.com[i][a]=X_COLOR;
        if(a==0)
        {
            COM.txt[i]=0;
            COM.blend1[i]=rst.s_blend1;
            COM.blend2[i]=rst.s_blend2;
        }
    }
    if(a==0)
    {
        COM.env=0;
    }
    COM.passes=0;
}

void com_alpha1mapping(void)
{
    int i;
    for(i=0;i<4;i++)
    {
        COM.col[i][1]=C_ONE;
        COM.com[i][1]=X_COLOR;
    }
}

void com_copyalphan(int n)
{
    int i;
    for(i=1;i<n;i++)
    {
        COM.col[i][1]=COM.col[0][1];
        COM.com[i][1]=COM.com[0][1];
        // also override texture
        if(COM.com[0][1]!=X_ONE && COM.com[0][1]!=X_COLOR)
        {
            COM.txt[i]=COM.txt[0];
        }
    }
}

void com_debugcolorshade(int r,int g,int b)
{
    COM.passes=1;
    COM.col[0][0]=C_MUL(C_COMB,C_SHADE);
    COM.com[0][0]=X_COLOR;
    COM.col[0][1]=C_ONE;
    COM.com[0][1]=X_COLOR;
    COM.blend1[0]=X_ONE;
    COM.blend2[0]=X_ZERO;
    COM.txt[0]=0;
    setcolor(C_COMB,(r<<24)+(g<<16)+(b<<8)+255);
}

void com_debugcolor(int r,int g,int b)
{
    COM.passes=1;
    COM.col[0][0]=C_COMB;
    COM.com[0][0]=X_COLOR;
    COM.col[0][1]=C_ONE;
    COM.com[0][1]=X_COLOR;
    COM.blend1[0]=X_ONE;
    COM.blend2[0]=X_ZERO;
    COM.txt[0]=0;
    setcolor(C_COMB,(r<<24)+(g<<16)+(b<<8)+255);
}

void com_debugshade(void)
{
    COM.passes=1;
    COM.col[0][0]=C_SHADE;
    COM.com[0][0]=X_COLOR;
    COM.col[0][1]=C_ONE;
    COM.com[0][1]=X_COLOR;
    COM.blend1[0]=X_ONE;
    COM.blend2[0]=X_ZERO;
    COM.txt[0]=0;
    COM.env=1;
    COM.texenable=0;
}

void com_debugtexture(int t)
{
    COM.passes=1;
    COM.col[0][0]=C_COMB;
    COM.com[0][0]=X_TEXTURE;
    COM.col[0][1]=C_COMB;
    COM.com[0][1]=X_TEXTURE;
    COM.txt[0]=t;
    COM.blend1[0]=X_ONE;
    COM.blend2[0]=X_ZERO;
    COM.texenable=1<<t;
    setcolor(C_COMB,(64<<24)+(64<<16)+(64<<8)+255);
}

// find a single cycle mapping
int com_createmapping1(int a)
{
    int c=COM.passes;

    if(!rst.s_combinetex[a])
    { // no texture
        if(dump) logd(" #notxt(%i) ",rst.s_combinestyle[a]);
        switch(rst.s_combinestyle[a])
        {
        case STYLE_CONST:
            COM.col[c][a]=rst.s_combx[a][RDP_A];
            COM.com[c][a]=X_COLOR;
            c++;
            break;
        default:
            COM.col[c][a]=C_FULL; // special handling in mixcolors
            COM.com[c][a]=X_COLOR;
            c++;
            break;
        }
    }
    else if(rst.s_combinetex[a]==1 || rst.s_combinetex[a]==2)
    { // single cycle with one texture
        // texture is in X param (if not CONST in which case it's A)
        int tex,texa;
        if(dump) logd(" #1txt(%i) ",rst.s_combinestyle[a]);
        if(rst.s_combinetex[a]==1)
        { // TEX0
            COM.txt[c]=0;
            tex=C_TEX0;
            texa=C_TEX0A;
        }
        else
        { // TEX1
            COM.txt[c]=1;
            tex=C_TEX1;
            texa=C_TEX1A;
        }
        switch(rst.s_combinestyle[a])
        {
        case STYLE_BLEND:
            // color->TEX0,COLOR
            if(rst.s_combinestyle[a]==STYLE_BLEND &&
               ISCOLOR(rst.s_combx[a][RDP_A])     &&
               ISCOLOR(rst.s_combx[a][RDP_M]))
            {
                if(dump) logd(" #Color->TEX0,COLOR ");
                COM.col[c][a]=C_MUL(rst.s_combx[a][RDP_M],rst.s_combx[a][RDP_Y])|C_NEGATE;
                COM.com[c][a]=X_COLOR;
                c++;
                COM.col[c][a]=rst.s_combx[a][RDP_M];
                COM.com[c][a]=X_MUL;
                COM.blend1[c]=COM.blend1[c-1];
                COM.blend2[c]=X_ONE;
                c++;
                break;
            }
            // color->TEX0,TEX0A detect (opaque only)
            else if(rst.s_combinestyle[a]==STYLE_BLEND &&
               ISCOLOR(rst.s_combx[a][RDP_A])     &&
               rst.s_combx[a][RDP_M]==texa        )
            {
                if(dump) logd(" #Color->TEX0,TEXA ");
                COM.col[c][a]=rst.s_combx[a][RDP_A];
                COM.com[c][a]=X_TEXTUREBLEND;
                c++;
                break;
            }
            // color->color,TEX0
            else if(rst.s_combinestyle[a]==STYLE_BLEND &&
                    ISCOLOR(rst.s_combx[a][RDP_Y])     &&
                    ISCOLOR(rst.s_combx[a][RDP_X])     )
            {
                if(dump) logd(" #Color->Color");
                if(rst.s_combx[a][RDP_Y]==C_SHADE)
                {
                    COM.env=rst.s_combx[a][RDP_X];
                    COM.col[c][a]=rst.s_combx[a][RDP_Y];
                    COM.com[c][a]=X_TEXTUREENVCR;
                    if(COM.com[c][1]==X_TEXTUREENVCR) COM.com[c][1]=X_TEXTURE;

                }
                else
                {
                    COM.env=rst.s_combx[a][RDP_Y];
                    COM.col[c][a]=rst.s_combx[a][RDP_X];
                    COM.com[c][a]=X_TEXTUREENVC;
                    if(COM.com[c][1]==X_TEXTUREENVC) COM.com[c][1]=X_TEXTURE;
                }
                c++;
                break;
            }
            else return(1);
            break;
        case STYLE_CONST:
            COM.col[c][a]=0;
            COM.com[c][a]=X_TEXTURE;
            c++;
            break;
        case STYLE_MUL:
            COM.col[c][a]=rst.s_combx[a][RDP_M];
            COM.com[c][a]=X_MUL;
            c++;
            break;
        case STYLE_ADD:
            if(rst.s_combx[a][RDP_X]==C_TEX0 ||
               rst.s_combx[a][RDP_X]==C_TEX1)
            {
                COM.col[c][a]=rst.s_combx[a][RDP_A];
                COM.com[c][a]=X_ADD;
                c++;
            }
            else if(rst.s_combx[a][RDP_A]==C_TEX0 ||
                    rst.s_combx[a][RDP_A]==C_TEX1)
            {
                COM.col[c][a]=rst.s_combx[a][RDP_X];
                COM.com[c][a]=X_ADD;
                c++;
            }
            break;
        default:
            return(1);
        }
    }
    else if(rst.s_combinetex[a]==3)
    { // dual txt
        if(dump) logd(" #2txt(%i) ",rst.s_combinestyle[a]);
        if(rst.s_combinestyle[a]==STYLE_MUL &&
           !ISCOLOR(rst.s_combx[a][RDP_X]) &&
           !ISCOLOR(rst.s_combx[a][RDP_M]) )
        {
            // T1*T2
            COM.col[c+0][a]=C_ONE;
            COM.col[c+1][a]=C_ONE;
            COM.com[c+0][a]=X_TEXTURE;
            COM.com[c+1][a]=X_TEXTURE;
            COM.txt[c+0]=0;
            COM.txt[c+1]=1;
            COM.blend1[c+1]=X_OTHER;
            COM.blend2[c+1]=X_ZERO;
            c+=2;
        }
        else if(rst.s_combinestyle[a]==STYLE_BLEND &&
           ISCOLOR(rst.s_combx[a][RDP_M]) )
        {
            // [ T1->T2,C1 ]
            // pass1: T1*(1-C1)
            // pass2: T2*(  C1)
            // also handles case when C1=C_MUL(x,a) where *a was on cycle2 [C_SPECIALCASE]
            if(rst.s_combx[a][RDP_Y]==C_TEX0 &&
               rst.s_combx[a][RDP_X]==C_TEX1)
            {
                COM.txt[c+0]=0;
                COM.txt[c+1]=1;
            }
            else if(rst.s_combx[a][RDP_Y]==C_TEX1 &&
                    rst.s_combx[a][RDP_X]==C_TEX0)
            {
                COM.txt[c+0]=0;
                COM.txt[c+1]=1;
            }
            else return(1);

            if((rst.s_combx[a][RDP_M]&C_MULCASE) &&
              !(rst.s_combx[a][RDP_M]&C_SPECIALCASE))
            {
                logd(" #mulnotspecial! ");
                return(1);
            }

            COM.col[c+0][a]=rst.s_combx[a][RDP_M]|C_NEGATE;
            COM.col[c+1][a]=rst.s_combx[a][RDP_M]         ;
            COM.com[c+0][a]=X_MUL;
            COM.com[c+1][a]=X_MUL;
            COM.blend2[c+1]=X_ONE;
            COM.blend2[c+1]=X_ONE;

            c+=2;
        }
        else
        {
            return(1);
        }
    }

    COM.passes=c;
    return(0);
}

// find the second cycle mapping
int com_createmapping2(int a)
{
    int c=COM.passes;

    if(dump) logd(" #pass2(%i): ",a);

    if(!rst.s_combinetex[a])
    { // no texture
        if(dump) logd(" #notxt(%i) ",rst.s_combinestyle[a]);
        switch(rst.s_combinestyle[a])
        {
        case STYLE_MUL:
            COM.col[c][a&1]=rst.s_combx[a][RDP_M];
            //print("mul col=%i (%i) c=%i a=%i\n",rst.s_combx[a][RDP_M],COM.col[1][0],c,a);
            COM.com[c][a&1]=X_COLOR;
            COM.blend1[c]=X_OTHER;
            COM.blend2[c]=X_ZERO;
            c++;
            break;
        case STYLE_ADD:
            COM.col[c][a&1]=rst.s_combx[a][RDP_A];
            COM.com[c][a&1]=X_COLOR;
            COM.blend1[c]=X_ONE;
            COM.blend2[c]=X_ONE;
            c++;
            break;
        default:
            return(1);
        }
    }
    else if(rst.s_combinetex[a]==1 || rst.s_combinetex[a]==2)
    { // single cycle with one texture (highlight only supported)
        // texture is in X param (if not CONST in which case it's A)
        int tex,texa;
        if(dump) logd(" #1txt ");
        if(rst.s_combinetex[a]==1)
        { // TEX0
            COM.txt[c]=0;
            tex=C_TEX0;
            texa=C_TEX0A;
        }
        else
        { // TEX1
            COM.txt[c]=1;
            tex=C_TEX1;
            texa=C_TEX1A;
        }
        switch(rst.s_combinestyle[a])
        {
        case STYLE_BLEND:
            // COMB->ENV,TEX0
            if(rst.s_combinestyle[a]==STYLE_BLEND &&
               ISCOLOR(rst.s_combx[a][RDP_X])     &&
               rst.s_combx[a][RDP_Y]==C_COMB      &&
               ( rst.s_combx[a][RDP_M]==C_TEX0 ||
                 rst.s_combx[a][RDP_M]==C_TEX1 ) )
            {
                COM.col[c][0]=rst.s_combx[a][RDP_X];
                COM.com[c][0]=X_SUB;
                COM.blend1[c]=X_ONE;
                COM.blend2[c]=X_ONE;
                c++;
                break;
            }
            break;
        default:
            return(1);
        }
    }
    else if(rst.s_combinetex[a]==3)
    { // dual txt
        if(dump) logd(" #2txt-fail ");
        return(1);
    }

    COM.passes=c;
    return(0);
}

// try to create a mode for 3DFX based on N64 mode [final analyze]
// this is for single cycle modes
int com_createmapping(int a)
{
    int e;
    com_clearmapping(a);
    if(com_issingle(a))
    {
        if(dump) logd(" #1P%i ",a);
        e=com_createmapping1(a);
    }
    else
    {
        if(dump) logd(" #2P%i ",a);
        e =com_createmapping1(a);
        e+=com_createmapping2(a+2);
    }
    if(dump) logd(" #C%i ",COM.passes);
    return(e);
}

int combx_flames[16]={
C_TEX1,C_PRIM,C_PLODF,C_TEX0, C_TEX1,C_ONE ,C_PLODF,C_TEX0,
C_PRIM,C_ENV ,C_COMB ,C_ENV , C_COMB,0     ,C_PRIM ,0     };
// Cycle1: (TEX1-PRIM)*PLODF+TEX0 Cycle2: ENV->PRIM,COMB
// Cycle1: (TEX1-1)*PLODF+TEX0 Cycle2: COMB*PRIM

// modepatch
int com_patchedmodes(void)
{
    int c=0;
    if(!rst.dualtmu)
    {
        return(0);
    }

    // patch Zelda flames using Voodoo2 dual texture
    if(!memcmp(&rst.s_combx,&combx_flames,16*sizeof(int)))
    {
//        COM.env      =C_MUL(rst.s_combx[2][RDP_Y],C_ALP75);
//        COM.col[c][0]=C_MUL(rst.s_combx[2][RDP_X],C_ALP75);
        COM.env      =rst.s_combx[2][RDP_Y];
        COM.col[c][0]=rst.s_combx[2][RDP_X];
        COM.com[c][0]=X_TEXTUREENVC;
        COM.col[c][1]=rst.s_combx[3][RDP_M];
        COM.com[c][1]=X_MUL;
        COM.txt[0]   =0;
        COM.txt[1]   =1;
        COM.blend1[c]=X_ALPHA;
        COM.blend2[c]=X_INVOTHERALPHA;
        COM.dualtxt  =X_MUL; // X_MULADD

        COM.texenable=3;
        COM.passes=1;
        return(1);
    }
    return(0);
}

void com_new(dword x0,dword x1)
{
    int a,oldp,unknown=0;

    com_convertcombine(x0,x1);

    // clear second cycle if singlepass
    if(rst.s_combinecycles==1)
    {
        com_setcomb(2);
        com_setcomb(3);
    }

    com_calculatestyles();
    rst.s_combinetexbothbak=rst.s_combinetexboth;

    if(dump) dumpscombine("*+mode combine-00");

    if(rst.flat) com_flat();

    if(!com_patchedmodes())
    {
        // use generic mode compiler

        com_reorder();
        com_calculatestyles();

        com_removefullblend();
        com_calculatestyles();

        if(dump) dumpscombine("*+mode combine-01");

        com_removelod(); // destructive!
        com_calculatestyles();

        com_removetextex2(); // destructive!
        com_calculatestyles();

        com_removemuladd(); // destructive!
        com_calculatestyles();

        if(dump) dumpscombine("*+mode combine-02");

        com_combinemul2();
        com_calculatestyles();
        com_combinemul1();
        com_calculatestyles();
        com_combinemulone();
        com_calculatestyles();

        if(dump) dumpscombine("*+mode combine-03");

        com_joincycles();
        com_calculatestyles();

        if(dump) dumpscombine("*+mode combine-04");

        com_combinemul2();
        com_calculatestyles();
        com_combinemul1();
        com_calculatestyles();
        com_combinemulone();
        com_calculatestyles();

        if(dump) dumpscombine("+mode combine-91");

        // work on the color
        a=com_createmapping(0);
        if(a)
        {
            logd(" #fail1?? ");
            com_forcesingle(0);
            com_calculatestyles();
            if(dump) dumpscombine("+mode combine-92");
            a=com_createmapping(0);
        }
        if(a)
        {
            logd(" #fail2?? ");
            com_forcesimple(0);
            com_calculatestyles();
            if(dump) dumpscombine("+mode combine-93");
            a=com_createmapping(0);
        }
        if(a)
        {
            logd(" #fail ");
            unknown|=1;
        }
        logd(" p=%i ",COM.passes);

        if(dump) dumpscombine("+mode combine-99");

        if(dump) dumpscombine("!+mode combine-91");

        // work on alpha
        oldp=COM.passes; // save passes
        {
            if(!com_issingle(1))
            {
                com_forcesingle(1); // multicycle won't work
                com_calculatestyles();
                if(dump) logd(" #failA1?? ");
            }
            a=com_createmapping(1);
            if(a)
            {
                if(dump) logd(" #failA2?? ");
                com_forcesimple(1);
                com_calculatestyles();
                if(dump) dumpscombine("!+mode combine-92");
                a=com_createmapping(1);
            }
            if(a)
            {
                if(dump) logd(" #failA3?? ");
                com_alpha1mapping();
            }
        }
        logd(" p=%i ",COM.passes);
        COM.passes=oldp; // restore passes
        if(COM.passes>1)
        { // copy alpha pass 1->2
            com_copyalphan(COM.passes);
        }

        if(dump) dumpscombine("!+mode combine-99");
    //    if(dump) dump_combineresult();
    }

    COM.texenable=rst.s_combinetexboth;

    if(unknown) com_unknown(unknown);
}

void com_settest(void)
{
    if(showtest==0)
    {
        // normal
    }
    else if(showtest==1)
    {
        // show txt0
        if(rst.s_combinetexbothbak&1)
        {
            com_debugtexture(0);
        }
        else
        {
            com_debugcolor(64,64,64);
        }
    }
    else if(showtest==2)
    {
        // show txt1
        if(rst.s_combinetexbothbak&2)
        {
            com_debugtexture(1);
        }
        else
        {
            com_debugcolor(64,64,64);
        }
    }
    else if(showtest==3)
    {
        // show passcount
        if(COM.passes==0) com_debugcolorshade(64,64,64);
        if(COM.passes==1) com_debugcolorshade(0,0,192);
        if(COM.passes==2) com_debugcolorshade(0,128,128);
        if(COM.passes> 2) com_debugcolorshade(0,128,0);
    }
}

void change_combine(dword x0,dword x1,dword chg0,dword chg1)
{
    int ci;

    dump=st.dumpgfx;

    if(dump) logd("\n+mode combine----: %i cycles",rst.s_cycles);

    if(rst.s_cycles==4)
    { // fill
        if(dump) logd(" fill ");
        rst.s_c=&comfill;
        COM.texenable=0;
        return;
    }
    if(rst.s_cycles==3)
    { // copy
        if(dump) logd(" copy ");
        rst.s_c=&comcopy;
        COM.texenable=1;
        return;
    }

    // combinemode (caching)
    ci=mode_findcache(rst.combine0[0],rst.combine1[0],rst.other0[0],rst.other1[0]);
    if(ci<0)
    {
        ci=mode_newcache(rst.combine0[0],rst.combine1[0],rst.other0[0],rst.other1[0]);
        if(dump) logd(" combcachemiss ");
    }
    else
    {
        if(dump) logd(" combcachehit(%i) ",ci);
    }

    com_new(x0,x1);

    if(showtest) com_settest();

    /*
    if(rst.other0[0]==0xef08ac10 && rst.other1[0]==0x0f0a4000)
    {
        COM.passes=0;
    }
    */
}

void com_init(void)
{
    static int flip=0;
    //
    rst.s_c=&rst.combdummy;
    // fill
    comfill.passes=1;
    comfill.com[0][0]=X_COLOR;
    comfill.com[0][1]=X_COLOR;
    comfill.col[0][0]=C_FILL;
    comfill.col[0][1]=C_FILL;
    comfill.txt[0]=0;
    // copy
    comcopy.passes=1;
    comcopy.com[0][0]=X_TEXTURE;
    comcopy.com[0][1]=X_TEXTURE;
    comcopy.col[0][0]=0;
    comcopy.col[0][1]=0;
    comcopy.txt[0]=0;
    // setup static colors
    setcolor(C_ZERO ,0);
    setcolor(C_ONE  ,0xffffffff);
    setcolor(C_DUNNO,0xff00ffff+(flip<<24));
    setcolor(C_ALP50,0xffffff7f);
    setcolor(C_COL50,0x7f7f7fff);
    setcolor(C_ALP75,0xffffffaf);
    setcolor(C_ALL50,0x7f7f7f7f);
    setcolor(C_ALL25,0x3f3f3f3f);
    // clear any combine modes
    com_setzero(0);
    com_setzero(1);
    com_setzero(2);
    com_setzero(3);
    com_calculatestyles();
}

/****************************************************************************
/* Rendering State conversion - Other
*/

void dump_other(dword o0,dword o1)
{ // doesn't dump blend or redermode
    int a,c,cn,line;
    dword x;

    for(line=0;line<6;line++)
    {
        if(line==0)
        {
            logd("\n+mode other/misc1:");

            x=o0;

            a=(x>>4)&3;
            switch(a)
            {
            case 0: logd(" adither(pattern)"); break;
            case 1: logd(" adither(notpat)"); break;
            case 2: logd(" adither(noise)"); break;
            case 3: logd(" adither(disable)"); break;
            }

            a=(x>>6)&3;
            switch(a)
            {
            case 0: logd(" cdither(magicsq)"); break;
            case 1: logd(" cdither(bayer)"); break;
            case 2: logd(" cdither(noise)"); break;
            case 3: logd(" cdither(disable)"); break;
            }

            a=(x>>8)&1;
            switch(a)
            {
            case 0: logd(" combkey(0)"); break;
            case 1: logd(" combkey(1)"); break;
            }

            a=(x>>9)&7;
            switch(a)
            {
            case 0: logd(" tconv(conv)"); break;
            case 5: logd(" tconv(filtconv)"); break;
            case 6: logd(" tconv(filt)"); break;
            default: logd(" tconv(%i)",a); break;
            }
        }
        if(line==1)
        {
            logd("\n+mode other/misc2:");

            x=o0;

            a=(x>>12)&7;
            switch(a)
            {
            case 0: logd(" tfilt(point)"); break;
            case 2: logd(" tfilt(bilerp)"); break;
            case 3: logd(" tfilt(aver)"); break;
            default: logd(" tfilt(%i)",a); break;
            }

            a=(x>>14)&3;
            switch(a)
            {
            case 0: logd(" tlut(none)"); break;
            case 2: logd(" tlut(rgba16)"); break;
            case 3: logd(" tlut(ia16)"); break;
            default: logd(" tlut(%i)",a); break;
            }

            a=(x>>16)&1;
            switch(a)
            {
            case 0: logd(" tlod(tile)"); break;
            case 1: logd(" tlod(lod)"); break;
            }

            a=(x>>17)&3;
            switch(a)
            {
            case 0: logd(" tdet(clamp)"); break;
            case 1: logd(" tdet(sharpen)"); break;
            case 2: logd(" tdet(detail)"); break;
            default: logd(" tdet(%i)",a); break;
            }

            a=(x>>19)&1;
            logd(" tpersp(%i)",a);
        }
        if(line==2)
        {
            logd("\n+mode other/misc3:");

            x=o0;

            a=(x>>20)&3;
            switch(a)
            {
            case 0: logd(" cycle(1c)"); break;
            case 1: logd(" cycle(2c)"); break;
            case 2: logd(" cycle(copy)"); break;
            case 3: logd(" cycle(fill)"); break;
            }
            //rst.cycles=a;

            a=(x>>23)&1;
            switch(a)
            {
            case 0: logd(" pip(Npr)"); break;
            case 1: logd(" pip(1pr)"); break;
            }

            x=o1;

            a=(x>>0)&3;
            switch(a)
            {
            case 0: logd(" acmp(none)"); break;
            case 1: logd(" acmp(thresh)"); break;
            case 2: logd(" acmp(2)"); break;
            case 3: logd(" acmp(dither)"); break;
            }

            a=(x>>2)&1;
            switch(a)
            {
            case 0: logd(" zsrc(pixel)"); break;
            case 1: logd(" zsrc(prim)"); break;
            }
        }
        if(line==3)
        {
            logd("\n+mode other/rend1:");

            x=o1&(8191<<3);

            a=(x&0x08)?1:0;
            logd(" aa(%i)",a);

            a=(x&0x10)?1:0;
            logd(" zcmp(%i)",a);
            //rst.zcmp=a;

            a=(x&0x20)?1:0;
            logd(" zupd(%i)",a);
            //rst.zupd=a;

            a=(x&0x40)?1:0;
            logd(" crd(%i)",a);

            a=(x&0xc00);
            switch(a)
            {
            case 0x000: logd(" zmode(opa)"); break;
            case 0x400: logd(" zmode(inter)"); break;
            case 0x800: logd(" zmode(xlu)"); break;
            case 0xc00: logd(" zmode(dec)"); break;
            }
            //rst.zmode=a>>10;
        }
        if(line==4)
        {
            logd("\n+mode other/rend2:");

            x=o1&(8191<<3);

            a=(x&0x4000)?1:0;
            logd(" fbl(%i)",a);
            //rst.forcebl=a;

            a=(x&0x80)?1:0;
            logd(" clrC(%i)",a);

            a=(x&0x300);
            switch(a)
            {
            case 0x000: logd(" cvg(dst_clamp)"); break;
            case 0x100: logd(" cvg(dst_wrap)"); break;
            case 0x200: logd(" cvg(dst_full)"); break;
            case 0x300: logd(" cvg(dst_save)"); break;
            }

            a=(x&0x1000)?1:0;
            logd(" cvg*(%i)",a);

            a=(x&0x2000)?1:0;
            logd(" atst(%i)",a);
            //rst.alphatest=a;
        }
        if(line==5)
        {
            int a0;

            logd("\n+mode other/blend:");

            x=o1>>16;

            x&=0xffff;
            logd(" %04X",x);

            if(rst.s_cycles==1) x>>=2;
            x&=0x3333;
            logd(" %04X ",x);

            if(x==0x0011)
            {
                logd("NORMAL");
            }
            else if(x==0x0302)
            {
                logd("OVERWR");
            }
            else if(x==0x0010)
            {
                logd("ALPHA ");
            }
            else if(x==0x1310)
            {
                logd("DARKEN");
            }
            else
            {
                logd("???   ");
            }

            x=o0;

            cn=rst.s_cycles;
            if(cn>2) cn=1;
            for(c=0;c<2;c++)
            {
                if(c==0) x=(o1>>18);
                else     x=(o1>>16);
                x&=0x3333;

                logd("  c%i:%04X ",c,x);

                if(c==1 && x==a0)
                {
                    logd("Same ");
                    continue;
                }
                a0=x;

                a=(x>>12)&3;
                switch(a)
                {
                case 0: logd("CIN"); break;
                case 1: logd("CMEM"); break;
                case 2: logd("CBL"); break;
                case 3: logd("CFOG"); break;
                }
                logd("*");
                a=(x>>8)&3;
                switch(a)
                {
                case 0: logd("AIN"); break;
                case 1: logd("FOGSH"); break;
                case 2: logd("FOGPR"); break;
                case 3: logd("0"); break;
                }

                logd("+");

                a=(x>>4)&3;
                switch(a)
                {
                case 0: logd("CIN"); break;
                case 1: logd("CMEM"); break;
                case 2: logd("CBL"); break;
                case 3: logd("?3"); break;
                }
                logd("*");
                a=(x>>0)&3;
                switch(a)
                {
                case 0: logd("(1-AIN)"); break;
                case 1: logd("AMEM"); break;
                case 2: logd("1"); break;
                case 3: logd("0"); break;
                }
            }
        }
    }
}

void change_other(dword x0,dword x1,dword c0,dword c1)
{
    if(FIELD(c0,20,3))
    {
        rst.s_cycles=FIELD(x0,20,3)+1;
        // cyclecount affects most settings, force changes
        rst.modechange=2;
        c1=c0=0xffffffff;
    }

    if(c0&0x7000)
    {
        rst.s_txtfilt=(x0>>12)&7;
    }
    if(c1&0xc00)
    {
        rst.s_zmode=(x1>>10)&3; // opa,inter,xlu,dec
    }
    if(c1&0x1800)
    {
        rst.s_cvgmode=(x1&0x1800)>>11;
    }
    if(c0&0xC000)
    {
        rst.s_tluttype=(x0>>14)&3;
    }
    if(c1&4)
    {
        rst.s_zsrc=(x1>>2)&1;
    }
    if(c1&0x30)
    {
        if(rst.s_cycles>2)
        {
            rst.s_zcmp=0;
            rst.s_zupd=0;
        }
        else
        {
            rst.s_zcmp=(x1&0x10)?1:0;
            rst.s_zupd=(x1&0x20)?1:0;
        }
        rst.s_noz=(!rst.s_zupd && !rst.s_zcmp);
    }
    if(c1&(0x1003))
    {
        rst.s_alphatst=x1&3;
        if((x1&0x1000)) rst.s_alphatst=4;
        //if(rst.s_alphatst && rst.s_cycles>2) rst.s_alphatst=4;
    }
    if(c1&0x4000)
    {
        rst.s_forcebl =(x1&0x4000)?1:0;
    }
    if(c1&0xffff0000)
    {
        dword x;
        int b1,b2;

        x=x1>>16;

        #if 1

        if(rst.s_cycles==1) x>>=2;
        x&=0x3333;

        rst.s_blendbits=x;

        if(!x || x==0x0302)
        {
            b1=X_ONE;
            b2=X_ZERO;
        }
        else if(x==0x0011)
        {
            b1=X_ONE;
            b2=X_ZERO;
        }
        else if(x==0x0010)
        {
            b1=X_ALPHA;
            b2=X_INVOTHERALPHA;
        }
        else if(x==0x1310)
        {
            b1=X_ZERO;
            b2=X_ONE; //X_INVOTHERALPHA;
        }
        else
        {
            b1=X_ONE;
            b2=X_ZERO;
        }

        #else

        if(rst.s_cycles==2)
        { // use second cycle code
            x<<=2;
        }
        x&=0xcccc;

        if(x==0x0044)                         a=RDP_BLEND_AA_OPAQUE;
        else if(x==0x0050)                    a=RDP_BLEND_ALPHAMIX;
        else if(x==0x0040)                    a=RDP_BLEND_ALPHAMIX2;
        else if(x==0x4c40)                    a=RDP_BLEND_DARKEN;
        else if(x==0x0055)                    a=RDP_BLEND_MULALPHA;
        else if(x==0x0C08 || x==0x080C || !x) a=RDP_BLEND_NORMAL;
        else                                  a=RDP_BLEND_DUNNO;

        switch(a)
        {
        case RDP_BLEND_MULALPHA:
            b1=X_ALPHA;
            b2=X_ZERO;
            break;
        case RDP_BLEND_NORMAL:
        case RDP_BLEND_AA_OPAQUE:
            b1=X_ONE;
            b2=X_ZERO;
            break;
        case RDP_BLEND_ALPHAMIX2:
            b1=X_ALPHA;
            b2=X_INVOTHERALPHA;
            break;
        case RDP_BLEND_ALPHAMIX:
            b1=X_ALPHA;
            b2=X_INVOTHERALPHA;
            break;
        case RDP_BLEND_DARKEN:
            b1=X_ZERO;
            b2=X_INVOTHERALPHA;
            break;
        default:
            b1=b2=X_ONE;
            break;
        }

        #endif

        rst.s_blend1=b1;
        rst.s_blend2=b2;
        rst.modechange=2; // force combinechange
    }
}

int findwirecolor(dword o0,dword o1,dword c0,dword c1)
{
    dword x;

    if(1)
    {
        x =rst.other0[0];
        x=(x<<3)|(x>>29);
        x^=rst.other1[0];
        x=(x<<3)|(x>>29);
        x^=rst.combine0[0];
        x=(x<<3)|(x>>29);
        x^=rst.combine1[0];
        x=(x<<3)|(x>>29);
        x=(x%14)+1; // 1..14
        return(x);
    }

    return 7;
}

void newmode(void) // setmode
{
    dword m0,m1;    // mode bits    (set by IFCHANGED)
    dword m0c,m1c;  // bits changed (set by IFCHANGED)

    if(st.dumpgfx)
    {
        logd("\n\n+newmode() flush %i prims",rst.prtabi);
    }

    flushprims();

    rst.flat=rst.setflat;
    rst.texturetile=rst.nexttexturetile;

    if(rst.firstmodechange)
    {
        rst.modechange=2;
        rst.firstmodechange=0;
    }

    if(st.dumpgfx)
    {
        logd("\n+mode change %i data: other %08X %08X combine %08X %08X",
            rst.modechange,
            rst.other0[0],rst.other1[0],
            rst.combine0[0],rst.combine1[0]);
    }

    setintensityfromalpha(C_PRIA,C_PRIM);
    setintensityfromalpha(C_ENVA,C_ENV);

    // othermode
    m0=rst.other0[0];
    m1=rst.other1[0];
    m0c=m0^rst.other0[1];
    m1c=m1^rst.other1[1];
    if(rst.modechange>1) m0c=m1c=0xffffffff;
    if(m0c|m1c)
    {
        rst.other0[1]=m0;
        rst.other1[1]=m1;
        change_other(m0,m1,m0c,m1c);
        if(st.dumpgfx) dump_other(m0,m1);
    }

    m0=rst.combine0[0];
    m1=rst.combine1[0];
    m0c=m0^rst.combine0[1];
    m1c=m1^rst.combine1[1];
    if(rst.modechange>1) m0c=m1c=0xffffffff;
    if(m0c|m1c)
    {
        rst.combine0[1]=m0;
        rst.combine1[1]=m1;
        change_combine(m0,m1,m0c,m1c);
        if(st.dumpgfx) dump_combine(m0,m1);
    }

    // prepare textures
    if(rst.txtchange)
    {
        if(COM.texenable&2) txt_prepare(1);
        if(COM.texenable&1) txt_prepare(0);
        rst.txtchange=0;
    }

    if(showinfo || showwire || st.dumpgfx)
    { // mode not supported, make wires
        rst.debugwirecolor=findwirecolor(rst.other0[0],rst.other1[0],rst.combine0[0],rst.combine1[0]);
        logd("\n+wirecolor %i",rst.debugwirecolor);
        if(!rst.debugwirecolor) rst.debugwirecolor=2;
    }
    else
    {
        rst.debugwirecolor=0;
    }

    rst.modechange=0;
}

/****************************************************************************
/* Frame init/deinit
*/

static void clear(int color)
{
    x_clear(1,1,0.03*color,0.03*color,0.03*color);
}

static void swap(void)
{
    x_finish();

    st.doframesync=1;

    if(showinfo)
    {
        x_viewport(0,0,init.gfxwid-1,init.gfxhig-1);
        clear(5);
        x_viewport(0,init.gfxhig*0.5,init.gfxwid-1,init.gfxhig-1);
        clear(0);
        realdrawmode();
    }
    else
    {
        x_viewport(0,0,init.gfxwid-1,init.gfxhig-1);
        clear(0);
        realdrawmode();
    }
}

static void opendisplay(void)
{
    print("Graphics initialized: %ix%i\n",init.gfxwid,init.gfxhig);

    x_init();
    //x_open(NULL,NULL,init.gfxwid,init.gfxhig,2,1);
      {
         extern void *hwndMain; // void* since windows.h not included
         x_open(NULL,hwndMain,init.gfxwid,init.gfxhig,2,1);
      }
    rst.fullscreen=1;

    if(!init.novoodoo2 && x_combine2(X_WHITE,X_WHITE,0)==0) rst.dualtmu=1;
    else rst.dualtmu=0;

    swap();
}

static void closedisplay(void)
{
    rst.combcacheused=0;
    rdp_freetexmem();

    x_deinit();
    rst.fullscreen=0;
}

/****************************************************************************
/* Public Entrypoints
*/

void rdp_segment(int seg,dword base)
{
    if(seg>15)
    {
        error("rdp: segment > 15\n");
        seg=0;
    }
    rst.segment[seg]=base;
}

int rdp_cmd(dword *cmd)
{
    int c=cmd[0]>>24,last;
    static int last0;

    last=last0;
    last0=c;

    if(rst.prtabi!=rst.last_prtabi)
    {
        flushprims();
    }

    if(rst.wordsleft>0)
    {
        // handle multiword commands (texrect)
        // the actual command code is ignored
        if(rst.wordsleft==2)
        {
            rst.texrect.s0=(short)FIELD(cmd[1],16,16);
            rst.texrect.t0=(short)FIELD(cmd[1],0,16);
            rst.wordsleft=1;
        }
        else
        {
            rst.texrect.s1=(short)FIELD(cmd[1],16,16);
            rst.texrect.t1=(short)FIELD(cmd[1],0,16);
            rst.wordsleft=0;
            if(st.dumpgfx)
            {
                logd("\n# texrect (%f,%f)-(%f,%f) (%f,%f)+*(%f,%f)",
                    rst.texrect.x0,
                    rst.texrect.y0,
                    rst.texrect.x1,
                    rst.texrect.y1,
                    rst.texrect.s0,
                    rst.texrect.s0,
                    rst.texrect.s1,
                    rst.texrect.s1);
            }
            rdp_texrect(&rst.texrect);
        }
        return(rst.wordsleft);
    }
    else switch(c)
    {
    case 0xff: // RDP_SETCIMG
        setbuffer(RDP_BUF_C,cmd[0],cmd[1]);
        break;
    case 0xfe: // RDP_SETZIMG
        setbuffer(RDP_BUF_Z,cmd[0],cmd[1]);
        break;
    case 0xfd: // RDP_SETTIMG
        setbuffer(RDP_BUF_TXT,cmd[0],cmd[1]);
        logd("\n+tile settimg bpp=%s fmt=%s wid=%i %08X",
            bpp[rst.bufbpp[RDP_BUF_TXT]],
            fmt[rst.buffmt[RDP_BUF_TXT]],
            rst.bufwid[RDP_BUF_TXT],
            rst.bufbase[RDP_BUF_TXT]);
        break;
    case 0xfc: // RDP_SETCOMBINE
        rst.combine0[0]=cmd[0];
        rst.combine1[0]=cmd[1];
        rst.modechange=1;
        break;
    case 0xfb: // RDP_SETENVCOLOR
        setcolor(C_ENV,cmd[1]);
        break;
    case 0xfa: // RDP_SETPRIMCOLOR
        setcolor(C_PRIM,cmd[1]);
        // copy lod factor
        setcolorintensity(C_PLODF,cmd[0]&255);
        // lod clamp not copied
        break;
    case 0xf9: // RDP_SETBLENDCOLOR
        setcolor(C_BLEND,cmd[1]);
        break;
    case 0xf8: // RDP_SETFOGCOLOR
        setcolor(C_FOG,cmd[1]);
        rdp_fogrange(rst.fogmin,rst.fogmax); // reload fog
        break;
    case 0xf7: // RDP_SETFILLCOLOR
        rst.rawfillcolor=cmd[1];
        setfillcolor(C_FILL,cmd[1]);
        break;
    case 0xf6: // RDP_FILLRECT
        rst.texrect.x0=0.25*FIELD(cmd[0],12,12);
        rst.texrect.y0=0.25*FIELD(cmd[0],0,12);
        rst.texrect.x1=0.25*FIELD(cmd[1],12,12);
        rst.texrect.y1=0.25*FIELD(cmd[1],0,12);
        if(st.dumpgfx)
        {
            logd("# fillrect (%f,%f)-(%f,%f)",
                rst.texrect.x0,
                rst.texrect.y0,
                rst.texrect.x1,
                rst.texrect.y1);
        }
        rdp_fillrect(&rst.texrect);
        break;
    case 0xf5: // RDP_SETTILE
        {
            int ti=FIELD(cmd[1],24,3);
            Tile *t=rst.tile+ti;
            t->fmt     =FIELD(cmd[0],21,3);
            t->bpp     =FIELD(cmd[0],19,2);
            t->tmemrl  =8*FIELD(cmd[0],9,9);
            t->tmembase=8*FIELD(cmd[0],0,9);
            t->palette =FIELD(cmd[1],20,4);
            t->cmt     =FIELD(cmd[1],18,2);
            t->maskt   =FIELD(cmd[1],14,4);
            t->shiftt  =FIELD(cmd[1],10,4);
            t->cms     =FIELD(cmd[1],8,2);
            t->masks   =FIELD(cmd[1],4,4);
            t->shifts  =FIELD(cmd[1],0,4);
            t->settilemark=0;

            /*
            if(cart.iszelda && (rst.lastloadb&0xffff0000)==0x802b0000)
            {
                if(rst.lastloadb==0x802b1df0 ||
                   rst.lastloadb==0x802b21d0 ||
                   rst.lastloadb==0x802b23d0 ||
                   rst.lastloadb==0x802b25f0 ||
                   rst.lastloadb==0x802b29d0 ||
                   rst.lastloadb==0x802b2bd0)
                {
                    if(t->fmt==4 && t->bpp==1) t->bpp=0;
                }
            }
            */

            // force clamps
            if(!t->maskt && t->cmt==0) t->cmt=2;
            if(!t->masks && t->cms==0) t->cms=2;
            //
            if(st.dumpgfx)
            {
                logd("\n+tile %i settile fmt=%s bpp=%s tmemrl=%i tmembase=%i pal=%i"
                     "\n+tile   cmt=%s maskt=%i shiftt=%i cms=%s masks=%i shifts=%i",
                    ti,
                    fmt[t->fmt],
                    bpp[t->bpp],
                    t->tmemrl,
                    t->tmembase,
                    t->palette,
                    cm[t->cmt],
                    t->maskt,
                    t->shiftt,
                    cm[t->cms],
                    t->masks,
                    t->shifts);
            }
        }
        break;
    case 0xf4: // RDP_LOADTILE
        {
            int i,x0,y0,x1,y1;
            int ti=FIELD(cmd[1],24,3);
            x0=FIELD(cmd[0],12,12)>>2;
            y0=FIELD(cmd[0],0,12) >>2;
            x1=FIELD(cmd[1],12,12)>>2;
            y1=FIELD(cmd[1],0,12) >>2;
            if(st.dumpgfx)
            {
                logd("\n+tile   loadtile (%08X->%04X) rl=%i (%i,%i)-(%i,%i)",
                    rst.bufbase[RDP_BUF_TXT],
                    rst.tile[ti].tmembase,
                    rst.bufwid[RDP_BUF_TXT],
                    x0,y0,x1,y1);
            }
            i=(rst.tile[ti].tmembase>>3)&511;
            rst.tmemsrc[i]=rst.bufbase[RDP_BUF_TXT];
            rst.tmemrl [i]=(rst.bufwid[RDP_BUF_TXT]<<rst.bufbpp[RDP_BUF_TXT])>>1;
            rst.tmemx0 [i]=(x0<<rst.bufbpp[RDP_BUF_TXT])>>1;
            rst.tmemy0 [i]=y0;
            rst.modechange=rst.txtchange=1;
            rst.lastloadb=0;
        }
        break;
    case 0xf3: // RDP_LOADBLOCK
        {
            int i,j,a,s;
            int ti=FIELD(cmd[1],24,3);
            int xx1=FIELD(cmd[1],12,12);
            int dxt=FIELD(cmd[1],0,12);

            if(st.dumpgfx)
            {
                logd("\n+tile   loadblock (%08X->%04X) xxx=%i dxt=%i",
                    rst.bufbase[RDP_BUF_TXT],rst.tile[ti].tmembase,xx1,dxt);
            }
            i=(rst.tile[ti].tmembase>>3)&511;

            s=((xx1+1)<<rst.tile[ti].bpp)>>1;
            if(s==4096)
            {
                j=2;
            }
            else
            {
                j=1;
            }

            // set locations at START and MIDDLE of loaded block only
            // quake loads 2 similar size textures at once, other games
            // only load one texture at the time
            a=rst.bufbase[RDP_BUF_TXT];
            for(;;)
            {
                rst.tmemsrc[i]=a;
                rst.tmemx0 [i]=0;
                rst.tmemy0 [i]=0;

                if(dxt)
                {
                    int a;
                    //a=0;
                    a=(8*2048/dxt+6)&~7; // see util\koe2.c
                    rst.tmemrl[i]=a;
                }
                else
                {
                    rst.tmemrl[i]=-1;
                }

                if(!--j) break;

                i+=s>>4;
                a+=s>>4;

                if(i>=511) break;
            }

            rst.modechange=rst.txtchange=1;
            rst.lastloadb=rst.bufbase[RDP_BUF_TXT];
        }
        break;
    case 0xf2: // RDP_SETTILESIZE
        {
            int ti=FIELD(cmd[1],24,3);
            int xs,ys;
            Tile *t=rst.tile+ti;

            t->x0full=FIELD(cmd[0],12,12);
            t->y0full=FIELD(cmd[0],0,12);

            if(t->settilemark)
            {
                // already set, this is probably a offset call (x0y0 change)
                break;
            }
            t->settilemark=1;

            xs=FIELD(cmd[1],14,10)-FIELD(cmd[0],14,10);
            ys=FIELD(cmd[1],2,10) -FIELD(cmd[0],2,10) ;
            if(!xs) xs=1;
            if(!ys) ys=1;
            if(xs*ys>=16384)
            {
                // can't be right!?
                // keep the scrolling part, but
                // ignore new size
                logd("\n+tile %i settilesize (%i,%i)-(%i,%i) error! ",
                    ti,
                    FIELD(cmd[0],14,10),
                    FIELD(cmd[0],2,10),
                    FIELD(cmd[1],14,10),
                    FIELD(cmd[1],2,10));
                break;
            }

            t->x0=FIELD(cmd[0],14,10);
            t->y0=FIELD(cmd[0],2,10);
            t->x1=FIELD(cmd[1],14,10);
            t->y1=FIELD(cmd[1],2,10);
            t->xs=t->x1-t->x0+1;
            t->ys=t->y1-t->y0+1;

            if(rst.lastloadb)
            {
                t->xs=t->x1+1;
                t->ys=t->y1+1;
            }

            if(ti!=7)
            {
                rst.lastusedtile=ti;
                t->membase=rst.bufbase[RDP_BUF_TXT];
                t->memfmt =rst.buffmt[RDP_BUF_TXT];
                t->membpp =rst.bufbpp[RDP_BUF_TXT];
                rst.modechange=rst.txtchange=1;
            }
            if(st.dumpgfx)
            {
                logd("\n+tile %i settilesize (%i,%i)-(%i,%i) sz(%i,%i) base %08X",
                    ti,
                    t->x0,t->y0,
                    t->x1,t->y1,
                    t->xs,t->ys,
                    t->membase);
            }
        }
        break;
    case 0xf1: // RDP_RDPHALF_RDP
        logd("!skiprdp ");
        break;
    case 0xf0: // RDP_LOADTLUT
        {
            int ti=FIELD(cmd[1],24,3);
            int t,siz,base,pal,start;
            t=FIELD(cmd[1],24,3);
            siz=FIELD(cmd[1],14,10)+1; // texels
            start=FIELD(cmd[1],2,10)+1;
            pal=(rst.tile[ti].tmembase-2048)/(16*8);
            if(siz>256) rst.geyemode=1;
            if(rst.geyemode) siz/=4;
            base=rst.bufbase[RDP_BUF_TXT];

            if(start!=0)
            {
                int x0,y0,x1,y1;
                x0=FIELD(cmd[0],12,12)>>2;
                y0=FIELD(cmd[0],0,12) >>2;
                x1=FIELD(cmd[1],12,12)>>2;
                y1=FIELD(cmd[1],0,12) >>2;
                if(st.dumpgfx)
                {
                    logd("\n+tile loadtlut range (%i,%i)-(%i,%i)",
                        x0,y0,x1,y1);
                }
                base+=2*y0+2*x0;
                siz=(x1-x0)+1;
                if(siz>256) siz=256;
            }
            logd("\n+tile loadtlut buffer base %08X pal=%i size=%i start=%i",
                rst.bufbase[RDP_BUF_TXT],pal,siz);

            rst.tlut_base=base;
            rst.tlut_palbase=pal;
            rst.tlut_num=siz;
        }
        break;
    case 0xef: // RDP_RDPSETOTHERMODE
        logd("OtherSet %08X %08X",cmd[0],cmd[1]);
        rst.other0[0]=cmd[0];
        rst.other1[0]=cmd[1];
        rst.modechange=1;
        break;
    case 0xee: // RDP_SETPRIMDEPTH
        logd("!skiprdp ");
        break;
    case 0xed: // RDP_SETSCISSOR
        logd("!skiprdp ");
        break;
    case 0xec: // RDP_SETCONVERT
        logd("!skiprdp ");
        break;
    case 0xeb: // RDP_SETKEYR
        logd("!skiprdp ");
        break;
    case 0xea: // RDP_SETKEYGB
        logd("!skiprdp ");
        break;
    case 0xe9: // RDP_RDPFULLSYNC
        st2.gfxdpsyncpending=1;
        break;
    case 0xe8: // RDP_RDPTILESYNC
    case 0xe6: // RDP_RDPLOADSYNC
    case 0xe7: // RDP_RDPPIPESYNC
        rst.modechange=1;
        break;
    case 0xe5: // RDP_TEXRECTFLIP
        rst.texrect.flip=1;
        rst.texrect.x0=0.25*FIELD(cmd[0],12,12);
        rst.texrect.y0=0.25*FIELD(cmd[0],0,12);
        rst.texrect.x1=0.25*FIELD(cmd[1],12,12);
        rst.texrect.y1=0.25*FIELD(cmd[1],0,12);
        rst.texrect.tile=FIELD(cmd[1],24,3);
        rst.wordsleft=2;
        rst.wordcmd=0xe5;
        if(st.dumpgfx)
        {
            logd("\n# texrectflip (%f,%f)-(%f,%f) ...",
                rst.texrect.x0,
                rst.texrect.y0,
                rst.texrect.x1,
                rst.texrect.y1);
        }
        return(2); // need 2 more commands
    case 0xe4: // RDP_TEXRECT
        // 2 bits of sub in these
        rst.texrect.flip=0;
        rst.texrect.x0=0.25*FIELD(cmd[0],12,12);
        rst.texrect.y0=0.25*FIELD(cmd[0],0,12);
        rst.texrect.x1=0.25*FIELD(cmd[1],12,12);
        rst.texrect.y1=0.25*FIELD(cmd[1],0,12);
        rst.texrect.tile=FIELD(cmd[1],24,3);
        rst.wordsleft=2;
        rst.wordcmd=0xe4;
        if(st.dumpgfx)
        {
            logd("\n# texrect (%f,%f)-(%f,%f) ...",
                rst.texrect.x0,
                rst.texrect.y0,
                rst.texrect.x1,
                rst.texrect.y1);
        }
        return(2); // need 2 more commands
    default:
        error("rdp: unknown command %02X!",c);
        return(-1); // unknown command
    }

//    logd("&mc%i",rst.modechange);

    return(0); // no more data needed
}

void rdp_opendisplay(void)
{
    if(rst.opened) return;
    rst.opened=1;

    logd("Opendisplay.\n");
    logd(NULL);
    opendisplay();
}

void rdp_closedisplay(void)
{
    if(!rst.opened) return;
    rst.opened=0;

    logd("Closedisplay.\n");
    logd(NULL);
    closedisplay();
}

void rdp_framestart(void)
{
    int i;

    if(rst.frameopen) return;
    rst.frameopen=1;

    rst.firstfillrect=1;

    rdp_opendisplay();

    rdp_viewport(init.gfxwid/2,init.gfxhig/2,init.gfxwid/2,init.gfxhig/2);

    rst.testcnt=0;

    for(i=0;i<MAXRDPVX;i++) rdpvx[i]=rdpdummyvx+i;

    if(rst.myframe<=0) rst.myframe=1;

    rst.view_x0=0;
    rst.view_y0=0;
    rst.view_x1=init.gfxwid-1;
    rst.view_y1=init.gfxhig-1;

    rst.txtloads=0;

    rst.fillrectcnt=0;

    rst.lastalphatst=-1;
    rst.lastzmode=-1;

    rst.nexttexturetile=rst.texturetile=0;

    rst.starttimeus=timer_us(&st2.timer);

    rst.fogmin=rst.fogmax=0.0;
    rdp_fogrange(0.0,0.0);

    if(!(rst.myframe&15))
    {
        // clear combine cache every now and then
        rst.combcacheused=0;
    }

    // clear cache entries not used for 3 frames
    for(i=0;i<MAXTXT;i++)
    {
        if(rst.txt[rst.txtrefreshcnt].used_vidframe>rst.myframe-3)
        {
            rst.txt[rst.txtrefreshcnt].membase=0;
        }
    }

    if(st.dumpgfx)
    {
        // clear texture cache
        logd("Dump mode, clearing texture cache.\n");
        rdp_freetexmem();
    }

    com_init();

    clearprims();

    rst.tlut_lastbase=0xffffffff;

    rst.modechange=2; // full mode set
    rst.firstmodechange=1;

    if(showinfo) rst.debugwirecolor=rand();
    else rst.debugwirecolor=0;

    rst.tris=0;
}

void rdp_frameend(void)
{
    if(!rst.frameopen) return;
    rst.frameopen=0;

    st2.gfxdpsyncpending=1;
    flushprims();
    // debuginfo
    if(1)
    {
        // draw debug wireframe
        if(showinfo || showwire)
        {
            wireprims();
        }
        if(showinfo)
        {
            drawtextures();
        }

        /*
        // marker dots and misc stuff
        if(showinfo)
        {
            x_flush();
            debugdrawmode();
            x_flush();
            drawmarkers();
            x_flush();
            drawtextures();
            x_flush();
        }
        */
    }

    // show buffer on screen
    if(rst.tris>0)
    {
        swap();
    }

    if(st.dumpinfo)
    {
        print("Frame%6i: %i modes, %i newtext, %i text, %i vtx, %i prim\n",
            st.frames,
            rst.cnt_setting,
            rst.cnt_texturegen,
            rst.cnt_texture,
            rst.vxtabi,
            rst.prtabi);
    }

    rst.cnt_setting=0;
    rst.cnt_texture=0;
    rst.cnt_texturegen=0;

    st.ops_fast=0;
    st.ops_slow=0;

    logd("\nSwap - Cputime:%08X <{([+*#*+])}>\n\n",(dword)st.cputime);

    rst.myframe++;
}

void rdp_swap(void)
{
    rst.swapflag=1;
}

void rdp_copybackground(dword base,int wid,int hig)
{
    static int    xhandle[2];
    static char   buf[256*256][4];
    static dword  lastcrc;
    int i,x,y,t;
    float x0,y0,x1,y1;
    dword d,crc;

    if(!xhandle[0])
    {
        xhandle[0]=x_createtexture(X_RGBA5551|X_CLAMP,256,256);
        xhandle[1]=x_createtexture(X_RGBA5551|X_CLAMP,256,256);
    }
    crc=0;
    for(i=0;i<320*240*2;i+=9035)
    {
        crc^=mem_read32p(i+base);
        crc =(crc<<7)|(crc>>(32-7));
    }
    if(crc!=lastcrc)
    {
        lastcrc=crc;
        // update txt
        for(t=0;t<2;t++)
        {
            for(y=0;y<240;y++)
            {
                for(x=0;x<256;x++)
                {
                    d=base+(t*256+x)*2+y*wid*2;
                    d=mem_read16(d);
                    buf[x+y*256][0]=FIELD(d,11,5)*8;
                    buf[x+y*256][1]=FIELD(d, 6,5)*8;
                    buf[x+y*256][2]=FIELD(d, 1,5)*8;
                    buf[x+y*256][3]=255;
                }
            }
            x_loadtexturelevel(xhandle[t],0,(char *)buf);
        }
    }

    // draw
    x_mask(X_ENABLE,X_DISABLE,X_DISABLE);
    x_combine(X_TEXTURE);
    x_blend(X_ONE,X_ZERO);
    x_texture(xhandle[0]);

    x0=0.0;
    y0=0.0;
    x1=256.0;
    y1=256.0;
    x_begin(X_QUADS);
    x_vxtex(0,0); x_vxpos(x0*oxm+oxa,y0*oym+oya,1.0);
    x_vxtex(0,1); x_vxpos(x0*oxm+oxa,y1*oym+oya,1.0);
    x_vxtex(1,1); x_vxpos(x1*oxm+oxa,y1*oym+oya,1.0);
    x_vxtex(1,0); x_vxpos(x1*oxm+oxa,y0*oym+oya,1.0);
    x_end();

    x0=256.0;
    y0=0.0;
    x1=512.0;
    y1=256.0;
    x_texture(xhandle[1]);
    x_begin(X_QUADS);
    x_vxtex(0,0); x_vxpos(x0*oxm+oxa,y0*oym+oya,1.0);
    x_vxtex(0,1); x_vxpos(x0*oxm+oxa,y1*oym+oya,1.0);
    x_vxtex(1,1); x_vxpos(x1*oxm+oxa,y1*oym+oya,1.0);
    x_vxtex(1,0); x_vxpos(x1*oxm+oxa,y0*oym+oya,1.0);
    x_end();

    realdrawmode();
    rst.modechange=2;
    newmode();
}

void rdp_flat(int flat)
{
    rst.setflat=flat;
    rst.modechange=1;
}

int  rdp_gfxactive(void)
{
    return(rst.opened&rst.fullscreen);
}

void rdp_togglefullscreen(void)
{
    rst.fullscreen^=1;
    x_fullscreen(rst.fullscreen);
    if(init.shutdownglide)
    {
        if(!rst.fullscreen)
        {
            rdp_closedisplay();
            if(st.graphicsenable>0) st.graphicsenable=-1;
        }
        else
        {
            if(st.graphicsenable<0) st.graphicsenable=1;
        }
    }
    // clear combine cache and texture cache (when enabling display)
    if(rst.fullscreen)
    {
    }
}

