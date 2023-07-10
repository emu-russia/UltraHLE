#include "stdsdk.h"
extern ROMLIST romList;

#define IFIS(x,str) if(!memicmp(x,str,sizeof(str)-1))

#define PATCH_DONE    0
#define PATCH_RESCAN  1
#define PATCH_PATCH   2
#define PATCH_BYTE    3
#define PATCH_WORD    4

#define MAXPATCH      1024

typedef struct
{
    int    type;
    int    dma;
    dword  addr;
    dword  data;
} IniPatch;

typedef struct
{
    dword  virt;
    dword  phys;
    dword  size;
} IniMap;

static IniMap   vmmap[MAXPATCH];
static int      vmmapnum;

static IniPatch patch[MAXPATCH];
static int      patchnum;
static int      nextpatchdma;

static int      tempread;

static void applypatch(int i)
{
    dword beg,end,d;
    //print(WHITE"patch: %i,%08X,%i,%08X\n",patch[i].dma,patch[i].addr,patch[i].type,patch[i].data);
    switch(patch[i].type)
    {
    case PATCH_RESCAN:
        beg=(patch[i].addr>>8)&0x00fffe00;
        end=(patch[i].addr<<8)&0x00fffe00;
        if(patch[i].addr&1)
        {
            sym_findoscalls(beg,end-beg,0);
        }
        else
        {
            sym_findoscalls(beg,end-beg,1);
        }
        sym_addpatches();
        break;
    case PATCH_PATCH:
        d=PATCH(patch[i].data);
        mem_write32(patch[i].addr,d);
        break;
    case PATCH_WORD:
        mem_write32(patch[i].addr,patch[i].data);
        break;
    case PATCH_BYTE:
        mem_write8(patch[i].addr,patch[i].data);
        break;
    }
}

void inifile_patches(int dmanum)
{
    int i,cnt;
    if(dmanum==0)
    {
        int i,j;
        dword v,p;
        // apply memmaps
        for(j=0;j<vmmapnum;j++)
        {
            print("ultra.ini: memmap %08X <- %08X (%X)\n",
                vmmap[j].virt,
                vmmap[j].phys,
                vmmap[j].size);
            if(vmmap[j].phys>=0x10000000)
            {
                // †† MEMORY WILL BE LOST HERE, MUST ADD FREEING
                // IF ROM CHANGES OR SOMETHING!!
                char *tmpdata;
                int s;

                p=vmmap[j].phys;
                p&=0xfffffff;

                s=vmmap[j].size;
                if(p+s>cart.size) s=cart.size-p;

                print("ultra.ini: WARNING: permanent malloc for %iK\n",s/1024);
                tmpdata=malloc(s);
                memcpy(tmpdata,cart.data+p,s);

                for(i=0;i<s;i+=4096)
                {
                    v=vmmap[j].virt+i;
                    mem_mapexternal(v,MAP_RW,tmpdata+i);
                }
            }
            else
            {
                for(i=0;i<vmmap[j].size;i+=4096)
                {
                    v=vmmap[j].virt+i;
                    p=vmmap[j].phys+i;
                    mem_mapphysical(v,MAP_RW,p);
                }
            }
        }
    }
    if(dmanum>=0 && dmanum<nextpatchdma)
    {
        return;
    }
    // scan for applicable patches and also set nextpatchdma
    cnt=0;
    nextpatchdma=0x7fffffff;
    for(i=0;i<patchnum;i++)
    {
        if(patch[i].type!=PATCH_DONE)
        {
            if(patch[i].dma==dmanum)
            {
                // do the patch
                cnt++;
                applypatch(i);
                if(dmanum!=-1) patch[i].type=PATCH_DONE;
            }
            else
            {
                if(patch[i].dma>dmanum && patch[i].dma<nextpatchdma)
                {
                    nextpatchdma=patch[i].dma;
                }
            }
        }
    }
    if(cnt>0 && dmanum!=-1)
    {
        print("ultra.ini: applied %i patches (dma %i)\n",cnt,dmanum);
    }
}

// The most complex command is patch. It is used to patch the memory
// image. The syntax is:
//
// patch=when,addr,data
//
// when is number of DMA transfer after which patch is applioed.
// 0=just after image loaded, -1=every frame
//
// addr is memory address
//
// data may be:
// 'rescan'  = causes os routine rescanning
//             addr=AAAABBBB where start=00AAAA00 and end=00BBBB00
// 'patch #' = patch.c patched routine
// 'byte #'  = byte (8bit)
// 'word #'  = word (32bit)
//
// write is only done if olddata=always or the memlocation has olddata

void inifile_command(char *cmd)
{
    char *param;

    // extract param part (after '=')
    param=cmd;
    while(*param && *param!='=') param++;
    if(*param) param++;

    IFIS(cmd,"reset")
    {
        nextpatchdma=0;
        patchnum=0;
        vmmapnum=0;
    }
    else IFIS(cmd,"print")
    {
        if(!tempread) print("ultra.ini: "NORMAL"%s\n",param);
    }
    else IFIS(cmd,"bootloader")
    {
        cart.bootloader=1;
    }
    else IFIS(cmd,"savepath")
    {
        strncpy(init.savepath,param,MAXFILE-1);
        fixpath(init.savepath,0);
    }
    else IFIS(cmd,"rompath")
    {
        strncpy(init.rompath,param,MAXFILE-1);
        fixpath(init.rompath,0);
    }
    else IFIS(cmd,"resolution")
    {
        sscanf(param,"%i,%i",&init.gfxwid,&init.gfxhig);
        if(init.gfxwid<320) init.gfxwid=320;
        if(init.gfxhig<240) init.gfxhig=240;
    }
    else IFIS(cmd,"viewport")
    {
        sscanf(param,"%i,%i",&init.viewportwid,&init.viewporthig);
    }
    else IFIS(cmd,"sound")
    {
        int i=atoi(param);
        st.soundenable=i;
    }
    else IFIS(cmd,"optimize")
    {
        int i=atoi(param);
        st.optimize=i;
    }
    else IFIS(cmd,"directsp")
    {
        int i=atoi(param);
        st.memiosp=i;
    }
    else IFIS(cmd,"osrange")
    {
        dword beg,end;
        sscanf(param,"%X,%X",&beg,&end);
        cart.osrangestart=beg;
        cart.osrangeend=end;
    }
    else IFIS(cmd,"mapmem")
    {
        dword to,phys,size;
        sscanf(param,"%X,%X,%X",&to,&phys,&size);
        vmmap[vmmapnum].phys=phys;
        vmmap[vmmapnum].virt=to;
        vmmap[vmmapnum].size=size;
        vmmapnum++;
    }
    else IFIS(cmd,"patch")
    {
        int   dma;
        dword addr;
        char  patchtype[64];
        dword data;
        int   type;

        *patchtype=0;
        data=0;
        sscanf(param,"%i,%X,%s %i",&dma,&addr,patchtype,&data);

        type=PATCH_DONE;
        IFIS(patchtype,"rescan") type=PATCH_RESCAN;
        IFIS(patchtype,"byte")   type=PATCH_BYTE;
        IFIS(patchtype,"word")   type=PATCH_WORD;
        IFIS(patchtype,"patch")  type=PATCH_PATCH;

        if(type==PATCH_DONE)
        {
            print("ultra.ini: Unrecognized patch '%s'\n",patchtype);
            return;
        }
        if(patchnum<MAXPATCH)
        {
            patch[patchnum].type=type;
            patch[patchnum].dma =dma;
            patch[patchnum].addr=addr;
            patch[patchnum].data=data;
            patchnum++;
        }
    }
    // GH - Added
    else IFIS(cmd,"comment")
    {
        strncpy(romList.comment,param,MAX_PATH);
    }
    else IFIS(cmd,"alttitle")
    {
        strncpy(romList.alttitle,param,MAX_PATH);
    }
}

// reads global settings from ini file,
// then adds cart specific settings (if cartname!=NULL)
void inifile_read(char *cartnamep)
{
    FILE *f1;
    char  sectionorg[64];
    char  section[64];
    char  cartname[64];
    char  line[256];
    char  inifile[MAXFILE];
    char *p;
    int   i;
    int   sectionok;

    strcpy(cartname,cartnamep);
    strlwr(cartname);

    strcpy(inifile,init.rootpath);
    strcat(inifile,"ultra.ini");
    f1=fopen(inifile,"rt");
    if(!f1)
    {
        exception("%s not found.",inifile);
        return;
    }

    inifile_command("reset");

    sectionok=0;
    while(!feof(f1))
    {
        *line=0;
        fgets(line,255,f1);

        // remove enter at end
        i=strlen(line);
        if(line[i-1]=='\n') line[i-1]=0;

        // remove comment
        p=line;
        while(*p)
        {
            if(p[0]=='/' && p[1]=='/')
            {
                p[0]=0;
                break;
            }
            p++;
        }

        // skip starting space
        p=line;
        while(*p<=' ' && *p) p++;

        // empty line
        if(!*p) continue;

        if(*p=='[')
        {
            p++;
            for(i=0;i<63;i++)
            {
                if(*p==']' || !*p) break;
                section[i]=*p++;
            }
            section[i]=0;
            strcpy(sectionorg,section);
            strlwr(section);

            sectionok=0;
            if(!stricmp(section,"default"))
            {
                // always read defaults
                sectionok=1;
            }
            else
            {
                // is the section name a substring of cart name?
                if(strstr(cartname,section)) sectionok=1;
            }
            if(sectionok)
            {
                if(!tempread) print("ultra.ini: reading [%s]\n",sectionorg);
            }
        }

        // skip line if not in a section we are handling
        if(!sectionok) continue;

        inifile_command(line);
    }
    fclose(f1);
}

void inifile_readtemp(char *cartnamep)
{
    Cart bak;

    // backup Cart data
    memcpy(&bak,&cart,sizeof(cart));
    tempread=1;

    inifile_read(cartnamep);

    // restore Cart data
    memcpy(&cart,&bak,sizeof(cart));
    tempread=0;
}


