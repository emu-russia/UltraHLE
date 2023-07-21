#include "ultra.h"

// note: cart must be loaded first

void boot_boot(void)
{
    int a;

    // memory init (Must do before cpuinit!)
    mem_init(RDRAMSIZE);

    // cpu/compiler init
    cpu_init();

    // rsp
    rsp_init();

    // os emulator structures
    os_init();

    // map cart into memory (read only)
    for(a=0;a<cart.size;a+=4096)
    {
        mem_mapexternal(0x10000000+a,MAP_R,cart.data+a);
        mem_mapexternal(0x90000000+a,MAP_R,cart.data+a);
        mem_mapexternal(0xb0000000+a,MAP_R,cart.data+a);
    }

    // copy pif rom
#ifdef PIFROM
    memcpy(RPIF,pifRomImage,0x7c0);
    memcpy(WPIF,pifRomImage,0x7c0);
#endif

    cart.codebase=mem_read32(0x10000008);
    cart.codesize=0x100000; // guess, always same?

    if(cart.codesize>4096*1024)
    {
        print("error: codeblock too large");
        return;
    }

    if(mem_read32(0x10000540)!=0) cart.bootloader=1;

    if(cart.bootloader==1)
    {
        print("Alternate boot loader. ");
        cart.codebase&=~0x300000; // fzero
    }

    if(0)
    { // Load IPL3 into DMEM and see what happens :)  (IPL1 and IPL2 are skipped because they require a PIF-ROM)
        mem_writerangeraw(DMEM_ADDRESS+0x40,0xfc0,cart.data+0x40);
        cpu_goto(DMEM_ADDRESS+0x40);
    }
    else
    { // C-Boot
        mem_writerangeraw(cart.codebase,cart.codesize,cart.data+0x1000);
        cpu_goto(cart.codebase);
    }

    sym_load(cart.symname);

    st.framesync=cart.framesync;

    view.codebase=cart.codebase;
    view_changed(VIEW_CODE);
}

void boot(char *cartname,int nomemmap)
{
    view.hidestuff=1; view_changed(VIEW_RESIZE); flushdisplay();

    st.pc=0; // pc displayed in exceptions generated by load

    view_status("loading rom");
    flushdisplay();

    if(!cartname) cart_dummy();
    else cart_open(cartname,!nomemmap);

    reset();
}

void reset(void)
{
    view_status("booting rom");
    flushdisplay();

    view_status("loading ultra.ini");
    inifile_read(cart.title);

    boot_boot();

    // have to load a second time... boot_boot overwrites some stuff
    // should be fixed
    view_status("loading ultra.ini");
    inifile_read(cart.title);

    view.hidestuff=0; view_changed(VIEW_RESIZE); flushdisplay();

    sym_findfirstos();
    if(cart.isdocalls)
    {
        print("Creating OSCALLS list.\n");
        sym_demooscalls(); // generate oscalls
    }
    else
    {
        sym_addpatches();
    }

    inifile_patches(0);
}

