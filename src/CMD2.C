#include "ultra.h"

// copies/refs to cmd.c

#define IFIS(x,str) if(!stricmp(x,str))
#define IS(x,str) !stricmp(x,str)

extern void setaddress(char *text,int *addr);
extern char *param(char **tp);
extern qword atoq(char *p);

// secondary command routine. This is called first so this routine
// can override commands defined in cmd.c. It should return 1 if
// it processes a command, and 0 if not.

void printhelp2(void)
{
#if !RELEASE
    // help for command_2 routines
    print(
    "\x1\x3------------------ command2 ----------------------------------------------------------\n"
          "example <n>      - just an example, prints the number\n"
          "example2 <s> <n> - just an example, prints the string, number\n"
         );
#endif
}

int command_2(char *p,char *tp)
{
    if(0) { }
    else IFIS(p,"example")
    {
        int value;
        value=atoq(param(&tp));
        print("example(%i)\n",value);
    }
    else IFIS(p,"example2")
    {
        char txt[80];
        int value;
        strcpy(txt,param(&tp)); // NOTE strcpy, param uses a static buffer!
        value=atoi(param(&tp));
        print("example2(%s,%i)\n",txt,value);
    }
    else return(0);
    return(1);
}

