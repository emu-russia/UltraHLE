!ifdef RELEASE
copt = -Ox -GF -DWIN32 -DRELEASE=1
lopt = -incremental:no -release
!else
copt = -W3 -Z7 -O2 -DWIN32
lopt = -incremental:no -debug
!endif

uiobjs = ultrahle.obj loadsave.obj listview.obj

objs = $(uiobjs) main.obj cmd.obj cmd2.obj boot.obj \
       console.obj disasm.obj disasm86.obj debugui.obj log.obj \
       cpu.obj cpuc.obj cpua.obj cpuautil.obj cpuaold.obj cpuanew.obj \
       mem.obj sync.obj timer.obj inifile.obj \
       hw.obj rdp.obj dlist.obj slist.obj zlist.obj sound.obj \
       cart.obj pad.obj patch.obj os.obj sym.obj symfind.obj \

libs  = xglide.lib glide2x.lib
libs2 = winmm.lib user32.lib gdi32.lib dsound.lib comctl32.lib comdlg32.lib

ultra.exe: $(objs) $(libs) ultrahle.res
    link $(lopt) -out:$@ $(objs) $(libs) $(libs2) ultrahle.res

ultrahle.res : ultrahle.rc
    rc -r -DWIN32 ultrahle.rc

.c.obj:
    cl $(copt) -c -DWINVER=0x0400 $*.c

main.obj : version.h
sym.obj : osignore.h ospatch.h oscall.h
cpuanew.obj : noc.h
