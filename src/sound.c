#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>
#include "ultra.h"

extern void debugprint(char *str,...);

static LPDIRECTSOUND       ds;
static LPDIRECTSOUNDBUFFER sbuf;
static int                 bufrate=32000;
static int                 bufsize=256*1024;  // bytes
static int                 wpos;
static int                 initdone;

typedef struct
{
    BYTE        RIFF[4];          // "RIFF"
    DWORD       dwSize;           // Size of data to follow
    BYTE        WAVE[4];          // "WAVE"
    BYTE        fmt_[4];          // "fmt "
    DWORD       dw16;             // 16
    WORD        wOne_0;           // 1
    WORD        wChnls;           // Number of Channels
    DWORD       dwSRate;          // Sample Rate
    DWORD       BytesPerSec;      // Sample Rate
    WORD        wBlkAlign;        // 1
    WORD        BitsPerSample;    // Sample size
    BYTE        DATA[4];          // "DATA"
    DWORD       dwDSize;          // Number of Samples
} Waveheader;

static void dserror(HRESULT hErr,char *text)
{
    char *err;

	switch (hErr)
	{
        case DSERR_ALLOCATED :          err="DSERR_ALLOCATED";          break;
        case DSERR_CONTROLUNAVAIL :     err="DSERR_CONTROLUNAVAIL";     break;
        case DSERR_INVALIDPARAM :       err="DSERR_INVALIDPARAM";       break;
        case DSERR_INVALIDCALL :        err="DSERR_INVALIDCALL";        break;
        case DSERR_GENERIC :            err="DSERR_GENERIC";            break;
        case DSERR_PRIOLEVELNEEDED :    err="DSERR_PRIOLEVELNEEDED";    break;
        case DSERR_OUTOFMEMORY :        err="DSERR_OUTOFMEMORY";        break;
        case DSERR_BADFORMAT :          err="DSERR_BADFORMAT";          break;
        case DSERR_UNSUPPORTED :        err="DSERR_UNSUPPORTED";        break;
        case DSERR_NODRIVER :           err="DSERR_NODRIVER";           break;
        case DSERR_ALREADYINITIALIZED : err="DSERR_ALREADYINITIALIZED"; break;
        case DSERR_NOAGGREGATION :      err="DSERR_NOAGGREGATION";      break;
        case DSERR_BUFFERLOST :         err="DSERR_BUFFERLOST";         break;
        case DSERR_OTHERAPPHASPRIO :    err="DSERR_OTHERAPPHASPRIO";    break;
        case DSERR_UNINITIALIZED :      err="DSERR_UNINITIALIZED";      break;
        default :                       err="Unknown Error";            break;
	}

    print("DirectSound Error %08X at %s: %s\n", hErr,text,err);
	//PostQuitMessage(0);
}

int sound_init(int rate)
{
    HRESULT e;
    HWND    hwnd;

    if(initdone) return(0);

    hwnd=main_gethwnd();

    e=DirectSoundCreate(NULL,&ds,NULL);
    if(e!=DS_OK) return(-1);

    e=IDirectSound_SetCooperativeLevel(ds,hwnd,DSSCL_NORMAL);
    if(e!=DS_OK) return(-1);

    if(!ds) return(0);

    initdone=1;

    return(0);
}

void sound_start(int rate)
{
    HRESULT e;
    PCMWAVEFORMAT form;
    DSBUFFERDESC  desc;

    if(!initdone) return;

    bufrate=rate;

    // Set up wave format structure.
    memset(&form, 0, sizeof(PCMWAVEFORMAT));
    form.wf.wFormatTag         = WAVE_FORMAT_PCM;
    form.wf.nChannels          = 2;
    form.wf.nBlockAlign        = 4;
    form.wf.nSamplesPerSec     = bufrate;
    form.wf.nAvgBytesPerSec    = bufrate*4;
    form.wBitsPerSample        = 16;

	// Set up DSBUFFERDESC structure.
    memset(&desc, 0, sizeof(DSBUFFERDESC));
    desc.dwSize              = sizeof(DSBUFFERDESC);
//    desc.dwFlags             = DSBCAPS_STICKYFOCUS|DSBCAPS_CTRLFREQUENCY; // makes sound play even if window not active
    desc.dwFlags             = DSBCAPS_STICKYFOCUS;
    desc.dwBufferBytes       = bufsize;
    desc.lpwfxFormat         = (LPWAVEFORMATEX)&form;


    if((e=IDirectSound_CreateSoundBuffer(ds, &desc, &sbuf, NULL))!=DS_OK)
    {
        dserror(e,"createsoundbuffer");
        sbuf=NULL;
        return;
    }

    if((e=IDirectSoundBuffer_Play(sbuf,0,0,DSBPLAY_LOOPING))!=DS_OK)
    {
        dserror(e,"startsound");
    }

    wpos=bufsize/2;
}

void sound_stop(void)
{
    HRESULT e;
    if(!initdone) return;
    if((e=IDirectSoundBuffer_Stop(sbuf))!=DS_OK)
    {
        dserror(e,"stopsound");
    }
    IDirectSoundBuffer_Release(sbuf);
    sbuf=NULL;
}

// space in buffer (samples)

static int playpos;
static int safepos;

int  sound_buffered(void)
{
    int playpos,safepos,a;
    if(!initdone || !sbuf) return(0);

    IDirectSoundBuffer_GetCurrentPosition(sbuf,&playpos,&safepos);

    // advance wpos above safepos
    a=wpos-safepos;
    if(a<-bufsize/2) a+=bufsize;
    if(a> bufsize/2) a=bufsize-a;

	return(a);
}

int  sound_position(int *bufsizeptr)
{
    int playpos,safepos;
    if(!initdone || !sbuf) return(0);

    IDirectSoundBuffer_GetCurrentPosition(sbuf,&playpos,&safepos);

    if(bufsizeptr) *bufsizeptr=bufsize;
    return(playpos);
}

// add samples to buffer (no checking for space, call sound_buffered first)
int  sound_add(short *data0,int bytes)
{
    HRESULT e;
    char *data=(char *)data0;

    if(!initdone || !sbuf) return(-2);
    if(bytes<=0 || bytes>65535) return(-1);

    {
        LPVOID  data1;
        DWORD   size1;
        LPVOID  data2;
        DWORD   size2;
        if((e=IDirectSoundBuffer_Lock(sbuf,wpos,bytes,&data1,&size1,&data2,&size2,0))!=DS_OK)
        {
            dserror(e,"add1");
            return(1);
        }
        if(size1>0)
        {
            memcpy(data1,data,size1);
            data+=size1;
        }
        if(size2>0)
        {
            memcpy(data2,data,size2);
            data+=size2;
        }
        if((e=IDirectSoundBuffer_Unlock(sbuf,data1,size1,data2,size2))!=DS_OK)
        {
            dserror(e,"add2");
            return(1);
        }
        // print("soundadd: to %06X,%04X (%p/%05X,%p%/05X)\n",wpos,bytes,data1,size1,data2,size2);
        wpos+=bytes;
        wpos&=(bufsize-1);
    }

    return(0);
}

void sound_resync(int target)
{
    HRESULT e;
    LPVOID  data1;
    DWORD   size1;
    LPVOID  data2;
    DWORD   size2;
    int playpos,safepos;
    if(!initdone || !sbuf) return;

    IDirectSoundBuffer_GetCurrentPosition(sbuf,&playpos,&safepos);

    wpos=(safepos+target)&(bufsize-1);

    if((e=IDirectSoundBuffer_Lock(sbuf,safepos,target,&data1,&size1,&data2,&size2,0))!=DS_OK)
    {
        dserror(e,"");
        return;
    }
    if(size1>0)
    {
        memset(data1,0,size1);
    }
    if(size2>0)
    {
        memset(data2,0,size2);
    }
    if((e=IDirectSoundBuffer_Unlock(sbuf,data1,size1,data2,size2))!=DS_OK)
    {
        dserror(e,"");
        return;
    }
}

unsigned char wavheaderstereo[]={
0x52,0x49,0x46,0x46,0x32,0x00,0x00,0x00,
0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
0x12,0x00,0x00,0x00,0x01,0x00,0x02,0x00,
0x22,0x56,0x00,0x00,0x88,0x58,0x01,0x00,
0x04,0x00,0x10,0x00,0x00,0x00,0x64,0x61,
0x74,0x61,0x00,0x00,0x00,0x00,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0};

unsigned char wavheadermono[]={
0x52,0x49,0x46,0x46,0x32,0x00,0x00,0x00,
0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,
0x22,0x56,0x00,0x00,0x2b,0xec,0x00,0x00,
0x04,0x00,0x10,0x00,0x00,0x00,0x64,0x61,
0x74,0x61,0x00,0x00,0x00,0x00,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0};

static void wav_fixheader(FILE *f1,char *header,int stereo)
{
    int size,size1,size2,a;

    fseek(f1,0,SEEK_END);
    size=ftell(f1);
    size1=size-0x8;
    size2=size-0x8-0x2a;

    fseek(f1,0,SEEK_SET);
    fwrite(header,0x40,1,f1);

    fseek(f1,0x2a,SEEK_SET);
    fwrite(&size2,1,4,f1);

    fseek(f1,0x04,SEEK_SET);
    fwrite(&size1,1,4,f1);

    fseek(f1,0x18,SEEK_SET);
    fwrite(&bufrate,1,4,f1);

    a=bufrate*2;
    fseek(f1,0x1c,SEEK_SET);
    fwrite(&a,1,4,f1);

    fseek(f1,0,SEEK_END);
}

void sound_addwavfile(char *file,short *data,int cnt,int stereo)
{
    FILE *f1;
    static int dumpcnt=0;
    f1=fopen(file,"r+b");
    if(!f1)
    {
        f1=fopen(file,"wb");
        fwrite(wavheaderstereo,1,0x40,f1);
    }
    fseek(f1,0,SEEK_END);
    fwrite(data,1,cnt,f1);
    if(stereo) wav_fixheader(f1,wavheaderstereo,1);
    else       wav_fixheader(f1,wavheadermono,0);
    fclose(f1);

    /*
    if(dumpcnt<30000)
    {
        cnt>>=1;
        for(i=0;i<cnt;i++) if(data[i]) break;
        if(i==cnt && !dumpcnt) return;
        dumpcnt+=cnt;
        for(i=0;i<cnt;i++)
        {
            loga("\n%5i: %6i wave",dumpcnt+i,data[i]);
            printcurve(data[i],30000);
        }
    }
    */
}

void sound_debugsavebuffer(char *file)
{
    FILE *f1;
    f1=fopen(file,"wb");
    {
        HRESULT e;
        LPVOID  data1;
        DWORD   size1;
        LPVOID  data2;
        DWORD   size2;
        if((e=IDirectSoundBuffer_Lock(sbuf,0,bufsize,&data1,&size1,&data2,&size2,0))!=DS_OK)
        {
            dserror(e,"");
            return;
        }
        if(size1>0)
        {
            fwrite(data1,1,size1,f1);
        }
        if(size2>0)
        {
            fwrite(data1,2,size2,f1);
        }
        if((e=IDirectSoundBuffer_Unlock(sbuf,data1,size1,data2,size2))!=DS_OK)
        {
            dserror(e,"");
            return;
        }
        print("(%p/%05X,%p/%05X)\n",data1,size1,data2,size2);
    }
    fclose(f1);
    print("sound: debugsavebuffer %s\n",file);
}

