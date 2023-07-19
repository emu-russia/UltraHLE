#include "ultra.h"
#include "x.h"

//#define logd(x)

//#define LOGOPROJ

//#define DUMPVXTRI

#define EXTRAWIRETRIS

//#define SHOWBPP

#define MAXVX      64
#define TEXCACHE   256 // textures in cache
#define CACHEDELAY 16
#define CACHERAND  2  // randomization to delay

#define OVLZ   1.0

typedef struct
{
    dword   memaddr; // memaddress and contents crc for topleftmost pixel was loaded from (updated by createtexture)
    int     tmembase;
    int     tmemrl;  // rowlen
    int     memrl;  //memory rowlen
    int     fmt;
    int     bpp;
    int     xs,ys;
    int     cmt,maskt,shiftt;
    int     cms,masks,shifts;
    int     x0,y0;
    int     x1,y1;
    int     palette;
} Tile;

typedef struct
{
    Tile    tile;
    int     creation_vidframe;
    int     used_vidframe;
    int     debugtilepos;
    int     xhandle;
    int     xs,ys;
    int     cms,cmt;
} Texture;

typedef struct
{
    dword   data[4];
    float   col[3];
    float   dir[4];
    float   xfdir[4];
} Light;

typedef struct
{
    // memory
    dword   segment[16];
    // matrices
    float   mtx[2][16][16];
    float   xform[16]; // active full transform
    int     mtxstackp[2];
    // lights
    int     lightnum;
    Light   light[8];
    float   lightmat[16];
    int     lightnumchanged;
    // fog
    float   fognear;
    float   fogfar;
    float   zmax;
    // misc
    int     projection;     // PROJ_*
    int     lastframehadbackground;
    int     framehadbackground;
    dword   loadrspdata;
    // what has changed (checked at startdrawing())
    int     newsettings;
    int     newtexture;
    int     newmatrices;
    int     newvertices;
    int     newvertices_lo;
    int     newvertices_hi;
    // raw modebits (omode just sent to RDP after changes)
    dword   gmode,lastgmode;
    dword   omodel;
    dword   omodeh;
    // parsed geometry mode
    int     lightvx;
    int     flat;
    int     texgen;
    int     cull;
    // texture scaling
    int     texenable;
    float   scales;
    float   scalet;
    // counts
    int     cnt_triin; // input triangles
    int     cnt_tri;   // sent to rdp
    int     cnt_vtx;
    int     cnt_mtx;
    // misc
    int     cammoveset;
    float   cammove[3];

    int     errorcnt[256];

    int     ignore;
} GeomState;

#define PROJ_NONE  0 // set to this when matrices change
#define PROJ_PERSP 1
#define PROJ_DEBUG 2

#define COLOR_ZERO       0
#define COLOR_DUNNO      1
#define COLOR_SHADE      2
#define COLOR_PRIM       3
#define COLOR_ENV        4
#define COLOR_SHADE_ENV  5
#define COLOR_SHADE_PRIM 6
#define COLOR_ONE        7

#define COMBINE_COLOR   0
#define COMBINE_MUL     1
#define COMBINE_TEXTURE 2
#define COMBINE_PRO     3

// combine colors
#define CC_ZERO   0
#define CC_DUNNO  1
#define CC_TEXT   2
#define CC_PRIM   3
#define CC_SHADE  4
#define CC_ENV    5
#define CC_TEXTA  6

#define BLEND_DUNNO              0
#define BLEND_MULALPHA           1 // 0055
#define BLEND_AA_OPAQUE          2 // 0044
#define BLEND_NORMAL             3 // 0F0A or 0A0F
#define BLEND_ALPHAMIX           4 // 0050
#define BLEND_ALPHAMIX2          5 // 0040

#define TRISTORESIZE 4096
struct
{
    xt_pos  vx[3];
    int     cull;
    int     color; // 0=red, 1=green, 2=blue, 3=white
    int     RESERVED;
} tristore[TRISTORESIZE];
int tristorenum;

xt_data  g_data;

xt_pos   g_pos;

int  opened;

GeomState gst;

dword stack[256];
dword stackp;
dword dlpnt;
int   dllimit;
int   errors;
dword cmd[8]; // current loaded command

float identity[16]={
 1.0,   0.0,   0.0,   0.0,
 0.0,   1.0,   0.0,   0.0,
 0.0,   0.0,   1.0,   0.0,
 0.0,   0.0,   0.0,   1.0,
};

float orthoproj[16]={
 1.0,   0.0,   0.0, -OVLZ,
 0.0,   1.0,   0.0,  OVLZ,
 0.0,   0.0,   0.0,   0.0,
 0.0,   0.0,   0.0,  OVLZ,
};

#define CH_OTHERL   0x1
#define CH_OTHERH   0x2
#define CH_GEOM     0x4
#define CH_COMBINE  0x8
#define CH_DUMP     0x100
void g_modechange(int what);
void g_mtxxform(void);

void startdebugdrawing(void);
void startdrawing(void);

void clear(int color);
void opendisplay(void);
void flushtextstore(void);
void flushtristore(void);
void framedone(void);
void reloadsettings(void);

void t_selecttexture(int t,int txt);

void setcolor(float *col,dword a);
void setcolor2(float *col,dword a);
void setnormal(float *col,dword a);
void setnormal2(float *col,dword a);

/***********************************************************************/
// logging

/***********************************************************************/
// utilities

void setcolor(float *col,dword a)
{
    col[0]=((a>>8 )&255)*(1.0/258.0);
    col[1]=((a>>16)&255)*(1.0/258.0);
    col[2]=((a>>24)&255)*(1.0/258.0);
    col[3]=((a>>0 )&255)*(1.0/258.0);
}

void setcolor2(float *col,dword a)
{
    col[0]=((a>>24)&255)*(1.0/258.0);
    col[1]=((a>>16)&255)*(1.0/258.0);
    col[2]=((a>>8 )&255)*(1.0/258.0);
    col[3]=((a>>0 )&255)*(1.0/258.0);
}

void setnormal(float *col,dword a)
{
    col[0]=((char)((a>>8 )&255))*(1.0/128.0);
    col[1]=((char)((a>>16)&255))*(1.0/128.0);
    col[2]=((char)((a>>24)&255))*(1.0/128.0);
    col[3]=((char)((a>>0 )&255))*(1.0/128.0);
}

void setnormal2(float *col,dword a)
{
    col[0]=((char)((a>>24)&255))*(1.0/128.0);
    col[1]=((char)((a>>16)&255))*(1.0/128.0);
    col[2]=((char)((a>>8 )&255))*(1.0/128.0);
    col[3]=((char)((a>>0 )&255))*(1.0/128.0);
}

void TransposeMatrix(float *m,float *m1)
{
    int x,y;
    for(y=0;y<4;y++)
    {
        for(x=0;x<4;x++)
        {
            m[x+y*4]=m1[y+x*4];
        }
    }
}

void MultMatrix(float *m,float *m1,float *m2)
{
    int x,y,i;
    float f;
    for(y=0;y<4;y++)
    {
        for(x=0;x<4;x++)
        {
            f=0;
            for(i=0;i<4;i++)
            {
                //f+=m1[i+y*4] * m2[x+i*4];
                f+=m1[i+y*4] * m2[x+i*4];
            }
            m[x+y*4]=f;
        }
    }
}

void FastMultMatrix(float *m,float *m1,float *m2)
{ // assume rightmost column is 0 0 0 1
    int x,y;
    float f;
    for(y=0;y<4;y++)
    {
        for(x=0;x<3;x++)
        {
            f=m1[0+y*4] * m2[x+0*4] +
              m1[1+y*4] * m2[x+1*4] +
              m1[2+y*4] * m2[x+2*4] +
                          m2[x+3*4] ;
            m[x+y*4]=f;
        }
        m[3+y*4]=0.0;
    }
    m[3+3*4]=1.0;
}

void InvertMatrix(float *dest,float *m)
{
    float *d,*a;
    d=(float *)dest;
    a=(float *)m;

    #define D(y,x) d[(x)*4+(y)]
    #define S(y,x) a[(x)*4+(y)]
    #define DET(a,b,c,d) (S(a,b)*S(c,d)-S(a,d)*S(c,b))

    D(0,0)=+DET(1,1,2,2);
    D(0,1)=-DET(0,1,2,2);
    D(0,2)=+DET(0,1,1,2);
    D(1,0)=-DET(1,0,2,2);
    D(1,1)=+DET(0,0,2,2);
    D(1,2)=-DET(0,0,1,2);
    D(2,0)=+DET(1,0,2,1);
    D(2,1)=-DET(0,0,2,1);
    D(2,2)=+DET(0,0,1,1);
    D(0,3)=-(S(0,3)*D(0,0)+S(1,3)*D(0,1)+S(2,3)*D(0,2));
    D(1,3)=-(S(0,3)*D(1,0)+S(1,3)*D(1,1)+S(2,3)*D(1,2));
    D(2,3)=-(S(0,3)*D(2,0)+S(1,3)*D(2,1)+S(2,3)*D(2,2));
    D(3,0)=0.0f;
    D(3,1)=0.0f;
    D(3,2)=0.0f;
    D(3,3)=1.0f;

    #undef DET
    #undef D
    #undef S
}

void InvertMatrixRotate(float *dest,float *m)
{
    float *d,*a;
    d=(float *)dest;
    a=(float *)m;

    #define D(y,x) d[(x)*4+(y)]
    #define S(y,x) a[(x)*4+(y)]
    #define DET(a,b,c,d) (S(a,b)*S(c,d)-S(a,d)*S(c,b))

    D(0,0)=+DET(1,1,2,2);
    D(0,1)=-DET(0,1,2,2);
    D(0,2)=+DET(0,1,1,2);
    D(1,0)=-DET(1,0,2,2);
    D(1,1)=+DET(0,0,2,2);
    D(1,2)=-DET(0,0,1,2);
    D(2,0)=+DET(1,0,2,1);
    D(2,1)=-DET(0,0,2,1);
    D(2,2)=+DET(0,0,1,1);

    #undef DET
    #undef D
    #undef S
}

void DumpMatrix(float *m)
{
    int x,y;
    if(!st.dumpgfx) return;
    for(y=0;y<4;y++)
    {
        logd("\n{ ");
        for(x=0;x<4;x++)
        {
            logd("%13.5f ",m[x+y*4]);
        }
        logd(" } ");
    }
}

void PrintMatrix(float *m)
{
    int x,y;
    for(y=0;y<4;y++)
    {
        print("{ ");
        for(x=0;x<4;x++)
        {
            print("%13.5f ",m[x+y*4]);
        }
        print(" }\n");
    }
}

void DumpMatrix2(float *m)
{
    int x,y;
    if(!st.dumpgfx) return;
    for(y=0;y<4;y++)
    {
        print("\n{ ");
        for(x=0;x<4;x++)
        {
            print("%13.5f ",m[x+y*4]);
        }
        print(" } ");
    }
}

void dlist_showa0matrix(void)
{
    float m[16];
    int   i;
    if(!st.dumpgfx) return;
    for(i=0;i<16;i++)
    {
        int x;
        x=mem_read32p(i*4+A0.d);
        m[i]=*(float *)&x;
    }
    print("\nguMtxF2L:");
    DumpMatrix2(m);
    print("\n");
}

/***********************************************************************/
// display list geometry commands

void g_viewport(dword pos)
{
    dword x;
    float xm,ym,xa,ya,zm;

    x=mem_read32p(pos);
    xm=(x>>16  )*0.25;
    ym=(x&65535)*0.25;

    x=mem_read32p(pos+4);
    zm=(x>>16  )*1.00;

    x=mem_read32p(pos+8);
    xa=(x>>16  )*0.25;
    ya=(x&65535)*0.25;

    logd("\n!viewport *(%f,%f)+(%f,%f) z(%f) ",xm,ym,xa,ya,zm);

    gst.zmax=zm;
    rdp_viewport(xm,ym,xa,ya);
}

void g_lightnum(int num)
{
    if(num<0) num=0;
    if(num>7) num=7;
    logd(" Lightnum=%i",num);
    gst.lightnum=num;
    gst.lightnumchanged=1;
}

void g_loadlight(int li,dword pos)
{
    Light *l=gst.light+li;
    int i;
    l->data[0]=mem_read32p(pos+0);
    l->data[1]=mem_read32p(pos+4);
    l->data[2]=mem_read32p(pos+8);
    l->data[3]=mem_read32p(pos+12);
    setcolor2(l->col,l->data[0]);
    setnormal2(l->dir,l->data[2]);
    logd("\n!light %i color %.2f %.2f %.2f dir %.2f %.2f %.2f %.2f (%i lights) [ ",
        li,
        l->col[0],l->col[1],l->col[2],
        l->dir[0],l->dir[1],l->dir[2],l->dir[3],
        gst.lightnum);
    for(i=0;i<4;i++) logd("%08X ",l->data[i]);
    logd("]");
}

void g_xformlights(void)
{
    Light *l;
    int    i;
    float  m[16];

    logd("\n!light xform (%i lights)",gst.lightnum);

    // calc inverse modelview (only rotate part)
    InvertMatrixRotate(m,gst.mtx[0][gst.mtxstackp[0]]);
    /*
    DumpMatrix(gst.mtx[0][gst.mtxstackp[0]]);
    DumpMatrix(m);
    */

    for(i=0;i<gst.lightnum;i++)
    {
        l=&gst.light[i];

        if(0)
        {
            // copy light
            l->xfdir[0]=l->dir[0];
            l->xfdir[1]=l->dir[1];
            l->xfdir[2]=l->dir[2];
        }
        else
        {
            float inv;
            // transform lights with inverse modelview and normalize
            l->xfdir[0]=l->dir[0]*m[4*0+0]+l->dir[1]*m[4*1+0]+l->dir[2]*m[4*2+0];
            l->xfdir[1]=l->dir[0]*m[4*0+1]+l->dir[1]*m[4*1+1]+l->dir[2]*m[4*2+1];
            l->xfdir[2]=l->dir[0]*m[4*0+2]+l->dir[1]*m[4*1+2]+l->dir[2]*m[4*2+2];
            inv=1.0/sqrt(l->xfdir[0]*l->xfdir[0]+
                         l->xfdir[1]*l->xfdir[1]+
                         l->xfdir[2]*l->xfdir[2]);
            l->xfdir[0]*=inv;
            l->xfdir[1]*=inv;
            l->xfdir[2]*=inv;
            logd("\n!light %i: %.2f %.2f %.2f -> %.2f %.2f %.2f",
                i,
                l->dir[0],l->dir[1],l->dir[2],
                l->xfdir[0],l->xfdir[1],l->xfdir[2]);
        }
    }
}

void g_initvx(int i)
{
    Vertex *v;
    float  nor[4];

    v=rdpvx[i];

    if(gst.lightvx || gst.texgen)
    {
        // get normal
        nor[0]=((char)((v->icol>>24)&255))*(1.0/128.0);
        nor[1]=((char)((v->icol>>16)&255))*(1.0/128.0);
        nor[2]=((char)((v->icol>>8 )&255))*(1.0/128.0);
    }

    if(gst.lightvx)
    {
        float  cr,cg,cb,sc;
        Light *l;
        int    j;
        // ambient
        l=&gst.light[gst.lightnum];
        cr=l->col[0];
        cg=l->col[1];
        cb=l->col[2];
        // directional
        for(j=0;j<gst.lightnum;j++)
        {
            l=&gst.light[j];
            sc=l->xfdir[0]*nor[0]+
               l->xfdir[1]*nor[1]+
               l->xfdir[2]*nor[2];
            if(sc<0) sc=0;
            cr+=l->col[0]*sc;
            cg+=l->col[1]*sc;
            cb+=l->col[2]*sc;
        }
        // clamp
        if(cr>0.99) cr=0.99;
        if(cg>0.99) cg=0.99;
        if(cb>0.99) cb=0.99;
        // set color
        v->col[0]=cr;
        v->col[1]=cg;
        v->col[2]=cb;
        // set alpha
        v->col[3]=(((v->icol    )&255))*(1.0/256.0);
//        v->col[3]=1.0;
    }
    else
    {
        // get color
        v->col[0]=(((v->icol>>24)&255))*(1.0/256.0);
        v->col[1]=(((v->icol>>16)&255))*(1.0/256.0);
        v->col[2]=(((v->icol>> 8)&255))*(1.0/256.0);
        // get alpha
        v->col[3]=(((v->icol    )&255))*(1.0/256.0);
    }

    if(gst.texenable)
    {
        v->tex[0]*=gst.scales;
        v->tex[1]*=gst.scalet;
    }

    rdpvxflag[i]|=VX_INITDONE;
}

void g_loadvtx(dword addr,int v0,int vn)
{
    int     i,flag;
    int     itex,ipos1,ipos2;
    float   x,y,z,l;
    Vertex *v;
    float  *m=gst.xform;

    st2.gfx_vxin+=4;

    if(gst.newmatrices)
    {
        g_mtxxform();
    }
    if(gst.lightvx && gst.lightnumchanged)
    {
        gst.lightnumchanged=0;
        g_xformlights();
    }

    rdp_newvtx(v0,vn);

    for(i=v0;i<v0+vn;i++)
    {
        v=rdpvx[i];
        flag=0;

        // read raw data & extract color
        ipos1  =mem_read32p(addr+0);
        ipos2  =mem_read32p(addr+4);
        itex   =mem_read32p(addr+8);
        v->icol=mem_read32p(addr+12);
        addr+=16;

        if(st.dumpgfx)
        {
            float fu,fv;
            fu=(itex>>16)/32.0;
            fv=((itex<<16)>>16)/32.0;
            logd("\nlvx[%02i: xyz %04X %04X %04X fl %04X uv %04X %04X rgba %08X uvf %7.2f %7.2f]",
                i,
                (ipos1>>16)&0xffff,
                ipos1&0xffff,
                (ipos2>>16)&0xffff,
                ipos2&0xffff,
                (itex>>16)&0xffff,
                itex&0xffff,
                v->icol,
                fu,fv);
        }

        // extract texture coordinates
        if(gst.texgen==1)
        {
            float  nor[4];
            float s,t,u;
            // texgen generation for highlights, pretty slow but seldom used
            // get normal
            nor[0]=((char)((v->icol>>24)&255))*(1.0/128.0);
            nor[1]=((char)((v->icol>>16)&255))*(1.0/128.0);
            nor[2]=((char)((v->icol>>8 )&255))*(1.0/128.0);
            // calc rotated normal
            s=nor[0]*m[0+0*4]+nor[1]*m[0+1*4]+nor[2]*m[0+2*4];
            t=nor[0]*m[1+0*4]+nor[1]*m[1+1*4]+nor[2]*m[1+2*4];
            u=nor[0]*m[2+0*4]+nor[1]*m[2+1*4]+nor[2]*m[2+2*4];
            l=0.4/sqrt(s*s+t*t+u*u);
            s=(0.4+s*l);
            t=(0.4+t*l);
            s=s*32768.0;
            t=t*32768.0;
            v->tex[0]=s;
            v->tex[1]=t;
        }
        else
        {
            v->tex[0]=(float)((itex    )>>16);
            v->tex[1]=(float)((itex<<16)>>16);
        }

        // extract position
        x=+(float)((ipos1    )>>16);
        y=+(float)((ipos1<<16)>>16);
        z=+(float)((ipos2    )>>16);

        // transform
        v->pos[0]=x*m[0+0*4]+y*m[0+1*4]+z*m[0+2*4]+m[0+3*4];
        v->pos[1]=x*m[1+0*4]+y*m[1+1*4]+z*m[1+2*4]+m[1+3*4];
        v->pos[2]=x*m[3+0*4]+y*m[3+1*4]+z*m[3+2*4]+m[3+3*4]; // from W!!

        // clipcheck
        if(v->pos[0]<-v->pos[2]) flag|=VX_CLIPX1;
        if(v->pos[0]>+v->pos[2]) flag|=VX_CLIPX2;
        if(v->pos[1]<-v->pos[2]) flag|=VX_CLIPY1;
        if(v->pos[1]>+v->pos[2]) flag|=VX_CLIPY2;

        if(st.dumpgfx)
        {
            logd(" screen %.3f %.3f %.3f clip %02X ",
                v->pos[0],
                v->pos[1],
                v->pos[2],
                flag);
        }

        rdpvxflag[i]=flag;
    }
}

void g_loadvtx_diddly(dword addr,int v0,int vn)
{
    int     i,flag;
    int     itex,ipos1,ipos2;
    float   x,y,z;
    Vertex *v;
    float  *m=gst.xform;

    st2.gfx_vxin+=4;

    if(gst.newmatrices)
    {
        g_mtxxform();
    }
    if(gst.lightvx && gst.lightnumchanged)
    {
        gst.lightnumchanged=0;
        g_xformlights();
    }

    rdp_newvtx(v0,vn);

    for(i=v0;i<v0+vn;i++)
    {
        v=rdpvx[i];
        flag=0;

        // read raw data & extract color
        ipos1  =(mem_read16(addr+0)<<16)|mem_read16(addr+2);
        ipos2  = mem_read16(addr+4)<<16;
        itex   =0;
        v->icol=(mem_read16(addr+6)<<16)|mem_read16(addr+8);
        addr+=10;

        if(st.dumpgfx)
        {
            logd("\nlvx[%02i: xyz %04X %04X %04X rgba %08X diddly]",
                i,
                (ipos1>>16)&0xffff,
                ipos1&0xffff,
                (ipos2>>16)&0xffff,
                v->icol);
        }

        // extract position
        x=+(float)((ipos1    )>>16);
        y=+(float)((ipos1<<16)>>16);
        z=+(float)((ipos2    )>>16);

        // transform
        v->pos[0]=x*m[0+0*4]+y*m[0+1*4]+z*m[0+2*4]+m[0+3*4];
        v->pos[1]=x*m[1+0*4]+y*m[1+1*4]+z*m[1+2*4]+m[1+3*4];
        v->pos[2]=x*m[3+0*4]+y*m[3+1*4]+z*m[3+2*4]+m[3+3*4]; // from W!!

        // clipcheck
        if(v->pos[0]<-v->pos[2]) flag|=VX_CLIPX1;
        if(v->pos[0]>+v->pos[2]) flag|=VX_CLIPX2;
        if(v->pos[1]<-v->pos[2]) flag|=VX_CLIPY1;
        if(v->pos[1]>+v->pos[2]) flag|=VX_CLIPY2;

        if(st.dumpgfx)
        {
            logd(" screen %.3f %.3f %.3f clip %02X ",
                v->pos[0],
                v->pos[1],
                v->pos[2],
                flag);
        }

        rdpvxflag[i]=flag;
    }
}

int g_culltri(int *vxind)
{
    float xd1,yd1,zd1;
    float xd2,yd2,zd2;
    float bx,by,bz;
    float x,y,z,d;
    if(1)
    {
        bx =rdpvx[vxind[0]]->pos[0];
        by =rdpvx[vxind[0]]->pos[1];
        bz =rdpvx[vxind[0]]->pos[2];
        xd1=rdpvx[vxind[1]]->pos[0];
        yd1=rdpvx[vxind[1]]->pos[1];
        zd1=rdpvx[vxind[1]]->pos[2];
        xd2=rdpvx[vxind[2]]->pos[0];
        yd2=rdpvx[vxind[2]]->pos[1];
        zd2=rdpvx[vxind[2]]->pos[2];
        x=(yd1*zd2)-(yd2*zd1);
        y=(zd1*xd2)-(zd2*xd1);
        z=(xd1*yd2)-(xd2*yd1);
        d=x*bx+y*by+z*bz;
    }
    else
    {
        bx =rdpvx[vxind[0]]->pos[0];
        by =rdpvx[vxind[0]]->pos[1];
        bz =rdpvx[vxind[0]]->pos[2];
        xd1=rdpvx[vxind[1]]->pos[0]-bx;
        yd1=rdpvx[vxind[1]]->pos[1]-by;
        zd1=rdpvx[vxind[1]]->pos[2]-bz;
        xd2=rdpvx[vxind[2]]->pos[0]-bx;
        yd2=rdpvx[vxind[2]]->pos[1]-by;
        zd2=rdpvx[vxind[2]]->pos[2]-bz;
        x=(yd1*zd2)-(yd2*zd1);
        y=(zd1*xd2)-(zd2*xd1);
        z=(xd1*yd2)-(xd2*yd1);
        d=x*bx+y*by+z*bz;
    }
    if((gst.cull&1) && d<=0) return(1);
    if((gst.cull&2) && d>=0) return(1);
    return(0);
}

void g_tri(int flag,int *vxind)
{
    int clipmask;

    st2.gfx_trisin++;

    if(st.dumpgfx)
    {
        logd(" (%i,%i,%i)",vxind[0],vxind[1],vxind[2]);
    }
    // check visibility
    clipmask =VX_CLIPALL;
    clipmask&=rdpvxflag[vxind[0]];
    clipmask&=rdpvxflag[vxind[1]];
    clipmask&=rdpvxflag[vxind[2]];
    if(clipmask)
    {
        if(st.dumpgfx) logd(" Cl%02X",clipmask);
        return;
    }

    // check culling
    if(gst.cull && g_culltri(vxind))
    {
        if(st.dumpgfx) logd(" Cull");
        return;
    }

    // init vertices (if not inited)
    if(!(rdpvxflag[vxind[0]]&VX_INITDONE)) g_initvx(vxind[0]);
    if(!(rdpvxflag[vxind[1]]&VX_INITDONE)) g_initvx(vxind[1]);
    if(!(rdpvxflag[vxind[2]]&VX_INITDONE)) g_initvx(vxind[2]);

    // send triangle to drawpipe
    rdp_tri(vxind);
    gst.cnt_tri++;
}

int g_culldl(int v0,int vn)
{
    int i,m=VX_CLIPALL;
    for(i=v0;i<vn;i++) m&=rdpvxflag[i];
    if(m) return(1); // hidden
    else return(0);
}

/***********************************************************************/
// display list matrix commands

void g_resetmtx(void)
{
    gst.mtxstackp[0]=0;
    gst.mtxstackp[1]=0;
    memcpy(gst.mtx[0][0],identity,16*sizeof(float));
    memcpy(gst.mtx[1][0],identity,16*sizeof(float));
}

float testmat[16]={
 1.0,   0.0,   0.0,   0.0,
 0.0,  -1.0,   0.0,   0.0,
 0.0,   0.0,   1.0,   1.0,
-1.0,   1.0,   0.0, 500.0,
};

void g_mtxxform(void)
{
    if(!gst.newmatrices) return;
    gst.newmatrices=0;

    if(gst.cammoveset)
    {
        gst.mtx[0][gst.mtxstackp[0]][3*4+0]+=gst.cammove[0];
        gst.mtx[0][gst.mtxstackp[0]][3*4+1]+=gst.cammove[1];
        gst.mtx[0][gst.mtxstackp[0]][3*4+2]+=gst.cammove[2];
    }

    if(cart.dlist_diddlyvx)
    {
        float tmp[16];
        memcpy(gst.xform,gst.mtx[0][1],16*4);
        /*
        MultMatrix(tmp,
                   gst.mtx[0][2],
                   gst.mtx[0][1]);
        MultMatrix(gst.xform,
                   gst.mtx[0][1],
                   identity);
        */
        /*
        MultMatrix(gst.xform,
                   gst.mtx[0][1],
                   gst.mtx[0][2]);
        */
    }
    else
    {
        MultMatrix(gst.xform,
                   gst.mtx[0][gst.mtxstackp[0]],  // modelview
                   gst.mtx[1][gst.mtxstackp[1]]); // projection
    }

    if(gst.cammoveset)
    {
        gst.mtx[0][gst.mtxstackp[0]][3*4+0]-=gst.cammove[0];
        gst.mtx[0][gst.mtxstackp[0]][3*4+1]-=gst.cammove[1];
        gst.mtx[0][gst.mtxstackp[0]][3*4+2]-=gst.cammove[2];
    }

    if(st.dumpgfx)
    {
        if(!cart.dlist_diddlyvx)
        {
            logd("\n{ Active Projection Matrix [%i]:",gst.mtxstackp[1]);
            DumpMatrix(gst.mtx[1][gst.mtxstackp[1]]);
            logd("\n{ Active Modelview Matrix [%i]:",gst.mtxstackp[0]);
            DumpMatrix(gst.mtx[0][gst.mtxstackp[0]]);
        }
        logd("\n{ Active Xform:");
        DumpMatrix(gst.xform);
    }
}

void g_loadmtx(dword pos,int proj,int load,int push)
{
    ushort sh[32];
    float  m[16],m2[16];
    int    mi[16];
    int    i,x;

    proj=(proj!=0);
    if(load) logd(" Load:"); else logd(" Mul:");
    if(proj) logd("Prj ");  else logd("Mod ");
    if(push) logd("Push ");

    gst.newmatrices=1;
    gst.lightnumchanged=1;

    for(i=0;i<32;i+=2)
    {
        x=mem_read32p(pos+i*2);
        sh[i+0]=x>>16;
        sh[i+1]=x;
    }

    for(i=0;i<16;i++)
    {
        mi[i]=((int)sh[i]<<16) | (int)sh[i+16];
        m[i]=(float)(mi[i])*(1.0/65536.0);
    }

    /*
    if(gst.mtxstackp[proj]>1)
    {
        int x,y;
        print("4x %08X\n",pos);
        for(y=0;y<3;y++) for(x=0;x<3;x++)
        {
            m[x+y*4]*=4.0;
        }
    }
    */

    if(st.dumpgfx)
    {
        if(load) logd("\nLoad matrix(%i):",proj);
        else     logd("\nMul matrix(%i):",proj);
        if(push) logd(" (push)");
        DumpMatrix(m);
    }
    /*
    if(proj)
    {
        print("-proj-%08X-\n",pos);
        PrintMatrix(m);
    }
    */

    if(push)
    {
        gst.mtxstackp[proj]++;
        if(gst.mtxstackp[proj]>15)
        {
            gst.mtxstackp[proj]--;
            error("dlist: matrix stack overflow");
        }
        memcpy(gst.mtx[proj]+gst.mtxstackp[proj],
               gst.mtx[proj]+gst.mtxstackp[proj]-1,16*sizeof(float));
    }

    if(load)
    {
        memcpy(gst.mtx[proj]+gst.mtxstackp[proj],m,16*sizeof(float));
    }
    else
    {
        memcpy(m2,gst.mtx[proj][gst.mtxstackp[proj]],16*sizeof(float));
        MultMatrix(gst.mtx[proj][gst.mtxstackp[proj]],m,m2);
    }
}

void g_popmtx(int num)
{
    int proj=0;
    if(st.dumpgfx) logd("(stackp=%i) ",gst.mtxstackp[0]);
    gst.newmatrices=1;
    gst.mtxstackp[proj]--;
    if(gst.mtxstackp[proj]<0)
    {
        gst.mtxstackp[proj]=0;
        error("dlist: matrix stack underflow");
    }
}

/***********************************************************************/

static struct
{
    int   cmd;
    char *name;
} cmdnames[]={
/* RDP commands: */
{0xff,"G_SETCIMG"},
{0xfe,"G_SETZIMG"},
{0xfd,"G_SETTIMG"},
{0xfc,"G_SETCOMBINE"},
{0xfb,"G_SETENVCOLOR"},
{0xfa,"G_SETPRIMCOLOR"},
{0xf9,"G_SETBLENDCOLOR"},
{0xf8,"G_SETFOGCOLOR"},
{0xf7,"G_SETFILLCOLOR"},
{0xf6,"G_FILLRECT"},
{0xf5,"G_SETTILE"},
{0xf4,"G_LOADTILE"},
{0xf3,"G_LOADBLOCK"},
{0xf2,"G_SETTILESIZE"},
{0xf1,"G_RDPHALF_RDP"},
{0xf0,"G_LOADTLUT"},
{0xef,"G_RDPSETOTHERMODE"},
{0xee,"G_SETPRIMDEPTH"},
{0xed,"G_SETSCISSOR"},
{0xec,"G_SETCONVERT"},
{0xeb,"G_SETKEYR"},
{0xea,"G_SETKEYGB"},
{0xe9,"G_RDPFULLSYNC"},
{0xe8,"G_RDPTILESYNC"},
{0xe7,"G_RDPPIPESYNC"},
{0xe6,"G_RDPLOADSYNC"},
{0xe5,"G_TEXRECTFLIP"},
{0xe4,"G_TEXRECT"},
/* ZELDA RSP commands: */
{0xe3,"Zelda_SETOTHERMODEH"},
{0xe2,"Zelda_SETOTHERMODEL"},
{0xe1,"Zelda_LOADRSPDATA?"},          // LOAD RSP-DATA TO 0x000
{0xe0,"Zelda_???118c"},
{0xdf,"Zelda_ENDDL"},
{0xde,"Zelda_DL"},
{0xdd,"Zelda_LOADRSPCODE?"},          // LOAD RSP-CODE TO 0x080
{0xdc,"Zelda_MOVEMEM"},
{0xdb,"Zelda_MOVEWORD"},
{0xda,"Zelda_LOADMTX"},
{0xd9,"Zelda_SETGEOMETRYMODE"},
{0xd8,"Zelda_POPMTX"},
{0xd7,"Zelda_TEXTURE"},
{0xd6,"Zelda_???11b8"},
{0xd5,"Zelda_???118c"},
{0xd4,"Zelda_???118c"},
{0xd3,"Zelda_???118c"},
{0xd2,"Zelda_???1188"},
{0xd1,"Zelda_???1078"},
{0xd0,"Zelda_???1188"},
/* RDP triangles */
{0xcf,"RAWTRI_SHADE_TXTR_ZBUF"},
{0xce,"RAWTRI_SHADE_TXTR"},
{0xcd,"RAWTRI_SHADE_ZBUF"},
{0xcc,"RAWTRI_SHADE"},
{0xcb,"RAWTRI_TXTR_ZBUF"},
{0xca,"RAWTRI_TXTR"},
{0xc9,"RAWTRI_FILL_ZBUF"},
{0xc8,"RAWTRI_FILL"},
{0xc7,"RAWTRI_EDGE_SHADE_TXTR_ZBUF"},
{0xc6,"RAWTRI_EDGE_SHADE_TXTR"},
{0xc5,"RAWTRI_EDGE_SHADE_ZBUF"},
{0xc4,"RAWTRI_EDGE_SHADE"},
{0xc3,"RAWTRI_EDGE_TXTR_ZBUF"},
{0xc2,"RAWTRI_EDGE_TXTR"},
{0xc1,"RAWTRI_EDGE_FILL_ZBUF"},
{0xc0,"G_NOOP"},
/* IMMEDIATE commands: */
{0xbf,"Zelda_?16f (tri1?)"},
{0xbe,"Zelda_?1018"},
{0xbd,"Zelda_?0"},
{0xbc,"Zelda_?1000"},
{0xbb,"Zelda_?97"},
{0xba,"Zelda_?f80"},
/* IMMEDIATE commands: */
{0xbf,"G_TRI1"},
{0xbe,"G_CULLDL"},
{0xbd,"G_POPMTX"},
{0xbc,"G_MOVEWORD"},
{0xbb,"G_TEXTURE"},
{0xba,"G_SETOTHERMODE_H"},
{0xb9,"G_SETOTHERMODE_L"},
{0xb8,"G_ENDDL"},
{0xb7,"G_SETGEOMETRYMODE"},
{0xb6,"G_CLEARGEOMETRYMODE"},
{0xb5,"G_LINE3D"},
{0xb4,"G_RDPHALF_1"},
{0xb3,"G_RDPHALF_2"},
{0xb2,"G_RDPHALF_CONT"},
{0xb1,"G_TRI2 (WAVE)"},
/* Lowops Zelda */
{0x0A,"Zelda_BACKGROUND"},
{0x09,"Zelda_???"},
{0x08,"Zelda_???118c"},
{0x07,"Zelda_TRI3"},
{0x06,"Zelda_TRI2"},
{0x05,"Zelda_TRI1"},
{0x04,"Zelda_DLINMEM"},
{0x03,"Zelda_CULLDL"},
{0x02,"Zelda_???1c74"},
{0x01,"Zelda_LOADVTX"},
{0x00,"Zelda_0"},
/* DMA */
{0x09,"G_SPRITE2D"},
{0x07,"G_DLINMEM"},
//{0x08,"G_RESERVED3"},
//{0x07,"G_RESERVED2"},
{0x06,"G_DL"},
{0x05,"G_DMATRI"},
{0x04,"G_VTX"},
{0x03,"G_MOVEMEM"},
//{0x02,"G_RESERVED0"},
{0x01,"G_MTX"},
{0x00,"G_SPNOOP"},
0,NULL};

void dumpcmd(dword addr,dword *cmd)
{
    int j;
    int c=cmd[0]>>24;

    logd("%08X: %08X %08X CMD ",dlpnt,cmd[0],cmd[1]);
    if(cart.iszelda)
    {
        for(j=0;cmdnames[j].name;j++)
        {
            if(cmdnames[j].cmd==c) break;
        }
    }
    else
    {
        for(j=0;cmdnames[j].name;j++)
        {
            if(cmdnames[j].cmd==c && *cmdnames[j].name!='Z') break;
        }
    }
    if(!cmdnames[j].name) logd("??? ");
    else logd("%s ",cmdnames[j].name);
}

/***********************************************************************/

static __inline dword address(dword address)
{ // segment convert to physical address
    int seg=(address>>24)&0x3f;
    if(seg>15)
    {
        logd("\nERROR Segment > 15\n");
        error("dlist: segment > 15");
        seg=0;
    }
    return( (address&0xffffff) + gst.segment[seg] );
}

/***********************************************************************/

void dump_geom(dword x)
{
    logd("\n*mode geometry: %08X ",x);

    logd(" texgen(%i)",gst.texgen);
    logd(" flat(%i)",gst.flat);
    logd(" cull(%i)",gst.cull);
    logd(" light(%i)",gst.lightvx);
}

void change_geom(dword x)
{
    if(cart.dlist_zelda==1)
    {
        gst.cull   =(x>>9)&3;
        gst.lightvx=(x&0x20000)?1:0; // 10000
        gst.texgen =0;
        gst.flat   =(x&0x80000)?1:0; //††
        if(!(x&4)) gst.flat=0;
        rdp_flat(gst.flat);
    }
    else
    {
        gst.cull   =(x&0x03000)>>12;
        gst.lightvx=(x&0x20000)?1:0;
        gst.texgen =(x&0xc0000)>>18;
        gst.flat   =0;
    }
    if(gst.lastframehadbackground && !gst.framehadbackground)
    {
        gst.lightvx=0;
    }
}

/***********************************************************************/
// command helpers

void c_dlbranch(dword a,int branch,int limit)
{
    dword addr=address(a);
    if(limit) dllimit=limit+1;
    logd(" Displaylist at %08X (stackp %i, limit %i)",addr,stackp,limit);
    if(branch)
    { // branch
        logd(" (branch)");
        dlpnt=addr;
    }
    else
    { // call
        stack[stackp++]=dlpnt;
        dlpnt=addr;
    }
}

void c_dlend(void)
{
    dlpnt=stack[--stackp];
    logd(" Displaylist end (stackp %i)\n",stackp);
}

void c_dmavtx(dword a,int v0,int vn)
{
    dword addr=address(a);

    logd(" Vertex %02i..%02i at %08X",v0,v0+vn-1,addr);
    if(v0+vn>MAXVX || vn>MAXVX)
    {
        logd(" ERROR v0=%i vn=%i ",v0,vn);
        error("dlist: invalid vertex load");
        return;
    }

    if(cart.dlist_diddlyvx) g_loadvtx_diddly(addr,v0,vn);
    else g_loadvtx(addr,v0,vn);

    gst.cnt_vtx+=vn;
    if(!gst.newvertices)
    {
        gst.newvertices=1;
        gst.newvertices_lo=v0;
        gst.newvertices_hi=vn+v0-1;
    }
    else
    {
        int a;
        a=v0;
        if(a<gst.newvertices_lo) gst.newvertices_lo=a;
        if(a>gst.newvertices_hi) gst.newvertices_hi=a;
        a=v0+vn-1;
        if(a<gst.newvertices_lo) gst.newvertices_lo=a;
        if(a>gst.newvertices_hi) gst.newvertices_hi=a;
    }
}

void c_dmatri_diddly(dword a,int vn)
{
    dword addr=address(a);
    int min=999,max=-999;

    gst.texenable=1;
    gst.scales=1.0;
    gst.scalet=1.0;

    logd(" Triangles %i at %08X",vn,addr);
    while(vn-->0)
    {
        dword iw;
        int vxind[3];
        iw=mem_read32(addr+0);
        vxind[2]=(iw>>0)&255;
        vxind[1]=(iw>>8)&255;
        vxind[0]=(iw>>16)&255;
        if(vxind[0]>MAXRDPVX) return;
        if(vxind[1]>MAXRDPVX) return;
        if(vxind[2]>MAXRDPVX) return;
        logd("\ntri[ %i %i %i ]",vxind[0],vxind[1],vxind[2]);
        // setup texcoords
        iw=mem_read32(addr+4);
        rdpvx[vxind[0]]->tex[0]=(short)(iw>>16);
        rdpvx[vxind[0]]->tex[1]=(short)(iw);
        iw=mem_read32(addr+8);
        rdpvx[vxind[1]]->tex[0]=(short)(iw>>16);
        rdpvx[vxind[1]]->tex[1]=(short)(iw);
        iw=mem_read32(addr+12);
        rdpvx[vxind[2]]->tex[0]=(short)(iw>>16);
        rdpvx[vxind[2]]->tex[1]=(short)(iw);
        // draw
        g_tri(0,vxind);
        addr+=16;
/*
        {
            int i;
            for(i=0;i<3;i++)
            {
                if(vxind[i]<min) min=vxind[i];
                if(vxind[i]>max) max=vxind[i];
            }
        }
*/
    }
//    logd("\nG_VTX-DMATRI-Range %i..%i",min,max);
}

void c_dmamtx(dword a,int proj,int load,int push)
{
    dword addr=address(a);
    logd(" {Matrix} at %08X ",addr);
    g_loadmtx(addr,proj,load,push);
    gst.cnt_mtx++;
}

void c_dmamtx_diddly(dword a,int ind)
{
    dword addr=address(a);
    logd(" {Matrix} at %08X ind %i ",addr,ind);
    gst.mtxstackp[0]=ind;
    g_loadmtx(addr,0,1,0);
    gst.cnt_mtx++;
}

void c_setgeommode(int mode,dword bits)
{
    if(mode==2)   gst.gmode&=bits;  // 2=AND
    else if(mode) gst.gmode|=bits;  // 1=OR
    else          gst.gmode&=~bits; // 2=AND NOT

    if(mode!=2)
    {
        change_geom(gst.gmode);
        if(st.dumpgfx) dump_geom(gst.gmode);
    }
}

void c_setothermode(int hi,dword *cmd)
{
    int pos,bits,mask,data;

    pos=(cmd[0]>>8)&31;
    bits=(cmd[0])&31;
    mask=((1<<bits)-1)<<pos;
    data=cmd[1];

    if(hi)
    {
        gst.omodeh&=~mask;
        gst.omodeh|=data;
    }
    else
    {
        gst.omodel&=~mask;
        gst.omodel|=data;
    }

    // generate RDP othermode change command
    cmd[0]=0xef000000+(gst.omodeh&0xffffff);
    cmd[1]=gst.omodel;
    rdp_cmd(cmd);
}

void c_movemem(int ind,dword a)
{
    dword addr=address(a);
    int i;

    logd(" Movemem[%04X] <- %08X",ind,a);

    if(ind>=0x86 && ind<=0x94)
    {
        i=(ind-0x86)/2;
        g_loadlight(i,addr);
    }
    else if(ind==0x080)
    { // viewport
        g_viewport(addr);
    }

    logd("\ndata(%08X): ",addr);
    for(i=0;i<4;i++)
    {
        logd("%08X ",mem_read32p(addr+i*4));
    }
}

void c_movemem_zelda(int ind,dword a)
{
    dword addr=address(a);
    int i;

    logd(" Movemem[%04X] <- %08X",ind,a);

    if(ind>=0x0806 && ind<=0x0806+3*8)
    {
        i=(ind-0x0806)/3;
        g_loadlight(i,addr);
    }
    else if(ind==0x0800 && (cmd[0]&0xffffff)==0x080008)
    { // viewport
        g_viewport(addr);
    }

    logd("\ndata(%08X): ",addr);
    for(i=0;i<4;i++)
    {
        logd("%08X ",mem_read32p(addr+i*4));
    }
}

void c_moveword(int bank,int index,dword x)
{
    logd(" Mem[%i][%02X]=%08X",bank,index,x);
    index>>=2;
    if(bank==0x6)
    {
        logd(" Segment[%i]=%08X",index,x);
        gst.segment[index]=x;
        rdp_segment(index,x);
    }
    else if(bank==0x8 && index==0)
    {
        int a,b,min,max;
        float scalen,scalef;
        // fog
        a=(x>>24)&255;
        b=(x>>8)&255;

        min=b-a;
        max=b+a;
        gst.fognear=min/256.0;
        gst.fogfar=max/256.0;

        logd(" Fogrange %.3f..%.3f zmax=%f",gst.fognear,gst.fogfar,gst.zmax);
        if(cart.iszelda)
        {
            scalen=1024.0;
            scalef=4096.0;
        }
        else
        {
            scalen=16384;
            scalef=16384;
        }
        rdp_fogrange(gst.fognear*scalen,gst.fogfar*scalef);
    }
    else if(bank==0x2 && index==0)
    {
        int lightnum;
        if(cart.dlist_zelda==1)
        { // zelda
            lightnum=(cmd[1]&0xfff)/16-1;
        }
        else
        {
            lightnum=(cmd[1]&0xfff)/32-1;
        }
        g_lightnum(lightnum);
    }
    else
    {
        logd(" !skipmoveword");
    }
}

void c_texture(dword *cmd)
{
    int bowtie,level,tile,on,s,t;
    bowtie=FIELD(cmd[0],16,8);
    level=FIELD(cmd[0],11,3);
    tile=FIELD(cmd[0],8,3);
    on=FIELD(cmd[0],0,8);
    s=FIELD(cmd[1],16,16);
    t=FIELD(cmd[1],0,16);
    gst.scales=s*(1.0/65536.0);
    gst.scalet=t*(1.0/65536.0);
    gst.texenable=on;
    logd(" texture on=%i tile=%i s=%4.2f t=%4.2f",
        on,tile,gst.scales,gst.scalet);
    rdp_texture(on,tile,level);
}

void c_zeldabackground(dword addr)
{
    dword wid,hig,base;
    static int saved=0;
    wid=mem_read32p(addr)/4;
    hig=mem_read32p(addr+8)/4;
    base=mem_read32p(addr+16);
    base=address(base);
    logd("\n+background: %08X size(%i,%i)",base,wid,hig);
//    print("Background: %08X size(%i,%i)\n",base,wid,hig);

    /*
    if(!saved)
    {
        FILE *f1;
        int i;
        dword x;
        f1=fopen("bg.dat","wb");
        for(i=0;i<320*240*4;i+=4)
        {
            x=mem_read32p(i+base);
            x=FLIP32(x);
            fwrite(&x,1,4,f1);
        }
        fclose(f1);
        saved=1;
    }
    */

    rdp_copybackground(base,wid,hig);
}

/***********************************************************************/

void dlisterror(void)
{
    int c=(cmd[0]>>24);
    errors++;
    gst.errorcnt[c]++;
    if(gst.errorcnt[c]<10)
    {
        error("dlist: unknown command %08X %08X at %08X",cmd[0],cmd[1],dlpnt);
    }
}

void rsp_cmd_basic(int c)
{
    switch(c)
    {
    case 0x00:
        {
            static int zerocnt=0;
            zerocnt++;
            if(zerocnt>4)
            {
                zerocnt=0;
                warning("dlist: unexpected stream of nops, aborting list.");
                c_dlend();
            }
        }
        break;
//------------------------------------------ vertices
    case 0x05: // DMA triangles
        {
            int vn;
            vn=(cmd[0]&0xfff)/16+1;
            c_dmatri_diddly(cmd[1],vn);
            cart.dlist_diddlyvx=1;
        }
        break;
//------------------------------------------ vertices
    case 0x04: // DMA vertex vtx
        {
            int v0,vn;
            if(cart.dlist_diddlyvx)
            {
                /*
                int a,b;
                a=cmd[0]&0xfff;
                b=(cmd[0]>>12)&0xfff;
                logd(" #(%i,%i,%f) ",a,b,(float)b/(float)a);
                */
                v0=0;
                vn=((cmd[0]&0xfff)-8)/18+1; //††
                if(vn>64) vn=64;
            }
            else if(!cart.dlist_wavevx)
            {
                v0=((cmd[0]>>16)&0xff)>>1;
                vn=((cmd[0]>> 8)&0xff)>>2;
                if(!vn) cart.dlist_wavevx=1;
            }
            else
            {
                v0=((cmd[0]>>16)&0xf);
                vn=((cmd[0]>>20)&0xf)+1;
            }
            c_dmavtx(cmd[1],v0,vn);
        }
        break;
//------------------------------------------ matrices
    case 0x01: // DMA matrix
        {
            int pos,load,push,proj,a;
            pos=address(cmd[1]);
            if(cart.dlist_diddlyvx)
            {
                a=((cmd[0]>>20)&0xf);
                proj=!(a&0x8);
                load=!(a&0x4);
                push=0;
                c_dmamtx_diddly(pos,a/4);
            }
            else
            {
                a=(cmd[0]>>16)&0xf;
                proj=(a&0x1);
                load=(a&0x2);
                push=(a&0x4);
                c_dmamtx(pos,proj,load,push);
            }
        }
        break;
    case 0xBD: // POPMATRIX
        g_popmtx(cmd[1]);
        if(st.dumpgfx) logd("{matrix}");
        break;
//------------------------------------------ drawing
    case 0xB5: // QUAD (mariokart)
        {
            int vxind2[4],i;
            int vxind[4];
            if(cart.dlist_wavevx)
            {
                for(i=0;i<4;i++)
                {
                    vxind2[i]=( (cmd[1]>>(24-i*8)) &255)/10;
                }
            }
            else
            { // waverace
                for(i=0;i<4;i++)
                {
                    vxind2[i]=( (cmd[1]>>(24-i*8)) &255) >> 1;
                }
            }

            vxind[0]=vxind2[3];
            vxind[1]=vxind2[2];
            vxind[2]=vxind2[1];
            vxind[3]=vxind2[0];

            g_tri(0,vxind);
            vxind[1]=vxind[2];
            vxind[2]=vxind[3];
            g_tri(0,vxind);
        }
        break;
    case 0xBF: // TRI1
        {
            int vxind[3],i;
            if(cart.dlist_wavevx)
            {
                for(i=0;i<3;i++) vxind[i]=( (cmd[1]>>(i*8)) &255)/10;
            }
            else
            { // waverace
                for(i=0;i<3;i++) vxind[i]=( (cmd[1]>>(i*8)) &255) >> 1;
            }
            g_tri(0,vxind);
        }
        break;
    case 0xB1: // TRI2
        {
            int vxind[3];
            if(cart.dlist_geyevx || (cmd[1]>>24))
            { // goldeneye
                cart.dlist_geyevx=1;

                vxind[1]=((cmd[1]>> 0)&15);
                vxind[0]=((cmd[1]>> 4)&15);
                if(vxind[0]|vxind[1])
                {
                    vxind[2]=((cmd[0]>> 0)&15);
                    g_tri(0,vxind);
                }

                vxind[1]=((cmd[1]>> 8)&15);
                vxind[0]=((cmd[1]>>12)&15);
                if(vxind[0]|vxind[1])
                {
                    vxind[2]=((cmd[0]>> 4)&15);
                    g_tri(0,vxind);
                }

                vxind[1]=((cmd[1]>>16)&15);
                vxind[0]=((cmd[1]>>20)&15);
                if(vxind[0]|vxind[1])
                {
                    vxind[2]=((cmd[0]>> 8)&15);
                    g_tri(0,vxind);
                }

                vxind[1]=((cmd[1]>>24)&15);
                vxind[0]=((cmd[1]>>28)&15);
                if(vxind[0]|vxind[1])
                {
                    vxind[2]=((cmd[0]>>12)&15);
                    g_tri(0,vxind);
                }
            }
            else
            {
                vxind[0]=((cmd[0]>> 0)&127) >> 1;
                vxind[1]=((cmd[0]>> 8)&127) >> 1;
                vxind[2]=((cmd[0]>>16)&127) >> 1;
                g_tri(0,vxind);
                vxind[0]=((cmd[1]>> 0)&127) >> 1;
                vxind[1]=((cmd[1]>> 8)&127) >> 1;
                vxind[2]=((cmd[1]>>16)&127) >> 1;
                g_tri(0,vxind);
            }
        }
        break;
//------------------------------------------ textures
    case 0xBB: // TEXTURE
        c_texture(cmd);
        break;
//-------------------------------------- displaylists
    case 0x06: // DL DMA displaylist
        c_dlbranch(cmd[1],cmd[0]&0x10000,0);
        break;
    case 0xB8: // DLEND
        c_dlend();
        break;
    case 0x07: // DLINMEM
        {
            c_dlbranch(address(cmd[1]),0,(cmd[0]>>16)&255);
        } break;
//-------------------------------------- settings
    case 0xB6: // Geometrymode
    case 0xB7: // Geometrymode
        c_setgeommode(c==0xb7,cmd[1]);
        break;
    case 0xB9: // Othermode_l
        c_setothermode(0,cmd);
        break;
    case 0xBA: // Othermode_H
        c_setothermode(1,cmd);
        break;
    case 0x03: // MOVEMEM
        {
            int v0;
            v0=(cmd[0]>>16)&0xff;
            c_movemem(v0,cmd[1]);
        }
        break;
    case 0xBC: // MOVEWORD
        {
            int ind=(cmd[0]>>8)&255;
            int m=(cmd[0]>>0)&15;
            c_moveword(m,ind,cmd[1]);
        }
        break;
    default:
        if((c>0x10 && c<0xa0) || !c) dlisterror();
        logd("!skip");
        break;
    }
}

void rsp_cmd_zelda(int c)
{
    switch(c)
    {
//------------------------------------------ vertices
    case 0x00:
        {
            static int zerocnt=0;
            zerocnt++;
            if(zerocnt>4)
            {
                zerocnt=0;
                warning("dlist: unexpected stream of nops, aborting list.");
                c_dlend();
            }
        }
        break;
    case 0x01: // loadvtx
        {
            int v0,vn;
            v0=0;
            vn=((cmd[0]&0xff000)>>12);
            v0=((cmd[0]&0x000ff)>>1)-vn;
            c_dmavtx(cmd[1],v0,vn);
        }
        break;
    case 0x03: // culling
        {
            int v0,vn;
            v0=(cmd[0]&0xff)>>1;
            vn=1+((cmd[1]&0xff)>>1);
            if(g_culldl(v0,vn))
            {
                if(st.dumpgfx) logd("\n+culling result: hidden ");
                c_dlend();
            }
            else
            {
                if(st.dumpgfx) logd("\n+culling result: visible ");
            }
        }
        break;
//------------------------------------------ matrices
    case 0xda: // LOADMTX
        {
            int pos,load,push,proj,a;
            a=cmd[0]&15;
            proj= (a&4);
            load= (a&2);
            push=!(a&1);
            pos=address(cmd[1]);
            c_dmamtx(pos,proj,load,push);
        }
        break;
    case 0xd8: // POPMTX
        g_popmtx(cmd[1]);
        break;
//------------------------------------------ drawing
    case 0x05: // ZELDA_TRI1
        {
            int vxind[3];
            vxind[0]=((cmd[0]>> 0)&127) >> 1;
            vxind[1]=((cmd[0]>> 8)&127) >> 1;
            vxind[2]=((cmd[0]>>16)&127) >> 1;
            g_tri(0,vxind);
        }
        break;
    case 0x06: // ZELDA_TRI2
        {
            int vxind[3];
            vxind[0]=((cmd[0]>> 0)&127) >> 1;
            vxind[1]=((cmd[0]>> 8)&127) >> 1;
            vxind[2]=((cmd[0]>>16)&127) >> 1;
            g_tri(0,vxind);
            vxind[0]=((cmd[1]>> 0)&127) >> 1;
            vxind[1]=((cmd[1]>> 8)&127) >> 1;
            vxind[2]=((cmd[1]>>16)&127) >> 1;
            g_tri(0,vxind);
        }
        break;
    case 0x07: // ZELDA_TRI3 // as TRI2, but no uvscale?
        {
            int vxind[3];
            vxind[0]=((cmd[0]>> 0)&127) >> 1;
            vxind[1]=((cmd[0]>> 8)&127) >> 1;
            vxind[2]=((cmd[0]>>16)&127) >> 1;
            g_tri(0,vxind);
            vxind[0]=((cmd[1]>> 0)&127) >> 1;
            vxind[1]=((cmd[1]>> 8)&127) >> 1;
            vxind[2]=((cmd[1]>>16)&127) >> 1;
            g_tri(0,vxind);
        }
        break;
//------------------------------------------ textures
    case 0xd7: // TEXTURE
        c_texture(cmd);
        break;
//------------------------------------------ displaylists
    case 0xde: // ZELDA_DL
        c_dlbranch(cmd[1],cmd[0]&0x10000,0);
        break;
    case 0x0a: // ZELDA_BACKGROUND
        c_zeldabackground(address(cmd[1]));
        gst.framehadbackground=1;
        break;
    case 0xdf: // ENDDL
        c_dlend();
        break;
    case 0xe1:
        gst.loadrspdata=address(cmd[1]);
        logd(" data at %08X\n",gst.loadrspdata);
        break;
    case 0x04:
        {
            c_dlbranch(gst.loadrspdata,0,0);
        } break;
//------------------------------------------ settings
    case 0xd9: // SETGEOMETRYMODE
        c_setgeommode(2,cmd[0]&0xffffff); // and
        c_setgeommode(1,cmd[1]&0xffffff); // or
        break;
    case 0xdc: // MOVEMEM
        {
            int v0;
            v0=(cmd[0]>>8)&0xffff;
            c_movemem_zelda(v0,cmd[1]);
        }
        break;
    case 0xdb: // MOVEWORD
        {
            int ind=(cmd[0])&255;
            int m=(cmd[0]>>16)&15;
            c_moveword(m,ind,cmd[1]);
        }
        break;
    case 0xE2: // Othermode_l
        c_setothermode(0,cmd);
        break;
    case 0xE3: // Othermode_H
        c_setothermode(1,cmd);
        break;
    default:
        if((c>0x10 && c<0xa0) || !c) dlisterror();
        logd("!skipz");
        break;
    }
}

/***********************************************************************/

void dlist_abort(void)
{
}

void dlist_execute(OSTask_t *task)
{
    int cmdnum,i,n,c,nopcount=0,errorcount=0;
    int vtxcnt=0,tricnt=0,cmdcnt=0;
    static int firsttime=1;
    int starttime;
    static int nosynccnt=0;

    if(gst.ignore)
    {
        os_event(OS_EVENT_SP);
        os_event(OS_EVENT_DP);
        return;
    }

    starttime=timer_us(&st2.timer);

    cmdnum=task->data_size/8;
    dllimit=0;

    if(st.dumpgfx)
    {
        print("displaylist-start\n"); flushdisplay();
    }

    logd("Displaylist:\n");
    logd("list: bootucode %5i bytes\n",task->ucode_boot_size);
    logd("list: ucode     %5i bytes (%08X)\n",task->ucode_size,task->m_ucode);
    logd("list: ucodedata %5i bytes\n",task->ucode_data_size);
    logd("list: dramstack %5i bytes\n",task->dram_stack_size);
    logd("list: data      %5i bytes (%08X)\n",task->data_size,task->m_data_ptr);
    logd("list: yielddata %5i bytes\n",task->yield_data_size);
    logd("list: %i commands:\n",cmdnum);
    logd("mode: zelda=%i diddly=%i wave=%i geye=%i\n",
        cart.dlist_zelda,
        cart.dlist_diddlyvx,
        cart.dlist_wavevx,
        cart.dlist_geyevx);

    if(firsttime && st.dumpgfx)
    {
        firsttime=0;
        disasm_dumpucode("rsp.log",
            task->m_ucode     ,task->ucode_size,
            task->m_ucode_data,task->ucode_data_size,
            cart.iszelda?0x1008:0x1080);
        logd("RSP microcode/data dumped to RSP.LOG\n");
    }

    if(cmdnum>50000)
    {
        error("dlist: display list too large (%i commands)\n",cmdnum);
        dlist_abort();
        return;
    }

    dlpnt=task->m_data_ptr;

    if(1)
    { // check start of cmdlist for errors
        if(cmdnum>16) n=16;
        else n=cmdnum;
        errorcount=0;
        for(i=0;i<n;i++)
        {
            cmd[0]=mem_read32p(dlpnt+i*8+0);
            cmd[1]=mem_read32p(dlpnt+i*8+4);
            c=cmd[0]>>24;
            if((c>0x10 && c<0xb0) || !c)
            {
                //error("dlist: unexpected command %08X %08X at %08X",cmd[0],cmd[1],dlpnt+i*8);
                errorcount++;
            }
        }
        if(errorcount>n/4)
        {
            error("dlist: display list doesn't look right (ecnt=%i/%i)",errorcount,n/4);
            dlist_abort();
            return;
        }
        errorcount=0;
    }

    if(cart.ismario) cart.dlist_wavevx=1;
    else if(cart.iszelda) cart.dlist_zelda=1;
    else cart.dlist_zelda=0;

    x_fastfpu(1);

    rdp_framestart();
    g_resetmtx();
    gst.lastframehadbackground=gst.framehadbackground;
    gst.framehadbackground=0;
    gst.lightnum=0;
    gst.lightnumchanged=1;

    stackp=1;
    while(stackp>0)
    {
        if(st.breakout) break;

        if(dllimit>0)
        {
            dllimit--;
            if(!dllimit)
            {
                c_dlend();
                continue;
            }
        }

        cmd[0]=mem_read32p(dlpnt+0);
        cmd[1]=mem_read32p(dlpnt+4);
        c=cmd[0]>>24;
        if(st.dumpgfx)
        {
            dumpcmd(dlpnt,cmd);
            logd(NULL); // flush
        }
        dlpnt+=8;

        if(cmdcnt++>20000)
        {
            error("dlist: display list too large (%i commands)\n",cmdcnt);
            dlist_abort();
            return;
        }
        if(errors>100)
        {
            error("dlist: display list has too many errors",cmdnum);
            errors=0;
            dlist_abort();
            return;
        }

        if(c==0xEF)
        {
            gst.omodeh=cmd[0]&0xffffff;
            gst.omodel=cmd[1];
        }

        if(c>=0xe4)
        {
            int extra;
            logd("-> RDP ");
            extra=rdp_cmd(cmd);
            if(st.dumpgfx) logd("\n");
            while(extra>0)
            {
                cmd[0]=mem_read32p(dlpnt+0);
                cmd[1]=mem_read32p(dlpnt+4);
                if(st.dumpgfx) dumpcmd(dlpnt,cmd);
                dlpnt+=8;
                logd("-> RDP (extra data) ");
                extra=rdp_cmd(cmd);
                if(st.dumpgfx) logd("\n");
            }
        }
        else
        {
            if(cart.dlist_zelda)
            {
                rsp_cmd_zelda(c);
            }
            else
            {
                rsp_cmd_basic(c);
            }
            if(st.dumpgfx) logd("\n");
        }
    }

    if(st2.gfxdpsyncpending || nosynccnt>10)
    {
        rdp_frameend();
        nosynccnt=0;
    }
    else
    {
        nosynccnt++;
    }

    x_fastfpu(0);

    logd("Additionally: %i vertices, %i triangles, %i matrices\n",
        gst.cnt_vtx,gst.cnt_tri,gst.cnt_mtx);
    logd(NULL); // flush

    if(st.gfxthread)
    {
        logd("Displaylist %i commands (separate gfxthread).\n",cmdcnt);
        logh("Displaylist %i commands (separate gfxthread).\n",cmdcnt);
    }
    else
    {
        logd("Displaylist %i commands.\n",cmdcnt);
        logh("Displaylist %i commands.\n",cmdcnt);
    }

    starttime=timer_us(&st2.timer)-starttime;
    st.us_gfx+=starttime;

    if(st.dumpgfx)
    {
        print("displaylist-end (%i commands)\n",cmdcnt);
        flushdisplay();
    }
}

void dlist_cammove(float x,float y,float z)
{
    gst.cammove[0]=x;
    gst.cammove[1]=y;
    gst.cammove[2]=z;
    gst.cammoveset=1;
}

void dlist_ignoregraphics(int ignore)
{
    gst.ignore=ignore;
}

