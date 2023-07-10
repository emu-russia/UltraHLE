#include "ultra.h"

int godisabled;

extern void printhelp2(void);
extern int command_2(char *,char *);

static byte *snap; 

#define IFIS(x,str) if(!stricmp(x,str))
#define IS(x,str) !stricmp(x,str)

char *param(char **tp)
{
    static char buf[256];
    char *d=buf,*s=*tp;

    *buf=0;

    // skip space
    while(*s && *s<=32) s++;

    // copy nonspace
    while(*s && *s>32) *d++=*s++;
    *d=0;

    // skip space
    while(*s && *s<=32) s++;

    *tp=s;

    return(buf);
}

qword atoq(char *p)
{
    qword res=0;
    dword x;
    // skip space
    while(*p<=32 && *p) p++;
    // check for hex
    if(p[1]=='x' || p[1]=='X')
    {
        sscanf(p,"%i",&x);
        res=x;
    }
    else
    { // construct number
        while(*p>='0' && *p<='9')
        {
            res*=10;
            res+=(*p)-'0';
            p++;
        }
    }
    // check trailing 'k' or 'm'
    if(*p=='k' || *p=='K') res*=1000;
    else if(*p=='m' || *p=='M') res*=1000000;
    return(res);
}

void setaddress(char *text,int *addr)
{
    int l,a,o;
    unsigned int mask;
    l=strlen(text);
    if(l<8) mask=0xffffffffU >> (32-l*4);
    else mask=0xffffffffU;
    sscanf(text,"%X",&a);
    o=*addr;
    *addr=(o&(~mask)) | (a&mask);
//    print("old %08X new %08X mask %08X = %08X\n",o,a,mask,*addr);
}

//------------------------------------------------------------

void printhelp(void)
{
    extern void printcopyright(void);
    printcopyright();
#if !RELEASE
    print(
    "\x1\x3------------------ misc --------------------------------------------------------------\n"
          "+             - execute last command again\n"
          "hide3dfx\n"
          "show3dfx\n"
          "pad 0..3\n"
          "keys 0/1      - enable/disable pad keys\n"
          "zeldajap      - set zelda language to Japanese\n"
          "zeldaeng      - set zelda language to English\n"
          "screen <file> - take a screenshow (no parameter in 1.tga,2.tga,..)\n"
          "sound         - toggle sound (slist.c)\n"
          "soundsync     - toggle sound syncronization (will slow gfx)\n"
          "gfxthread     - execute graphics in separate thread\n"
          "graphics      - toggle graphics (dlist.c)\n"
          "memory        - show the big picture on rdram usage\n"
          "camera # # #  - move camera (DLIST hack)\n"
          "resolution #  - change resolution #=512/640/800/1024\n"
          "group <addr>  - disassemble X86 compiled group at <addr>\n"
          "wireframe #   - set gfx wireframe 0/1\n"
          "compile <add> - compile and disassemble (default add=pc)\n"
    "\x1\x3------------------ os and statistics -------------------------------------------------\n"
          "osinfo        - list os tasks/queues/events\n"
          "emptyq        - empty all queues\n"
          "event <num>   - send event; <number>,all,sp\n"
          "swap          - force framesync\n"
          "send <x>      - send 0 to queue x\n"
          "stat          - compiler statistics\n"
          "stat2         - list used opcodes (in compiler)\n"
          "stat3         - list execution profile (must be defined in cpua.c)\n"
    "\x1\x3------------------ memory ------------------------------------------------------------\n"
          "disasm <file> <start> <cnt>    - disasseble memory to file\n"
          "disasmrsp <file> <code> <data> - disasseble memory to file\n"
          "savemem <file> <start> <cnt>   - save memory to file\n"
    "\x1\x3------------------ symbols -----------------------------------------------------------\n"
          "sym           - dump symbols\n"
          "findos        - rescan memory for os routines\n"
          "listos        - list currently found os routines\n"
    "\x1\x3------------------ examine/change ----------------------------------------------------\n"
          ".             - view code at pc\n"
          "u  <addr>     - view code at <addr>\n"
          "d  <addr>     - view data at <addr>\n"
          "e  <addr> <d> - set DWORD in memory\n"
          "eb <addr> <d> - set BYTE in memory\n"
          "ss <str>      - search memory for string\n"
          "ssr <str>     - search memory for string (relative chars)\n"
          "filltst <addr>\n"
          "fill <addr> <cnt> <byte>\n"
          "snap          - take memory snapshot\n"
          "snaph <old> <new> - find changed halfword (new <value>)\n"
          "regs          - toggle integer/fpu regs on display (F6)\n"
          "setreg <r> <v>- set register <r> (0..31) to value <v>\n"
    "\x1\x3----------------- execute------------------------------------------------------------\n"
          "s  [cnt]      - step <cnt> instructions using c-sim\n"
          "f  [cnt]      - step <cnt> instructions using a-sim\n"
          "t  <addr>     - execute until PC=address (execute to)\n"
          "mw <addr>     - execute until memory at <addr> written\n"
          "mr <addr>     - execute until memory at <addr> read\n"
          "ma <addr>     - execute until memory at <addr> accessed\n"
          "mc <addr>     - execute until memory at <addr> changed\n"
          "skip <cnt>    - skip <cnt> instructioons\n"
          "goto <addr>   - set PC=addr\n"
          "n             - execute until next instruction (over calls)\n"
          "nr            - execute until next return (to end of THIS routine)\n"
          "nc            - execute until next call\n"
          "nt            - execute until next thread\n"
          "nf            - execute until next frame\n"
          "nm <queue>    - execute until next message op in queue <queue>\n"
    "\x1\x3------------------ logging and stopping ----------------------------------------------\n"
          "info <0/1>    - toggle generic info (console)\n"
          "trace <0/1>   - toggle trace dumping (console)\n"
          "ops <0/1>     - toggle full operation trace info (console)\n"
          "asm <0/1>     - toggle asm compiler info\n"
          "os <0/1>      - toggle os info (ultra.log)\n"
          "hw <0/1>      - toggle os info (ultra.log)\n"
          "gfx <0/1>     - toggle gfx info (dlist.log)\n"
          "snd <0/1>     - toggle sound info (slist.log)\n"
          "all <0/1>     - toggle all of the above on/off\n"
          "stop <0/1/2>  - stopping: 0=exceptions only, 1=and errors (def.), 2=and warnings\n");
    printhelp2();
    print(
    "\x1\x3------------------ special keys when NOT executing -----------------------------------\n"
          "F1 - show stats      F5 - go      F9 - step next   \n"
          "F2 - window data     F6 - regs    UP - command history\n"
          "F3 - window code     F7 - fstep   PGUP - scroll console\n"
          "F4 - window console  F8 - step    ESC  - stop scrolling\n"
    "\x1\x3------------------ special keys when executin ----------------------------------------\n"
          "See top of screen for pad keys, additionally:\n"
          "F3 - toggle joystick  F6 - quicksave (ultra.sav)  F8  - pause             ESC - break\n"
          "F4 - toggle mouse     F9 - quickload (ultra.sav)  F12 - toggle 3dfx  \n"

          "Graphics test keys:\n"
          "W - wireframe  Q - textures  E - passes  R - misc test toggle\n"
    "\x1\x3------------------ basic commands ----------------------------------------------------\n"
          "save <file>   - save emulator state to file (about 4MB). Default ULTRA.SAV\n"
          "load <file>   - load emulator state from file. Default ULTRA.SAV.\n"
          "rom <file>    - load a new rom file\n"
          "reset         - reset/restart rom\n"
          "sgo           - slow go! (using c emulator)\n"
          "ogo           - old go! (using older compiler)\n"
          "go, z         - go! (using compiler)\n"
          "exit, x       - exit\n"
          );
#endif
}

void memorypic(void)
{
    int a,i,x,y,cnt,codecnt,d;
    print("Memory RDRAM contents (4MB): C=code? d=data? e=almostempty .=empty\n");

    print("      ");
    for(x=0;x<64;x+=8)
    {
        print("  +%05X ",x*4096);
    }
    print("\n");

    for(y=0;y<16;y++)
    {
        a=(0+y*64)*4096;
        print("%04X: ",a>>16);
        for(x=0;x<64;x++)
        {
            a=(x+y*64)*4096;
            cnt=codecnt=0;
            for(i=0;i<4096;i+=4)
            {
                d=mem_read32(a+i);
                if(d) cnt++;
                if(OP_OP(d)==35 || OP_OP(d)==43 || OP_OP(d)==15) codecnt++;
            }
            if(!cnt) print(".");
            else if(codecnt*8>cnt) print("C");
            else if(cnt<64) print("e");
            else print("d");
            if((x&7)==7) print(" ");
        }
        print("\n");
    }
}

void printdumping(void)
{
    char *t[2]={"off","ON "};
    print("Dumping: ");
    print("info:%i "  ,st.dumpinfo);
    print("os:%i "    ,st.dumpos);
    print("hw:%i "    ,st.dumphw);
    print("gfx:%i " ,st.dumpgfx);
    print("snd:%i " ,st.dumpsnd);
    print("wav:%i "   ,st.dumpwav);
    print("trace:%i " ,st.dumptrace);
    print("ops:%i "   ,st.dumpops);
    print("asm:%i "   ,st.dumpasm);
    print("\n");
}

void savestate(char *name)
{
    FILE *f1;
    a_clearcodecache(); // will remove GROUP opcodes from mem
    f1=fopen(name,"wb");
    if(f1)
    {
        putc('U',f1);
        putc('0',f1);
        putc('2',f1);
        putc(0x1a,f1);
        mem_save(f1);
        cpu_save(f1);
        os_save(f1);
        fclose(f1);
        print("State saved to %s\n",name);
    }
    else print("Error saving state.\n");
}

void loadstate(char *name)
{
    FILE *f1;
    f1=fopen(name,"rb");
    if(f1)
    {
        int error=0;
        if(getc(f1)!='U') error=1;
        if(getc(f1)!='0') error=1;
        if(getc(f1)!='2') error=1;
        getc(f1);
        if(error)
        {
            print("State %s not compatible.\n",name);
        }
        else
        {
            int opt;

            // do not change optimize setting when loading
            opt=st.optimize;

            st2.exception=0;
            mem_load(f1);
            if(!st2.exception) cpu_load(f1);
            if(!st2.exception) os_load(f1);
            if(!st2.exception) sym_addpatches();
            if(!st2.exception) print("State loaded from %s\n",name);
            else print("Error loading state from %s.\n",name);

            st.optimize=opt;
        }
        fclose(f1);
    }
    else print("Error loading state from %s.\n",name);
}

int command_main(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,"help")
    {
        printhelp();
    }
    else if(IS(p,"x") || IS(p,"exit"))
    {
        exitnow();
    }
    else IFIS(p,"reset")
    {
        reset();
    }
    else IFIS(p,"save")
    {
        char buf[64];
        p=param(&tp);
        if(!p || *p<=32)
        {
            remove("ultrabak.sav");
            rename("ultra.sav","ultrabak.sav");
            remove("ultra.sav");
            p="ultra.sav";
        }
        strcpy(buf,p); if(!strstr(p,".")) strcat(buf,".sav");
        p=buf;
        savestate(p);
        flushdisplay();
        print("note: Saved state to '%s'\n",p);
    }
    else IFIS(p,"rom")
    {
        p=param(&tp);
        print("note: Loaded rom image from '%s'\n",p);
        boot(p,0);
    }
    else IFIS(p,"load")
    {
        char buf[64];
        p=param(&tp);
        if(!p || *p<=32) p="ultra.sav";
        strcpy(buf,p); if(!strstr(p,".")) strcat(buf,".sav");
        p=buf;
        loadstate(p);

        st.nextswitch=st.cputime+10000;
        flushdisplay();
        st.graphicsenable=1;

        // generate some events to avoid being stuck
        /*
        os_event(OS_EVENT_SI);
        os_event(OS_EVENT_PI);
        os_event(OS_EVENT_DP);
        */
        os_event(OS_EVENT_SP);

        print("note: Loaded state from '%s'\n",p);
    }
    else if(IS(p,"go") || IS(p,"sgo") || IS(p,"ogo"))
    {
        int a=view.showhelp;

        if(godisabled) return(0);

        view.showhelp=1; view_changed(VIEW_RESIZE); flushdisplay();

        cpu_clearbp();

        // slow startup
        if(IS(p,"go"))
        {
            cpu_exec(CYCLES_INFINITE,1);
        }
        else if(IS(p,"ogo"))
        {
            cpu_exec(CYCLES_INFINITE,2);
        }
        else
        {
            cpu_exec(CYCLES_INFINITE,0);
        }

        view.showhelp=a; view_changed(VIEW_RESIZE);
    }
    else return(0);
    return(1);
}

int command_step(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,"skip")
    {
        qword cnt;
        p=param(&tp);
        cnt=atoq(p);
        if(cnt<1) cnt=1;
        st.pc+=cnt*4;
        st.branchdelay=0;
    }
    else IFIS(p,"goto")
    {
        int to=st.pc;
        setaddress(param(&tp),&to);
        st.pc=to;
        view.codebase=st.pc;
    }
    else IFIS(p,"s")
    {
        qword cnt;
        p=param(&tp);
        cnt=atoq(p);
        if(cnt<1) cnt=1;
        cpu_clearbp();
        cpu_exec(cnt,0);
    }
    else IFIS(p,"f")
    {
        qword cnt;
        p=param(&tp);
        cnt=atoq(p);
        if(cnt<1) cnt=1;
        cpu_clearbp();
        cpu_exec(cnt,1);
    }
    else IFIS(p,"of")
    {
        qword cnt;
        p=param(&tp);
        cnt=atoq(p);
        if(cnt<1) cnt=1;
        cpu_clearbp();
        cpu_exec(cnt,2);
    }
    else IFIS(p,"n")
    {
        cpu_onebp(BREAK_PC,st.pc+4,0); // stop at next instruction
        cpu_addbp(BREAK_FWBRANCH,0,0); // stop on a forward branch
        cpu_addbp(BREAK_NEXTRET,0,0);  // stop on ret
        st.quietbreak=1;
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"t")
    {
        int to=st.pc;
        setaddress(param(&tp),&to);
        print("Executing until address %08X\n",to);
        cpu_onebp(BREAK_PC,to,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"mw")
    {
        int to=0;
        setaddress(param(&tp),&to);
        view.database=to;
        print("Executing until address %08X written\n",to);
        cpu_onebp(BREAK_MEMW,to,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"mr")
    {
        int to=0;
        setaddress(param(&tp),&to);
        view.database=to;
        print("Executing until address %08X written\n",to);
        cpu_onebp(BREAK_MEMR,to,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"ma")
    {
        int to=0;
        setaddress(param(&tp),&to);
        view.database=to;
        print("Executing until address %08X accessed\n",to);
        cpu_onebp(BREAK_MEM,to,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"mc")
    {
        int to=0;
        setaddress(param(&tp),&to);
        view.database=to;
        print("Executing until address %08X changed\n",to);
        cpu_onebp(BREAK_MEMDATA,to,mem_read32(to));
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"nt")
    { // nextthread
        cpu_onebp(BREAK_NEXTTHREAD,0,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"nf")
    { // nextframe
        cpu_onebp(BREAK_NEXTFRAME,0,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"nm")
    { // nextmsg
        cpu_onebp(BREAK_MSG,0,atoi(param(&tp)));
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"nr")
    { // next ret
        cpu_onebp(BREAK_NEXTRET,0,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else IFIS(p,"nc")
    { // next call
        cpu_onebp(BREAK_NEXTCALL,0,0);
        cpu_exec(CYCLES_STEP,0);
    }
    else return(0);
    return(1);
}

int command_dump(char *p,char *tp)
{
    int infoset=0;
    if(0) { }
    else IFIS(p,"info")
    {
        p=param(&tp);
        if(*p=='0') st.dumpinfo=0;
        else if(*p=='1') st.dumpinfo=1;
        else st.dumpinfo^=1;
        infoset=1;
    }
    else IFIS(p,"os")
    {
        p=param(&tp);
        if(*p=='0') st.dumpos=0;
        else if(*p=='1') st.dumpos=1;
        else st.dumpos^=1;
    }
    else IFIS(p,"hw")
    {
        p=param(&tp);
        if(*p=='0') st.dumphw=0;
        else if(*p=='1') st.dumphw=1;
        else st.dumphw^=1;
    }
    else IFIS(p,"ops")
    {
        p=param(&tp);
        if(*p=='0') st.dumpops=0;
        else if(*p=='1') st.dumpops=1;
        else st.dumpops^=1;
    }
    else IFIS(p,"asm")
    {
        p=param(&tp);
        if(*p=='0') st.dumpasm=0;
        else if(*p=='1') st.dumpasm=1;
        else st.dumpasm^=1;
    }
    else IFIS(p,"gfx")
    {
        p=param(&tp);
        if(*p=='0') st.dumpgfx=0;
        else if(*p=='1') st.dumpgfx=1;
        else st.dumpgfx^=1;
    }
    else IFIS(p,"snd")
    {
        p=param(&tp);
        if(*p=='0') st.dumpsnd=0;
        else if(*p=='1') st.dumpsnd=1;
        else st.dumpsnd^=1;
    }
    else IFIS(p,"wav")
    {
        p=param(&tp);
        if(*p=='0') st.dumpwav=0;
        else if(*p=='1') st.dumpwav=1;
        else st.dumpwav^=1;
    }
    else IFIS(p,"trace")
    {
        p=param(&tp);
        if(*p=='0') st.dumptrace=0;
        else if(*p=='1') st.dumptrace=1;
        else st.dumptrace^=1;
    }
    else IFIS(p,"all")
    {
        p=param(&tp);
        if(*p!='0' && *p!='1')
        {
            if(st.dumpinfo || st.dumpsnd || st.dumpgfx || st.dumphw ||
               st.dumpos   || st.dumpops || st.dumpasm || st.dumptrace) p="0";
            else p="1";
        }

        if(*p=='0')
        {
            st.dumpinfo=0;
            st.dumpsnd=0;
            st.dumpgfx=0;
            st.dumpos=0;
            st.dumptrace=0;
        }
        else if(*p=='1')
        {
            st.dumpinfo=1;
            st.dumpsnd=1;
            st.dumpgfx=1;
            st.dumpos=1;
            st.dumptrace=1;
        }
    }
    else IFIS(p,"stop")
    {
        p=param(&tp);
        if(*p=='0')
        {
            st.stoperror=0;
            st.stopwarning=0;
        }
        else if(*p=='1')
        {
            st.stoperror=1;
            st.stopwarning=0;
        }
        else if(*p=='2')
        {
            st.stoperror=1;
            st.stopwarning=1;
        }
        print("Stopping on: exceptions ");
        if(st.stoperror) print("errors ");
        if(st.stopwarning) print("warnings ");
        print("\n");
    }
    else return(0);

    if(!infoset)
    {
        if(st.dumpinfo || st.dumpsnd || st.dumpgfx || st.dumphw ||
           st.dumpos   || st.dumpops || st.dumpasm || st.dumptrace)
        {
            st.dumpinfo=1;
        }
    }

    printdumping();

    return(1);
}

int command_exam(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,".")
    {
        view.codebase=st.pc-4*(view.coderows/3);
        print("Unassemble at %08X\n",view.codebase);
    }
    else IFIS(p,"u")
    {
        setaddress(param(&tp),&view.codebase);
        print("Unassemble at %08X\n",view.codebase);
    }
    else IFIS(p,"d")
    {
        setaddress(param(&tp),&view.database);
        print("Data at %08X\n",view.database);
    }
    else IFIS(p,"e")
    {
        dword addr,data;
        setaddress(param(&tp),&addr);
        p=param(&tp);
        data=atoq(p);
        mem_write32(addr,data);
        view.database=addr;
    }
    else IFIS(p,"eb")
    {
        dword addr,data;
        setaddress(param(&tp),&addr);
        p=param(&tp);
        data=atoq(p);
        mem_write8(addr,data);
        view.database=addr;
    }
    else IFIS(p,"ss")
    {
        char *str=param(&tp);
        int i,j,l,n;
        l=strlen(str);
        n=RDRAMSIZE-l;
        print("Searching for '%s':\n",str);
        for(i=0;i<n;i++)
        {
            if(mem.ram[i^3]==str[0])
            {
                for(j=0;j<l;j++)
                {
                    if(mem_read8(i+j)!=str[j]) break;
                }
                if(j==l)
                {
                    print("- %08X\n",i);
                }
            }
        }
    }
    else IFIS(p,"ssmario")
    {
        char *str=param(&tp);
        int i,j,l,n,flag;
        l=strlen(str);
        n=RDRAMSIZE-l;
        print("Searching for\n",str);
        for(i=16;i<n-16;i++)
        {
            if(mem.ram[i]==37)
            {
                flag=0;
                for(j=-16;j<16;j++)
                {
                    if(mem.ram[i+j]==76) flag|=1;
                    if(mem.ram[i+j]==61) flag|=2;
                    if(mem.ram[i+j]==68) flag|=4;
                }
                if(flag==7)
                {
                    print("- %08X\n",i);
                }
            }
        }
    }
    else IFIS(p,"snap")
    {
        if(!snap)
        {
            snap=malloc(RDRAMSIZE);
        }
        memcpy(snap,mem.ram,RDRAMSIZE);
        print("Snapshot taken\n");
    }
    else IFIS(p,"snaphw")
    {
        char *p;
        int xold,xnew;
        p=param(&tp); sscanf(p,"%i",&xold);
        p=param(&tp); sscanf(p,"%i",&xnew);
        if(!snap)
        {
            print("no snapshot to compare to\n");
        }
        else
        {
            short *mo,*mn;
            int i,cnt=0;
            mo=(short *)snap;
            mn=(short *)mem.ram;
            for(i=0;i<RDRAMSIZE/2;i++)
            {
                if(mo[i]==xold && mn[i]==xnew)
                {
                    print("- %08X\n",i*2^2);
                    cnt++;
                    if(cnt>30)
                    {
                        print("too many\n");
                        break;
                    }
                }
            }
        }
    }
    else IFIS(p,"fill")
    {
        int addr,cnt,data;
        p=param(&tp); sscanf(p,"%i",&addr);
        p=param(&tp); sscanf(p,"%i",&cnt);
        p=param(&tp); sscanf(p,"%i",&data);
        while(cnt-->0)
        {
            mem_write8(addr++,data);
        }
    }
    else IFIS(p,"filltst")
    {
        int addr,i;
        p=param(&tp); sscanf(p,"%i",&addr);
        for(i=0;i<32;i++)
        {
            mem_write16(addr+i*2,i+0x30);
        }
    }
    else IFIS(p,"ssr")
    {
        char *str=param(&tp);
        int i,j,l,n,r0;
        l=strlen(str);
        n=RDRAMSIZE-l;
        print("Searching for '%s' (relative):\n",str);
        for(i=0;i<n;i++)
        {
            r0=mem_read8(i);
            for(j=1;j<l;j++)
            {
                if(mem_read8(i+j)-r0!=str[j]-str[0]) break;
            }
            if(j==l)
            {
                print("- %08X\n",i);
            }
        }
    }
    else IFIS(p,"regs")
    {
        view.showfpu++;
        if(view.showfpu>2) view.showfpu=0;
    }
    else return(0);
    return(1);
}

int command_sym(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,"saveoscalls")
    {
        sym_demooscalls();
    }
    else IFIS(p,"findos")
    {
        sym_load(cart.symname); // reload symbols
        sym_findoscalls(0,4*1024*1024,0);
        sym_addpatches();
    }
    else IFIS(p,"listos")
    {
        sym_dumposcalls();
    }
    else IFIS(p,"sym")
    {
        sym_dump();
    }
    /*
    else IFIS(p,"addos")
    {
        char name[80];
        dword addr=view.codebase;
        setaddress(param(&tp),&addr);
        strcpy(name,param(&tp));
        if(!*name) strcpy(name,"?");
        print("Addos %08X name %s\n",addr,name),
        symfind_saveroutine(addr,name);
    }
    */
    else return(0);
    return(1);
}

int command_misc(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,"pad")
    {
        int a;
        a=atoi(param(&tp));
        hw_selectpad(a);
    }
    else IFIS(p,"keys")
    {
        p=param(&tp);
        if(*p=='1') st.keyboarddisable=0;
        else if(*p=='0') st.keyboarddisable=1;
        else st.keyboarddisable^=1;
        print("keyboard: %s\n",!st.keyboarddisable?"enabled":"disabled");
    }
    else IFIS(p,"hide3dfx")
    {
        dlist_ignoregraphics(1);
        rdp_closedisplay();
    }
    else IFIS(p,"show3dfx")
    {
        dlist_ignoregraphics(0);
    }
    else IFIS(p,"swap")
    {
        rdp_framestart();
        rdp_frameend();
    }
    else IFIS(p,"sound")
    {
        p=param(&tp);
        if(*p=='1') st.soundenable=1;
        else if(*p=='0') st.soundenable=0;
        else st.soundenable^=1;
        print("Sound: %s\n",st.soundenable?"enabled":"disabled");
    }
    else IFIS(p,"pimem")
    {
        int i;
        for(i=0;i<4;i++)
        {
            print("%08X ",WPI[i]);
        }
        print("\n");
    }
    else IFIS(p,"soundsync")
    {
        p=param(&tp);
        if(*p=='1') st.soundslowsgfx=1;
        else if(*p=='0') st.soundslowsgfx=0;
        else st.soundslowsgfx^=1;
        print("Soundsync: %s\n",st.soundenable?"enabled":"disabled");
    }
    else IFIS(p,"gfxthread")
    {
        p=param(&tp);
        if(*p=='1') st.gfxthread=1;
        else if(*p=='0') st.gfxthread=0;
        else st.gfxthread^=1;
        print("Gfxthread: %s\n",st.gfxthread?"enabled":"disabled");
    }
    else IFIS(p,"graphics")
    {
        p=param(&tp);
        if(*p=='1') st.graphicsenable=1;
        else if(*p=='0') st.graphicsenable=0;
        else st.graphicsenable^=1;
        print("Graphics: %s\n",st.graphicsenable?"enabled":"disabled");
    }
    else IFIS(p,"zeldajap")
    {
        if(cart.iszelda)
        {
            mem_write8(0x8011b9d9,0);
            print("Zelda-language: Japanese\n");
        }
        else
        {
            print("Cart not Zelda.\n");
        }
    }
    else IFIS(p,"zeldaeng")
    {
        if(cart.iszelda)
        {
            mem_write8(0x8011b9d9,1);
            print("Zelda-language: English\n");
        }
        else
        {
            print("Cart not Zelda.\n");
        }
    }
    else IFIS(p,"setreg")
    {
        int r,v;
        sscanf(param(&tp),"%i",&r);
        sscanf(param(&tp),"%i",&v);
        st.g[r].d=v;
    }
    else IFIS(p,"camera")
    {
        float x,y,z;
        x=atof(param(&tp));
        y=atof(param(&tp));
        z=atof(param(&tp));
        dlist_cammove(x,y,z);
    }
    else IFIS(p,"resolution")
    {
        int a;
        a=atoi(param(&tp));
        if(a<320 || a>2048) a=640;
        init.gfxwid=a;
        init.gfxhig=480*a/640;
        rdp_closedisplay();
    }
    else IFIS(p,"wireframe")
    {
        int a;
        a=atoi(param(&tp));
        showwire=a;
    }
    else IFIS(p,"boot")
    {
        int i;
        for(i=0;i<1024;i++)
        {
            WSPD[i]=((dword *)cart.data)[i];
            RSPD[i]=((dword *)cart.data)[i];
        }
        cpu_goto(0xa4000040);
    }
    else IFIS(p,"memory")
    {
        memorypic();
    }
    else if(IS(p,"group") || IS(p,"compile"))
    {
        dword  x;
        float  ratio=0.0;
        int    gi,compile;
        Group *g;
        char  *p2;
        int    i,ia;

        if(IS(p,"compile")) compile=1;
        else compile=0;

        p2=param(&tp);
        if(*p2<'0')
        {
            x=st.pc;
        }
        else
        {
            x=view.codebase;
            setaddress(p2,&x);
        }

        view.codebase=x;
        print("Unassemble group at %08X:",x);

        gi=mem_groupat(x);
        if(gi<0 && compile)
        {
            print(" compiling:");
            a_compilegroupat(x);
            gi=mem_groupat(x);
        }
        if(gi<0)
        {
            print(" not found\n");
            i=0;
        }
        else
        {
            g=mem.group+gi;
            print(" group %i, len %i, code %08X\n",gi,g->len,g->code);
            print("grouptable %08X st %08X greg %08X freg %08X\n",
                mem.group,&st,&st.g[0],&st.f[0]);
            if(g->code)
            {
                byte *code=g->code;
                print("Prefixed DWORDS: pc:%08X j1:%08X j2:%08X\n",
                    *(dword *)(code-14),
                    *(dword *)(code-8),
                    *(dword *)(code-4));
                for(i=0;i<1000;i++)
                {
                    if(*code==0xcc) break; // int 3 marks end
                    print("%08X: "NORMAL"%s\n",
                        (dword)code,
                        disasmx86(code,(int)code,&ia));
                    code+=ia;
                }
                ratio=(float)(code-g->code)/(g->len*4);
            }
            else i=0;
        }
        print("Total %i x86ops, expansion ratio %.2f\n",i,ratio);
    }
    else IFIS(p,"disasm")
    {
        char  file[256];
        dword base,cnt;
        p=param(&tp);
        strcpy(file,p);
        p=param(&tp);
        sscanf(p,"%i",&base);
        p=param(&tp);
        sscanf(p,"%i",&cnt);
        if(cnt>0)
        {
            print("Disassembling %08X..%08X to %s\n",
                base,base+cnt,file);
            disasm_dumpcode(file,base,cnt,0x10000000,0x40);
        }
        else
        {
            print("usage: disasm <file> <base> <cnt>\n");
        }
    }
    else IFIS(p,"disasmrsp")
    {
        char  file[256];
        dword base,cnt;
        p=param(&tp);
        strcpy(file,p);
        p=param(&tp);
        sscanf(p,"%i",&base);
        p=param(&tp);
        sscanf(p,"%i",&cnt);
        if(base>0 && cnt>0)
        {
            print("Disassembling ucode %08X,%08X to %s\n",
                base,base+cnt,file);
            disasm_dumpucode(file,base,4096,cnt,4096,0x80);
        }
        else
        {
            print("usage: disasmrsp <file> <base> <cnt>\n");
        }
    }
    else IFIS(p,"savemem")
    {
        char  file[256];
        dword base,cnt;
        p=param(&tp);
        strcpy(file,p);
        p=param(&tp);
        sscanf(p,"%i",&base);
        p=param(&tp);
        sscanf(p,"%i",&cnt);
        if(cnt>0)
        {
            FILE *f1;
            print("Saving %08X..%08X to %s\n",
                base,base+cnt,file);
            f1=fopen(file,"wb");
            if(f1)
            {
                int i;
                dword x;
                for(i=0;i<cnt;i+=4)
                {
                    x=mem_read32(base+i);
                    x=FLIP32(x);
                    fwrite(&x,1,4,f1);
                }
                fclose(f1);
            }
        }
        else
        {
            print("usage: disasm <file> <base> <cnt>\n");
        }
    }
    else IFIS(p,"stat")
    {
        a_stats();
    }
    else IFIS(p,"stat2")
    {
        a_stats2();
    }
    else IFIS(p,"stat3")
    {
        a_stats3();
    }
    else IFIS(p,"screen")
    {
        p=param(&tp);
        if(!p || *p<=32) p=NULL;
        rdp_screenshot(p);
    }
    else IFIS(p,"osinfo")
    {
        os_dumpinfo();
    }
    else IFIS(p,"clearosinfo")
    {
        os_clearthreadtime();
    }
    else IFIS(p,"emptyq")
    {
        print("Queues cleared\n");
        os_clearqueues();
    }
    else IFIS(p,"event")
    {
        int cnt,a;
        p=param(&tp);
        a=toupper(*p);
        if(a=='A') cnt=-1;
        else if(a=='S') cnt=OS_EVENT_SP;
        else if(a=='R') cnt=OS_EVENT_RETRACE;
        else cnt=atoi(p);
        if(cnt==-1)
        {
            for(cnt=0;cnt<10;cnt++)
            {
                print("Generated event %i\n",cnt);
                os_event(cnt);
            }
            cnt=15;
            print("Generated event %i\n",cnt);
            os_event(cnt);
        }
        else
        {
            print("Generated event %i\n",cnt);
            os_event(cnt);
        }
    }
    else IFIS(p,"send")
    {
        int cnt,data;
        p=param(&tp);
        cnt=atoi(p);
        p=param(&tp);
        data=atoi(p);
        print("Sending %i to queue %i\n",data,cnt);
        os_stuffqueue(cnt,data);
    }
    else return(0);
    return(1);
}

void command_fkey(int a)
{
    switch(a)
    {
    case KEY_F1:
        view_changed(VIEW_RESIZE);
//        view.shrink^=1;
        view.showhelp^=1;
        break;
    case KEY_F2:
        view.active=WIN_DATA;
        break;
    case KEY_F3:
        view.active=WIN_CODE;
        break;
    case KEY_F4:
        view.active=WIN_CONS;
        break;
    case KEY_F5:
        command("go");
        break;
    case KEY_F6:
        command("regs");
        break;
    case KEY_F7:
        command("f 1");
        break;
    case KEY_F8:
        command("s 1");
        break;
    case KEY_F9:
        command("n");
        break;
    case KEY_F12:
        print("3dfx screen flipped.\n");
        rdp_togglefullscreen();
        break;
    }
}

void command(char *cmd)
{
    char *tp,*p,*cs,*cd;
    int   a;
    char  buf[256];

    view_writeconsole("\x01\x17\r");
    con_cursorxy(0,0,0);
    view.consolecursor=-1;

    cs=cmd;
    while(*cs)
    {
        // copy next command (until eos or ;)
        cd=buf;
        while(*cs && *cs<=32) cs++;
        while(*cs && *cs!=';')
        {
            *cd++=*cs++;
        }
        *cd=0;
        if(*cs==';') cs++;
        while(*cs && *cs<=32) cs++;

        // get command name
        tp=buf;
        p=param(&tp);

        // execute
        a=0;
        if(!a) a=command_2(p,tp);
        if(!a) a=command_main(p,tp);
        if(!a) a=command_step(p,tp);
        if(!a) a=command_exam(p,tp);
        if(!a) a=command_dump(p,tp);
        if(!a) a=command_sym(p,tp);
        if(!a) a=command_misc(p,tp);
        if(!a) print("Unknown command '%s', try help.\n",p);
    }

    print(NULL);
}

