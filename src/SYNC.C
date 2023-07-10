// This file is a mess, look away :)
// real pain this audio syncronization! :)

#include <windows.h>
#include "ultra.h"

/* Sync routines:
**
** sync_checkretrace - called from cpu.c periodically, checks if retrace should be done
** sync_retrace      - a retrace occurred (called by the above)
** sync_gfxframedone - called by RDP.C when a new gfx frame is complete
** sync_audio        - checks and adjusts sound related syncing info (from sync_retrace)
**
*/

static Timer retracetimer;
static Timer longtimer;

#define SOUNDTARGET      20000   // try to keep 20Kbytes of sound in buffer

void sync_init(void)
{
    timer_reset(&retracetimer);
    timer_reset(&longtimer);
}

// These sync_* routines called from the main thread

void sync_audio(void)
{
    int a,b,bufsize;
    static int lastsndpos;

    if(st2.audiorequest)
    {
        // slist.c has found audio and wants to enable
        // directsound playback
        if(!st2.audioon)
        {
            if(!st.audiorate) st.audiorate=32000;
            print("Sound initialized: %ihz\n",st.audiorate);
            sound_init(st.audiorate);
            sound_start(st.audiorate);
            st2.audioon=1;
            remove("audio.wav");
        }
        st2.audiorequest=0;
    }

    if(st2.audioon)
    {
        int buffered;
        int target;

        target=SOUNDTARGET;
        /*
        if(st.audiorate<23000) target=SOUNDTARGET/2;
        else target=SOUNDTARGET;
        */

        a=sound_position(&bufsize);
        b=(a-lastsndpos); if(b<0) b+=bufsize;
        if(b<0 || b>bufsize) b=0;
        lastsndpos=a;
        st2.sync_soundused+=b;

        // we have audio, syncronize to it
        buffered=sound_buffered(); // how many bytes in buffer
        st2.audiobuffered=buffered-target;

        st2.audiobufferedsum+=st2.audiobuffered;
        st2.audiobufferedcnt++;

        {
            if(buffered>target*4)
            { // buffer overflowing, resync
                sound_resync(target);
                st2.audiostatus=9;
                st2.audioresync++;
                st2.audiobufferedcnt+=0x10000;
            }
            else if(buffered<4000)
            { // ran out of buffer! (4K safety)
                sound_resync(target*3);
                st2.audiostatus=-9;
                st2.audioresync++;
                st2.audiobufferedcnt+=0x10000;
            }
            else if(buffered>target*6/4)
            { // too much data, slow down
                st2.audiostatus=2;
                st2.audioresync++;
            }
            else if(buffered<target*2/4)
            { // too little data, more speed
                st2.audiostatus=-2;
            }
            else if(buffered>target*5/4)
            { // too much data, slow down
                st2.audiostatus=1;
            }
            else if(buffered<target*3/4)
            { // too little data, more speed
                st2.audiostatus=-1;
            }
            else
            { // about the correct amount, reset status when we cross target
                if(buffered<target && st2.audiostatus>0) st2.audiostatus=0;
                if(buffered>target && st2.audiostatus<0) st2.audiostatus=0;
            }
        }

        if(st.timing) a=51;
        else a=61;
             if(st2.audiostatus>1)  a-=8;
        else if(st2.audiostatus<-1) a+=16;
        else if(st2.audiostatus>0)  a-=3;
        else if(st2.audiostatus<0)  a+=8;
        st2.frameus=1000000/a;
    }

    if(st.retraces%60==0)
    {
        st2.sync_soundused=0;
        st2.sync_soundadd=0;
    }
}

void sync_retrace(void)
{
    int a;

    st2.lastretracecputime=st.cputime;

    // report framebuffer swap to os routines
    st.fb_current=st.fb_next;
    st.fb_next^=320*240*2;

    st2.retracetime=st.cputime;
    st.retraces++;

//    logh("--retrace--\n");
    os_event(OS_EVENT_RETRACE);
    st2.pendingretraces--;

    if(st.dumpinfo)
    {
//        print("--retrace-- audiobuf %8i (%3i, %.0fhz)\n",st2.audiobuffered,st2.audiostatus,1000000.0/st2.frameus);
        if(0)
        {
            float fr;
            fr=(60.0f/1000.0f)*timer_ms(&longtimer);
            logi("--retrace--(wait %2ims, time %7.2f, %2i pending)--",a,fr,st2.pendingretraces);
        }
        if(0)
        {
            static Timer tim;
            static int cnt;
            int    ms;
            if(!cnt) timer_reset(&tim);

            if(cnt>50)
            {
                ms=timer_ms(&tim);
                if(!ms) ms=1;

                print("ret/sec: %.2f (frameus=%.2f)\n",
                    1000.0*cnt/ms,
                    1000000.0/st2.frameus);

                timer_reset(&tim);
                cnt=0;
            }

            cnt++;
        }
    }

    a_cleardeadgroups();

    inifile_patches(-1);

    pad_frame();

    sync_audio();
}

// called by RDP when a frame sync is done
void sync_gfxframedone(void)
{
    st2.ops+=(int)(st.cputime-st.synctime);
    st.synctime=st.cputime;

    pad_drawframe();

    if(1)
    {
        logi("[sound: added %6i, used %6i, sync %+6i, status %2i]--\n",
            st2.sync_soundadd,
            st2.sync_soundused,
            st2.audiobuffered,
            st2.audiostatus);
    }

    st.frames++;
}

// called from cpu.c
void sync_checkretrace(void)
{
    static Timer retracetimer;
    int          us;

    us=timer_usreset(&retracetimer);
    if(us>1000000 || us<0) us=0;
    st2.usleft-=us;

    if(st2.usleft<0)
    {
        st2.pendingretraces++;

        if(!st2.frameus) st2.frameus=1000000/60;

        st2.usleft+=st2.frameus;
    }

    if(st2.pendingretraces>0)
    {
        static int cnt;
        if(os_eventqueuefree(OS_EVENT_RETRACE)) cnt++;
        else cnt=0;
        if(cnt>2)
        {
            cnt=0;

            if(st.nicebreak)
            {
                hw_check();
                st.breakout=1;
                st.nicebreak=0;
                return;
            }

            sync_retrace();

            if(st2.pendingretraces>9)
            {
                warning("retraces pending counter reset");
                st2.pendingretraces=0;
            }
        }
    }
}

