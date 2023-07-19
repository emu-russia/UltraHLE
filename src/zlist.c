#include "ultra.h"

typedef struct
{
    dword   m_pic;
    int     wid;
    int     hig;
    dword   m_q1;
    dword   m_q2;
    dword   m_q3;
    dword   zero1;
    dword   zero2;
} ZData;

ZData zdata;

static int    in[6][64];
static int    rgb[256][4];
static int    out[16*16];

static int    q1[64];
static int    q2[64];
static int    q3[64];


void readdata(void *dst,dword addr,int bytes)
{
    dword *d=(dword *)dst;
    int i,x;
    bytes>>=2;
    for(i=0;i<bytes;i++)
    {
        x=mem_read32(addr+i*4);
        d[i]=x;
    }
}

void readshort(int *q,dword addr,int bytes)
{
    int x,i;
    bytes>>=1;
    for(i=0;i<bytes;i+=2)
    {
        x=mem_read32(addr+i*2);
        q[i+0]=(short)(x>>16);
        q[i+1]=(short)(x&65535);
    }
}

void writeshort(int *q,dword addr,int bytes)
{
    int x,a,b,i;
    bytes>>=1;
    for(i=0;i<bytes;i+=2)
    {
        a=q[i+0]&0xffff;
        b=q[i+1]&0xffff;
        x=(a<<16)+b;
        mem_write32(addr+i*2,x);
    }
}

void readq(int *q,dword addr)
{
    int i;
    readshort(q,addr,64*2);
//    for(i=0;i<64;i++) q[i]=8;
    for(i=1;i<64;i++) q[i]*=2;
}

int zigzag[64]={
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63};

int	curve[8][8]={ // old values, overwritten by dctinit()
{ 16, 16, 16, 16, 16, 16, 16, 16},
{ 15, 13,  8,  3, -3, -8,-13,-15},
{ 14,  6, -6,-14,-14, -6,  6, 14},
{ 13, -3,-15, -8,  8, 15,  3,-13},
{ 11,-11,-11, 11, 11,-11,-11, 11},
{  8,-15,  3, 13,-13, -3, 15, -8},
{  6,-14, 14, -6, -6, 14,-14,  6},
{  3, -8, 13,-15, 15,-13,  8, -3}
};

void dctdump(int *in,char *text)
{
    static FILE *f1;
    int x,y;

    if(!f1) f1=fopen("dct.log","wt");
    fprintf(f1,"%s\n",text);
    for(y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            fprintf(f1,"%4i ",in[x+y*8]);
        }
        fprintf(f1,"\n");
    }
}

static void dctinit(void)
{
    int x,y;
    float f,fx,fy;
    if(1)
    { // more accurate
        for(y=0;y<8;y++)
        {
            for(x=0;x<8;x++)
            {
                fx=x;
                fy=y;
                f=(2*fx+0)*fy*3.1415926535/16.0;
                f=cos(f)*256.0;
                curve[x][y]=f;
                //print("curve %i,%i: %3i float %.3f\n",x,y,curve[y][x],f/32.0);
            }
        }
    }
    else
    {
        for(y=0;y<8;y++)
        {
            for(x=0;x<8;x++)
            {
                curve[x][y]*=16;
            }
        }
    }
}

static void dct(int *d,int *s,int *quant,int dump)
{
    int tmp[8];
    int in[8][8];
    int x,y,a,i;
    static int initdone=0;

    if(!initdone)
    {
        dctinit();
        initdone=1;
    }

    for(y=0;y<8;y++) for(x=0;x<8;x++)
    {
        i=zigzag[x+y*8];
        in[y][x]=s[i]*quant[i]>>3;
    }

//    if(dump) dctdump((int *)in,"\nInput:");

    for(x=0;x<8;x++)
    {
        for(a=0;a<8;a++) tmp[a]=in[0][x]<<8;
        for(y=1;y<8;y++)
        {
            for(a=0;a<8;a++)
            {
                tmp[a]+=in[y][x]*curve[y][a];
            }
        }
        for(a=0;a<8;a++) in[a][x]=tmp[a]>>8;
    }
    for(y=0;y<8;y++)
    {
        for(a=0;a<8;a++) tmp[a]=in[y][0]<<8;
        for(x=1;x<8;x++)
        {
            for(a=0;a<8;a++)
            {
                tmp[a]+=in[y][x]*curve[x][a];
            }
        }
        for(a=0;a<8;a++) in[y][a]=tmp[a]>>8;
    }

//    if(dump) dctdump((int *)in,"Output:");

    for(y=0;y<8;y++) for(x=0;x<8;x++)
    {
        d[x+y*8]=in[y][x];
    }
}

static void uncompress(void)
{
    int x2,y2,x,y,a,b;
    int i1,u1,v1;
    int ci[64];
    int cu[64];
    int cv[64];
    dct(cu,in[4],q2,0);
    dct(cv,in[5],q3,0);
    for(y=0;y<16;y++) for(x=0;x<16;x++)
    {
        u1=cu[(x>>1)+(y>>1)*8];
        v1=cv[(x>>1)+(y>>1)*8];
        rgb[x+y*16][0]=(         291*v1 ) >> 8;
        rgb[x+y*16][1]=( -101*u1-148*v1 ) >> 8;
        rgb[x+y*16][2]=(  564*u1        ) >> 8;
    }
    for(y2=0;y2<=8;y2+=8) for(x2=0;x2<=8;x2+=8)
    {
        dct(ci,in[(x2/8)+(y2/8)*2],q1,1);
        for(y=0;y<8;y++) for(x=0;x<8;x++)
        {
            i1=ci[x+y*8];
            rgb[(x+x2)+(y+y2)*16][0]+=i1+128;
            rgb[(x+x2)+(y+y2)*16][1]+=i1+128;
            rgb[(x+x2)+(y+y2)*16][2]+=i1+128;
        }
    }
    // convert to short
    for(y=0;y<16;y++) for(x=0;x<16;x++)
    {
        b=1;

        a=rgb[x+y*16][0];
        a=(a>>3);
        if(a<0) a=0; if(a>31) a=31;
        b+=(a<<11);

        a=rgb[x+y*16][1];
        a=(a>>3);
        if(a<0) a=0; if(a>31) a=31;
        b+=(a<<6);

        a=rgb[x+y*16][2];
        a=(a>>3);
        if(a<0) a=0; if(a>31) a=31;
        b+=(a<<1);

        out[x+y*16]=b;
    }
}

void zlist_uncompress(OSTask_t *task)
{
    int z;
    static int cnt=1;
    static int lastframe=-1;
    static int firsttime=1;

    readdata(&zdata,task->m_data_ptr,sizeof(ZData));

    if((zdata.m_pic&0x7f000000)!=0 ||
       (zdata.m_q1 &0x7f000000)!=0 ||
       (zdata.m_q2 &0x7f000000)!=0 ||
       (zdata.m_q3 &0x7f000000)!=0 ||
       !zdata.m_pic ||
       !zdata.m_q1  ||
       !zdata.m_q2  ||
       !zdata.m_q3)
    {
        print("zelda: JPEG-DCT (buffer %08X, quant %08X) INVALID, ignored.\n",
            zdata.m_pic,zdata.m_q1,zdata.m_q2,zdata.m_q3);
        return;
    }

    readq(q1,zdata.m_q1);
    readq(q2,zdata.m_q2);
    readq(q3,zdata.m_q3);

    if(firsttime)
    {
        firsttime=0;
        disasm_dumpucode("rspzlist.log",
            task->m_ucode     ,task->ucode_size,
            task->m_ucode_data,task->ucode_data_size,0x80);
        logd("RSP microcode/data dumped to RSPZLIST.LOG\n");
    }

    if(st.frames!=lastframe)
    {
        print("zelda: JPEG-DCT (buffer %08X, quant %08X %08X %08X)\n",
            zdata.m_pic,zdata.m_q1,zdata.m_q2,zdata.m_q3);
        lastframe=st.frames;
    }

    for(z=0;z<4;z++)
    {
        readshort ((int *)in ,zdata.m_pic+z*768,8*8*2*6);
        uncompress();
        writeshort((int *)out,zdata.m_pic+z*768,16*16*2);
    }

    /*

    if(cnt==1)
    {
        FILE *f1;
        f1=fopen("bgin.dat","wb");
        for(z=0;z<3072*2;z+=4)
        {
            d=mem_read32(zdata.m_pic+z);
            d=FLIP32(d);
            fwrite(&d,1,4,f1);
        }
        fclose(f1);
    }

    d1=(cnt)+(cnt<<8)+(cnt<<16)+(cnt<<24);
    d2=d1^-1;
    for(z=0;z<4;z++)
    {
        for(y=0;y<16;y++) for(x=0;x<16;x++)
        {
            if(x==0 || y==0 || x==15 || y==15)
            {
                mem_write16(zdata.m_pic+z*768+x*2+y*2*16,d1);
            }
        }
    }

    */

    cnt++;
}

