//*************************************************************************
//  FILENAME:   TriOS.cpp
//  AUTHOR:     SCSimonoff 10-1-96
//  CPU:        Portable but contitionally compiles for x86 optimizations.
//  O/S:        Windows or Linux
//
//      Portable OS Utility Functions.
//
//      Compile with /D WIN32 or WIN64 for Windows instead of Linux.
//      Compile with /D NOI86 for C-only without Intel x86 assembly.
//      Compile with /D DOGUI for GUI API (OSBrowser,OSAskFont,OSAskColor).
//      Compile with /D DOSASK for OSFlAsk under Windows not just Linux.
//      Compile with /D DOPCAP for WinPcap raw ethernet Windows support.
//      Compile with /D OSBASE to omit OSCleanup,OSConnect,OSConSend,OSConRecv
//      Compile with /D CNWIDE to omit UTF8 Unicode filename support.
//      Compile with /D CNOBRK to stop log CDEBUG WIN32 BREAK3 on the fly debug.
//      Compile with /D CNABRK to stop log CDEBUG WIN32 BREAK3 debug for OSAlloc.
//      Compile with /D CWINIX for Windows simulation of Unix environment.
//      Compile with /D CDEBUG for debugging version (exit beep if memory leak).
//      Compile with /D XDEBUG for special memory overwrite testing.
//      Compile with /D DO_MACAD to enable returning raw mac address in OSConnect.
//      Compile with /D DO_WARNS to allow depreciated code with compiler warnings.
//      Compile with /D MS_LINUX to enable millisec OSTickCount in Linux.
//      Note that CI_DONTWAIT and CI_WAITALL are not supported by Windows.
//      The C-only implementation requires the C runtime library (LIBC.LIB).
//
//      For Windows link with ws2_32.lib and wininet.lib (unless OSBASE defined)
//      For Windows normally link applications with the following libraries:
//      zlib.lib;wpcap.lib;ws2_32.lib;iphlpapi.lib;htmlhelp.lib;wininet.lib;
//      For Linux compile with -Wno-write-strings -Wno-format
//      For Linux link with -lpthread -lrt -lm
//      Note this file has a cpp extension but does not use any C++ features.
//      For Linux it can optionally be compiled as C rather than C++ with: -x c
//
//  This software was created 1996-2021 by SCSimonoff for Triscape Inc.
//  This may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifdef  _WIN32                                  //Before we include CDefs.h
#define WIN32 1                                 //so duplicate definitions so
#define ISWIN 1                                 //we can include correct OS
#endif
#ifdef  _WIN64
#define WIN64 1
#define ISWIN 1
#endif

#ifdef ISWIN                                    //=== Windows
#ifdef DOPCAP                                   //---
#include <pcap.h>
#endif                                          //---
#include <Windows.h>
#include <Wininet.h>
#include <richedit.h>
#include <math.h>
#include <Shlobj.h>
#ifdef CDEBUG                                   //---
#include <crtdbg.h>
#endif                                          //---
#ifdef DO_MACAD                                 //---
#include <iphlpapi.h>                           //GetAdaptersInfo (use IPHLPAPI.lib too)
#endif                                          //---
#define ASM __asm

#else                                           //=== Linux
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/vfs.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <semaphore.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#endif                                          //===

#include "TriOS.h"                              //Includes CDefs.h

// Functions in this file.

CSTYLE_BEG
void* xproc     OSAlloc (aword bytes);
void* xproc     OSReAlloc (void* mem, aword bytes);
void* xproc     OSFree (void* mem);

qword xproc     OSMemory (int mode);
dword xproc     OSMemCount (int mode);
void xproc      OSCleanup (int mode);

void xproc      OSMemClear (void* pdest, aword bytes);
void xproc      OSMemFillB (void* pdest, byte value, aword bytes);
void xproc      OSMemFillD (void* pdest, dword value, aword bytes);
void xproc      OSMemCopy (void* pdest, void* psource, aword bytes);
void xproc      OSMemCopyR (void* pdest, void* psource, aword bytes);
aint xproc      OSMemMatch (void* pmem1, void* pmem2, aword bytes);
aint xproc      OSMemDiffs (void* pmem1, void* pmem2, aword bytes);
aint xproc      OSMemFindB (byte find, void* pmem, aword bytes);
aint xproc      OSMemFindD (dword find, void* pmem, aword dwords);
aint xproc      OSMemFindND (dword find, void* pmem, aword dwords);
aint xproc      OSMemSearch (lpbyte* ppmem, aword* pcmem, byte* pfind, aword cfind, int minsz);

uint xproc      OSTxtSize (text* string);
text* xproc     OSTxtCopy (text* pdest, text* psource);
text* xproc     OSTxtStop (text* pdest, text* psource, uint max);
text* xproc     OSTxtSafe (lptext* ppdest, text* psource, uint max);
uint xproc      OSTxtMany (int mode, lptext* ppmany, text* pone);
text* xproc     OSTxtSubs (int mode, lptext* ppmany, text* pone, lint index);

int xproc       OSTxtCmp (text* ptxt1, text* ptxt2);
int xproc       OSTxtCmpU (text* ptxt1, text* ptxt2);
int xproc       OSTxtCmpS (text* ptxt1, text* ptxt2);
int xproc       OSTxtCmpN (text* ptxt1, text* ptxt2);
int xproc       OSTxtSkip (text* ptxt1, text* ptxt2, lptext* ppskip);
text* xproc     OSTxtFind (text* ptxt, text* pdoc, text* pend, int mode);
text* xproc     OSTxtQuad (text* pdest, lint ext);
text* xproc     OSTxt2MIP (int mode, byte* pmac, text* ptxt);

int xproc       OSWideUTF8 (int mode, wtxt* pwtxt, text* ptxt, int num, uint max);
text* xproc     OSDateTime (text* pdest, qdtime* pout, qdtime* puse, int mode);
dword xproc     OSTickCount ();

int xproc       OSFileRoot (text* pbuf, text* pname);
int xproc       OSFileDir (text* pbuf, text* pname);
int xproc       OSFilePar (text* pbuf, text* pname);
int xproc       OSFileBase (text* pbuf, text* pname, int ext);
int xproc       OSFileType (text* pbuf, text* pname, text* pext);
lint xproc      OSFileExt (text* pbuf, text* pname);
lint xproc      OSNameInt (text* pname, lptext* ppend);
text* xproc     OSLocation (text* pbuf, int mode);
int xproc       OSDirCur (text* pdir);

lint xproc      OSNumGet (text* ptext, lptext* ppend);

text* xproc     OSIntPut (lint num, text* ptext);
lint xproc      OSIntGet (text* ptext, lptext* ppend);
text* xproc     OSIntPutQ (qint num, text* ptext);
qint xproc      OSIntGetQ (text* ptext, lptext* ppend);
text* xproc     OSIntPutU (dword num, text* ptext);

text* xproc     OSHexPut (dword num, text* ptext);
dword xproc     OSHexGet (text* ptext, lptext* ppend);
text* xproc     OSHexPutQ (qword num, text* ptext);
qword xproc     OSHexGetQ (text* ptext, lptext* ppend);

text* xproc     OSFloatPut (flx num, text* buf, int width, int sigs);
flx xproc       OSFloatGet (text* buf, lptext* end);
void xproc      OSFloatEven (flx begin, flx delta, int tics, fflx* pfirst, fflx* pinc);

flx xproc       OSSqrt (flx value);
flx xproc       OSSin (flx value);
flx xproc       OSCos (flx value);
flx xproc       OSArcTan (flx value);
flx xproc       OSArcTan2 (flx yval, flx xval);

int xproc       OSInterp (int mode, flx* Y2, int npt2, flx beg2, flx end2, flx beg, flx end, flx* X, flx* Y, int npts);
flx xproc       OSSpline (int mode, flx xval, flx* pV, flx* pX, int npts);
flx xproc       OSRaise (flx base, flx exp);
flx xproc       OSLog10 (flx value);

int xproc       OSDebug  (int mode, text* pline);
void xproc      OSPrint  (int mode, text* pout, lint hex);
void xproc      OSPrintI (int mode, text* pout, lint dec);
void xproc      OSPrintF (int mode, text* pout, flt f1, flt f2, lint dec, lint hex);
void xproc      OSPrintS (int mode, text* pout, text* ptwo, lint hex);

int xproc       OSBrowser (int mode, ftext* purl);
int xproc       OSAskFile (int mode, ftext* ptitle, ftext* type, ftext* filter, ftext* pname, OSApp* papp);
int xproc       OSAskFont (int mode, SFont* pfont, OSApp* papp);
int xproc       OSAskColor (int mode, lint* pcolor, OSApp* papp);
int xproc       OSMessageBox (int mode, ftext* ptitle, ftext* ptext, ftext* pname, OSApp* papp);
int xproc       OSFlAsk (int mode, text* pask, OSApp* papp);

int xproc       OSWideRich (int mode, hand hctl, lptext* pptxt, uint min, uint max);

int xproc       OSExec (int mode, text* papp, text* parg);
void xproc      OSExit (uint code);
void xproc      OSBeep (int mode);

int xproc       OSTask (int mode, lpvoid* pphand, LPFTASK pfunc, void* parm);
int xproc       OSLock (int mode, lpvoid* pplock);
int xproc       OSWait (int mode, OSWAITS* pwaits, void* plock, lint timeout);
int xproc       OSMapMem (int mode, dword key, lpvoid* phand, lpvoid* ppmem, dword memsz);
lint xproc      OSLastError (int mode);

int xproc       OSAddLog (int mode, text* plog, text* pmsg, int code, lint numb, text* pnam);

aint xproc      OSOpen (int mode, text* pfilen);
lint xproc      OSSeek (aint file, lint move, int org);
qint xproc      OSHuge (aint file, qint move, int org);
lint xproc      OSRead (aint file, void* pv, dword cb);
lint xproc      OSWrite (aint file, void* pv, dword cb);
int xproc       OSClose (aint file);

int xproc       OSSync (aint file);
int xproc       OSChop (aint file);
int xproc       OSErase (text* pfilen);
int xproc       OSRename (text* poldn, text* pnewn);
int xproc       OSDirNew (int mode, text* pdir);
int xproc       OSRegIni (int mode, IReg* preg, text* psec, text* pkey, text* pval);
int xproc       OSReg (int mode, text* pfile, text* pkey, text* pname, text* pvalue);

int xproc       OSExists (int mode, text* pname);
int xproc       OSFindFile (int mode, text* pname, FFHand* phand);
int xproc       OSClipboard(int mode, text* ptxt, int ntxt, hand hwin);
int xproc       OSTemporary (int mode, ftext* pname, text* pdir, text* pbase, text* pext);
int xproc       OSDiskDrive (int mode, text* proot);
qword xproc     OSDiskSpace (int mode, ftext* pdisk);
int xproc       OSComPort (int mode, int numb, byte* pdat, ComInfo* pcom);
dword xproc     OSMMap (int mode, dword addr, dword used, CMMap* pmap);

aint xproc      OSConnect (int mode, aint sock, text* paddr, int port, ConEx* pex);
int xproc       OSConSend (aint sock, void* pv, dword cv, ConEx* pex);
int xproc       OSConRecv (aint sock, void* pv, dword cv, ConEx* pex);

aint xproc      OSipcPipe (int mode, aint hpipe, text* pname);
int xproc       OSipcSend (aint hpipe, void* pv, dword cv);
int xproc       OSipcRecv (aint hpipe, void* pv, dword cv);

dword xproc     OSRandom (dword seed);
void xproc      OSCipher (lint mode, dword* pkey, void* pdata, lint bytes);
void xproc      OSPassword (int mode, dword* pkey, text* pass);
void xproc      OSInputArg (text* prompt, text* line, lptext** pargv);

int  xproc      Similar (lint* pone, lint* ptwo, lint npts, flt frac);
int  xproc      OSTest (int mode, text* pdir);
CSTYLE_END

static int      OSFloatOneTen(flx* num);

#ifdef __cplusplus                              //===
int xproc       OSReadWeb (int mode, StmFile* pfile, text* purl);

//StmFile::StmFile ();
//StmFile::~StmFile ();
//isproc StmFile::StmOpen (int mode, text* pfilen);
//isproc StmFile::StmClose ();
//isproc StmFile::StmChop (int mode);
//isproc StmFile::Read (void* pv, dword cb, dword* pcbRead);
//isproc StmFile::Write (void* pv, dword cb, dword* pcbDone);
//isproc StmFile::Seek (qint move, int org, qword* pnewpos);
//isproc StmFile::AddRef ();
//isproc StmFile::Release ();

csproc StmRead (fStm* pstream, fvoid* pv, dword cb);
csproc StmWrite (fStm* pstream, fvoid* pv, dword cb);
csproc StmSeek (fStm* pstream, long pos, int org, dword* poff);
csproc StmLoad (fStm* pstream, fvoid* ppmem, fdword* pbytes);
csproc StmMemory (lpStm* ppstm, void* pmem, lint cmem);
csproc StmErrors (int mode, int ecode);

ctproc(fStm*)  StmFOpen (int mode, ftext* pfilen);
csproc         StmFClose (fStm* file);

CSTYLE int     StmFRead (void* file, fvoid* pv, int cb);
CSTYLE int     StmFWrite (void* file, fvoid* pv, int cb);
#endif                                          //=== __cplusplus

CSTYLE void*   FOSAlloc (int bytes);
CSTYLE void*   FOSFree (void* mem);

// These routines are actually macros defined in TriOS.h:
// OSConsole:   Get whole-line keyboard input with prompt on command line.
// OSConGets:   Get whole-line keyboard input without prompt.
// OSConLine:   Show prompt or line of text on command line.
// OSKeyHit:    Get command line key press or 0 if none.
// OSSleep:     Wait for given milliseconds.
// OSCOPY:      In-place copy block of bytes.
// OSCOPYD:     In-place copy block of dwords.
// OSFILLB:     In-place fill block with byte values.
// OSFILLD:     In-place fill block with dword values.

#ifdef CDEBUG   //-------
// Used for memory leak tests.
// Must use power of two for NDALL.
#define NDALL   (1024 * 64)
void*   DAllPt[NDALL] = {NULL};
aword   DAllSz[NDALL] = {0};
aword   DTotalA = 0;
aword   DTotalB = 0;
OSWAITS DMutexM = {0};

#define XDPADS  256
#endif          //-------

// Used by OSAddLog to divide application instances.
// Used by OSPrint to avoid infinate error popup.
// 1 (TRUE) if OSAddLog used, 2 (TRUETWO) OSPrint error popup used.
dword   s_First = 0;

//*************************************************************************
//  OSAlloc:
//      Allocates fixed memory.
//      Fixed memory need not be locked or unlocked.
//
//  Arguments:
//      void* OSAlloc (aword bytes)
//      bytes   Desired byte size of memory.
//
//  Returns:
//      Non-null pointer to allocated memory or NULL if error.
//*************************************************************************

CSTYLE void* xproc
OSAlloc (aword bytes) {

#ifndef CDEBUG      //-------
    #ifdef ISWIN    //===
    return( GlobalAlloc(GMEM_FIXED, bytes) );
    #else           //===
    return( malloc((size_t)bytes) );
    #endif          //===

#else               //-------
#ifdef XDEBUG       //---
    void* ptr;
    dword total = bytes + XDPADS+XDPADS+4;
    void* mem = GlobalAlloc(GMEM_FIXED, total);
    if (mem) {
        OSMemFillB(mem, 0x55, total);
        *((dword*)mem) = total;
        ptr = ADDOFF(void*, mem, (XDPADS+4));
    }
#else               //---
    #ifdef ISWIN    //===
    void* ptr = GlobalAlloc(GMEM_FIXED, (SIZE_T)bytes);
    #else           //===
    void* ptr = malloc((size_t)bytes);
    #endif          //===
#endif              //---
    if (ptr) {
        if (DMutexM.alloc == 0) {
            OSWait(OS_LOCKNEW, &DMutexM, NULL, 0);
        }
        OSWait(OS_GRAB, &DMutexM, NULL, 0);
        int i = A32(ptr) & (NDALL-1);
        int c = NDALL;
        do {
            if (DAllPt[i] == NULL) {
                DAllPt[i] = ptr;
                DAllSz[i] = bytes;
                break;
            }
            i += 1;
            if (i >= NDALL) i = 0;
        } while (--c);
        OSWait(OS_DONE, &DMutexM, NULL, 0);
        if (c == 0) {
            OSBeep(0);
            OSAddLog(2, NULL, "OSAlloc: No DAllPt Space", A32(ptr), (dword)bytes, NULL);
        }
    } else {
        OSBeep(0);
        #ifndef CNABRK                          //---
        #ifdef ISWIN                            //===
        errc e = GetLastError();
        MEMORYSTATUS stat;
        stat.dwLength = sizeof(stat);
        GlobalMemoryStatus(&stat);
        #else                                   //===
        errc e = ECMEMORY;
        #endif                                  //===
        OSAddLog(2, NULL, "OSAlloc: Not allocated", e, (dword)bytes, NULL);
        #endif                                  //---
    }
    //Not really needed (uncomment if you want to know).
    //OSMemCount(0);
    return(ptr);
#endif              //-------
}

//*************************************************************************
//  OSReAlloc:
//      Grows or shrinks previously allocated memory.
//      Fixed memory need not be locked or unlocked.
//
//  Arguments:
//      void* OSReAlloc (void* mem, aword bytes)
//      mem     Pointer to previously allocated memory.
//              If NULL then calls OSAlloc to allocate initial memory.
//      bytes   Desired new byte size of memory.
//
//  Returns:
//      Non-null pointer to allocated memory or NULL if error.
//*************************************************************************

CSTYLE void* xproc
OSReAlloc (void* mem, aword bytes) {
    #ifdef ISWIN                                //===
    void* ptr;
    aword total;
    #endif                                      //===

    if (mem == NULL) {                          //no previously allocated mem?
        return( OSAlloc(bytes) );               //allocate initial memory
    }

#ifndef CDEBUG      //-------
    #ifdef ISWIN                                //===
    //It may crash Vista to realloc smaller or maybe just realloc.
    //So to be safe, we now do it ourselves (which may waste memory).
    //ptr = GlobalReAlloc(mem, bytes, GMEM_FIXED);
    //if (ptr) return(ptr);

    total = (aword) GlobalSize(mem);
    ptr = GlobalAlloc(GMEM_FIXED, bytes);
    if (ptr) {
        if (bytes < total) total = bytes;
        if (total) OSMemCopy(ptr, mem, total);
    }
    GlobalFree(mem);
    if (ptr) return(ptr);

// There seems to be a Win95 bug which does gives a 212 "locked" error
// when GlobalReAlloc is used for small fixed blocks.
// Try to do the realloc manually if we encounter an error.

    ptr = GlobalAlloc(GMEM_FIXED, bytes);
    if (ptr) {
        OSMemCopy(ptr, mem, (aword)GlobalSize(mem));
        GlobalFree(mem);
    }
    return(ptr);
    
    #else                                       //===
    return( (void*) realloc(mem, (size_t)bytes) );
    #endif                                      //===

#else               //-------
#ifdef XDEBUG       //---
    ptr = SUBOFF(void*, mem, (XDPADS+4));
    #ifdef ISWIN    //===
    total = GlobalSize(ptr) - (XDPADS+XDPADS+4);
    ptr = OSAlloc(bytes);
    if (ptr) {
        if (bytes < total) total = bytes;
        if (total) OSMemCopy(ptr, mem, total);
    }
    OSFree(mem);
    #else           //===
    ptr = (void*) realloc(ptr, (size_t)bytes);
    #endif          //===
    return(ptr);
#else               //---

    #ifdef ISWIN                                //===
    total = GlobalSize(mem);
    ptr = GlobalAlloc(GMEM_FIXED, (SIZE_T)bytes);
    if (ptr) {
        if (bytes < total) total = bytes;
        if (total) OSMemCopy(ptr, mem, total);
    }
    GlobalFree(mem);
    #else                                       //===

    ptr = (void*) realloc(mem, (size_t)bytes);
    #endif                                      //===

    if (ptr) {
        if (DMutexM.alloc == 0) {
            OSWait(OS_LOCKNEW, &DMutexM, NULL, 0);
        }
        OSWait(OS_GRAB, &DMutexM, NULL, 0);
        int i = A32(mem) & (NDALL-1);
        int c = NDALL;
        do {
            if (DAllPt[i] == mem) {
                DAllPt[i] = NULL;
                DAllSz[i] = 0;
                break;
            }
            i += 1;
            if (i >= NDALL) i = 0;
        } while (--c);
        OSWait(OS_DONE, &DMutexM, NULL, 0);
        if (c == 0) {
            OSBeep(0);
            OSAddLog(2, NULL, "OSReAlloc: Not found in DAllPt", A32(mem), (dword)bytes, NULL);
        }
        OSWait(OS_GRAB, &DMutexM, NULL, 0);
        i = A32(ptr) & (NDALL-1);
        c = NDALL;
        do {
            if (DAllPt[i] == NULL) {
                DAllPt[i] = ptr;
                DAllSz[i] = bytes;
                break;
            }
            i += 1;
            if (i >= NDALL) i = 0;
        } while (--c);
        OSWait(OS_DONE, &DMutexM, NULL, 0);
        if (c == 0) {
            OSBeep(0);
            OSAddLog(2, NULL, "OSReAlloc: No DAllPt Space", A32(ptr), (dword)bytes, NULL);
        }
    } else {
        OSBeep(0);
        #ifdef ISWIN                            //===
        errc e = GetLastError();
        MEMORYSTATUS stat;
        stat.dwLength = sizeof(stat);
        GlobalMemoryStatus(&stat);
        #else                                   //===
        errc e = ECMEMORY;
        #endif                                  //===
        OSAddLog(2, NULL, "OSReAlloc: Not allocated", e, (dword)bytes, NULL);
    }
    //Not really needed (uncomment if you want to know).
    //OSMemCount(0);
    return(ptr);
#endif              //---
#endif              //-------
}

//*************************************************************************
//  OSFree:
//      Fress previously allocated memory.
//
//  Arguments:
//      void* OSFree (void* mem)
//      mem     Pointer to previously allocated memory.
//              Does nothing if NULL.
//
//  Returns:
//      NULL if successfully freed.
//      Returns mem pointer if failed to free.
//*************************************************************************

CSTYLE void* xproc
OSFree (void* mem) {
    if (mem == NULL) return(NULL);
#ifdef CDEBUG       //-------
    if (DMutexM.alloc == 0) {
        OSWait(OS_LOCKNEW, &DMutexM, NULL, 0);
    }
    OSWait(OS_GRAB, &DMutexM, NULL, 0);
    int i = A32(mem) & (NDALL-1);
    int c = NDALL;
    do {
        if (DAllPt[i] == mem) {
            DAllPt[i] = NULL;
            DAllSz[i] = 0;
            break;
        }
        i += 1;
        if (i >= NDALL) i = 0;
    } while (--c);
    OSWait(OS_DONE, &DMutexM, NULL, 0);
    //Not really needed (uncomment if you want to know).
    //OSMemCount(0);
    if (c == 0) {
        OSBeep(2);
        text line[SZDISK];
        text* ptxt = line;
        int cnt = 8;
        dword* pdw = (dword*) mem;
        do {
            ptxt = OSHexPut(*(dword*)pdw, ptxt);
            pdw += 1;
            *ptxt++ = ',';
            *ptxt = 0;
        } while (--cnt);
        dword bytes = 0;
        #ifdef ISWIN    //===
        bytes = (dword) GlobalSize(mem);
        #endif          //===
        OSAddLog(2, NULL, "OSFree: Not found in DAllPt", A32(mem), bytes, line);
    }
#ifdef XDEBUG       //---
    dword* ptr;
    ptr = SUBOFF(dword*, mem, (XDPADS+4));
    dword total = GlobalSize(ptr);
    dword bytes = *ptr++;
    if (bytes != total) {
        OSBeep(2);
    }
    lint nmem = XDPADS/4;
    do {
        if (*ptr++ != 0x55555555) {
            OSBeep(2);
            text line[SZDISK];
            text* ptxt = line;
            int cnt = 8;
            dword* pdw = (dword*) mem;
            do {
                ptxt = OSHexPut(*(dword*)pdw, ptxt);
                pdw += 1;
                *ptxt++ = ',';
                *ptxt = 0;
            } while (--cnt);
            OSAddLog(2,NULL,"OSFree: XDEBUG Begin Overwrite",*(ptr-1),nmem,line);
            break;
        }
    } while (--nmem);
    ptr = ADDOFF(dword*, mem, (bytes - (XDPADS+XDPADS+4)));
    nmem = XDPADS/4;
    do {
        if (*ptr++ != 0x55555555) {
            OSBeep(2);
            text line[SZDISK];
            text* ptxt = line;
            int cnt = 8;
            dword* pdw = (dword*) mem;
            do {
                ptxt = OSHexPut(*(dword*)pdw, ptxt);
                pdw += 1;
                *ptxt++ = ',';
                *ptxt = 0;
            } while (--cnt);
            OSAddLog(2,NULL,"OSFree: XDEBUG End Overwrite",*(ptr-1),nmem,line);
            break;
        }
    } while (--nmem);
    mem = SUBOFF(void*, mem, (XDPADS+4));
#endif              //---
#endif              //-------
    #ifdef ISWIN    //===
    #ifdef CDEBUG   //---
    mem = GlobalFree(mem);
    if (mem) {
        OSAddLog(2,NULL,"OSFree: GlobalFree error",A32(mem),GetLastError(),NULL);
    }
    return(mem);
    #else           //---
    return( GlobalFree(mem) );
    #endif          //---
    #else           //===
    free(mem);
    return(NULL);
    #endif          //===
}

//*************************************************************************
//  OSMemory:
//      Returns the total number of bytes of physical memory.
//      There is typically more virtual than physical memory, but the physical
//      memory is a better indicator guide to how much memory to allocate.
//
//      The Linux version works on linux-gnu, solaris2 and cygwin.
//      See www.Codase.com "physmem.c" for other Unix systems.
//      Unfortunately, Cygwin returns 64KB as the "page size" instead of 4KB
//      (because Windows minumum memory map size is 64KB).
//      We fix this with a hack.
//
//  Arguments:
//      qword OSMemory (int mode)
//      mode    Reserved for future use.  Must be zero.
//
//  Returns:
//      Total number of bytes of physical memory installed in computer.
//      Returns 0 if error or unknown.
//*************************************************************************

CSTYLE qword xproc
OSMemory (int mode) {
    #ifdef ISWIN                                //=== Windows
    MEMORYSTATUSEX stat;

    stat.dwLength = sizeof (stat);
    stat.ullTotalPhys = 0;                      //just in case
    GlobalMemoryStatusEx(&stat);
    return(stat.ullTotalPhys);

    #else                                       //=== Linux
    int pages, pagesize;
    qword bytes;

    pages = sysconf (_SC_PHYS_PAGES);
    if (pages < 0) return(0);
    pagesize = sysconf (_SC_PAGESIZE);
    if (pagesize < 0) return(0);
    if ((pagesize == (64*1024))&&(pages > 163840)) {
        pagesize = 4096;                        //correct cygwin pagesize bug
    }
    bytes = (qword)pages * (qword)pagesize;
    return(bytes);
    #endif                                      //===
}

//*************************************************************************
//  OSMemCount:
//      Debugging routine to count the number of allocated memory blocks
//      or the total size of all allocated blocks.
//      Also sets DTotalA global dword to number of allocated blocks.
//      Also sets DTotalB global dword to number of allocated bytes.
//      Returns zero unless compiled with /D "CDEBUG".
//
//      Warning: Should be DMutexM locked for totally accurate total counts.
//      But we do not bother since DTotalA and DTotalB are not really used.
//
//  Arguments:
//      dword OSMemCount (int mode)
//      mode    0 = Return number of allocated memory blocks.
//              1 = Return total byte size of allocated memory blocks.
//
//  Returns:
//      Number of allocated block or total allocated bytes.
//*************************************************************************

CSTYLE dword xproc
OSMemCount (int mode) {
#ifndef CDEBUG      //-------
    return(0);
#else               //-------
    aword a, b;
    int i;

    a = 0;
    b = 0;
    i = 0;
    while (i < NDALL) {
        if (DAllPt[i]) {
            a += 1;
            b += DAllSz[i];
        }
        i += 1;
    }
    DTotalA = a;
    DTotalB = b;
    return(O32((mode & 1) ? b : a));
#endif              //-------
}

#ifndef OSBASE      //--- (Omit because needs special link libraries?)
//*************************************************************************
//  OSCleanup:
//      Called just before the application exists to check for unfreed memory.
//      Does error beep if any unfreed memory is present.
//      Does short beep if OSAlloc (GlobalAlloc) memory leak.
//      Does longer stop beep if CRT (new) memory leak.
//      Also dumps error info to debugger for CRT memory leak.
//      Does nothing unless compiled with /D "CDEBUG".
//
//  Arguments:
//      void OSCleanup (int mode)
//      mode    0 = Normal full exit check.
//              1 = Just check for OSAlloc/OSReAlloc memory leaks.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSCleanup (int mode) {
#ifdef CDEBUG       //-------
    boolv error;
    int i;

    error = FALSE;
    if (mode != 1) {
        if (DMutexM.alloc) {
            OSWait(OS_LOCKOFF, &DMutexM, NULL, 0);
        }

        #ifdef ISWIN                            //===
        //if ( CrtDumpMemoryLeaks() ) {        //look for C runtime leak
        //    OSAddLog(2, NULL, "OSCleanup: CrtDumpMemoryLeaks", 0, 0, NULL);
        //    error = TRUE;                     //error if new without delete
        //}
        #endif                                  //===
    }

    i = 0;
    while (i < NDALL) {
        if (DAllPt[i]) {
            void* mem = DAllPt[i];
            text line[SZDISK];
            text* ptxt = line;
            int cnt = 8;
            dword* pdw = (dword*) mem;
            do {
                ptxt = OSHexPut(*(dword*)pdw, ptxt);
                pdw += 1;
                *ptxt++ = ',';
                *ptxt = 0;
            } while (--cnt);
            dword bytes = 0;
            #ifdef ISWIN    //===
            bytes = (dword) GlobalSize(mem);
            #endif          //===
            OSAddLog(2, NULL, "OSCleanup: DAllPt Memory Leak", A32(mem), bytes, line);
            OSFree(DAllPt[i]);
            error = TRUE;
        }
        i += 1;
    }
    if (error) {
        OSBeep(2);
    }
#endif              //-------
    return;
}
#endif              //--- OSBASE

//*************************************************************************
//  OSMemClear:
//      Zeros a block of memory.
//
//  Arguments:
//      void OSMemClear (void* pdest, aword bytes)
//      pdest   Pointer to memory to be cleared.
//      bytes   Number of bytes to zero.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSMemClear (void* pdest, aword bytes) {
    #ifdef NOI86                                //-------
    memset(pdest, 0, (size_t)bytes);
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    aword cnt = bytes >> 3;
    __stosq((PDWORD64)pdest, 0, cnt);
    __stosb(ADDOFF(PBYTE, pdest, (cnt << 3)), 0, (bytes & 7));
    #else                                       //===
    asm ( "                                     \
        movq    %0, %%rdi       ;               \
        movq    %1, %%rcx       ;               \
        cld                     ;               \
        mov     %%rcx, %%rdx    ;               \
        xor     %%rax, %%rax    ;               \
        shr     $3, %%rcx       ;               \
        rep     stosq (%%rdi)   ;               \
        mov     %%rdx, %%rcx    ;               \
        and     $7, %%rcx       ;               \
        rep     stosb (%%rdi)   ;               \
          " :
            : "m"(pdest), "m"(bytes)
            : "%rax", "%rcx", "%rdx", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     edi,pdest
        mov     ecx,bytes
        cld
        mov     edx,ecx
        xor     eax,eax
        shr     ecx,2
        rep     stos dword ptr[edi]
        mov     ecx,edx
        and     ecx,3
        rep     stos byte ptr[edi]
    }
    #else                                       //===
    asm ( "                                     \
        movl    %0, %%edi       ;               \
        movl    %1, %%ecx       ;               \
        cld                     ;               \
        mov     %%ecx, %%edx    ;               \
        xor     %%eax, %%eax    ;               \
        shr     $2, %%ecx       ;               \
        rep     stosl (%%edi)   ;               \
        mov     %%edx, %%ecx    ;               \
        and     $3, %%ecx       ;               \
        rep     stosb (%%edi)   ;               \
          " :
            : "m"(pdest), "m"(bytes)
            : "%eax", "%ecx", "%edx", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return;
}

//*************************************************************************
//  OSMemFillB:
//      Fills a buffer with a specified byte value.
//
//  Arguments:
//      void OSMemFillB (void* pdest, byte value, aword bytes)
//      pdest   Destination buffer pointer.
//      value   Byte value to replicate throught the destination buffer.
//      bytes   Byte size of buffer to be filled.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSMemFillB (void* pdest, byte value, aword bytes) {
    #ifdef NOI86                                //-------
    memset(pdest, value, (size_t)bytes);        //C runtime (LIBC.LIB)
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    __stosb((PBYTE)pdest, value, bytes);
    #else                                       //===
    asm ( "                                     \
        movq    %0, %%rdi       ;               \
        movb    %1, %%al        ;               \
        movl    %2, %%rcx       ;               \
        cld                     ;               \
        rep     stosb (%%rdi)   ;               \
          " :
            : "m"(pdest), "m"(value), "m"(bytes)
            : "%rax", "%rcx", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     edi,pdest
        mov     al,value
        mov     ecx,bytes
        cld
        rep     stos byte ptr[edi]
    }
    #else                                       //===
    asm ( "                                     \
        movl    %0, %%edi       ;               \
        movb    %1, %%al        ;               \
        movl    %2, %%ecx       ;               \
        cld                     ;               \
        rep     stosb (%%edi)   ;               \
          " :
            : "m"(pdest), "m"(value), "m"(bytes)
            : "%eax", "%ecx", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return;
}

//*************************************************************************
//  OSMemFillD:
//      Fills a buffer with a specified 32-bit dword value.
//
//  Arguments:
//      void OSMemFillD (void* pdest, dword value, aword bytes)
//      pdest   Destination buffer pointer.
//      value   32-bit vallue to replicate throught the destination buffer.
//      bytes   Byte size of buffer to be filled.
//              Beware that the number of dwords is this divided by four!
//              This should be a multiple of four.
//              Extra bytes after the last dword are ignored.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSMemFillD (void* pdest, dword value, aword bytes) {
    #ifdef NOI86                                //-------
    dword* pdw;

    pdw = (dword*) pdest;
    bytes = bytes >> 2;
    while (bytes--) {
        *pdw++ = value;
    }
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    __stosd((PDWORD)pdest, value, (bytes >> 2));
    #else                                       //===
    asm ( "                                     \
        movq    %0, %%rdi       ;               \
        movl    %1, %%eax       ;               \
        movq    %2, %%rcx       ;               \
        shr     $2, %%rcx       ;               \
        cld                     ;               \
        rep     stosl (%%rdi)   ;               \
          " :
            : "m"(pdest), "m"(value), "m"(bytes)
            : "%rax", "%rcx", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     edi,pdest
        mov     eax,value
        mov     ecx,bytes
        shr     ecx,2
        cld
        rep     stos dword ptr[edi]
    }
    #else                                       //===
    asm ( "                                     \
        movl    %0, %%edi       ;               \
        movl    %1, %%eax       ;               \
        movl    %2, %%ecx       ;               \
        shr     $2, %%ecx       ;               \
        cld                     ;               \
        rep     stosl (%%edi)   ;               \
          " :
            : "m"(pdest), "m"(value), "m"(bytes)
            : "%eax", "%ecx", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return;
}

//*************************************************************************
//  OSMemCopy:
//      Copies a block of memory.
//
//  Arguments:
//      void OSMemCopy (void* pdest, void* psource, aword bytes)
//      pdest   Destination buffer pointer.
//      psource Source buffer pointer.
//      bytes   Number of bytes to copy.
//
//  Returns:
//      Nothing
//*************************************************************************

CSTYLE void xproc
OSMemCopy (void* pdest, void* psource, aword bytes) {
    #ifdef NOI86                                //-------
    memcpy(pdest, psource, (size_t)bytes);
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    aword cnt = bytes >> 3;
    __movsq((PDWORD64)pdest, (PDWORD64)psource, cnt);
    cnt = cnt << 3;
    __movsb(ADDOFF(PBYTE,pdest,cnt), ADDOFF(PBYTE,psource,cnt), (bytes & 7));
    #else                                       //===
    asm ( "                                     \
        movq    %0, %%rsi               ;       \
        movq    %1, %%rdi               ;       \
        movq    %2, %%rcx               ;       \
        cld                             ;       \
        mov     %%rcx, %%rax            ;       \
        shr     $3, %%rcx               ;       \
        rep     movsq (%%rsi), (%%rdi)  ;       \
        movq    %%rax, %%rcx            ;       \
        and     $7, %%rcx               ;       \
        rep     movsb (%%rsi), (%%rdi)  ;       \
          " :
            : "m"(psource), "m"(pdest), "m"(bytes)
            : "%rax", "%rcx", "%rsi", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     esi,psource
        mov     edi,pdest
        mov     ecx,bytes
        cld
        mov     eax,ecx
        shr     ecx,2
        rep     movs dword ptr[edi],dword ptr[esi]
        mov     ecx,eax
        and     ecx,3
        rep     movs byte ptr[edi],byte ptr[esi]
    }
    #else                                       //===
    asm ( "                                     \
        movl    %0, %%esi               ;       \
        movl    %1, %%edi               ;       \
        movl    %2, %%ecx               ;       \
        cld                             ;       \
        mov     %%ecx, %%eax            ;       \
        shr     $2, %%ecx               ;       \
        rep     movsl (%%esi), (%%edi)  ;       \
        mov     %%eax, %%ecx            ;       \
        and     $3, %%ecx               ;       \
        rep     movsb (%%esi), (%%edi)  ;       \
          " :
            : "m"(psource), "m"(pdest), "m"(bytes)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return;
}

//*************************************************************************
//  OSMemCopyR:
//      Same as OSMemCopy except copies bytes from end of buffer to beginning.
//
//  Arguments:
//      void OSMemCopyR (void* pdest, void* psource, aword bytes)
//      pdest   Destination buffer pointer.
//      psource Source buffer pointer.
//      bytes   Number of bytes to copy.
//
//  Returns:
//      Nothing
//*************************************************************************

CSTYLE void xproc
OSMemCopyR (void* pdest, void* psource, aword bytes) {
    #ifdef NOI86                                //-------
    memmove(pdest, psource, (size_t)bytes);
    #else                                       //-------

    #ifdef ISX64                                //---
    memmove(pdest, psource, (size_t)bytes);
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     esi,psource
        mov     edi,pdest
        mov     ecx,bytes

        add     edi,ecx         ;point to the end
        add     esi,ecx
        mov     eax,ecx
        shr     eax,2
        and     ecx,3
        dec     esi             ;point to last byte
        dec     edi

        std                     ;copy downward
        rep     movs byte ptr[edi],byte ptr[esi]

        sub     esi,3           ;align on next dword down
        sub     edi,3
        mov     ecx,eax
        rep     movs dword ptr[edi],dword ptr[esi]
        cld                     ;leave standard direction flag
    }
    #else                                       //===
    asm ( "                                     \
        movl    %0, %%esi               ;       \
        movl    %1, %%edi               ;       \
        movl    %2, %%ecx               ;       \
                                        ;       \
        add     %%ecx, %%edi            ;       \
        add     %%ecx, %%esi            ;       \
        mov     %%ecx,%%eax             ;       \
        shr     $2, %%eax               ;       \
        and     $3, %%ecx               ;       \
        dec     %%esi                   ;       \
        dec     %%edi                   ;       \
                                        ;       \
        std                             ;       \
        rep     movsb (%%esi), (%%edi)  ;       \
                                        ;       \
        sub     $3, %%esi               ;       \
        sub     $3, %%edi               ;       \
        mov     %%eax,%%ecx             ;       \
        rep     movsl (%%esi), (%%edi)  ;       \
        cld                             ;       \
          " :
            : "m"(psource), "m"(pdest), "m"(bytes)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return;
}

//*************************************************************************
//  OSMemMatch:
//      Compares two blocks of memory for equality.
//      Can return a partial match at the beginning or a complete match to end.
//
//  Arguments:
//      aint OSMemMatch (void* pmem1, void* pmem2, aword bytes)
//      pmem1   Pointer to first memory block.
//      pmem2   Pointer to second memory block.
//      bytes   Maximum number of bytes that can match (blocks' size).
//
//  Returns:
//      Number of bytes matching at beginnings of blocks.
//      Returns zero if even the first byte did not match.
//      Returns bytes argument if blocks completely match to their ends.
//      Blocks completely match if returned value equals the bytes size.
//*************************************************************************

CSTYLE aint xproc
OSMemMatch (void* pmem1, void* pmem2, aword bytes) {
    aword left;

    #ifdef NOI86                                //-------
    byte* pbyte1;
    byte* pbyte2;

    left = bytes;
    pbyte1 = (fbyte*) pmem1;
    pbyte2 = (fbyte*) pmem2;
    do {
        if (*pbyte1++ != *pbyte2++) break;
    } while (--left);
    #else                                       //-------

    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    byte* pbyte1;                               //WIN64 omits inline assembly!
    byte* pbyte2;                               //(would be faster with masm)

    left = bytes;
    pbyte1 = (fbyte*) pmem1;
    pbyte2 = (fbyte*) pmem2;
    do {
        if (*pbyte1++ != *pbyte2++) break;
    } while (--left);
    #else                                       //---
    ASM {
        cld
        mov     esi,pmem1
        mov     edi,pmem2
        mov     ecx,bytes
        mov     edx,ecx
        shr     ecx,2
        jz      mattwo
        repe    cmpsd                           //compare dwords
        jz      mattwo                          //matched to end (zero ecx)?
        inc     ecx                             //decremented for mismatch
        and     edx,3
        shl     ecx,2                           //mismatch somewhere in 4 bytes
        add     edx,ecx
        sub     esi,4                           //back up to mismatch dword
        sub     edi,4
        mov     ecx,4
        repe    cmpsb                           //check last dword
        add     ecx,edx                         //subtract matched dword bytes
        sub     ecx,3
        jmp     matsam

mattwo: mov     ecx,edx
        and     ecx,3
        jz      matsam
        repe    cmpsb                           //compare ending bytes
        jz      matsam                          //matched to end (zero ecx)?
        inc     ecx                             //decremented for mismatch
matsam: mov     left,ecx                        //bytes not matching at end
    }
    #endif                                      //---
    #else                                       //===
    #ifdef ISX64                                //--- 64 bits
    asm ( "                                     \
        cld                             ;       \
        movq    %1, %%rsi               ;       \
        movq    %2, %%rdi               ;       \
        movq    %3, %%rcx               ;       \
        mov     %%rcx, %%rdx            ;       \
        shr     $2, %%rcx               ;       \
        jz      mattwo                  ;       \
        repe    cmpsd                   ;       \
        jz      mattwo                  ;       \
        inc     %%rcx                   ;       \
        and     $3, %%rdx               ;       \
        shl     $2, %%rcx               ;       \
        add     %%rcx, %%rdx            ;       \
        sub     $4, %%rsi               ;       \
        sub     $4, %%rdi               ;       \
        mov     $4, %%rcx               ;       \
        repe    cmpsb                   ;       \
        add     %%rdx, %%rcx            ;       \
        sub     $3, %%rcx               ;       \
        jmp     matsam                  ;       \
mattwo: mov     %%rdx, %%rcx            ;       \
        and     $3, %%rcx               ;       \
        jz      matsam                  ;       \
        repe    cmpsb                   ;       \
        jz      matsam                  ;       \
        inc     %%rcx                   ;       \
matsam: movq    %%rcx, %0               ;       \
          " : "=m"(left)
            : "m"(pmem1), "m"(pmem2), "m"(bytes)
            : "%rax", "%rcx", "%rdx", "%rsi", "%rdi"
    );
    #else                                       //--- 32 bits
    asm ( "                                     \
        cld                             ;       \
        movl    %1, %%esi               ;       \
        movl    %2, %%edi               ;       \
        movl    %3, %%ecx               ;       \
        mov     %%ecx, %%edx            ;       \
        shr     $2, %%ecx               ;       \
        jz      mattwo                  ;       \
        repe    cmpsd                   ;       \
        jz      mattwo                  ;       \
        inc     %%ecx                   ;       \
        and     $3, %%edx               ;       \
        shl     $2, %%ecx               ;       \
        add     %%ecx, %%edx            ;       \
        sub     $4, %%esi               ;       \
        sub     $4, %%edi               ;       \
        mov     $4, %%ecx               ;       \
        repe    cmpsb                   ;       \
        add     %%edx, %%ecx            ;       \
        sub     $3, %%ecx               ;       \
        jmp     matsam                  ;       \
mattwo: mov     %%edx, %%ecx            ;       \
        and     $3, %%ecx               ;       \
        jz      matsam                  ;       \
        repe    cmpsb                   ;       \
        jz      matsam                  ;       \
        inc     %%ecx                   ;       \
matsam: movl    %%ecx, %0               ;       \
          " : "=m"(left)
            : "m"(pmem1), "m"(pmem2), "m"(bytes)
            : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    #endif                                      //---
    #endif                                      //===
    #endif                                      //-------
    return (bytes - left);
}

//*************************************************************************
//  OSMemDiffs:
//      Compares two blocks of memory for inequality.
//      Works exactly like OSMemMatch except tests for not equal.
//      Can return a partial match at beginning or a complete mismatch to end.
//
//  Arguments:
//      aint OSMemDiffs (void* pmem1, void* pmem2, aword bytes)
//      pmem1   Pointer to first memory block.
//      pmem2   Pointer to second memory block.
//      bytes   Maximum number of bytes that can mismatch (blocks' size).
//
//  Returns:
//      Number of bytes not matching at beginnings of blocks.
//      Returns zero if even the first byte matched.
//      Returns bytes argument if blocks completely mismatch to their ends.
//      Blocks completely mismatch if returned value equals the bytes size.
//*************************************************************************

CSTYLE aint xproc
OSMemDiffs (void* pmem1, void* pmem2, aword bytes) {
    aword left;

    #ifdef NOI86                                //-------
    byte* pbyte1;
    byte* pbyte2;

    left = bytes;
    pbyte1 = (fbyte*) pmem1;
    pbyte2 = (fbyte*) pmem2;
    do {
        if (*pbyte1++ == *pbyte2++) break;
    } while (--left);
    #else                                       //-------

    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    byte* pbyte1;                               //WIN64 omits inline assembly!
    byte* pbyte2;                               //(would be faster with masm)

    left = bytes;
    pbyte1 = (fbyte*) pmem1;
    pbyte2 = (fbyte*) pmem2;
    do {
        if (*pbyte1++ == *pbyte2++) break;
    } while (--left);
    #else                                       //---
    ASM {
        cld
        mov     esi,pmem1
        mov     edi,pmem2
        mov     ecx,bytes
        repne   cmpsb
        jnz     difsam
        inc     ecx
difsam: mov     left,ecx
    }
    #endif                                      //---
    #else                                       //===
    #ifdef ISX64                                //--- 64 bits
    asm ( "                                     \
        cld                             ;       \
        movq    %1, %%rsi               ;       \
        movq    %2, %%rdi               ;       \
        movq    %3, %%rcx               ;       \
        repne   cmpsb                   ;       \
        jnz     difsam                  ;       \
        inc     %%rcx                   ;       \
difsam: movq    %%rcx, %0               ;       \
          " : "=m"(left)
            : "m"(pmem1), "m"(pmem2), "m"(bytes)
            : "%rax", "%rcx", "%rsi", "%rdi"
    );
    #else                                       //--- 32 bits
    asm ( "                                     \
        cld                             ;       \
        movl    %1, %%esi               ;       \
        movl    %2, %%edi               ;       \
        movl    %3, %%ecx               ;       \
        repne   cmpsb                   ;       \
        jnz     difsam                  ;       \
        inc     %%ecx                   ;       \
difsam: movl    %%ecx, %0               ;       \
          " : "=m"(left)
            : "m"(pmem1), "m"(pmem2), "m"(bytes)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif                                      //---
    #endif                                      //===
    #endif                                      //-------
    return (bytes - left);
}

//*************************************************************************
//  OSMemFindB:
//      Finds the first matching byte in a memory buffer.
//
//  Arguments:
//      aint OSMemFindB (byte find, void* pmem, aword bytes)
//      find    Byte value to find.
//      pmem    Memory buffer to search.
//      bytes   Number of bytes in pmem memory buffer to search.
//
//  Returns:
//      Index in pmem to first occurance of find byte or -1 none found.
//*************************************************************************

CSTYLE aint xproc
OSMemFindB (byte find, void* pmem, aword bytes) {
    byte* pbyte;

    #ifdef NOI86                                //-------
    if (bytes == 0) return(-1);
    pbyte = (fbyte*) pmem;
    do {
        if (find == *pbyte++) break;
    } while (--bytes);

    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    if (bytes == 0) return(-1);                 //WIN64 omits inline assembly!
    pbyte = (fbyte*) pmem;                      //(would be faster with masm)
    do {
        if (find == *pbyte++) break;
    } while (--bytes);
    #else                                       //---
    ASM {
        cld
        mov     al,find
        mov     ecx,bytes
        mov     edi,pmem
        test    ecx,ecx
        jz      ntfind
        repne   scasb
        jne     ntfind
        inc     ecx
ntfind: mov     bytes,ecx
        mov     pbyte,edi
    }
    #endif                                      //---
    #else                                       //===
    #ifdef ISX64                                //--- 64 bits
    asm ( "                                     \
        cld                             ;       \
        movb    %2, %%al                ;       \
        movq    %3, %%rcx               ;       \
        movq    %4, %%rdi               ;       \
        test    %%rcx, %%rcx            ;       \
        jz      ntfind                  ;       \
        repne   scasb                   ;       \
        jne     ntfind                  ;       \
        inc     %%rcx                   ;       \
ntfind: movq    %%rcx, %0               ;       \
        movq    %%rdi, %1               ;       \
          " : "=m"(bytes), "=m"(pbyte)
            : "m"(find), "m"(bytes), "m"(pmem)
            : "%rax", "%rcx", "%rdi"
    );
    #else                                       //--- 32 bits
    asm ( "                                     \
        cld                             ;       \
        movb    %2, %%al                ;       \
        movl    %3, %%ecx               ;       \
        movl    %4, %%edi               ;       \
        test    %%ecx, %%ecx            ;       \
        jz      ntfind                  ;       \
        repne   scasb                   ;       \
        jne     ntfind                  ;       \
        inc     %%ecx                   ;       \
ntfind: movl    %%ecx, %0               ;       \
        movl    %%edi, %1               ;       \
          " : "=m"(bytes), "=m"(pbyte)
            : "m"(find), "m"(bytes), "m"(pmem)
            : "%eax", "%ecx", "%edi"
    );
    #endif                                      //---
    #endif                                      //===
    #endif                                      //-------

    if (bytes == 0) return(-1);
    return(PTROFF(pmem, (pbyte - 1)));
}

//*************************************************************************
//  OSMemFindD:
//      Finds the first matching dword in a memory buffer.
//
//  Arguments:
//      aint OSMemFindD (dword find, void* pmem, aword dwords)
//      find    32-bit value to find.
//      pmem    Memory buffer to search.
//      dwords  Number of dword values in pmem memory buffer to search.
//
//  Returns:
//      Index in pmem to first occurance of find dword or -1 none found.
//      This is a dword index not a byte offset.
//*************************************************************************

CSTYLE aint xproc
OSMemFindD (dword find, void* pmem, aword dwords) {
    dword* pdw;

    #ifdef NOI86                                //-------
    if (dwords == 0) return(-1);
    pdw = (dword*) pmem;
    do {
        if (find == *pdw++) break;
    } while (--dwords);
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    if (dwords == 0) return(-1);                //not sure whether AMD64
    pdw = (dword*) pmem;                        //allows rep scan for dword
    do {
        if (find == *pdw++) break;
    } while (--dwords);
    #else                                       //===
    asm ( "                                     \
        cld                             ;       \
        movl    %2, %%eax               ;       \
        movq    %3, %%rcx               ;       \
        movq    %4, %%rdi               ;       \
        test    %%rcx, %%rcx            ;       \
        jz      ndfind                  ;       \
        repne   scasl                   ;       \
        jne     ndfind                  ;       \
        inc     %%rcx                   ;       \
ndfind: movq    %%rcx, %0               ;       \
        movq    %%rdi, %1               ;       \
          " : "=m"(dwords), "=m"(pdw)
            : "m"(find), "m"(dwords), "m"(pmem)
            : "%rax", "%rcx", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        cld
        mov     eax,find
        mov     ecx,dwords
        mov     edi,pmem
        test    ecx,ecx
        jz      ndfind
        repne   scasd
        jne     ndfind
        inc     ecx
ndfind: mov     dwords,ecx
        mov     pdw,edi
    }
    #else                                       //===
    asm ( "                                     \
        cld                             ;       \
        movl    %2, %%eax               ;       \
        movl    %3, %%ecx               ;       \
        movl    %4, %%edi               ;       \
        test    %%ecx, %%ecx            ;       \
        jz      ndfind                  ;       \
        repne   scasl                   ;       \
        jne     ndfind                  ;       \
        inc     %%ecx                   ;       \
ndfind: movl    %%ecx, %0               ;       \
        movl    %%edi, %1               ;       \
          " : "=m"(dwords), "=m"(pdw)
            : "m"(find), "m"(dwords), "m"(pmem)
            : "%eax", "%ecx", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------

    if (dwords == 0) return(-1);
    return((pdw - 1) - (fdword*)pmem);
}

//*************************************************************************
//  OSMemFindND:
//      Finds the first non-matching dword in a memory buffer.
//
//  Arguments:
//      aint OSMemFindND (dword find, void* pmem, aword dwords)
//      find    32-bit value to find anything but.
//      pmem    Memory buffer to search.
//      dwords  Number of dword values in pmem memory buffer to search.
//
//  Returns:
//      Index in pmem to first non-match of find dword or -1 none found.
//      This is a dword index not a byte offset.
//*************************************************************************

CSTYLE aint xproc
OSMemFindND (dword find, void* pmem, aword dwords) {
    dword* pdw;

    #ifdef NOI86                                //-------
    if (dwords == 0) return(-1);
    pdw = (dword*) pmem;
    do {
        if (find != *pdw++) break;
    } while (--dwords);
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    if (dwords == 0) return(-1);                //not sure whether AMD64
    pdw = (dword*) pmem;                        //allows rep scan for dword
    do {
        if (find != *pdw++) break;
    } while (--dwords);
    #else                                       //===
    asm ( "                                     \
        cld                             ;       \
        movl    %2, %%eax               ;       \
        movq    %3, %%rcx               ;       \
        movq    %4, %%rdi               ;       \
        test    %%rcx, %%rcx            ;       \
        jz      rdfind                  ;       \
        repe    scasl                   ;       \
        je      rdfind                  ;       \
        inc     %%rcx                   ;       \
rdfind: movq    %%rcx, %0               ;       \
        movq    %%rdi, %1               ;       \
          " : "=m"(dwords), "=m"(pdw)
            : "m"(find), "m"(dwords), "m"(pmem)
            : "%rax", "%rcx", "%rdi"
    );
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        cld
        mov     eax,find
        mov     ecx,dwords
        mov     edi,pmem
        test    ecx,ecx
        jz      rdfind
        repe    scasd
        je      rdfind
        inc     ecx
rdfind: mov     dwords,ecx
        mov     pdw,edi
    }
    #else                                       //===
    asm ( "                                     \
        cld                             ;       \
        movl    %2, %%eax               ;       \
        movl    %3, %%ecx               ;       \
        movl    %4, %%edi               ;       \
        test    %%ecx, %%ecx            ;       \
        jz      rdfind                  ;       \
        repe    scasl                   ;       \
        je      rdfind                  ;       \
        inc     %%ecx                   ;       \
rdfind: movl    %%ecx, %0               ;       \
        movl    %%edi, %1               ;       \
          " : "=m"(dwords), "=m"(pdw)
            : "m"(find), "m"(dwords), "m"(pmem)
            : "%eax", "%ecx", "%edi"
    );
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------

    if (dwords == 0) return(-1);
    return((pdw - 1) - (fdword*)pmem);
}

//*************************************************************************
//  OSMemSearch:
//      Searches for a byte string as a substring of another.
//      This is like a text search but since it finds the first place
//      that the pone string appears as a substring inside the ptwo memory.
//      However it cannot usually be used for text search
//      because it would not match text with case differences.
//      Note that it can find only the beginning of the pfind bytes and finds
//      the first *ppmem substring with at least minsz matching bytes.
//      On Intel uses fast one cycle per byte assembly repeat instructions.
//
//  Arguments:
//      aint OSMemSearch (lpbyte* ppmem, aword* pcmem, byte* pfind, aword cfind,
//                        int minsz)
//      ppmem   Supplies beginning of memory to search.
//              Returns pointer just after the first match found.
//              Returns the first match of one or more of the pfind bytes.
//              If the first *pfind byte does not exist in *ppmem buffer
//              then returns *ppmem unchanged and returns zero bytes matched.
//      pcmem   Supplies the number of bytes available in *ppmem block.
//              Returns number of bytes left after returned *ppmem pointer.
//      pfind   Searches for this series of bytes.
//      cfind   Number of bytes in pfind memory to search.
//              However not all cfind bytes need match.
//              One or more bytes at the beginning of are found
//              if the entire cfind block does not match
//              as long as at least minsz bytes do match.
//      minsz   Minimum number of matching bytes.
//              Skips matches with fewer bytes.
//              Use 1 to find all matches of one or more pfind bytes.
//
//  Returns:
//      Number of matching pfind bytes (which may be less than cfind).
//*************************************************************************

CSTYLE aint xproc
OSMemSearch (lpbyte* ppmem, aword* pcmem, byte* pfind, aword cfind, int minsz) {
    byte* pmem;
    aword cmem, bytes;

    pmem = *ppmem;
    cmem = *pcmem;

    #ifdef NOI86                                //-------
    byte* puse;
    byte* ptwo;
    byte find;
    aint have;

    find = *pfind;
    ptwo = pfind + 1;

    while (TRUE) {
        bytes = cmem - (pmem - *ppmem);
        if (bytes == 0) return(0);
        do {
            if (find == *pmem++) break;
        } while (--bytes);

        if (bytes == 0) return(0);

        puse = ptwo;
        if (cfind < bytes) bytes = cfind;

        while (--bytes) {
            if (*puse++ == *pmem++) continue;
            puse -= 1;
            pmem -= 1;
            break;      
        }

        have = puse - pfind;
        if (have >= (int)minsz) {
            pmem = pmem - have;
            *pcmem = *pcmem - (pmem - *ppmem);
            *ppmem = pmem;
            return(have);
        }
    }
    #else                                       //-------

    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    byte* puse;                                 //WIN64 omits inline assembly!
    byte* ptwo;                                 //(would be faster with masm)
    byte find;
    aint have;

    find = *pfind;
    ptwo = pfind + 1;

    while (TRUE) {
        bytes = cmem - (pmem - *ppmem);
        if (bytes == 0) return(0);
        do {
            if (find == *pmem++) break;
        } while (--bytes);

        if (bytes == 0) return(0);

        puse = ptwo;
        if (cfind < bytes) bytes = cfind;

        while (--bytes) {
            if (*puse++ == *pmem++) continue;
            puse -= 1;
            pmem -= 1;
            break;      
        }

        have = puse - pfind;
        if (have >= (aint)minsz) {
            pmem = pmem - have;
            *pcmem = *pcmem - (pmem - *ppmem);
            *ppmem = pmem;
            return(have);
        }
    }
    #else                                       //---
    ASM {
        cld
        mov     edi,pmem
msmore: mov     ecx,cmem
        mov     eax,edi
        mov     esi,pfind
        sub     eax,pmem
        sub     ecx,eax
        mov     al,[esi]
        test    ecx,ecx
        jz      msdone
        repne   scasb
        jne     msdone

        mov     eax,cfind
        inc     ecx
        inc     esi
        cmp     ecx,eax
        jg      msroom
        mov     eax,ecx
msroom: mov     ecx,eax
        dec     ecx
        jz      msnone
        repz    cmpsb
        jz      msnone
        inc     ecx
        dec     edi
msnone: sub     eax,ecx
        cmp     eax,minsz
        jl      msmore
        mov     ecx,eax
        mov     pmem,edi

msdone: mov     bytes,ecx
    }
    #endif                                      //---
    #else                                       //===
    #ifdef ISX64                                //--- 64 bit
    qword qminsz = (qword)minsz;
    asm ( "                                     \
        cld                             ;       \
        movq    %2, %%rdi               ;       \
msmore: movq    %3, %%rcx               ;       \
        mov     %%rdi, %%rax            ;       \
        movq    %4, %%rsi               ;       \
        sub     %2, %%rax               ;       \
        sub     %%rax, %%rcx            ;       \
        mov     (%%rsi), %%al           ;       \
        test    %%rcx, %%rcx            ;       \
        jz      msdone                  ;       \
        repne   scasb                   ;       \
        jne     msdone                  ;       \
        movq    %5, %%rax               ;       \
        inc     %%rcx                   ;       \
        inc     %%rsi                   ;       \
        cmp     %%rax, %%rcx            ;       \
        jg      msroom                  ;       \
        mov     %%rcx, %%rax            ;       \
msroom: mov     %%rax, %%rcx            ;       \
        dec     %%rcx                   ;       \
        jz      msnone                  ;       \
        repz    cmpsb                   ;       \
        jz      msnone                  ;       \
        inc     %%rcx                   ;       \
        dec     %%rdi                   ;       \
msnone: sub     %%rcx, %%rax            ;       \
        cmpq    %6, %%rax               ;       \
        jl      msmore                  ;       \
        mov     %%rax, %%rcx            ;       \
        movq    %%rdi, %0               ;       \
msdone: movq    %%rcx, %1               ;       \
          " : "=m"(pmem), "=m"(bytes)
            : "m"(pmem), "m"(cmem), "m"(pfind), "m"(cfind), "m"(qminsz)
            : "%rax", "%rcx", "%rsi", "%rdi"
    );
    #else                                       //--- 32 bit
    asm ( "                                     \
        cld                             ;       \
        movl    %2, %%edi               ;       \
msmore: movl    %3, %%ecx               ;       \
        mov     %%edi, %%eax            ;       \
        movl    %4, %%esi               ;       \
        sub     %2, %%eax               ;       \
        sub     %%eax, %%ecx            ;       \
        mov     (%%esi), %%al           ;       \
        test    %%ecx, %%ecx            ;       \
        jz      msdone                  ;       \
        repne   scasb                   ;       \
        jne     msdone                  ;       \
        movl    %5, %%eax               ;       \
        inc     %%ecx                   ;       \
        inc     %%esi                   ;       \
        cmp     %%eax, %%ecx            ;       \
        jg      msroom                  ;       \
        mov     %%ecx, %%eax            ;       \
msroom: mov     %%eax, %%ecx            ;       \
        dec     %%ecx                   ;       \
        jz      msnone                  ;       \
        repz    cmpsb                   ;       \
        jz      msnone                  ;       \
        inc     %%ecx                   ;       \
        dec     %%edi                   ;       \
msnone: sub     %%ecx, %%eax            ;       \
        cmpl    %6, %%eax               ;       \
        jl      msmore                  ;       \
        mov     %%eax, %%ecx            ;       \
        movl    %%edi, %0               ;       \
msdone: movl    %%ecx, %1               ;       \
          " : "=m"(pmem), "=m"(bytes)
            : "m"(pmem), "m"(cmem), "m"(pfind), "m"(cfind), "m"(minsz)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif                                      //---
    #endif                                      //===
    #endif                                      //-------

    pmem = pmem - bytes;                        //get beginning of found stuff
    *pcmem = *pcmem - (pmem - *ppmem);          //return bytes left after found
    *ppmem = pmem;                              //return ptr to found substring
    return(bytes);                              //return matching bytes
}

//*************************************************************************
//  OSTxtSize:
//      Returns the number of characters in a non-unicode text string.
//      Includes the terminating NULL in the count.
//
//  Arguments:
//      uint OSTxtSize (text* string)
//      string  Pointer to null terminated text string.
//
//  Returns:
//      Length of the string in characters.
//      Length includes the terminating zero character.
//*************************************************************************

CSTYLE uint xproc
OSTxtSize (text* string) {
    uint rval;

    #ifdef NOI86                                //-------
    rval = (uint) strlen(string) + 1;
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    rval = (uint) strlen(string) + 1;
    #else                                       //===
    #if (SZCHR==2)  //=======
    asm ( "                                     \
        movq    %1, %%rdi               ;       \
        movq    $-1, %%rcx              ;       \
        xor     %%rax, %%rax            ;       \
        cld                             ;       \
        repne   scasw                   ;       \
        not     %%rcx                   ;       \
        movl    %%ecx, %0               ;       \
          " : "=m"(rval)
            : "m"(string)
            : "%rax", "%rcx", "%rdi"
    );
    #else           //=======
    asm ( "                                     \
        movq    %1, %%rdi               ;       \
        movq    $-1, %%rcx              ;       \
        xor     %%rax, %%rax            ;       \
        cld                             ;       \
        repne   scasb                   ;       \
        not     %%rcx                   ;       \
        movl    %%ecx, %0               ;       \
          " : "=m"(rval)
            : "m"(string)
            : "%rax", "%rcx", "%rdi"
    );
    #endif          //=======
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     edi,string
        mov     ecx,-1
        xor     eax,eax
        cld
        #if (SZCHR==2)  //=======
        repne   scasw
        #else           //=======
        repne   scasb
        #endif          //=======
        not     ecx
        mov     rval,ecx
    }
    #else                                       //===
    #if (SZCHR==2)  //=======
    asm ( "                                     \
        movl    %1, %%edi               ;       \
        movl    $-1, %%ecx              ;       \
        xor     %%eax, %%eax            ;       \
        cld                             ;       \
        repne   scasw                   ;       \
        not     %%ecx                   ;       \
        movl    %%ecx, %0               ;       \
          " : "=m"(rval)
            : "m"(string)
            : "%eax", "%ecx", "%edi"
    );
    #else           //=======
    asm ( "                                     \
        movl    %1, %%edi               ;       \
        movl    $-1, %%ecx              ;       \
        xor     %%eax, %%eax            ;       \
        cld                             ;       \
        repne   scasb                   ;       \
        not     %%ecx                   ;       \
        movl    %%ecx, %0               ;       \
          " : "=m"(rval)
            : "m"(string)
            : "%eax", "%ecx", "%edi"
    );
    #endif          //=======
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------
    return (rval);
}

//*************************************************************************
//  OSTxtCopy:
//      Copies a null-terminated text string.
//
//  Arguments:
//      text* OSTxtCopy (text* pdest, text* psource)
//      pdest   Destination text buffer pointer.
//              Text characters are copied from psource to pdest
//              up to and including the terminating zero.
//      psource Pointer to source text string.
//              Must be null terminated.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSTxtCopy (text* pdest, text* psource) {
    aword len;

    #ifdef NOI86                                //-------
    len = (aword) strlen(psource) + 1;
    memcpy(pdest, psource, (size_t)len);
    #else                                       //-------

    #ifdef ISX64                                //---
    #ifdef ISWIN                                //===
    len = (aword) strlen(psource) + 1;
    __movsb((PBYTE)pdest, (PBYTE)psource, len);
    #else                                       //===
    #if (SZCHR==2)  //=======
    asm ( "                                     \
        movq    %1, %%rdi               ;       \
        movq    $-1, %%rcx              ;       \
        mov     %%rdi, %%rsi            ;       \
        xor     %%rax, %%rax            ;       \
        cld                             ;       \
        repne   scasw                   ;       \
        not     %%rcx                   ;       \
        movq    %%rcx, %0               ;       \
        add     %%rcx,%%rcx             ;       \
        movq    %2, %%rdi               ;       \
        cld                             ;       \
        mov     %%rcx, %%rax            ;       \
        shr     $2, %%rcx               ;       \
        rep     movsl (%%rsi), (%%rdi)  ;       \
        mov     %%rax, %%rcx            ;       \
        and     $3, %%rcx               ;       \
        rep     movsb (%%rsi), (%%rdi)  ;       \
          " : "=m"(len)
            : "m"(psource), "m"(pdest)
            : "%rax", "%rcx", "%rsi", "%rdi"
    );
    #else           //=======
    asm ( "                                     \
        movq    %1, %%rdi               ;       \
        movq    $-1, %%rcx              ;       \
        mov     %%rdi, %%rsi            ;       \
        xor     %%rax, %%rax            ;       \
        cld                             ;       \
        repne   scasb                   ;       \
        not     %%rcx                   ;       \
        movq    %%rcx, %0               ;       \
        movq    %2, %%rdi               ;       \
        cld                             ;       \
        mov     %%rcx, %%rax            ;       \
        shr     $2, %%rcx               ;       \
        rep     movsl (%%rsi), (%%rdi)  ;       \
        mov     %%rax, %%rcx            ;       \
        and     $3, %%rcx               ;       \
        rep     movsb (%%rsi), (%%rdi)  ;       \
          " : "=m"(len)
            : "m"(psource), "m"(pdest)
            : "%rax", "%rcx", "%rsi", "%rdi"
    );
    #endif          //=======
    #endif                                      //===
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {                                       //duplicates OSTxtSize for speed
        mov     edi,psource
        mov     ecx,-1
        mov     esi,edi
        xor     eax,eax
        cld
        #if (SZCHR==2)  //=======
        repne   scasw
        #else           //=======
        repne   scasb
        #endif          //=======
        not     ecx
        mov     len,ecx

        #if (SZCHR==2)  //=======
        add     ecx,ecx
        #endif          //=======

        mov     edi,pdest
        cld
        mov     eax,ecx
        shr     ecx,2
        rep     movs dword ptr[edi],dword ptr[esi]
        mov     ecx,eax
        and     ecx,3
        rep     movs byte ptr[edi],byte ptr[esi]
    }
    #else                                       //===
    #if (SZCHR==2)  //=======
    asm ( "                                     \
        movl    %1, %%edi               ;       \
        movl    $-1, %%ecx              ;       \
        mov     %%edi, %%esi            ;       \
        xor     %%eax, %%eax            ;       \
        cld                             ;       \
        repne   scasw                   ;       \
        not     %%ecx                   ;       \
        movl    %%ecx, %0               ;       \
        add     %%ecx,%%ecx             ;       \
        movl    %2, %%edi               ;       \
        cld                             ;       \
        mov     %%ecx, %%eax            ;       \
        shr     $2, %%ecx               ;       \
        rep     movsl (%%esi), (%%edi)  ;       \
        mov     %%eax, %%ecx            ;       \
        and     $3, %%ecx               ;       \
        rep     movsb (%%esi), (%%edi)  ;       \
          " : "=m"(len)
            : "m"(psource), "m"(pdest)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #else           //=======
    asm ( "                                     \
        movl    %1, %%edi               ;       \
        movl    $-1, %%ecx              ;       \
        mov     %%edi, %%esi            ;       \
        xor     %%eax, %%eax            ;       \
        cld                             ;       \
        repne   scasb                   ;       \
        not     %%ecx                   ;       \
        movl    %%ecx, %0               ;       \
        movl    %2, %%edi               ;       \
        cld                             ;       \
        mov     %%ecx, %%eax            ;       \
        shr     $2, %%ecx               ;       \
        rep     movsl (%%esi), (%%edi)  ;       \
        mov     %%eax, %%ecx            ;       \
        and     $3, %%ecx               ;       \
        rep     movsb (%%esi), (%%edi)  ;       \
          " : "=m"(len)
            : "m"(psource), "m"(pdest)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif          //=======
    #endif                                      //===
    #endif                                      //---
    #endif                                      //-------

    return ((pdest + len) - 1);
}

//*************************************************************************
//  OSTxtStop:
//      Copies a null-terminated text string into a limited-size buffer.
//      Truncates the text if needed and inserts a terminating zero.
//
//  Arguments:
//      text* OSTxtStop (text* pdest, text* psource, uint max)
//      pdest   Destination text buffer pointer.
//              Text characters are copied from psource to pdest
//              up to and including the terminating zero or until
//              max-1 characters with an added terminating zero.
//      psource Pointer to source text string.
//              Must be null terminated.
//      max     Maximum number of text characters that fit in pdest
//              including the terminating zero character.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSTxtStop (text* pdest, text* psource, uint max) {
    text* pend;
    uint len;

    len = OSTxtSize(psource);
    if (len > max) len = max;
    #if (SZCHR==2)  //=======
    OSMemCopy(pdest, psource, len+len);
    #else           //=======
    OSMemCopy(pdest, psource, len);
    #endif          //=======
    pend = (pdest + len) - 1;
    *pend = 0;
    return(pend);
}

//*************************************************************************
//  OSTxtSafe:
//      Copies a null-terminated text string into a limited-size buffer.
//      Unlike OSTxtCopy or OSTxtStop, handles NULL pointers.
//      Uses OSAlloc to create the destination buffer if needed.
//      Copies a blank string if the source pointer is NULL.
//      Truncates the text if needed and inserts a terminating zero.
//
//  Arguments:
//      text* OSTxtSafe (lptext* ppdest, text* psource, uint max)
//      ppdest  Pointer to destination text buffer pointer.
//              Text characters are copied from psource to *ppdest
//              up to and including the terminating zero or until
//              max-1 characters with an added terminating zero.
//              If *ppdest is NULL then allocates a buffer with OSAlloc
//              and returns its pointer in *ppdest.  Beware that the
//              allocated destination is no larger than needed and will
//              usually be smaller than the secified max size.
//      psource Pointer to source text string.
//              Must be zero terminated.
//              If psource is NULL then copies a single zero (a blank string).
//      max     Maximum number of text characters that fit in pdest
//              including the terminating zero character.
//              Use zero to force any old *ppdest to be freed
//              and to allocate a new *ppdest buffer with minimum correct size.
//              Use one to force old *ppdest to be reallocated with SZPATH max.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//      If memory allocation error, returns NULL pointer.
//*************************************************************************

CSTYLE text* xproc
OSTxtSafe (lptext* ppdest, text* psource, uint max) {
    text* pdest;
    uint len;

    if (psource == NULL) psource = "";          //blank source?
    len = OSTxtSize(psource);
    pdest = *ppdest;
    if (max <= 1) {                             //free and realloc destination?
        OSFree(pdest);
        *ppdest = NULL;
        pdest = NULL;
        max = (max == 1) ? SZPATH : EMAXI;
    }
    if (len > max) len = max;
    if (pdest == NULL) {                        //allocate destination?
        pdest = (text*) OSAlloc(len*SZCHR);
        if (pdest == NULL) return(NULL);
        *ppdest = pdest;
    }
    #if (SZCHR==2)  //=======
    OSMemCopy(pdest, psource, len+len);
    #else           //=======
    OSMemCopy(pdest, psource, len);
    #endif          //=======
    pdest = (pdest + len) - 1;
    *pdest = 0;
    return(pdest);
}

//*************************************************************************
//  OSTxtMany:
//      Combines multiple text strings into one with | bar separators.
//      Also extracts individual text strings from multi string.
//      Can optionally use , comma separators.
//      Can optionally place spaces after the separators.
//
//  Arguments:
//      int OSTxtMany (int mode, lptext* ppmany, ftext* pone)
//      mode    1 = Add pone to end of *ppmany (and reallocate as needed).
//              2 = Get pone from *ppmany and advance *ppmany to next.
//             +4 = Use tab rather than bar separators. 
//            +32 = Allow blank sub-strings for mode 1.  
//           +256 = Limit mode 2 strings to SZDISK characters.   
//      ppmany  Pointer to multiple text string with | bar separators.
//              For mode 1, *ppmany may be NULL initially to start string.
//      pone    Pointer to single zero-terminated text string.
//              For mode 2, must be buffer large enough to hold string.
//
//  Returns:
//      Size of string or zero if error.
//*************************************************************************

CSTYLE uint xproc
OSTxtMany (int mode, lptext* ppmany, ftext* pone) {
    ftext* pmany;
    ftext* pmem;
    ftext* ptxt;
    lint ii;
    uint nc;
    text cc;

    pmany = *ppmany;
    cc = (mode & 4) ? '\t' : '|';
    if (mode & 1) {
        nc = (pmany) ? OSTxtSize(pmany) : 0;
        nc += OSTxtSize(pone);
        pmem = (ftext*) OSAlloc(nc*SZCHR);
        if (pmem == NULL) return(0);
        ptxt = pmem;
        if ((pmany)&&((*pmany)||(mode & 32))) {
            ptxt = OSTxtCopy(ptxt, pmany);
            *ptxt++ = cc;
        }
        ptxt = OSTxtCopy(ptxt, pone);
        if (pmany) OSFree(pmany);
        *ppmany = pmem;
        return(nc);
    }
    if (mode & 2) {
        if (pmany == NULL) return(0);
        nc = OSTxtSize(pmany);
        ii = (lint) OSMemFindB(cc, pmany, nc);
        if (ii == -1) {
            ii = (lint) OSTxtSize(pmany) - 1;
            *ppmany = pmany + ii;
        } else {
            *ppmany = pmany + ii + 1;
        }
        if ((mode & 256)&&(ii > (SZDISK-1))) ii = SZDISK-1;
        if (ii) OSMemCopy(pone, pmany, (ii * SZCHR));
        pone[ii] = 0;
        return((uint)ii);
    }
    return(0);
}

//*************************************************************************
//  OSTxtSubs:
//      Allows a single string to have a number of substrings.
//
//  Arguments:
//      text* OSTxtSubs (int mode, lptext* ppmany, text* pone, lint index)
//      mode    1 = Replace index substring with pone.
//              2 = Get index substring into pone.
//             +4 = Use tab rather than bar separators. 
//           +256 = Limit strings to SZDISK characters.   
//      ppmany  Pointer to multiple text string with | bar separators.
//              For mode 1, *ppmany may be NULL initially to start string.
//              For mode 2, *ppmany is never reallocated.
//      pone    Pointer to single zero-terminated text string.
//              Limits strings to SZPATH and replaces any separators with ;.
//      index   Substring index (0=1st).
//
//  Returns:
//      For mode 1 returns *ppmany or NULL if error.
//      For mode 2 returns pointer to end of returned pone substring.
//*************************************************************************

CSTYLE ftext* xproc
OSTxtSubs (int mode, lptext* ppmany, ftext* pone, lint index) {
    text bufr[SZPATH];
    text* pmany;
    text* pmake;
    text* pfree;
    text* ptxt;
    text cc;
    int smode, is, ok;

    smode = mode & 4;
    pmany = *ppmany;
    if (mode & 1) {                             //set substring
        pfree = pmany;
        pmake = NULL;
        while (index > 0) {                     //copy previous substrings
            index -= 1;
            bufr[0] = 0;
            is = OSTxtMany((2+smode), &pmany, bufr);
            ok = OSTxtMany((1+32+smode), &pmake, bufr);
            if (ok < is) {
                if (pmake) OSFree(pmake);
                return(NULL);
            }
        }
        OSTxtMany((2+smode), &pmany, bufr);     //skip old substring

        OSTxtStop(bufr, pone, ((mode & 256) ? SZDISK : SZPATH));
        cc = (mode & 4) ? '\t' : '|';
        ptxt = bufr;
        while (*ptxt) {
            if (*ptxt == cc) *ptxt = ';';       //substitute semicolon for seps
            ptxt += 1;
        }
        ok = OSTxtMany((1+32+smode), &pmake, bufr);
        if ((ok == 0)||(pmake == NULL)) {       //copy new substring for index
            if (pmake) OSFree(pmake);
            return(NULL);
        }        

        while (*pmany) {                        //copy substrings after index
            bufr[0] = 0;
            is = OSTxtMany((2+smode), &pmany, bufr);
            ok = OSTxtMany((1+32+smode), &pmake, bufr);
            if (ok < is) {
                if (pmake) OSFree(pmake);
                return(NULL);
            }
        }
        if (pfree) OSFree(pfree);
        *ppmany = pmake;
        return(pmake);

    } else {                                    //get substring
        smode |= 2;
        while (index > 0) {
            index -= 1;
            OSTxtMany(smode, &pmany, bufr);
        }
        ok = OSTxtMany(smode, &pmany, pone);
        return(pone + ok);
    }
}

//*************************************************************************
//  OSTxtCmp:
//      Compares two text strings without case sensitivity.
//      Hint: Use (!(OSTxtCmpU("BEG",str) & 1)) to see if str begins with "BEG"
//
//  Arguments:
//      int OSTxtCmp (text* ptxt1, text* ptxt2)
//      The ptxt1 and ptxt2 arguments point to two null terminated strings.
//
//  Returns:
//      Zero if strings match up to and including their terminating nulls.
//      Non-zero if strings do not match.
//      Returns +1 if ptxt1 > ptxt2.
//      Returns -1 if ptxt1 < ptxt2 but not all of ptxt1 matched.
//      Returns -2 if ptxt1 is beginning substring of ptxt2 (e.g. "ab" & "ABC").
//      Thus, returns negative when ptxt1<ptxt2 and positive when ptxt1>ptxt2.
//*************************************************************************

CSTYLE int xproc
OSTxtCmp (text* ptxt1, text* ptxt2) {
    int cmp;
    text c;

    while (TRUE) {
        c = *ptxt1;
        if ((cmp = CAPITAL(c) - CAPITAL(*ptxt2))) {
            if (cmp > 0) return(+1);
            if (c) return(-1);
            else return (-2);
        }
        if (c == 0) return(0);
        ptxt1 += 1;
        ptxt2 += 1;
    }
}

//*************************************************************************
//  OSTxtCmpU:
//      Compares upper-case and normal text strings without case sensitivity.
//      Same as OSTxtCmp except that first string is guaranteed all upper case.
//      Hint: Use (!(OSTxtCmpU("BEG",str) & 1)) to see if str begins with "BEG"
//
//  Arguments:
//      int OSTxtCmpU (text* ptxt1, text* ptxt2)
//      The ptxt1 and ptxt2 arguments point to two null terminated strings.
//      The ptxt1 string is assumed to have all capital letters.
//
//  Returns:
//      Zero if strings match up to and including their terminating nulls.
//      Non-zero if strings do not match.
//      Returns +1 if ptxt1 > ptxt2.
//      Returns -1 if ptxt1 < ptxt2 but not all of ptxt1 matched.
//      Returns -2 if ptxt1 is beginning substring of ptxt2 (e.g. "ab" & "ABC").
//      Thus, returns negative when ptxt1<ptxt2 and positive when ptxt1>ptxt2.
//*************************************************************************

CSTYLE int xproc
OSTxtCmpU (text* ptxt1, text* ptxt2) {
    int cmp;
    text c;

    while (TRUE) {
        c = *ptxt1;
        if ((cmp = c - CAPITAL(*ptxt2))) {
            if (cmp > 0) return(+1);
            if (c) return(-1);
            else return (-2);
        }
        if (c == 0) return(0);
        ptxt1 += 1;
        ptxt2 += 1;
    }
}

//*************************************************************************
//  OSTxtCmpS:
//      Compares two text strings with case sensitivity.
//      This is much faster than the case insensitive versions.
//
//  Arguments:
//      int OSTxtCmpS (text* ptxt1, text* ptxt2)
//      The ptxt1 and ptxt2 arguments point to two null terminated strings.
//
//  Returns:
//      Zero if strings match up to and including their terminating nulls.
//      Non-zero if strings to not match.
//      Returns 1 if ptxt1 > ptxt2 and -1 if ptxt1 < ptxt2.
//*************************************************************************

CSTYLE int xproc
OSTxtCmpS (text* ptxt1, text* ptxt2) {
    int cmp;

    #ifdef NOI86                                //-------
    text c1, c2;
    while (TRUE) {
        c1 = *ptxt1++;
        c2 = *ptxt2++;
        if (c1 != c2) break;
        if (c1 == 0) break;
    }
    if ((cmp = (c1 - c2))) {
        if (cmp < 0) return (-1);
        else return (+1);
    }
    return (0);
    #else                                       //-------

    #ifdef ISX64                                //---
    text c1, c2;
    while (TRUE) {
        c1 = *ptxt1++;
        c2 = *ptxt2++;
        if (c1 != c2) break;
        if (c1 == 0) break;
    }
    if ((cmp = (c1 - c2))) {
        if (cmp < 0) return (-1);
        else return (+1);
    }
    return (0);
    #else                                       //---

    #ifdef WIN32                                //===
    ASM {
        mov     edi,ptxt1
        mov     ecx,-1
        xor     eax,eax
        cld
        #if (SZCHR==2)  //=======
        repne   scasw
        #else           //=======
        repne   scasb
        #endif          //=======
        not     ecx             ;number of characters including null

        mov     esi,ptxt1
        mov     edi,ptxt2
        #if (SZCHR==2)  //=======
        repz    cmpsw
        #else           //=======
        repz    cmpsb
        #endif          //=======
        dec     esi             ;point to differing character or ending 0
        dec     edi
        mov     ptxt1,esi
        mov     ptxt2,edi
    }
    #else                                       //===
    #if (SZCHR==2)      //=======
    asm ( "                                     \
        movl    %2, %%edi                   ;   \
        movl    $-1, %%ecx                  ;   \
        xor     %%eax, %%eax                ;   \
        cld                                 ;   \
        repne   scasw                       ;   \
        not     %%ecx                       ;   \
        mov     %2, %%esi                   ;   \
        mov     %3, %%edi                   ;   \
        repz    cmpsw                       ;   \
        dec     %%esi                       ;   \
        dec     %%edi                       ;   \
        movl    %%esi, %0                   ;   \
        movl    %%edi, %1                   ;   \
          " : "=m"(ptxt1), "=m"(ptxt2)
            : "m"(ptxt1), "m"(ptxt2)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #else               //=======
    asm ( "                                     \
        movl    %2, %%edi                   ;   \
        movl    $-1, %%ecx                  ;   \
        xor     %%eax, %%eax                ;   \
        cld                                 ;   \
        repne   scasb                       ;   \
        not     %%ecx                       ;   \
        mov     %2, %%esi                   ;   \
        mov     %3, %%edi                   ;   \
        repz    cmpsb                       ;   \
        dec     %%esi                       ;   \
        dec     %%edi                       ;   \
        movl    %%esi, %0                   ;   \
        movl    %%edi, %1                   ;   \
          " : "=m"(ptxt1), "=m"(ptxt2)
            : "m"(ptxt1), "m"(ptxt2)
            : "%eax", "%ecx", "%esi", "%edi"
    );
    #endif              //=======
    
    #endif                                      //===
    if ((cmp = (*ptxt1 - *ptxt2))) {
        if (cmp < 0) return (-1);
        else return (+1);
    }
    return (0);

    #endif                                      //---
    #endif                                      //-------
}

//*************************************************************************
//  OSTxtCmpN:
//      Works link OSTxtCmpS to compare to strings without case sensitivity.
//      However also allows for NULL pointers and matches if both NULL.
//
//  Arguments:
//      int OSTxtCmpN (text* ptxt1, text* ptxt2)
//      The ptxt1 and ptxt2 arguments point to two null terminated strings.
//
//  Returns:
//      Zero if strings match up to and including their terminating nulls.
//      Non-zero if strings to not match.
//*************************************************************************

CSTYLE int xproc
OSTxtCmpN (text* ptxt1, text* ptxt2) {
    if (ptxt1 == NULL) {
        return ( ((ptxt2 == NULL)||(*ptxt2 == 0)) ? 0 : -1);
    }
    if (ptxt2 == NULL) {
        return ( ((ptxt1 == NULL)||(*ptxt1 == 0)) ? 0 : -1);
    }
    return( OSTxtCmpS(ptxt1, ptxt2) );
}

//*************************************************************************
//  OSTxtSkip:
//      Check for and skips over an expected text string.
//      Similar to OSTxtCmpU except returns pointer just after text if matches.
//      Also simplifies OSTxtCmpU return values to beginning match or not match.
//
//  Arguments:
//      int OSTxtSkip (text* ptxt1, text* ptxt2, lptext* ppskip)
//      The ptxt1 and ptxt2 arguments point to two null terminated strings.
//      The ptxt1 string is assumed to have all capital letters.
//      The ppskip returns pointer to just after matching part of ptxt2.
//      The ppskip returns initial ptxt2 if no match (non-zero return
//      The ppskip argument may be NULL if not needed.
//
//  Returns:
//      Returns 0 if strings match or ptxt is beginning substring of ptxt2.
//      Returns -1 if ptxt2 does not have ptxt1 at its beginning.
//*************************************************************************

CSTYLE int xproc
OSTxtSkip (text* ptxt1, text* ptxt2, lptext* ppskip) {
    int ret;

    ret = OSTxtCmpU(ptxt1, ptxt2);
    if (ret & 1) {                              //no match or begin substring?
        if (ppskip) *ppskip = ptxt2;
        return(-1);
    }
    if (ppskip == NULL) return(0);              //just return match
    *ppskip = ptxt2 + (OSTxtSize(ptxt1) - 1);   //return end of match
    return(0);       
}

//*************************************************************************
//  OSTxtFind:
//      Searches for a string as a substring of a larger document.
//      Ignores upper versus lower case.
//
//  Arguments:
//      text* OSTxtFind (text* ptxt, text* pdoc, text* pend, int mode)
//      ptxt    Zero-terminated substring to be found in pdoc string.
//              Must be no larger than SZPATH characters.
//      pdoc    Pointer to document text to search.
//      pend    Pointer to just after end of pdoc text to search.
//              May be NULL to search pdoc to a terminating zero.
//      mode    0 = Normal operation.
//             +1 = The ptxt search text is all upper case characters.
//             +2 = Return pointer to end of found string (not beginning).
//
//  Returns:
//      Pointer to beginning of found string within pdoc text.
//      If not found then returns a pointer to the pend or terminating zero.
//*************************************************************************

CSTYLE text* xproc
OSTxtFind (text* ptxt, text* pdoc, text* pend, int mode) {
    text txt[SZPATH];
    text cc, dd;
    text* pout;

    if (!(mode & 1)) {
        pout = txt;
        do {                                    //convert ptxt to capitals
            cc = *ptxt++;
            cc = CAPITAL(cc);
            *pout++ = cc;
        } while (cc);
        ptxt = txt;
    }
    cc = *ptxt++;

    if (pend == NULL) {                         //use terminating zero end
        while (TRUE) {                          //go through document
            dd = *pdoc;
            if (dd == 0) break;
            pdoc += 1;
            dd = CAPITAL(dd);
            if (cc != dd) continue;             //first chars do not match?
            if (OSTxtCmpU(ptxt, pdoc) & 1) continue;
            pdoc -= 1;                          //whole ptxt matches?
            if (mode & 2) pdoc = pdoc + OSTxtSize(ptxt);
            break;
        }
    } else {
        while (pdoc < pend) {                   //go through document
            dd = *pdoc++;
            dd = CAPITAL(dd);
            if (cc != dd) continue;             //first chars do not match?
            if (OSTxtCmpU(ptxt, pdoc) & 1) continue;
            pdoc -= 1;                          //whole ptxt matches?
            if (mode & 2) pdoc = pdoc + OSTxtSize(ptxt);
            break;
        }
    }
    return(pdoc);                               //return found ptr or pend
}

//*************************************************************************
//  OSTxtQuad:
//      Writes up to four characters of text given a 32-bit value with
//      up to four byte characters as returned by OSFileExt and OSNameInt.
//
//  Arguments:
//      text* OSTxtQuad (text* pdest, lint ext)
//      pdest   Pointer to output buffer.
//      ext     32-bit value with upper-case extension letters.
//              The first letter is in the lowest byte.
//              Unused high bytes are zero.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSTxtQuad (text* pdest, lint ext) {
    text c;
    fbyte* pbyte;
    int cnt;

    pbyte = (fbyte*) &ext;
    cnt = sizeof(ext);
    do {
        c = (text) *pbyte++;
        if (c == 0) break;
        *pdest++ = c;
    } while (--cnt);
    *pdest = 0;
    return(pdest);
}

//*************************************************************************
//  OSTxt2MIP:
//      Converts "HH:HH:HH:HH:HH:HH" text representation of a MAC address
//      to or from six byte representation.
//      Or converts "NN.NN.NN.NN" text representation of an IP address to
//      or from a four byte integer which is MSByte 1st network order!
//
//  Arguments:
//      text* OSTxt2MIP (int mode, byte* pmac, text* ptxt)
//      mode    1 = Convert pmac to ptxt zero-term text representation
//              2 = Convert ptxt HH:HH:HH:HH:HH:HH text to pmac bytes.
//
//              4 = Convert to and from IPV4 four byte "DDD.DDD.DDD.DDD"
//              5 = Convert pmac binary IP to ptxt "DDD.DDD.DDD.DDD".
//              6 = Convert ptxt "DDD.DDD.DDD.DDD" to four pmac IPV4 bytes.
//
//      pmac    Points to six bytes with binary mac address.
//              Or for +4 mode points to four bytes with IP.
//              Both MAC and IP numbers are most significant byte first!
//              Therefore IP dword number not Intel order!
//
//      ptxt    Points to "HH:HH:HH:HH:HH:HH" text representation.
//              Or for +4 mode points to "DDD.DDD.DDD.DDD" IP.
//
//  Returns:
//      Pointer to end of ptxt HH:HH:HH:HH:HH:HH string.
//*************************************************************************

CSTYLE text* xproc
OSTxt2MIP (int mode, byte* pmac, text* ptxt) {
    int cnt;

    if (mode & 4) {                             //using four-byte IP (not MAC)?
        cnt = 4;                                //four IP bytes
        if (mode & 2) {                         //convert text to four bytes?
            do {
                *pmac++ = (byte) OSIntGet(ptxt, &ptxt);
                if (*ptxt) ptxt += 1;           //skip dot
            } while (--cnt);
        } else {                                //binary IP to text?
            do {
                ptxt = OSIntPut(*pmac++, ptxt);
                *ptxt++ = '.';
            } while (--cnt);
            ptxt -= 1;
            *ptxt = 0;
        }
        return(ptxt);
    }

    cnt = 6;                                    //six MAC bytes
    if (mode & 2) {
        do {                                    //text to binary?
            *pmac++ = (byte) OSHexGet(ptxt, &ptxt);
            if (*ptxt) ptxt += 1;               //skip colon
        } while (--cnt);
    } else {                                    //binary MAC to text?
        do {
            if (*pmac < 0x10) *ptxt++ = '0';
            ptxt = OSHexPut(*pmac++, ptxt);
            *ptxt++ = ':';
        } while (--cnt);
        ptxt -= 1;
        *ptxt = 0;
    }
    return(ptxt);
}

//*************************************************************************
//  OSWideUTF8:
//      Converts between wide word and UTF-8 byte Unicode text.
//
//  Arguments:
//      int OSWideUTF8 (int mode, wtxt* pwtxt, text* ptxt, int num, uint max)
//      mode    0 = Convert from pwtxt Unicode wide to ptxt UTF-8   OS_2UTF8
//              1 = Convert from ptxt UTF-8 to pwtxt Unicode wide   OS_2WIDE
//      pwtxt   Wide Unicode source or target buffer (zero terminated string).
//              Target may be NULL to just return required buffer size.
//      ptxt    Byte UTF-8 source or target buffer (zero terminated string).
//              Target may be NULL to just return required buffer size.
//      num     Number of characters in the source string to be used.
//              May be -1 if source is zero terminated string.
//      max     Maximum characters in target buffer including ending zero.
//              Should be 0 to just get required buffer size for OS_2UTF8.
//
//  Returns:
//      Returns number of characters copied or needed including ending zero.
//      Returns zero if could not copy everything because buffer too small.
//      Also returns zero for mode 0 if UTF-8 text is malformed.
//*************************************************************************

CSTYLE int xproc
OSWideUTF8 (int mode, wtxt* pwtxt, text* ptxt, int num, uint max) {
    text* pold;
    text* pend;
    wtxt* pwend;
    wtxt* porig;
    byte cc, c1;
    wtxt wc;
    int cnt;

// Convert UTF-8 to wide Unicode.

    if (mode & OS_2WIDE) {                      //UTF-8 to wide?
        if (num < 0) num = OSTxtSize(ptxt) - 1;
        pend = ptxt + num;
        if (pwtxt) {
            porig = pwtxt;
            while ((ptxt < pend)&&(*ptxt)) {
                if (max < 2) {                  //buffer too small?
                    *pwtxt = 0;
                    return(0);
                }
                max -= 1;
                cc = *ptxt++;
                c1 = cc & 0xE0;
                if (cc <= 0x7F) {
                    *pwtxt++ = (word) cc;
                } else if (c1 == 0xC0) {
                    wc = ((wtxt)cc & 0x1F) << 6;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        *pwtxt = 0;
                        return(0);
                    }
                    wc |= (wtxt)cc & 0x3F;
                    if (wc <= 0x7F) {
                        *pwtxt = 0;             //spoofing lower character
                        return(0);              //can be security risk sometimes
                    }
                    *pwtxt++ = wc;
                } else if (c1 == 0xE0) {
                    wc = ((wtxt)cc & 0xF) << 12;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        *pwtxt = 0;
                        return(0);
                    }
                    wc |= ((wtxt)cc & 0x3F) << 6;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        *pwtxt = 0;
                        return(0);
                    }
                    wc |= (wtxt)cc & 0x3F;
                    if (wc <= 0x7FF) {
                        *pwtxt = 0;             //spoofing lower character
                        return(0);              //can be security risk sometimes
                    }
                    *pwtxt++ = wc;                    
                } else {
                    *pwtxt = 0;
                    return(0);
                }
            }
            *pwtxt++ = 0;
            return((int)(pwtxt - porig));

        } else {                                //just get required size?
            cnt = 0;
            while ((ptxt < pend)&&(*ptxt)) {
                cc = *ptxt++;
                c1 = cc & 0xE0;
                if (cc <= 0x7F) {
                    cnt += 1;
                } else if (c1 == 0xC0) {
                    wc = ((wtxt)cc & 0x1F) << 6;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        return(0);
                    }
                    wc |= (wtxt)cc & 0x3F;
                    if (wc <= 0x7F) {
                        return(0);
                    }
                    cnt += 1;
                } else if (c1 == 0xE0) {
                    wc = ((wtxt)cc & 0xF) << 12;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        return(0);
                    }
                    wc |= ((wtxt)cc & 0x3F) << 6;
                    cc = *ptxt++;
                    c1 = cc & 0xC0;
                    if (c1 != 0x80) {
                        return(0);
                    }
                    wc |= (wtxt)cc & 0x3F;
                    if (wc <= 0x7FF) {
                        return(0);
                    }
                    cnt += 1;
                } else {
                    return(0);
                }
            }
            return(cnt+1);
        }

// Convert wide Unicode to UTF-8.

    } else {                                    //to UTF-8?
        if (num < 0) {
            num = 0;
            pwend = pwtxt;
            while (*pwend) {
                pwend += 1;
                num += 1;
            }
        }
        pwend = pwtxt + num;
        if (ptxt) {
            pold = ptxt;
            while ((pwtxt < pwend)&&(*pwtxt)) {
                wc = *pwtxt++;
                if        (wc <= 0x7F) {
                    if (max < 2) {              //buffer too small?
                        *ptxt = 0;
                        return(0);
                    }
                    *ptxt++ = (byte) wc;
                    max -= 1;
                } else if (wc <= 0x7FF) {
                    if (max < 3) {              //buffer too small?
                        *ptxt = 0;
                        return(0);
                    }
                    *ptxt++ = (text)(0xC0 + ((wc & 0x7C0) >> 6));
                    *ptxt++ = (text)(0x80 +  (wc &  0x3F));
                    max -= 2;
                } else {
                    if (max < 4) {              //buffer too small?
                        *ptxt = 0;
                        return(0);
                    }
                    *ptxt++ = (text)(0xE0 + ((wc & 0xF000) >> 12));
                    *ptxt++ = (text)(0x80 + ((wc &  0xFC0) >> 6));
                    *ptxt++ = (text)(0x80 +  (wc &   0x3F));
                    max -= 3;
                }
            }
            *ptxt++ = 0;
            return((int)(ptxt - pold));
        } else {                                //just get required size?
            cnt = 0;
            while ((pwtxt < pwend)&&(*pwtxt)) {
                wc = *pwtxt++;
                if        (wc <= 0x7F) {
                    cnt += 1;
                } else if (wc <= 0x7FF) {
                    cnt += 2;
                } else {
                    cnt += 3;
                }
            }
            return(cnt+1);
        }
    }
}

//*************************************************************************
//  OSDateTime:
//      Returns a text string with a date and/or time.
//      Can either use the current date and time or can use a binary
//      date/time value formatted as a double dword.
//      Can also use a Windows FILETIME date and time.
//      Can use the local format or a standard YY/MM/DD HH:MM format.
//      Can also convert a date time to a single-dword or signed-dword value.
//
//  Arguments:
//      text* OSDateTime (text* pdest, qdtime* pout, qdtime* puse, int mode)
//
//      pdest   Pointer to buffer to receive the date and/or time text.
//              Must be large enough (SZNAME+SZNAME+2 characters recommended).
//              This is a text version of the pout output.
//              The 0,1,2,4,8,12 modes change the format.
//              May be NULL if don't want text date.
//
//      pout    Pointer to target date & time value in the double-dword format:
//              LSW LSB: reserved_zeros=16b, ms=10b, sec=6b
//              MSW LSB: min=6b, hour=5b, day=5b, month=4b, year=12b
//              For +64 mode points to FILETIME or time_t target.
//              This is a binary copy of the pdest output.
//              May be NULL if not needed.
//
//      puse    Pointer to source date & time value in the double-dword format:
//              LSW LSB: reserved_zeros=16b, ms=10b, sec=6b
//              MSW LSB: min=6b, hour=5b, day=5b, month=4b, year=12b
//              For +32 mode, points to FILETIME or time_t.
//              For +32 mode, converts to UTC (GMT) time if +128 mode.
//              But normally, the input qword is either UTC or local time
//              and this function does not use +64 to convert UTC vs local.
//              For +512 mode, adds puse date and time increments to pout.
//              For +512 mode, the puse fields are number of units to add.
//              Use NULL to get the current date and time.
//
//      mode    0 = Output both date and time local default format.  (OS_DTDEF)
//              1 = Output date only.                                (OS_DTDATE)
//              2 = Output time only.                                (OS_DTTIME)
//
//             +4 = Add in to output in standard YYYY/MM/DD HH:MM:SS format
//                  (rather than local date/time format).            (OS_DTYMD)
//             +8 = Add in to output long Mon DD, YYYY HH:MM format  (OS_DTMDY)
//                  (rather than local date/time format).
//            +12 = Add in to output very long Day, Month DD, YYYY HH:MM format.
//                  (this is long local date/time format).           (OS_DTDOW)
//
//            +32 = Interpret *puse as Windows FILETIME or Linux time_t ptr.
//                  (rather than our normal double-dword format).    (OS_DTFILE)
//                  You should normally also add in OS_DTGMT +128!
//            +64 = Output Windows FILETIME or Linux time_t to pout. (OS_DTFOUT) 
//                  The puse time should be UTC or should use +128.
//            +65 = Use mode 64+1 to output secs since 2000.         (OS_DTFSEC)
//           +128 = Use UTC (GMT) instead of local time if get time. (OS_DTGMT)
//                  This changes puse time to GMT from local.
//                  Or gets the current GMT time if puse is NULL.
//
//           +256 = Compare pout with puse date and time.            (OS_DTCMP)
//                  Returns TRUE if puse has reached pout: pout <= puse
//                  Returns NULL if pout is later than puse.
//                  This is actually just a general unsigned qword comparison.
//                  Cannot be combined with other mode flags.
//
//           +512 = Add puse date and time increment to pout date and time.
//                  The puse can have non-zero fields to add.        (OS_DTADD)
//                  For example, DT_SECSUNIT in the low dword would add 1 sec.
//                  The pout date and time is advanced accordingly.
//                  Cannot combine with modes 32,128,256,512,2048.
//
//          +1024 = Convert puse from GMT time to local time.        (OS_DTLOC)
//                  Cannot combine with modes 32,128,256,512,2048.
//
//           2048 = Convert *pdest local date time text to *pdate format.
//                  May add +4 OS_DTYMD to always use YY/MM/DD HH:MM format.
//                  Linux always uses YY/MM/DD HH:MM format.         (OS_DTGET)
//                  Uses *pdest text input and returns *pout date.
//                  Cannot be combined with other mode flags.
//
//           4096 = Get current time as seconds since time began.    (OS_DTSEC)
//                  Returns total seconds in *pout.
//                  This is seconds since 1/1/1970 for Linux.
//                  This is seconds since 1/1/1601 for Windows.
//                  This mode is more efficient in Linux.
//                  Must provide pout and puse must be NULL.
//                  Ignores pdest, which should also be NULL.
//
//           8192 = Use Year-2000 in pout and puse dates so that     (OS_DT2000)
//                  the qword value is not negative if signed.
//
//  Returns:
//      Returns a pointer to the zero placed at the end of the destination text.
//      If pdest is NULL then returns NULL.
//      For +256 mode comparison returns false NULL or true non-NULL pointer.
//      The 2048 get date mode and 4096 get seconds modes always return NULL.
//*************************************************************************

CSTYLE text* xproc
OSDateTime (text* pdest, qdtime* pout, qdtime* puse, int mode) {
    #ifdef ISWIN                                //===
    FILETIME file;
    FILETIME* pfile;
    dword flags;
    text* pform;
    #else                                       //===
    text* pend;
    time_t untime;
    struct tm* puns;
    struct tm uns;
    lint nday;
    #endif                                      //===
    QDW* pdate;
    SYSTIME when;
    SYSTIME* pwhen;
    dword date, frac;
    int num, days;
    qdtime local;

    if (mode & OS_DTGET) {                      //get date from text?
        text bufr[2];

        bufr[0] = 0;
        #ifdef ISWIN                            //===
        GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_IDATE, bufr, 2);
        #else                                   //===
        bufr[0] = '2';                          //Linux always YY/MM/DD
        #endif                                  //===
        if (mode & OS_DTTIME) bufr[0] = '2';

        OSMemClear(&when, sizeof(SYSTIME));
        while (*pdest == ' ') pdest += 1;

        num = OSIntGet(pdest, &pdest);
        if      (bufr[0] == '2') when.wYear  = (word)num;
        else if (bufr[0] == '1') when.wDay   = (word)num;
        else                     when.wMonth = (word)num;
        while (*pdest == ' ') pdest += 1;
        if ((*pdest == '/')||(*pdest == '-')) pdest += 1;
        while (*pdest == ' ') pdest += 1;
        
        num = OSIntGet(pdest, &pdest);
        if      (bufr[0] == '2') when.wMonth = (word)num;
        else if (bufr[0] == '1') when.wMonth = (word)num;
        else                     when.wDay   = (word)num;
        while (*pdest == ' ') pdest += 1;
        if ((*pdest == '/')||(*pdest == '-')) pdest += 1;
        while (*pdest == ' ') pdest += 1;

        num = OSIntGet(pdest, &pdest);
        if      (bufr[0] == '2') when.wDay   = (word)num;
        else if (bufr[0] == '1') when.wYear  = (word)num;
        else                     when.wYear  = (word)num;
        while (*pdest == ' ') pdest += 1;

        when.wHour = (word) OSIntGet(pdest, &pdest);
        while (*pdest == ' ') pdest += 1;
        if (*pdest == ':') pdest += 1;
        while (*pdest == ' ') pdest += 1;
        
        when.wMinute = (word) OSIntGet(pdest, &pdest);
        while (*pdest == ' ') pdest += 1;
        if (*pdest == ':') pdest += 1;
        while (*pdest == ' ') pdest += 1;

        when.wSecond = (word) OSIntGet(pdest, &pdest);
        while (*pdest == ' ') pdest += 1;
        
        if ((*pdest == 'P')||(*pdest == 'p')) {  //PM?
            if (when.wHour < 12) when.wHour += 12;
        }

        if ((when.wYear < 100)&&(when.wMonth)) {
            if (when.wYear < 30) when.wYear += 2000;
            else                 when.wYear += 1900;
        }

        date = when.wYear & 0xFFF;
        date = date << 4;
        date = date | (when.wMonth & 0xF);
        date = date << 5;
        date = date | (when.wDay & 0x1F);
        date = date << 5;
        date = date | (when.wHour & 0x1F);
        date = date << 6;
        date = date | (when.wMinute & 0x3F);
        pdate = (QDW*)pout;
        pdate->dw.hi = date;                    //return date in our format
        pdate->dw.lo = (dword)when.wSecond << DT_SECSSHFT;
        return(NULL);
    }

    if (mode & OS_DTCMP) {                      //compare times?
        if        ( ((QDW*)pout)->dw.hi > ((QDW*)puse)->dw.hi ) {
            return(NULL);                       //NULL => pout > puse
        } else if ( ((QDW*)pout)->dw.hi < ((QDW*)puse)->dw.hi ) {
            return((text*)TRUE);               //TRUE => pout <= puse
        } else if ( ((QDW*)pout)->dw.lo > ((QDW*)puse)->dw.lo ) {
            return(NULL);
        } else {
            return((text*)TRUE);
        }

    } else if (mode & OS_DTADD) {               //add to pout time?
        pdate = (QDW*)pout;
        date = pdate->dw.hi;
        frac = pdate->dw.lo;
        when.wYear = (word) ((date & 0xFFF00000) >> (6+5+5+4));
        if      (when.wYear <  80) when.wYear += 2000;
        else if (when.wYear < 100) when.wYear += 1900;
        when.wMonth =  (word) ((date & 0xF0000) >> (6+5+5));
        when.wDay =    (word) ((date & 0xF800) >> (6+5));
        when.wHour =   (word) ((date & 0x7C0) >> (6));
        when.wMinute = (word) (date & 0x3F);
        when.wSecond = (word) ((frac &0xFC000000) >> (16+10));
        when.wMilliseconds = (word) ((frac &0x3FF0000) >> (16));;
        when.wDayOfWeek = 0;
        pwhen = &when;

        pdate = (QDW*)puse;
        num = (pdate->dw.lo & DT_MSECMASK) >> DT_MSECSHFT;
        when.wMilliseconds += num;
        if (when.wMilliseconds > 999) {
            when.wMilliseconds -= 1000;
            when.wSecond += 1;
        }
        num = (pdate->dw.lo & DT_SECSMASK) >> DT_SECSSHFT;
        when.wSecond += num;
        if (when.wSecond > 59) {
            when.wSecond -= 60;
            when.wMinute += 1;
        }
        num = (pdate->dw.hi & DT_MINSMASK);
        when.wMinute += num;
        if (when.wMinute > 59) {
            when.wMinute -= 60;
            when.wHour += 1;
        }
        num = (pdate->dw.hi & DT_HOURMASK) >> DT_HOURSHFT;
        when.wHour += num;
        if (when.wHour > 23) {
            when.wHour -= 24;
            when.wDay += 1;
        }
        num = (pdate->dw.hi & DT_DAYSMASK) >> DT_DAYSSHFT;
        num += when.wDay;
        while (num > 28) {
            if      (when.wMonth == 1)  days = 31;
            else if (when.wMonth == 2) {
                days = (when.wYear & 3) ? 28 : 29;
                if ((when.wYear % 100) == 0) days = 29;
                if ((when.wYear % 400) == 0) days = 28;
            }
            else if (when.wMonth == 3)  days = 31;
            else if (when.wMonth == 4)  days = 30;
            else if (when.wMonth == 5)  days = 31;
            else if (when.wMonth == 6)  days = 30;
            else if (when.wMonth == 7)  days = 31;
            else if (when.wMonth == 8)  days = 31;
            else if (when.wMonth == 9)  days = 30;
            else if (when.wMonth == 10) days = 31;
            else if (when.wMonth == 11) days = 30;
            else                        days = 31;
            if (num <= days) break;
            num = num - days;
            when.wMonth += 1;
            if (when.wMonth > 12) {
                when.wMonth = 1;
                when.wYear += 1;
            }
        }
        when.wDay = (word)num;

        num = (pdate->dw.hi & DT_MNTHMASK) >> DT_MNTHSHFT;
        when.wMonth += num;
        if (when.wMonth > 12) {
            when.wHour -= 12;
            when.wYear += 1;
        }

        num = (pdate->dw.hi & DT_YEARMASK) >> DT_YEARSHFT;
        when.wYear += num;

    } else if (mode & OS_DTLOC) {               //convert GMT to local time?
        #ifdef ISWIN                            //===
        OSDateTime(0, (qdtime*)&file, puse, OS_DTFOUT);
        OSDateTime(0, &local, (qdtime*)&file, OS_DTFILE);
        #else                                   //===
        OSDateTime(0, (qdtime*)&untime, puse, OS_DTFOUT);
        OSDateTime(0, &local, (qdtime*)&untime, OS_DTFILE);
        #endif                                  //===
        pdate = (QDW*)&local;
        date = pdate->dw.hi;
        frac = pdate->dw.lo;
        when.wYear = (word) ((date & 0xFFF00000) >> (6+5+5+4));
        if      (when.wYear <  80) when.wYear += 2000;
        else if (when.wYear < 100) when.wYear += 1900;
        when.wMonth =  (word) ((date & 0xF0000) >> (6+5+5));
        when.wDay =    (word) ((date & 0xF800) >> (6+5));
        when.wHour =   (word) ((date & 0x7C0) >> (6));
        when.wMinute = (word) (date & 0x3F);
        when.wSecond = (word) ((frac &0xFC000000) >> (16+10));
        when.wMilliseconds = (word) ((frac &0x3FF0000) >> (16));;
        when.wDayOfWeek = 0;
        pwhen = &when;

    } else if ((mode & OS_DTFILE)&&(puse)) {    //FILETIME format?
        #ifdef ISWIN                            //===
        pfile = (FILETIME*)puse;
        if (!(mode & OS_DTGMT)) {               //show local time?
            num = FileTimeToLocalFileTime(pfile, &file);
            if (num) pfile = &file;
        }
        num = FileTimeToSystemTime(pfile, (LPSYSTEMTIME)&when);
        if (num == FALSE) GetLocalTime((LPSYSTEMTIME)&when);
        #else                                   //===
        untime = *((time_t*)puse);
        if (mode & OS_DTGMT) {
            puns = gmtime(&untime);
        } else {
            puns = localtime(&untime);
        }
        when.wYear      = (word)puns->tm_year + 1900;
        when.wMonth     = (word)puns->tm_mon + 1;
        when.wDayOfWeek = (word)puns->tm_wday;
        when.wDay       = (word)puns->tm_mday;
        when.wHour      = (word)puns->tm_hour;
        when.wMinute    = (word)puns->tm_min;
        when.wSecond    = (word)puns->tm_sec;
        when.wMilliseconds = 0;
        #endif                                  //===
        pwhen = &when;

    } else if (puse) {                          //get double-dword format?
        pdate = (QDW*)puse;
        date = pdate->dw.hi;
        frac = pdate->dw.lo;
        when.wYear = (word) ((date & 0xFFF00000) >> (6+5+5+4));
        if (mode & OS_DT2000) when.wYear += DT_YEARO;
        if      (when.wYear <  80) when.wYear += 2000;
        else if (when.wYear < 100) when.wYear += 1900;
        when.wMonth =  (word) ((date & 0xF0000) >> (6+5+5));
        when.wDay =    (word) ((date & 0xF800) >> (6+5));
        when.wHour =   (word) ((date & 0x7C0) >> (6));
        when.wMinute = (word) (date & 0x3F);
        when.wSecond = (word) ((frac &0xFC000000) >> (16+10));
        when.wMilliseconds = (word) ((frac &0x3FF0000) >> (16));;
        when.wDayOfWeek = 0;
        pwhen = &when;

    } else if ((mode & OS_DTSEC)&&(pout)) {     //get seconds since epoch?
        #ifdef ISWIN                            //===
        GetSystemTime((LPSYSTEMTIME)&when);
        SystemTimeToFileTime((LPSYSTEMTIME)&when, &file);
        ((QDW*)pout)->dw.lo = file.dwLowDateTime;
        ((QDW*)pout)->dw.hi = file.dwHighDateTime;
        ((QDW*)pout)->qw /= 10000000;           //100 nanosec intervals to secs
        #else                                   //===
        *pout = (qword) time(NULL);
        #endif                                  //===
        return(NULL);

    } else {                                    //get current time?
        #ifdef ISWIN                            //===
        if (mode & OS_DTGMT) {
            GetSystemTime((LPSYSTEMTIME)&when);
        } else {
            GetLocalTime((LPSYSTEMTIME)&when);
        }
        #else                                   //===
        untime = time(NULL);
        if (mode & OS_DTGMT) {
            puns = gmtime(&untime);
        } else {
            puns = localtime(&untime);
        }
        when.wYear      = (word)puns->tm_year + 1900;
        when.wMonth     = (word)puns->tm_mon + 1;
        when.wDayOfWeek = (word)puns->tm_wday;
        when.wDay       = (word)puns->tm_mday;
        when.wHour      = (word)puns->tm_hour;
        when.wMinute    = (word)puns->tm_min;
        when.wSecond    = (word)puns->tm_sec;
        when.wMilliseconds = 0;
        #endif                                  //===
        pwhen = &when;
    }

    if (pout) {                                 //return double-dword format?
        if (mode & OS_DTFOUT) {                 //file time output?
            #ifdef ISWIN                        //===
            pfile = (FILETIME*)pout;
            SystemTimeToFileTime((CONST SYSTEMTIME*)pwhen, pfile);
            if (mode & 1) {                     //make it secs since 2000?
                *pout /= 10000000;              //100 nanosec intervals to secs
                *pout -= 12591158400;           //epoch to 1/1/2000 in secs
            }

            #else                               //===
            //There is no Linux function to convert GMT tm info to time_t.
            //The mktime function only converts local tm info to time_t.
            //The following code mostly works but can be off by an hour
            //right around the time we switch to and from daylight savings time.
            //OSMemClear(&uns, sizeof(uns));
            //uns.tm_year  = pwhen->wYear - 1900;
            //uns.tm_mon   = pwhen->wMonth - 1;
            //uns.tm_wday  = pwhen->wDayOfWeek;
            //uns.tm_mday  = pwhen->wDay;
            //uns.tm_hour  = pwhen->wHour;  
            //uns.tm_min   = pwhen->wMinute;
            //uns.tm_sec   = pwhen->wSecond;
            //untime = mktime(&uns);            //first a test to see if summer
            //puns = localtime(&untime);        //get tm_isdst + any other stuff
            //OSMemCopy(&uns, puns, sizeof(uns));
            //uns.tm_year  = pwhen->wYear - 1900;
            //uns.tm_mon   = pwhen->wMonth - 1;
            //uns.tm_wday  = pwhen->wDayOfWeek;
            //uns.tm_mday  = pwhen->wDay;
            //uns.tm_hour  = pwhen->wHour;  
            //uns.tm_min   = pwhen->wMinute;
            //uns.tm_sec   = pwhen->wSecond;
            //untime = mktime(&uns);            //makes assuming pwhen is local
            //puns = gmtime(&untime);
            //secs = puns->tm_sec + (puns->tm_min * 60) + (puns->tm_hour * (60*60));
            //days = puns->tm_wday;
            //puns = localtime(&untime);
            //secs -= puns->tm_sec + (puns->tm_min * 60) + (puns->tm_hour * (60*60));
            //days -= puns->tm_wday;
            //if (days > 2) days = -1;
            //if (days < -2) days = 1;
            //secs += days * (60 * 60 * 24);    //GMT - Local delta seconds
            //untime -= secs;                   //mktime does local so adjust

            //Instead, this does the seconds after 1970 GMT conversion manually.
            //So it does not suffer from daylight savings time problems.
            //But it would break if Linux changed the time_t representation.
            nday = 0;                           //do in days rather rather than
            num = 1970;                         //secs so don't overflow 32 bits
            while (num > (int)pwhen->wYear) {   //before 1970?
                num -= 1;                       //backup with negative nday
                nday -= 365;                    //until given year
                if (  ((num % 4) == 0)
                    &&(((num % 100) != 0)||((num % 400) == 0))  ) {
                    nday -= 1;                  //leap year
                }
            }
            while (num < (int)pwhen->wYear) {   //count days to current year
                nday += 365;
                if (  ((num % 4) == 0)
                    &&(((num % 100) != 0)||((num % 400) == 0))  ) {
                    nday += 1;                  //leap year
                }
                num += 1;
            }
            num = 1;
            while (num < (int)pwhen->wMonth) {  //count days to current month
                if      (num == 1)  days = 31;
                else if (num == 2) {
                    days = (pwhen->wYear & 3) ? 28 : 29;
                    if ((pwhen->wYear % 100) == 0) days = 29;
                    if ((pwhen->wYear % 400) == 0) days = 28;
                }
                else if (num == 3)  days = 31;
                else if (num == 4)  days = 30;
                else if (num == 5)  days = 31;
                else if (num == 6)  days = 30;
                else if (num == 7)  days = 31;
                else if (num == 8)  days = 31;
                else if (num == 9)  days = 30;
                else if (num == 10) days = 31;
                else if (num == 11) days = 30;
                else                days = 31;
                nday += (dword) days;
                num += 1;
            }
            nday += (lint) (pwhen->wDay - 1);   //calculate secs in qword
            untime = ((time_t)nday) * (24*60*60);
            untime += (time_t) (pwhen->wHour * (60*60));
            untime += (time_t) (pwhen->wMinute * 60);
            untime += (time_t) pwhen->wSecond;
            
            if (mode & 1) untime -= 946684800;  //make it since 1/1/2000?
            *(time_t*)pout = untime;
            #endif                              //===

        } else {                                //our own qdtime output?
            date = pwhen->wYear;
            if (mode & OS_DT2000) date -= DT_YEARO;
            date = date & 0xFFF;
            date = date << 4;
            date = date | (pwhen->wMonth & 0xF);
            date = date << 5;
            date = date | (pwhen->wDay & 0x1F);
            date = date << 5;
            date = date | (pwhen->wHour & 0x1F);
            date = date << 6;
            date = date | (pwhen->wMinute & 0x3F);
            frac = pwhen->wSecond & 0x3F;
            frac = frac << 10;
            frac = frac | (pwhen->wMilliseconds & 0x3FF);
            frac = frac << 16;
            pdate = (QDW*)pout;
            pdate->dw.hi = date;
            pdate->dw.lo = frac;
        }
    }

    if (pdest == NULL) return(NULL);            //no text date and time needed?
    #ifdef ISWIN                                //===
    if ((mode & (1+2+4+8)) == 4) {              //YYYY/MM/DD HH:MM:SS?
        pdest = OSIntPut(pwhen->wYear, pdest);  //do ourself to guarantee format
        *pdest++ = '/';
        if (pwhen->wMonth < 10) *pdest++ = '0';
        pdest = OSIntPut(pwhen->wMonth, pdest);
        *pdest++ = '/';
        if (pwhen->wDay < 10) *pdest++ = '0';
        pdest = OSIntPut(pwhen->wDay, pdest);
        *pdest++ = ' ';

        if (pwhen->wHour < 10) *pdest++ = '0';
        pdest = OSIntPut(pwhen->wHour, pdest);
        *pdest++ = ':';
        if (pwhen->wMinute < 10) *pdest++ = '0';
        pdest = OSIntPut(pwhen->wMinute, pdest);
        *pdest++ = ':';
        if (pwhen->wSecond < 10) *pdest++ = '0';
        pdest = OSIntPut(pwhen->wSecond, pdest);
        *pdest = 0;
        return(pdest);
    }
    if (!(mode & 2)) {                          //output date?
        flags = (mode & 4) ? 0 : DATE_SHORTDATE;
        pform = (mode & 4) ? "yyyy/dd/MM" : NULL;
        if (mode & 8) {
            flags = 0;
            pform = "MMM dd, yyyy";
            if (mode & 4) {
                flags = DATE_LONGDATE;
                pform = NULL;
            }
        }
        num = GetDateFormatA(0, flags, (LPSYSTEMTIME)pwhen, pform, pdest, SZNAME);
        if (num) pdest += (num-1);
        if (!(mode & 1)) *pdest++ = ' ';
    }
    if (!(mode & 1)) {                          //output time?
        num = GetTimeFormatA(0,
                             ((mode & 4) ? 0 : TIME_NOSECONDS),
                             (LPSYSTEMTIME)pwhen,
                             ((mode & 4) ? "HH:mm:ss" : NULL),
                             pdest, SZNAME);
        if (num) pdest += (num-1);
    }
    *pdest = 0;
    return(pdest);

    #else                                       //===
    if (mode & (1+2+4+8)) {
        if (mode & 8) {
            if ((mode & 4)&&(!(mode & 2))) {
                if      (pwhen->wDayOfWeek == 0)  pdest = OSTxtCopy(pdest, "Sunday, ");
                else if (pwhen->wDayOfWeek == 1)  pdest = OSTxtCopy(pdest, "Monday, ");
                else if (pwhen->wDayOfWeek == 2)  pdest = OSTxtCopy(pdest, "Tuesday, ");
                else if (pwhen->wDayOfWeek == 3)  pdest = OSTxtCopy(pdest, "Wednesday, ");
                else if (pwhen->wDayOfWeek == 4)  pdest = OSTxtCopy(pdest, "Thursday, ");
                else if (pwhen->wDayOfWeek == 5)  pdest = OSTxtCopy(pdest, "Friday, ");
                else if (pwhen->wDayOfWeek == 6)  pdest = OSTxtCopy(pdest, "Saturday, ");

                if      (pwhen->wMonth == 1)  pdest = OSTxtCopy(pdest, "January ");
                else if (pwhen->wMonth == 2)  pdest = OSTxtCopy(pdest, "February ");
                else if (pwhen->wMonth == 3)  pdest = OSTxtCopy(pdest, "March ");
                else if (pwhen->wMonth == 4)  pdest = OSTxtCopy(pdest, "April ");
                else if (pwhen->wMonth == 5)  pdest = OSTxtCopy(pdest, "May ");
                else if (pwhen->wMonth == 6)  pdest = OSTxtCopy(pdest, "June ");
                else if (pwhen->wMonth == 7)  pdest = OSTxtCopy(pdest, "July ");
                else if (pwhen->wMonth == 8)  pdest = OSTxtCopy(pdest, "August ");
                else if (pwhen->wMonth == 9)  pdest = OSTxtCopy(pdest, "September ");
                else if (pwhen->wMonth == 10) pdest = OSTxtCopy(pdest, "October ");
                else if (pwhen->wMonth == 11) pdest = OSTxtCopy(pdest, "November ");
                else if (pwhen->wMonth == 12) pdest = OSTxtCopy(pdest, "December ");
                *pdest = 0;                     //just in case
            } else if (!(mode & 2)) {
                if      (pwhen->wMonth == 1)  pdest = OSTxtCopy(pdest, "Jan ");
                else if (pwhen->wMonth == 2)  pdest = OSTxtCopy(pdest, "Feb ");
                else if (pwhen->wMonth == 3)  pdest = OSTxtCopy(pdest, "Mar ");
                else if (pwhen->wMonth == 4)  pdest = OSTxtCopy(pdest, "Apr ");
                else if (pwhen->wMonth == 5)  pdest = OSTxtCopy(pdest, "May ");
                else if (pwhen->wMonth == 6)  pdest = OSTxtCopy(pdest, "Jun ");
                else if (pwhen->wMonth == 7)  pdest = OSTxtCopy(pdest, "Jul ");
                else if (pwhen->wMonth == 8)  pdest = OSTxtCopy(pdest, "Aug ");
                else if (pwhen->wMonth == 9)  pdest = OSTxtCopy(pdest, "Sep ");
                else if (pwhen->wMonth == 10) pdest = OSTxtCopy(pdest, "Oct ");
                else if (pwhen->wMonth == 11) pdest = OSTxtCopy(pdest, "Nov ");
                else if (pwhen->wMonth == 12) pdest = OSTxtCopy(pdest, "Dec ");
                *pdest = 0;                     //just in case
            }
            if (!(mode & 2)) {
                pdest = OSIntPut(pwhen->wDay, pdest);
                pdest = OSTxtCopy(pdest, ", ");
                pdest = OSIntPut(pwhen->wYear, pdest);
                if (!(mode & 1)) *pdest++ = ' ';
                *pdest = 0;
            }
            if (!(mode & 1)) {
                if (pwhen->wHour < 10) *pdest++ = '0';
                pdest = OSIntPut(pwhen->wHour, pdest);
                *pdest++ = ':';
                if (pwhen->wMinute < 10) *pdest++ = '0';
                pdest = OSIntPut(pwhen->wMinute, pdest);
                *pdest = 0;
            }
            return(pdest);
        }
        if (!(mode & 2)) {
            pdest = OSIntPut(pwhen->wYear, pdest);
            *pdest++ = '/';
            if (pwhen->wMonth < 10) *pdest++ = '0';
            pdest = OSIntPut(pwhen->wMonth, pdest);
            *pdest++ = '/';
            if (pwhen->wDay < 10) *pdest++ = '0';
            pdest = OSIntPut(pwhen->wDay, pdest);
            if (!(mode & 1)) *pdest++ = ' ';
            *pdest = 0;
        }
        if (!(mode & 1)) {
            if (pwhen->wHour < 10) *pdest++ = '0';
            pdest = OSIntPut(pwhen->wHour, pdest);
            *pdest++ = ':';
            if (pwhen->wMinute < 10) *pdest++ = '0';
            pdest = OSIntPut(pwhen->wMinute, pdest);
            *pdest++ = ':';
            if (pwhen->wSecond < 10) *pdest++ = '0';
            pdest = OSIntPut(pwhen->wSecond, pdest);
            *pdest = 0;
        }
        return(pdest);
    }
    uns.tm_year  = when.wYear - 1900;
    uns.tm_mon   = when.wMonth - 1;
    uns.tm_wday  = when.wDayOfWeek;
    uns.tm_mday  = when.wDay;
    uns.tm_hour  = when.wHour;  
    uns.tm_min   = when.wMinute;
    uns.tm_sec   = when.wSecond;
    pend = OSTxtCopy(pdest, asctime(&uns));
    if ((pend > pdest)&&(*(pend-1) == '\n')) {
        pend -= 1;
        *pend = 0;
    }
    return(pend);
    #endif                                      //===
}

//*************************************************************************
//  OSTickCount:
//      Gets the current processor clock tick count.
//      This is typically ms since the OS started and may wrap back around zero.
//      Linux typically does 100 ticks/sec and we multiply by 10 to return ms.
//      Compile with /D MS_LINUX to provide milliseconond resolution in Linux.
//
//  Arguments:
//      dword OSTickCount()
//
//  Returns:
//      Current tick count in milliseconds.
//*************************************************************************

CSTYLE dword xproc
OSTickCount () {
    #ifdef ISWIN                                //===
    return( GetTickCount() );

    #else                                       //===
    #ifndef MS_LINUX                            //---
    struct tms tm;
    return( times(&tm) * 10 );                  //try to return ms for Linux

    #else                                       //---
    struct timespec ts;       
    clock_gettime(CLOCK_MONOTONIC, &ts);        //current seconds and nsec
    return( ((dword)ts.tv_sec * 1000) + ((dword)ts.tv_nsec / 1000000) );
    #endif                                      //---
    #endif                                      //===
}

//*************************************************************************
//  OSFileRoot:
//      Gets the root name associated with a file.
//      This is either a drive such as "C:\"
//      or a root name such as "\\root\" or "http://www.triscape.com/".
//
//  Arguments:
//      int OSFileRoot (text* pbuf, text* pname)
//      pbuf    Destination buffer where root name is placed.
//              Includes ending '/' or '\' character.
//              This must be big enough for entire source name (SZPATH chars).
//              If there is neither a disk or root name ("subdir/name.ext")
//              then returns a blank string (a single zero).
//              If root name is missing ("\\" or "http://" or "///")
//              then returns a blank string (a single zero).
//              If root name does not end with slash ("\\root")
//              then adds a forward slash ("\\root/") in case URL.
//              If disk name does not end with a slash ("C:")
//              then adds a forward slash ("C:/") in case URL.
//              Forward slash also works for Windows even though
//              the backslash is normal.
//      pname   Source file name.
//
//  Returns:
//      Text size of result directory name.
//      Returns zero if error.
//*************************************************************************

CSTYLE int xproc
OSFileRoot (text* pbuf, text* pname) {
    text* pcolon;
    text* pslash;
    text* pdouble;
    text* ptxt;
    text c;

    OSTxtCopy(pbuf, pname);         //transfer to destination
    ptxt = pbuf;

    pcolon = NULL;
    pslash = NULL;
    pdouble = NULL;

    while (TRUE) {                  //find first colon, slash, and double slash
        c = *ptxt;
        if (c == 0) break;
        if ((c == ':')&&(pcolon == NULL)) {
            pcolon = ptxt;          //remember first colon
        }
        if ((c == '\\')||(c == '/')) {
            if (pslash == NULL) {   //slash or backslash are the same
                pslash = ptxt;      //remember first slash
            } else if (pslash == (ptxt - 1)) {
                pdouble = ptxt;     //remember first double slash
            }
        } else {
            if (pslash) break;      //need not look further than one char
        }                           //after first slash
        ptxt += 1;
    }
    if (pdouble) {                  //after double slash is root name
        ptxt = pdouble;

        ptxt += 1;
        c = *ptxt;
        if ((c == 0)||(c == '\\')||(c == '/')) {
            ptxt = pbuf;            //root name missing error => return blank
        } else {
            while (TRUE) {          //find end of root name
                c = *ptxt++;
                if (c == 0) {
                    ptxt -= 1;
                    *ptxt = OS_SLASH;
                    ptxt += 1;      //punt and end root with slash
                    break;
                }
                if ((c == '\\')||(c == '/')) {
                    break;          //root ends after next slash or backslash
                }
            }
        }
    } else if (pcolon) {            //otherwise prior to colon should be disk
        ptxt = pcolon;
        ptxt += 1;
        c = *ptxt;
        if ((c == '\\')||(c == '/')) {
            ptxt += 1;              //disk root name already ends with slash
        } else {
            *ptxt = OS_SLASH;       //assume slash
            ptxt += 1;
        }
    } else {                        //seems to be relative name without root?
        ptxt = pbuf;                //no disk or root => return blank
    }
    *ptxt = 0;                      //terminate output root name
    return (O32(ptxt - pbuf));      //return size of output string
}

//*************************************************************************
//  OSFileDir:
//      Gets the directory name associated with a file.
//
//  Arguments:
//      int OSFileDir (text* pbuf, text* pname)
//      pbuf    Destination buffer where directory name is placed.
//              Includes ending '/' or '\' or ':' character.
//              This must be big enough for entire source name (SZPATH chars).
//              May be the same as pname source.
//      pname   Source file name.
//
//  Returns:
//      Text size of result directory name.
//*************************************************************************

CSTYLE int xproc
OSFileDir (text* pbuf, text* pname) {
    text* pb;

    pb = OSTxtCopy(pbuf, pname);    //transfer to destination
    while (pb > pbuf) {             //find beginning of base name
        pb -= 1;
        if ((*pb == '/')||(*pb == '\\')||(*pb == ':')) {
            pb += 1;
            break;
        }
    }
    *pb = 0;                        //chop off base file name, leave directory
    return (O32(pb - pbuf));
}

//*************************************************************************
//  OSFilePar:
//      Gets parent directory name (dir the file's dir is in).
//
//  Arguments:
//      int OSFilePar (text* pbuf, text* pname)
//      pbuf    Destination buffer where directory name is placed.
//              Includes ending '/' or '\' or ':' character.
//              This must be big enough for entire source name (SZPATH chars).
//              May be the same as pname source.
//      pname   Source file name.
//
//  Returns:
//      Text size of result directory name.
//*************************************************************************

CSTYLE int xproc
OSFilePar (text* pbuf, text* pname) {
    text* pb;

    pb = OSTxtCopy(pbuf, pname);    //transfer to destination
    while (pb > pbuf) {             //find beginning of base name
        pb -= 1;
        if ((*pb == '/')||(*pb == '\\')||(*pb == ':')) {
            break;
        }
    }
    while (pb > pbuf) {             //find beginning of directory name
        pb -= 1;
        if ((*pb == '/')||(*pb == '\\')||(*pb == ':')) {
            pb += 1;
            break;
        }
    }

    *pb = 0;                        //chop off dir name, leave parent dir
    return (O32(pb - pbuf));
}

//*************************************************************************
//  OSFileBase:
//      Gets the base file name associated with a file.
//      Can optionally get both the base name and the extension.
//      For example "C:\DIRECTORY\BASE.EXT" returns "BASE" or "BASE.EXT".
//      Also handles URL's: "HTTP://SITE.COM/DIRECTORY/BASE.EXT".
//
//  Arguments:
//      int  OSFileBase (text* pbuf, text* pname, int ext)
//      pbuf    Destination buffer where base name is placed.
//              This must be big enough to hold extension even if ext=0.
//      pname   Source file name.  Must be null terminated.
//      ext     0 = base disk file name without extension.            (OS_NOEXT)
//              1 = base disk file name with extension.               (OS_EXTN)
//             +4 = Add to capitalize first letter with rest small.  (+OS_CAP1)
//             +8 = Add to check for directory name ending in slash. (+OS_DIRN)
//              Note that TriOS.h defines symbols for these exe values.
//
//  Returns:
//      Text size of result base name.
//*************************************************************************

CSTYLE int xproc
OSFileBase (text* pbuf, text* pname, int ext) {
    text* pb;
    text* pend;
    text* pdot;
    int c, nc;

    pend = pname + OSTxtSize(pname);
    pb = pend;
    pdot = NULL;

    while (pb > pname) {            //find beginning of base name
        pb -= 1;
        if (*pb == '.') pdot = pb;  //remember where extension starts
        if ((*pb == '\\')||(*pb == '/')||(*pb == ':')) {
            if (ext & OS_DIRN) {    //looking for base part of directory name?
                c = *pb;
                *pb = 0;            //get rid of ending slash temporarily
                nc = OSFileBase(pbuf, pname, (ext & (~OS_DIRN)));
                *pb = c;            //recurse to get base and restore source
                return(nc);
            }
            pb += 1;
            break;
        }
    }
    c = O32(pend - pb);
    OSMemCopy(pbuf, pb, c*SZCHR);   //copy base name and extension
    pbuf[c] = 0;

    if (!(ext & OS_EXTN) && pdot) {
        pb = pbuf + (pdot - pb);
        *pb = 0;                    //copy off extension if not needed
        pend = pb;
    } else pend = pbuf + (c - 1);
    if (ext & OS_CAP1) {            //convert to Capitalized (cap then lower)?
        for (pb=pbuf; pb<pend; ++pb) *pb = LOWCASE(*pb);
        if (pbuf<pend) *pbuf = CAPITAL(*pbuf);
    }
    return (O32(pend - pbuf));
}

//*************************************************************************
//  OSFileType:
//      Replaces a file name's extension with a different type.
//
//  Arguments:
//      int  OSFileType (text* pbuf, text* pname, text* pext)
//      pbuf    Destination buffer where name is placed.
//              This must be big enough to hold path name (SZPATH chars).
//              May be same buffer as pname source.
//      pname   Source file name.  Must be null terminated.
//              This may either have a different extension at the end
//              or may have no extension.
//      pext    New extension text to be added.
//              Normally this begins with a period (".new").
//              However period is added if not present ("new").
//              If NULL then just strips period and extension.
//
//  Returns:
//      Text size of result base name.
//*************************************************************************

CSTYLE int xproc
OSFileType (text* pbuf, text* pname, text* pext) {
    text* pb;
    text* pend;
    text* pdot;

    pend = OSTxtCopy(pbuf, pname);

    pb = pend;
    pdot = NULL;

    while (pb > pbuf) {             //find beginning of base name
        pb -= 1;
        if (*pb == '.') {
            pdot = pb;              //remember where extension starts
            break;
        }
        if ((*pb == '\\')||(*pb == '/')||(*pb == ':')) {
            pb += 1;                //no extension found
            break;
        }
    }

    if (pdot == NULL) pdot = pend;  //no extension => add to very end
    *pdot = 0;
    if (pext) {                     //add new extension
        if (*pext != '.') *pdot++ ='.';
        pdot = OSTxtCopy(pdot, pext);
    }
    return (O32(pdot - pbuf));
}

//*************************************************************************
//  OSFileExt:
//      Gets the extension name (with the period) associated with a file.
//      Can also get the extension letters (without period) normalized
//      to upper case in a single 32-bit value.  See OSTxtQuad put value.
//
//  Arguments:
//      lint OSFileExt (text* pbuf, text* pname)
//      pbuf    Destination buffer where extension name is placed.
//              The period character is included.
//              May be NULL if not needed.
//      pname   Source file name.  Must be null-terminated.
//
//  Returns:
//      32-bit value with upper-case extension letters.
//      The first letter is in the lowest byte.
//      Unused high bytes are zero.
//      Returns zero if there is no .ext at end of name.
//*************************************************************************

CSTYLE lint xproc
OSFileExt (text* pbuf, text* pname) {
    text* pb;
    text* pend;
    int c;

    pend = pname + OSTxtSize(pname);
    pb = pend;
    while (pb > pname) {
        pb -= 1;
        if ((*pb == '\\')||(*pb == ':')||(*pb == '/')) break;
        if (*pb == '.') {                       //return ext after last period
            c = O32(pend - pb);
            if (pbuf) {
                OSMemCopy(pbuf, pb, c);
                pbuf[c] = 0;
            }
            pb += 1;
            return( OSNameInt(pb, NULL) );      //return extension as integer
        }
    }
    if (pbuf) *pbuf = 0;                        //there is no extension
    return(0);
}

//*************************************************************************
//  OSNameInt:
//      Converts a text word to a single 32-bit integer representation.
//      Letters are normalized to all upper-case and stored beginning in
//      the integer's least significant byte.  Unused high bytes are zeroed.
//      The name stops with the first character which is not a letter or
//      number or underbar.  The text pointer is advanced to the end
//      of the word even if it has more than four characters, although
//      only the first four characters are saved in the integer.
//      Beware that C integer constants store most significant char first.
//      For example "abc" returns 'CBA' and "Small" returns 'LAMS'.
//      Beware that this routine always returns Intel low-endian byte order.
//
//  Arguments:
//      lint OSNameInt (text* pname, lptext* ppend)
//      pname   Pointer to the beginning of the name.
//              Automatically skips preliminary whitespace.
//      ppend   Pointer to where pointer to just after end of
//              name is returned.  May be zero if not needed.
//
//  Returns:
//      32-bit value with upper-case extension letters.
//      The first letter is in the lowest byte.
//      Unused high bytes are zero.
//*************************************************************************

CSTYLE lint xproc
OSNameInt (text* pname, lptext* ppend) {
    text c;
    lint ext;
    int shift;

    while ((*pname)&&((unsigned)*pname <= ' '))
         pname += 1;                    //skip preliminary whitespace
    ext = 0;
    shift = 0;
    while (*pname) {
        c = *pname;
        c = CAPITAL(c);
        if (!(  ((c >= 'A')&&(c <= 'Z'))
              ||((c >= '0')&&(c <= '9'))
              || (c == '?')
              || (c == '_')  )) break;  //end of word?
        pname += 1;
        if (shift >= 32) continue;      //can only save first four letters
        ext |= (lint)c << shift;
        shift += 8;
    }
    if (ppend) *ppend = pname;          //return end pointer
    return(ext);                        //return integer representation
}

//*************************************************************************
//  OSLocation:
//      Gets commonly used folder path or file names.
//      Can also get other system information in text form.
//
//  Arguments:
//      text* OSLocation (text* pbuf, int mode)
//      pbuf    Destination buffer where directory name is placed.
//              Folder names include ending '/' character.
//              This must be at least SZDISK chars.
//      mode    0 = Return OS binaries directory with ending slash.  (OS_LOCBIN)
//                  This is \Windows\System32 or /bin
//              1 = Return currently-running executable's filename.  (OS_LOCEXE)
//                  Gets the full path and also its file name.
//              2 = User's Documents directory with ending slash.    (OS_LOCDOC)
//                  This is My Documents, Documents, or exe's dir.
//              3 = Application Data directory with ending slash.    (OS_LOCAPP)
//                  This user\AppData or roaming under Windows
//                  This is currently the exe's directory for Linux.
//                  If CWINIX defined is also exe's dir for Windows.
//                  See mode 4=OS_LOCALL for all-users app data.
//              4 = Windows ...\ProgramData\ for All Users app data. (OS_LOCALL)
//                  Before Vista uses All Users folder.
//                  See mode 3=OS_LOCAPP for current user's app data.
//              5 = Temporary directory with ending slash.           (OS_LOCTMP)
//              6 = Get current directory with ending slash.         (OS_CURDIR)
//                  Also called the current working directory.
//              7 = Get currently-running executable's directory.    (OS_EXEDIR)
//                  Similar to mode 1 but without exe's filename.
//              8 = Windows My Documents or Linux exe's directory    (OS_DOCEXE)
//              20= Get OS version text information.                 (OS_VERNUM)
//                  This consists of text numbers: OS.Maj.Min.Build
//                  Thie OS number is OSN_WINDOWS=1 or OSN_LINUX=2.
//                  The Major.Minor.Build format may vary in Linux
//                  since it is just the text returned by uname -r.
//              21= Get User Name currently logged into the OS.      (OS_USER)
//                  This currently works for Windows but not Linux.
//              Note that TriOS.h defines symbols for these mode values.
//
//  Returns:
//      Pointer to the terminating zero at the end of the output pbuf path.
//      Returns blank (just terminating zero) if error.
//*************************************************************************

#pragma warning(disable : 4996)                 //stop GetVersionEx warning

CSTYLE text* xproc
OSLocation (text* pbuf, int mode) {
    #ifdef ISWIN                                //===
    OSVERSIONINFO osinfo;
    #else                                       //===
    aint file;
    text* puse;
    #endif                                      //===
    text name[SZPATH];
    text* ptxt;
    int ret;

    *pbuf = 0;                                  //in case of error
    ptxt = pbuf;                                //in case of error
    #ifdef ISWIN                                //=== Windows
    if (mode == OS_LOCBIN) {                    //OS binaries folder?
        GetWindowsDirectoryA(pbuf, SZDISK);
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);
        ptxt = OSTxtCopy(ptxt, "\\System32\\");

    } else if (mode == OS_LOCEXE) {             //Executable path and filename?
        #ifndef CNWIDE                          //---
        wtxt wide[SZPATH];
        GetModuleFileNameW(NULL, (LPWSTR)wide, SZDISK);
        int num = OSWideUTF8(OS_2UTF8, wide, pbuf, -1, SZPATH);
        if (num <= 0) *pbuf = 0;                //convert from wide chars
        #else                                   //---
        GetModuleFileNameA(NULL, pbuf, SZDISK);
        #endif                                  //---
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);

    } else if (mode == OS_EXEDIR) {             //Executable path only?
        #ifndef CNWIDE                          //---
        wtxt wide[SZPATH];
        GetModuleFileNameW(NULL, (LPWSTR)wide, SZDISK);
        int num = OSWideUTF8(OS_2UTF8, wide, name, -1, SZPATH);
        if (num <= 0) *name = 0;                //convert from wide chars
        #else                                   //---
        GetModuleFileNameA(NULL, name, SZDISK); //get path and filename
        #endif                                  //---
        ptxt = pbuf + OSFileDir(pbuf, name);    //and then just path part

    } else if ((mode == OS_LOCDOC)||(mode == OS_DOCEXE)) {
        ret = SHGetFolderPathA(0, CSIDL_PERSONAL, 0, 0, pbuf);
        if ((ret)||(pbuf[0] == 0)) {            //Documents folder?
            ret = GetWindowsDirectoryA(name, SZDISK);
            if (ret == 0) OSTxtCopy(name, "C:\\WINDOWS");
            ptxt = pbuf + OSFileRoot(pbuf, name);
            OSTxtCopy(ptxt, "My Documents");
        }
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);
        IS_ENDSLASH(pbuf, ptxt);

    } else if (mode == OS_LOCAPP) {             //Application data folder?
        #ifndef CWINIX                          //---
        ret = SHGetFolderPathA(0, CSIDL_APPDATA, 0, 0, pbuf);
        if ((ret)||(pbuf[0] == 0)) {
            return( OSLocation(pbuf, OS_LOCEXE) );
        }
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);
        IS_ENDSLASH(pbuf, ptxt);

        #else                                   //--- Unix Simulation
        GetModuleFileName(NULL, name, SZDISK);
        ret = OSFileDir(pbuf, name);            //use exe's directory
        ptxt = pbuf + ret;
        #endif                                  //---

    } else if (mode == OS_LOCALL) {             //Application All UsersDdata?
        #ifndef CWINIX                          //---
        ret = SHGetFolderPathA(0, CSIDL_COMMON_APPDATA, 0, 0, pbuf);
        if ((ret)||(pbuf[0] == 0)) {            //ProgramData Vista or later
            return( OSLocation(pbuf, OS_LOCEXE) );
        }
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);
        IS_ENDSLASH(pbuf, ptxt);

        #else                                   //--- Unix Simulation
        GetModuleFileName(NULL, name, SZDISK);
        ret = OSFileDir(pbuf, name);            //use exe's directory
        ptxt = pbuf + ret;
        IS_ENDSLASH(pbuf, ptxt);
        #endif                                  //---

    } else if (mode == OS_LOCTMP) {             //Temporary folder?
        ret = GetTempPathA(SZDISK, pbuf);       //use Window's temp directory
        ptxt = pbuf + ret;
        IS_ENDSLASH(pbuf, ptxt);

    } else if (mode == OS_CURDIR) {             //current directory?
        #ifndef CNWIDE                          //---
        wtxt wide[SZPATH];
        GetCurrentDirectoryW(SZDISK, (LPWSTR)wide);
        int num = OSWideUTF8(OS_2UTF8, wide, name, -1, SZPATH);
        if (num <= 0) *name = 0;                //convert from wide chars
        #else                                   //---
        name[0] = 0;
        GetCurrentDirectoryA(SZDISK, name);
        #endif                                  //---
        ptxt = OSTxtCopy(pbuf, name);
        IS_ENDSLASH(pbuf, ptxt);

    } else if (mode == OS_VERNUM) {             //OS version?
        OSMemClear(&osinfo, sizeof(OSVERSIONINFO));
        osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&osinfo)) {            //get Windows version
            ptxt = OSIntPut(OSN_WINDOWS, pbuf);
            *ptxt++ = '.';
            ptxt = OSIntPut(osinfo.dwMajorVersion, ptxt);
            *ptxt++ = '.';
            ptxt = OSIntPut(osinfo.dwMinorVersion, ptxt);
            *ptxt++ = '.';
            ptxt = OSIntPut(osinfo.dwBuildNumber, ptxt);
        }

    } else if (mode == OS_USER) {               //user name?
        dword siz = SZNAME;
        #ifndef CNWIDE                          //---
        wtxt wide[SZNAME];
        wide[0] = 0;
        ret = GetUserNameW((LPWSTR)wide, &siz);
        if (ret == 0) siz = 0;
        wide[siz] = 0;
        int num = OSWideUTF8(OS_2UTF8, wide, name, -1, SZPATH);
        if (num <= 0) *name = 0;                //convert from wide chars
        #else                                   //---
        name[0] = 0;
        ret = GetUserNameA(name,&siz);
        if (ret == 0) siz = 0;
        name[siz] = 0;
        #endif                                  //---
        ptxt = OSTxtCopy(pbuf, name);

    } else {
        return(pbuf);                           //bad mode argument
    }

    #else                                       //=== Linux
    if (mode == OS_LOCBIN) {                    //OS binaries folder?
        ptxt = OSTxtCopy(pbuf, "/bin/");

    } else if (mode == OS_LOCEXE) {             //Executable path and filename?
        ret = readlink("/proc/self/exe", pbuf, SZDISK-1);
        if (ret < 0) {
            *pbuf = 0;
            return(pbuf);
        }
        pbuf[ret] = 0;
        ptxt = pbuf + (OSTxtSize(pbuf) - 1);

    } else if ((mode == OS_EXEDIR)||(mode == OS_DOCEXE)) {
        ret = readlink("/proc/self/exe", name, SZDISK-1);
        if (ret < 0) {                          //Executable path only?
            *pbuf = 0;                          //get path and filename
            return(pbuf);
        }
        pbuf[ret] = 0;
        ptxt = pbuf + OSFileDir(pbuf, name);    //and then just path part

    } else if ((mode == OS_LOCAPP)||(mode == OS_LOCDOC)||(mode == OS_LOCALL)) {
        //Note: OS_LOCKDOC was formely as follows:
        //For a server daemon /var/opt/<package> is preferred to /usr/doc.
        //ptxt = OSTxtCopy(pbuf, "/usr/doc/");  //first LOCDOC version
        //ptxt = OSTxtCopy(pbuf, "/var/opt/");  //later LOCDOC version
        //But now OS_LOCDOC is same as OS_LOCAPP for Linux.
        ret = readlink("/proc/self/exe", name, SZDISK-1);
        if (ret < 0) {                          //Application data folder? 
            *pbuf = 0;                          //get exe's filename
            return(pbuf);
        }
        name[ret] = 0;
        ret = OSFileDir(pbuf, name);            //use exe's directory
        ptxt = pbuf + ret;

    } else if (mode == OS_LOCTMP) {             //Temporary folder?
        ptxt = OSTxtCopy(pbuf, "/tmp/");

    } else if (mode == OS_VERNUM) {             //OS version?
        puse = OSTxtCopy(name, "uname -r >>");
        ptxt = OSLocation(puse, OS_LOCAPP);
        ptxt = OSTxtCopy(ptxt, "tri_info.tmp");
        OSErase(puse);
        system(name);                           //get version number in file
        ptxt += 1;                              //skip over ending zero
        *ptxt = 0;                              //and start second buffer
        file = OSOpen(OS_READ, puse);           //and then read from file
        if (file != -1) {
            OSRead(file, ptxt, SZNAME);
            OSClose(file);
        }
        OSErase(puse);                          //delete temporary file
        puse = OSIntPut(OSN_LINUX, pbuf);
        *puse++ = '.';
        OSTxtStop(puse, ptxt, SZNAME);

    } else if (mode == OS_CURDIR) {             //current directory?
        name[0] = 0;                            //make relative to current dir?
        getcwd(name, SZDISK);
        ptxt = OSTxtCopy(pbuf, name);
        IS_ENDSLASH(pbuf, ptxt);

    } else {
        return(pbuf);                           //bad mode argument
    }
    #endif                                      //===

    return(ptxt);                               //return ending pointer
}

//*************************************************************************
//  OSDirCur:
//      Sets the current working directory, 
//      which is the default for opening relative files.
//
//  Arguments:
//      int OSDirCur (text* pdir)
//      pdir    Desired new current directory with or without ending slash.
//
//  Returns:
//      Returns TRUE (1) if no error.
//      Returns FALSE (0) if error.
//*************************************************************************

CSTYLE int xproc
OSDirCur (text* pdir) {
    text dir[SZPATH];
    text* ptxt;
    int ok;

    ptxt = OSTxtStop(dir, pdir, SZPATH);        //copy so adding slash is ok
    NO_ENDSLASH(dir, ptxt);                     //get rid of ending backslash

    #ifdef ISWIN                                //=== Windows
    #ifndef CNWIDE                              //---
    wtxt wide[SZPATH];
    int num = OSWideUTF8(OS_2WIDE, wide, dir, -1, SZPATH);
    if (num <= 0) return(FALSE);                //convert to wide chars
    ok = SetCurrentDirectoryW((LPCWSTR)wide);
    #else                                       //---
    ok = SetCurrentDirectoryA(dir);
    #endif                                      //---
    return(ok);
    #else                                       //=== Linux
    ok = chdir(dir);
    return((ok) ? FALSE : TRUE);
    #endif                                      //===
}

//*************************************************************************
//  OSNumGet:
//      Works like OSIntGet or OSHexGet to get a decimal or hex text number.
//      Assumes hex is text begins with 0x or 0X. Assumes int  otherwise.
//      Note that OSHexGet will get "0xHHHHHHHH" or "HHHHHHHH" if always hex.
//      Note this OSNumGet will get "0xHHHHHHHH" or "IIIIIIII" if hex or int.
//
//  Arguments:
//      lint OSNumGet (text* ptext, lptext* ppend)
//      ptext   Pointer to first character of decimal text or 0x hex number.
//      ppend   Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      The signed integer.
//*************************************************************************

CSTYLE lint xproc
OSNumGet (text* ptext, lptext* ppend) {
    while ((*ptext)&&((unsigned)*ptext <= ' ')) {
        ptext += 1;                             //skip beginning blanks, tabs, etc
    }
    if ((*ptext == '0')&&((*(ptext+1) == 'x')||(*(ptext+1) == 'X'))) {
        ptext += 2;                             //hex? (skip over the 0x)
        return(OSHexGet(ptext, ppend));
    } else {                                    //decimal?
        return(OSIntGet(ptext, ppend));
    }
}

//*************************************************************************
//  OSIntPut:
//      Converts a signed four byte integer to decimal text.
//
//  Arguments:
//      text* OSIntPut (lint num, text* ptext)
//      num     Signed number
//      ptext   Pointer to where first character of text should go.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSIntPut (lint num, text* ptext) {
    text buf[32];
    ftext *pbuf;

    if (num < 0) {
         *ptext++ = '-';
         num = -num;
    }

    pbuf = buf;
    do {                                //get digits in reverse order
        *pbuf++ = (text)(num % 10) + '0';
        num /= 10;
    } while (num);

    while (pbuf > buf)                  //reverse and output text digits
        *ptext++ = *(--pbuf);

    *ptext = 0;
    return(ptext);                      //return ending null pointer
}

//*************************************************************************
//  OSIntGet:
//      Converts a signed text string to a four byte integer value.
//
//  Arguments:
//      lint OSIntGet (text* ptext, lptext* ppend)
//      ptext   Pointer to first character of decimal text number.
//      ppend   Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      The signed integer.
//*************************************************************************

CSTYLE lint xproc
OSIntGet (text* ptext, lptext* ppend) {
    lint num;
    uint dig, sign;
    sign = 0;

    while ((*ptext)&&((unsigned)*ptext <= ' '))
        ptext += 1;                     //skip beginning blanks, tabs, etc

    if (*ptext == '-') {
        ptext += 1;
        sign = 1;
    } else if (*ptext == '+') ptext += 1;

    for (num = 0; (unsigned)(dig = *ptext - '0') <= 9; ptext++)
        num = (num*10) + dig;

    if (sign) num = -num;
    if (ppend) *ppend = ptext;
    return(num);                        //non-digit ends number
}

//*************************************************************************
//  OSIntPutQ:
//      Converts a signed qword to decimal text.
//
//  Arguments:
//      text* OSIntPutQ (qint num, text* ptext)
//      num     Signed eight byte number
//      ptext   Pointer to where first character of text should go.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSIntPutQ (qint num, text* ptext) {
    text buf[32];
    ftext *pbuf;

    if (num < 0) {
         *ptext++ = '-';
         num = -num;
    }

    pbuf = buf;
    do {                                //get digits in reverse order
        *pbuf++ = (text)(num % 10) + '0';
        num /= 10;
    } while (num);

    while (pbuf > buf)                  //reverse and output text digits
        *ptext++ = *(--pbuf);

    *ptext = 0;
    return(ptext);                      //return ending null pointer
}

//*************************************************************************
//  OSIntGetQ:
//      Converts a signed text string to an qword integer value.
//
//  Arguments:
//      qint OSIntGetQ (text* ptext, lptext* ppend)
//      ptext   Pointer to first character of decimal text number.
//      ppend   Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      The signed eight-byte integer.
//*************************************************************************

CSTYLE qint xproc
OSIntGetQ (text* ptext, lptext* ppend) {
    qint num;
    uint dig, sign;
    sign = 0;

    while ((*ptext)&&((unsigned)*ptext <= ' '))
        ptext += 1;                 //skip beginning blanks, tabs, etc

    if (*ptext == '-') {
        ptext += 1;
        sign = 1;
    } else if (*ptext == '+') ptext += 1;

    for (num = 0; (dig = *ptext - '0') <= 9; ptext++)
        num = (num*10) + dig;

    if (sign) num = -num;
    if (ppend) *ppend = ptext;
    return(num);                        //non-digit ends number
}

//*************************************************************************
//  OSIntPutU:
//      Converts an unsigned four byte integer to decimal text.
//
//  Arguments:
//      text* OSIntPutU (dword num, text* ptext)
//      num     Unsigned number
//      ptext   Pointer to where first character of text should go.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSIntPutU (dword num, text* ptext) {
    text buf[32];
    ftext *pbuf;

    pbuf = buf;
    do {                                //get digits in reverse order
        *pbuf++ = (text)(num % 10) + '0';
        num /= 10;
    } while (num);

    while (pbuf > buf)                  //reverse and output text digits
        *ptext++ = *(--pbuf);

    *ptext = 0;
    return(ptext);                      //return ending null pointer
}

//*************************************************************************
//  OSHexPut:
//      Converts an unsigned dword integer to hexidecimal text.
//
//  Arguments:
//      text* OSHexPut (dword num, text* ptext)
//      num     Unsigned number
//      ptext   Pointer to where first character of text should go.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSHexPut (dword num, text* ptext) {
    text buf[32];
    ftext *pbuf;
    text dig;

    pbuf = buf;
    do {                                //get digits in reverse order
        dig = (text)(num & 0xF);
        dig += (dig < 0xA) ? '0' : ('A'-10);
        *pbuf++ = dig;
        num = num >> 4;
    } while (num);

    while (pbuf > buf)                  //reverse and output text digits
        *ptext++ = *(--pbuf);

    *ptext = 0;
    return(ptext);                      //return ending null pointer
}

//*************************************************************************
//  OSHexGet:
//      Converts a hexidecimal number text string to an dword integer value.
//
//  Arguments:
//      dword OSHexGet (text* ptext, lptext* ppend)
//      ptext   Pointer to first character of hexidecimal text number.
//      ppend   Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      The integer number.
//*************************************************************************

CSTYLE dword xproc
OSHexGet (text* ptext, lptext* ppend) {
    text c;
    dword hex;

    while ((*ptext)&&((unsigned)*ptext <= ' '))
        ptext += 1;                     //skip beginning blanks, tabs, etc

    if ((*ptext == '0')&&(*(ptext+1) == 'x'))
        ptext += 2;                     //skip beginning 0x for hex if present

    hex = 0;
    while (TRUE) {
        c = *ptext;
        if (c < '0') break;
        if (c <= '9') {
            hex = (hex << 4) + (dword)(c - '0');
            ptext += 1;
            continue;
        }
        c = CAPITAL(c);
        if (c < 'A') break;
        if (c <= 'F') {
            hex = (hex << 4) + (dword)(c - ('A' - 0xA));
            ptext += 1;
            continue;
        }
        break;
    }
    if (ppend) *ppend = ptext;
    return(hex);
}

//*************************************************************************
//  OSHexPutQ:
//      Converts an unsigned eight byte qword to hexidecimal text.
//
//  Arguments:
//      text* OSHexPutQ (qword num, text* ptext)
//      num     Unsigned number
//      ptext   Pointer to where first character of text should go.
//
//  Returns:
//      Pointer to the terminating zero character placed at end of text.
//*************************************************************************

CSTYLE text* xproc
OSHexPutQ (qword num, text* ptext) {
    text buf[32];
    ftext *pbuf;
    text dig;

    pbuf = buf;
    do {                                //get digits in reverse order
        dig = (text)(num & 0xF);
        dig += (dig < 0xA) ? '0' : ('A'-10);
        *pbuf++ = dig;
        num = num >> 4;
    } while (num);

    while (pbuf > buf)                  //reverse and output text digits
        *ptext++ = *(--pbuf);

    *ptext = 0;
    return(ptext);                      //return ending null pointer
}

//*************************************************************************
//  OSHexGetQ:
//      Converts a hexidecimal number text string to an eight byte qword value.
//
//  Arguments:
//      qword OSHexGetQ (text* ptext, lptext* ppend)
//      ptext   Pointer to first character of hexidecimal text number.
//      ppend   Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      The eight byte qword integer number.
//*************************************************************************

CSTYLE qword xproc
OSHexGetQ (text* ptext, lptext* ppend) {
    text c;
    qword hex;

    hex = 0;
    while (TRUE) {
        c = *ptext;
        if (c < '0') break;
        if (c <= '9') {
            hex = (hex << 4) + (dword)(c - '0');
            ptext += 1;
            continue;
        }
        c = CAPITAL(c);
        if (c < 'A') break;
        if (c <= 'F') {
            hex = (hex << 4) + (dword)(c - ('A' - 0xA));
            ptext += 1;
            continue;
        }
        break;
    }
    if (ppend) *ppend = ptext;
    return(hex);
}

// *******
// This is the decimal point symbol (may be set to comma for Europe).
// Also other localized information.

text DecimalChar = '.';             //Decimal point

// Table of powers of ten from 1/10e10 through 10e10.
// To get 10^Exp, use eten[Exp+ETEN0].

#define ETENMIN -10                 //Minimum exponent of ten in table
#define ETENMAX +10                 //Maximum exponent of ten in table
#define ETEN0   10                  //Index in eten[] to 10^0

flx eten[21]=
 {         0.0000000001,            // 1/10000000000  10e-10  Index 0
           0.000000001 ,            // 1/1000000000   10e-9
           0.00000001  ,            // 1/100000000    10e-8
           0.0000001   ,            // 1/10000000     10e-7
           0.000001    ,            // 1/1000000      10e-6
           0.00001     ,            // 1/100000       10e-5
           0.0001      ,            // 1/10000        10e-4
           0.001       ,            // 1/1000         10e-3
           0.01        ,            // 1/100          10e-2
           0.1         ,            // 1/10           10e-1
           1.0         ,            // 1 & 1/1        10e+0   Index 10
          10.0         ,            // 10             10e+1
         100.0         ,            // 100            10e+2
        1000.0         ,            // 1000           10e+3
       10000.0         ,            // 10000          10e+4
      100000.0         ,            // 100000         10e+5
     1000000.0         ,            // 1000000        10e+6
    10000000.0         ,            // 10000000       10e+7
   100000000.0         ,            // 100000000      10e+8
  1000000000.0         ,            // 1000000000     10e+9
 10000000000.0         };           // 10000000000    10e+10  Index 20

// Table of roundup terms.

flx roundup[21] =
 { 0.,                              //first entry not used
   0.5,
   0.05,
   0.005,
   0.0005,
   0.00005,
   0.000005,
   0.0000005,
   0.00000005,
   0.000000005,
   0.0000000005,
   0.00000000005,
   0.000000000005,
   0.0000000000005,
   0.00000000000005,
   0.000000000000005,
   0.0000000000000005,
   0.00000000000000005,
   0.000000000000000005,
   0.0000000000000000005,
   0.00000000000000000005 };

//*************************************************************************
//  OSFloatPut:
//      Converts a floating number to text.
//
//      Normally (when width and sigs arguments are zero or positive),
//      outputs the number left justified in the buffer with a leading negative
//      sign if needed and enough digits after the decimal point to satisfy the
//      sigs argument.  But ignores sigs if more digits are required before the
//      decimal point. For example, if sigs is 6 then 1.2345678 would output
//      as "1.23457" but -12345678 would output as "-12345678".  Automatically
//      elimiates ending 0 digits after the decimal point.  Also elimiates
//      the decimal point if possible.  For example, if sigs is 6, 1.2 would
//      output as "1.2" and 1.0" as "1".  Does not fill in the complete
//      available buffer given by the width argument unless it must and
//      returns a pointer to an ending null after the last digit (or decimal
//      point) without adding any padding blanks.  Thus, the width is used
//      only to determine whether the number will fit in the available buffer.
//      if it will not fit, scientific notation is automatically used.
//
//      Scientfic notation is always used when the width argument is -1.
//      This is always in the form "<mantissa>E<exponent>".
//
//      When the sigs argument is less than zero then outputs the specified
//      number of digits after the decimal point and leaves ending zero digits.
//      Unless width is -2, it right justifies in the buffer, adding preceeding
//      blanks so that the decimal point is always in the same place.  For
//      example if sigs is -3 and width is 10, then 1.234567, -123, and 12.3
//      would output as "     1.234" and "  -123.000" and "    12.300".
//      But if width is -2 then left justifies the number and does not add
//      padding blanks afterward so that the last example would output as
//      "1.234" and "-123.000" and "12.300".
//
//  Arguments:
//      text* OSFloatPut (flx num, text* buf, int width, int sigs)
//      num     Floating point number (positive or negative).
//      buf     Pointer to where the number text is to be placed.
//              It must have at least width+1 text characters available
//              since a terminating null is placed at the end.
//      width   Character width of buf field.  If zero then 10 is used.
//              Also, if width is zero, very small numbers are forced to 0.
//              If width is -1 then scientific notation is always used.
//              If width is -2 then sigs must be negative as described below.
//              Use OS_FLXWIDE for suggested flx 64-bit 18 chars wide.
//      sigs    Number of significant digits to show.  If 0 then uses 6.
//              For example 1.234567 shows as "1.23457" if sigs is 6.
//              If negative, then absolute value is digits after the decimal
//              point and the number is right justified in the width field.
//              Note that the number is automatically rounded up below
//              the lowest significant digit to right of decimal point.
//              If sigs is negative and width is -2, then the absolute
//              value of sigs gives the number of digits after the decimal
//              point, but the number is left justified with at least a
//              leading 0 before the decimal place and the width field
//              is assumed to be at least 32 characters (SZNAME) wide.
//              Use OS_FLXSIGS for suggested flx 64-bit 14 digits.
//
//  Returns:
//      Pointer to the terminating null placed at the end of the buffer.
//*************************************************************************

CSTYLE text* xproc
OSFloatPut (flx num, text* buf, int width, int sigs) {
    int exp, sign, mins, cnt;
    uint dig;

    if (num != num) {                           //NaN?
        buf = OSTxtCopy(buf, "1.#IND");         //show indeterminant number
        while (width-- > 6) *buf++ = ' ';
        return(buf);
    }
    sign = 0;
    if (num < 0) {                              //deal with unsigned number
        num = -num;
        sign = 1;
    }
    if (width == 0) {                           //use default width?
        width = 10;                             //if using default width,
        if (num < 0.0000001) {                  //make very small into zero
            num = 0;
            sign = 0;
        }
    }
    if (sigs == 0) sigs = 6;                    //use default significant digs?

// Normalize the number into scientific notation format: num * 10^(exp-1)
// Leaves cnt set to -999 only if do not want to blank ending zeroes.

    exp = OSFloatOneTen(&num) + 1;

    if (sigs < 0) {                             //sigs<0 for fraction digits
        sigs = -sigs;                           //which are right justified
        if (width == -2) {                      //left justified w/ leading 0?
            if (exp <= 0) {
                if (exp > -10) {
                    num *= eten[(ETEN0-1)+exp];
                    exp = 1;                    //at least a zero before decimal
                } else {
                    num *= eten[ETEN0-10];
                    exp += 10;
                    while (exp <= 0) {
                        num *= 0.1;
                        exp += 1;
                    }
                }
            }
            sigs += exp;
            if (sigs<21) num += roundup[sigs];  //round up below lowest sig dig
            if (num >= 10.0) {                  //make sure roundup did
                num *= 0.1;                     //not push 9.9 to 10.0
                exp += 1;
            }
            width = SZNAME;
            cnt = -999;                         //do not blank ending zeroes
        } else {                                //rigtht justified?
            sigs += exp;
            if (sigs <= 0) width = -1;              //force scientific notation
            cnt = width - (sign + sigs + 1);
            if (sigs < 21) num += roundup[sigs];    //round up below lowest sig dig
            if (num >= 10.0) {                      //make sure roundup did
                num *= 0.1;                         //not push 9.9 to 10.0
                exp += 1;
                sigs += 1;
                if (cnt) cnt -= 1;
            }
            while (cnt-- > 0) *buf++ = ' ';         //left padding for right justify
        }
    } else {
        cnt = sigs;
        if (exp > cnt) cnt = exp;               //don't round up integers
        if (cnt < 21) num += roundup[cnt];      //round up below lowest sig dig
        if (num >= 10.0) {                      //make sure roundup did
            num *= 0.1;                         //not push 9.9 to 10.0
            exp += 1;
        }
    }

// Handle scientific notation formatted text.

    if (  ((exp + sign) > width)                //need scientific notation?
        ||((1 + sign + sigs - exp) > width)  ) {
        if (width <= 0) width = 9 + sign;
        mins = width - (sign + 6);              //leave room for sign & ".E+xxx"
        if (mins < sigs) sigs = mins;
        if (sigs < 2) {                         //if will not fit in width
            while (width--) *buf++ = '*';       //print '****'
            *buf = 0;
            return(buf);
        }
        if (sign) *buf++ = '-';                 //put scientific notation
        dig = (uint)num;                        //[-]n.nnE-ee
        *buf++ = '0' + dig;
        num = (num - dig) * 10;
        *buf++ = DecimalChar;
        while (--sigs) {
            dig = (uint)num;
            *buf++ = '0' + dig;
            num = (num - dig) * 10;
        }
        if (cnt != -999) {
            buf -= 1;
            while (*buf == '0') buf -= 1;       //omit ending 0 decimals
            if (*buf == DecimalChar) *(++buf) = '0';
            buf += 1;                           //except for first
        }
        *buf++ = 'E';
        exp -= 1;
        if (exp < 0) {
            exp = -exp;
            *buf++ = '-';
        } else *buf++ = '+';

        if (exp > 100) {
             dig = exp / 100;
             *buf++ = '0' + dig;
             exp -= (dig*100);
        }
        dig = exp/10;
        *buf++ = '0' + dig;
        dig = exp - (dig*10);
        *buf++ = '0' + dig;
        *buf = 0;
        return(buf);
    }

// Handle normal numbers.

    if (sign) *buf++ = '-';                     //put sign if negative
    while (exp > 0) {                           //put digits before decimal
        exp -= 1;
        sigs -= 1;
        dig = (uint)num;
        *buf++ = '0' + dig;
        num = (num - dig) * 10;
    }
    if (sigs > 0) {
        *buf++ = DecimalChar;                   //put decimal point
        while (exp < 0) {                       //put zeros after decimal
            exp += 1;                           //and before significant digits
            *buf++ = '0';
        }
        while (sigs--) {                        //put digits after decimal
            dig = (uint)num;
            *buf++ = '0' + dig;
            num = (num - dig) * 10;
        }
        if (cnt != -999) {
            buf -= 1;
            while (*buf == '0') *buf-- = ' ';   //blank ending 0 decimals
            if (*buf == DecimalChar) *buf-- = ' ';  //blank ending . too
            buf += 1;
        }
    }
    *buf = 0;                                   //put terminating null
    return(buf);
}

//*************************************************************************
//  OSFloatGet:
//      Converts text to a floating point number.
//      Handles handles scientific notation: 1.23E10, 1.23e-1, etc.
//      There may be no blanks between the mantissa an the exponent E.
//      Stops with first char other than digit, decimal point, +, -, E, or e.
//
//  Arguments:
//      flx OSFloatGet (text* buf, lptext* end)
//      buf     Pointer to text number.  Skips blanks and tabs before number.
//      end     Pointer to a pointer to where the character after the
//              end of the number is returned.  Can be NULL if not needed.
//
//  Returns:
//      Floating point number represented by text.
//*************************************************************************

CSTYLE flx xproc
OSFloatGet (text* buf, lptext* end) {
    flx num;
    boolv sign, esign;
    int decs, exp;

    num = 0;                                        //accumulate number
    decs = -999;                                    //assume no decimal point
    sign = 0;
    while ((*buf == ' ')||(*buf == 9)) buf += 1;    //skip blanks and tabs
    if (*buf == '-') {                              //notice sign
        sign = 1;
        buf += 1;
    } else if (*buf == '+') buf += 1;               //ignore plus sign
    while (TRUE) {                                  //go through number text
        if (*buf == DecimalChar) {                  //notice decimal point
            decs = 0;                               //count digits after decimal
            buf += 1;
        }

        else if ((*buf == 'e')||(*buf == 'E')) {    //handle scientific notation
            while (decs > 0) {                      //finish mantissa number
                num *= 0.1;
                decs -= 1;
            }
            if (sign) num = -num;                   //signed mantissa?
            buf += 1;
            exp = 0;                                //get integer exponent
            esign = 0;
            if (*buf == '-') {
                esign = 1;
                buf += 1;
            } else if (*buf == '+') buf += 1;

            while ((*buf >= '0')&&(*buf <= '9')) {
                exp = exp*10;
                exp = exp + (*buf - '0');
                buf += 1;
            }
            if (esign) {                            //raise number to exponent
                while (exp--) num *= 0.1;
            } else {
                while (exp--) num *= 10;
            }
            if (end) *end = buf;                    //return ending pointer
            return(num);                            //return scientific number
        }

        else if ((*buf >= '0')&&(*buf <= '9')) {
            num *= 10;
            num = num + (*buf - '0');
            decs += 1;
            buf += 1;
        }

        else break;
    }
    while (decs > 0) {                              //was there a decimal point?
        num *= 0.1;                                 //divide by (10^decs)
        decs -= 1;
    }
    if (sign) num = -num;
    if (end) *end = buf;                            //return ending pointer
    return (num);                                   //return floating number
}

//*************************************************************************
//  OSFloatOneTen:
//      Converts a floating point number into another floating point
//      number which is guaranteed to be less than 10 and greater than
//      or equal to 1 plus an exponent of 10.
//
//  Arguments:
//      int OSFloatOneTen (flx* num)
//      num     Pointer to source floating number (which must be positive),
//              and also where the result number between one and ten is put.
//
//  Returns:
//      Exponent of 10.  Result num * 10^exponent is equal to source number.
//*************************************************************************

static int OSFloatOneTen (flx* num) {
    dword msd, lsd;
    flx temp;
    fdword* pd;
    int exp;

// We treat the 64-bit floating number as two 32-bit integers
// which are the high and low bits of the IEEE floating representation.
// This allows us to do floating comparisons with fast integer arithmetic.
// Positive IEEE numbers can be compared as 64-bit unsigned integers.
// Greater positive IEEE floating numbers are always greater unsigned
// integers.  This is because the IEEE standard forces the mantissa
// to be left justified so that its high bit is only implied.
// Beware that this code assumes Intel word ordering so the LSDWord is first!

    pd = (fdword*) num;
    lsd = *pd;                          //get LSDWord of float number
    msd = *(pd+1);                      //get MSDWord
//  if (msd & ESIGN) {                  //enable this code to allow negatives
//      msd &= ~ESIGN;                  //and use absolute value
//      *num -= *num;
//  }
    if ((!(msd & (~ESIGN)))&&(lsd == 0))//zero floating number is an exception
        return(0);                      //return 0*(10^0)

    pd = (fdword*) &eten[ETEN0];
    pd += 1;                            //point to high word of 10^0 in table
    exp = 0;

    if ((msd > *pd) || ((msd == *pd)&&(lsd >= *(pd-1)))) {
        pd += 2;                        //handle numbers >= 1.0
        while ((msd > *pd) || ((msd == *pd)&&(lsd >= *(pd-1)))) {
            exp += 1;
            if (exp == ETENMAX) {       //handle stuff bigger than table
                temp = eten[ETENMAX+ETEN0] * 10;
                pd = ((fdword*) &temp) + 1;
                while ((msd > *pd) || ((msd == *pd)&&(lsd >= *(pd-1)))) {
                    exp += 1;
                    temp *= 10;
                }
                *num /= (temp * 0.1);
                return(exp);
            }
            pd += 2;
        }
        *num *= eten[ETEN0-exp];
        return(exp);

    } else {                            //handle numbers < 1.0
        exp -= 1;
        pd -= 2;
        while ((msd < *pd) || ((msd == *pd)&&(lsd < *(pd-1)))) {
            exp -= 1;
            if (exp < ETENMIN) {        //handle stuff smaller than table
                temp = eten[0] * 0.1;
                pd = ((fdword*) &temp) + 1;
                while ((msd < *pd) || ((msd == *pd)&&(lsd < *(pd-1)))) {
                    exp -= 1;
                    temp *= 0.1;
                }
                *num /= (temp * 10);
                return(exp);
            }
            pd -= 2;
        }
        *num *= eten[ETEN0-exp];
        return(exp);
    }
}

//*************************************************************************
//  OSFloatEven:
//      Determines an even increment which will divide two end
//      points so that evenly labeled tic marks can be used.
//
//  Arguments:
//      void OSFloatEven (flx begin,flx delta,int tics,fflx* pfirst,fflx* pinc)
//      begin   Beginning coordinate for axis.
//      delta   Delta coordinates to end of axis.
//      tics    Desired number of ticks.
//              If tics < 0, uses -tics and scales for log with increment of 1.
//      pfirst  First axis tic coordinate returned here (first "even" number).
//      pinc    Increment between "even" axis tic coordinates returned here.
//
//  Returns:
//      Nothing (except pfirst and pinc are fill in).
//*************************************************************************

CSTYLE void xproc
OSFloatEven (flx begin, flx delta, int tics, fflx* pfirst, fflx* pinc) {
    flx inc, first;
    int exp, i;
    boolv neg, log;

    log = 0;
    if (tics < 0) {
        log = 1;
        tics = -tics;
    }
    inc = delta / tics;                 //get desired increment between ticks
    neg = 0;
    if (inc < 0) {
        inc = -inc;
        neg = 1;
    }
    inc += inc * 0.0000000001;          //add a small amount to avoid truncate
    exp = OSFloatOneTen(&inc);          //get 1 <= inc < 10
    i = (int) inc;                      //truncate to integer portion of inc
    if      (i >= 5) inc = 10;          //get "even" increment
    else if (i >= 2) inc = 5;
    else             inc = 2;

    while (exp < ETENMIN) {
        inc *= 0.1;
        exp += 1;
    }
    while (exp > ETENMAX) {
        inc *= 10;
        exp -= 1;
    }
    inc *= (flx) eten[ETEN0 + exp];     //raise inc to exp
    if ((log)&&(inc < 1)) inc = 1;      //log scale must advance by 1 to be even
    if (neg) inc = -inc;

    first = begin / inc;
    i = (int) first;                    //even number of incs to begin label
    if ((first > 0)&&(first != (flx)i)) i += 1;

    *pinc = inc;                        //return increment between labels
    *pfirst = inc * (flx)i;             //and first label to use
    return;
}

//*************************************************************************
//  OSSqrt:
//      Returns the square root of a floating value.
//
//  Arguments:
//      flx OSSqrt (flx value)
//      value   Floating value whose square root is desired.
//              Negative values are not allowed and may crash.
//
//  Returns:
//      Square root of value.
//*************************************************************************

CSTYLE flx xproc
OSSqrt (flx value) {
    #ifdef NOI86                                //-------
    return( sqrt(value) );                      //C runtime (LIBC.LIB)
    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    return( sqrt(value) );                      //WIN64 omits inline assembly!
    #else                                       //---
    ASM {
        fld     value
        fsqrt
        fstp    value
        fwait
    }
    #endif                                      //---
    #else                                       //===
    asm ( "                                     \
        fldl    %1                          ;   \
        fsqrt                               ;   \
        fstpl   %0                          ;   \
        fwait                               ;   \
          " : "=m"(value)
            : "m"(value)
    );
    #endif                                      //===
    return(value);
    #endif      //-------
}

//*************************************************************************
//  OSSin:
//      Returns the sine of a floating radian value.
//      Note: The tangent can be computed as sin/cos.
//
//  Arguments:
//      flx OSSin (flx value)
//      value   Floating radian value whose sine is desired.
//              There are 2*Pi radians per 360 degrees.
//
//  Returns:
//      Sine of value.
//*************************************************************************

CSTYLE flx xproc
OSSin (flx value) {
    #ifdef NOI86                                //-------
    return( sin(value) );                       //C runtime (LIBC.LIB)
    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    return( sin(value) );                       //WIN64 omits inline assembly!
    #else                                       //---
    ASM {
        fld     value
        fsincos
        fstp    ST
        fstp    value
        fwait
    }
    #endif                                      //---
    #else                                       //===
    asm ( "                                     \
        fldl    %1                          ;   \
        fsincos                             ;   \
        fstp    %%ST                        ;   \
        fstpl   %0                          ;   \
        fwait                               ;   \
          " : "=m"(value)
            : "m"(value)
    );
    #endif                                      //===
    return(value);
    #endif                                      //-------
}

//*************************************************************************
//  OSCos:
//      Returns the cosine of a floating radian value.
//      Note: The tangent can be computed as sin/cos.
//
//  Arguments:
//      flx OSCos (flx value)
//      value   Floating radian value whose cosine is desired.
//              There are 2*Pi radians per 360 degrees.
//
//  Returns:
//      Cosine of value.
//*************************************************************************

CSTYLE flx xproc
OSCos (flx value) {
    #ifdef NOI86                                //-------
    return( cos(value) );                       //C runtime (LIBC.LIB)
    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    return( cos(value) );                       //WIN64 omits inline assembly!
    #else                                       //---
    ASM {
        fld     value
        fsincos
        fstp    value
        fstp    ST
    }
    #endif                                      //---
    #else                                       //===
    asm ( "                                     \
        fldl    %1                          ;   \
        fsincos                             ;   \
        fstpl   %0                          ;   \
        fstp    %%ST                        ;   \
        fwait                               ;   \
          " : "=m"(value)
            : "m"(value)
    );
    #endif                                      //===
    return(value);
    #endif                                      //-------
}

//*************************************************************************
//  OSArcTan:
//      Returns the inverse tangent of a floating value in radians.
//      Note that since:
//          tan = sin/cos
//          cos = sqrt(1 - sin^2)
//          sin = sqrt(1 - cos^2)
//          tan = sin / sqrt(1 - sin^2)
//          tan = sqrt(1 - cos^2) / cos
//      The inverse sine or cosine can be computed using this function:
//          arcsin(val) = arctan( val / sqrt(1 - val*val) )
//          arccos(val) = arctan( sqrt(1 - val*val) / val )
//
//  Arguments:
//      flx OSArcTan (flx value)
//      value   Floating value whose arc tangent is desired.
//
//  Returns:
//      Inverse tangent of value in radians.
//      There are 2*Pi radians per 360 degrees.
//*************************************************************************

CSTYLE flx xproc
OSArcTan (flx value) {
    #ifdef NOI86                                //-------
    return( atan(value) );                      //C runtime (LIBC.LIB)
    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    return( atan(value) );                      //WIN64 omits inline assembly!
    #else                                       //---
    word optemp1, optemp2;

    ASM {
        fld     value
        ftst
        fstsw   optemp2
        fwait
        test    optemp2,100h            ;CODE0
        jz      nnatop                  ;negative operand?
        fchs                            ;negative => make positive
nnatop: fld1
        fcomp   ST(1)                   ;compare with 1
        fstsw   optemp1
        fwait
        xor     eax,eax                 ;assume result between 0 and Pi/4
        test    optemp1,100h            ;CODE0
        jz      natg1                   ;less than 1?
        fld1
        fdivr
        not     eax
natg1:  fld1
        fpatan
        test    eax,eax
        jz      notao2
        fld1
        fchs
        fldpi
        fscale                          ;get Pi/2
        fxch    ST(1)
        fstp    ST                      ;pop the -1
        fsubr                           ;get Pi/2 - value
notao2: test    optemp2,100h            ;CODE0
        jz      atpos
        fchs
atpos:  fstp    value
        fwait
    }
    #endif                                      //---
    #else                                       //===
    word optemp1, optemp2;

    asm ( "                                     \
        fldl    %3                      ;       \
        ftst                            ;       \
        fstsw   %2                      ;       \
        fwait                           ;       \
        testw   $0x100, %5              ;       \
        jz      nnatop                  ;       \
        fchs                            ;       \
nnatop: fld1                            ;       \
        fcomp   %%ST(1)                 ;       \
        fstsw   %1                      ;       \
        fwait                           ;       \
        xor     %%eax, %%eax            ;       \
        testw   $0x100, %4              ;       \
        jz      natg1                   ;       \
        fld1                            ;       \
        fdivrp                          ;       \
        not     %%eax                   ;       \
natg1:  fld1                            ;       \
        fpatan                          ;       \
        test    %%eax, %%eax            ;       \
        jz      notao2                  ;       \
        fld1                            ;       \
        fchs                            ;       \
        fldpi                           ;       \
        fscale                          ;       \
        fxch    %%ST(1)                 ;       \
        fstp    %%ST                    ;       \
        fsubrp                          ;       \
notao2: testw   $0x100, %5              ;       \
        jz      atpos                   ;       \
        fchs                            ;       \
atpos:  fstpl   %0                      ;       \
        fwait                           ;       \
          " : "=m"(value), "=m"(optemp1), "=m"(optemp2)
            : "m"(value), "m"(optemp1), "m"(optemp2)
            : "%eax"
    );
    #endif                                      //===
    return(value);
    #endif      //-------
}

//*************************************************************************
//  OSArcTan2:
//      Returns the inverse tangent of floating X and Y velues in radians.
//
//  Arguments:
//      flx OSArcTan2 (flx yval, flx xval)
//      yval    Y direction measurement
//      xval    X direction measurement
//
//  Returns:
//      Inverse tangent of value in radians for X,Y vector from origin.
//      0 is right (Y=0,X=1), Pi/2 is up (Y=1,X=0), -Pi/2 is down (Y=-1,X=0).
//      Cannot handle yval=xval=0 case and returns Pi/2
//      (90 degrees up) instead of undefined NaN value.
//      There are 2*Pi radians per 360 degrees.
//*************************************************************************

CSTYLE flx xproc
OSArcTan2 (flx yval, flx xval) {
    #ifdef NOI86                                //-------
    if (FLXZERO(xval) && FLXZERO(yval)) {       //punt for undefined result
        return( 3.1415926535897932384626433832795 / 2.0);
    }
    return( atan2(yval, xval) );                //C runtime (LIBC.LIB)
    #endif                                      //-------
    #ifdef ISX64                                //---
    if (FLXZERO(xval) && FLXZERO(yval)) {
        return( 3.1415926535897932384626433832795 / 2.0);
    }                                           //punt for undefined result
    return( atan2(yval, xval) );                //WIN64 omits inline assembly!
    #endif                                      //---

    if (FLXMINUS(xval)) {                       //xval < 0?
        if (FLXMINUS(yval)) {                   //arctan(y/x) - Pi if Y < 0
            return( OSArcTan(yval/xval) - 3.1415926535897932384626433832795 );
        } else {                                //arctan(y/x) + Pi if Y >= 0
            return( OSArcTan(yval/xval) + 3.1415926535897932384626433832795 );
        }
    }
    if (FLXZERO(xval)) {                        //xval == 0?
        if (FLXMINUS(yval)) {                   //negative Pi/2 if Y < 0
            return(-3.1415926535897932384626433832795 / 2.0);
        } else {                                //positive Pi/2 if Y >= 0
            return( 3.1415926535897932384626433832795 / 2.0);
        }                                       //(ignore undefined if X=0,Y=0)
    }
    return( OSArcTan(yval/xval) );              //xval > 0?
}                                               //arctan works -Pi/2 to +Pi/2

//*************************************************************************
//  OSInterp:
//      Interpolates an X,Y or evenly-spaced Y-only spectrum
//      to a given evenly spaced Y-only spectrum with arbitrary begX,endX,npts.
//      Goes through target spectrum and uses OSSpline below for each point.
//
//  Arguments:
//      int OSInterp (int mode, flx* Y2, int npt2, flx beg2, flx end2,
//                    flx beg, flx end, flx* X, flx* Y, int npts)
//      mode    0 = Linear interpolation
//              1 = Spline interpolation
//             +8 = Do not assume zero values one point outside source X limits
//                  Does linear interpolation with last two points beyond limits
//                  Without +8 interpolates to 0 one point beyond limits
//                  and outputs zero values further beyond the limits
//      Y2      Buffer to receive interpolated output spectrum.
//              Must have space for npt2 double values.
//      npt2    Number of points in Y2 interpolated output spectrum.
//      beg2    Begin X coordinate for Y2 interpolated output spectrum.
//      end2    End X coordinate for Y2 interpolates output spectrum.
//      beg     Begin X coordinate for source Y array if X is NULL.
//      end     End X coordinate for source Y array if X is NULL.
//              The end coordinate must be greater than beg.
//      X       X coordinates for source X,Y spectrum or NULL for Y-only.
//              The X coordinates must be monitonically increasing.
//      Y       Y intensities for X,Y or Y-only source spectrum.
//      npts    Number of points in source X and Y arrays or Y-only array.
//              Must be at least 2 for linear or 4 for spline.
//
//  Returns:
//      Returns 0 or a negative error code.
//*************************************************************************
    
CSTYLE int xproc
OSInterp (int mode, flx* Y2, int npt2, flx beg2, flx end2,
          flx beg, flx end, flx* X, flx* Y, int npts) {
    double delta, xval, xend, xfac;
    int ival;

    if (beg2 >= end2) return(ECBADPEAK);        //must be ascending X
    ival = (mode & 1) ? 4 : 2;                  //minimum npts
    if ((npt2 < ival)||(npts < ival)) return(ECFEWPTS);

    if (X) {                                    //X,Y?
        xval = beg2;                            //start with first output X
        delta = (end2 - beg2)/(npt2 - 1);       //delta X per output point

    } else {                                    //Y-only?
        if (beg >= end) return(ECBADNPTS);      //must be ascending X
        xfac = (npts - 1) / (end - beg);        //points per X in source
        xval = (beg2 - beg) * xfac;             //starting point on source array
        xend = (end2 - beg) * xfac;             //ending point on source array
        delta = (xend - xval) / (npt2 - 1);     //delta source point per output
    }
    do {                                        //go through output points
        *Y2++ = OSSpline(mode, xval, Y, X, npts);
        xval += delta;                          //interpolate Y value for each
    } while (--npt2);
    return(0);                                  //all done
}

//*************************************************************************
//  OSSpline:
//      Calculates a four-point spline interpolated value for one coordinate.
//      Can optionally use less computationally intensive linear interpolation.
//      The output value may be Y in an X,Y plot,
//      or may be X for an X axis equi-width transformation.
//
//      Handles any xval input, even outside the given X,Y or Y-only points.
//      However, uses linear interpolation outside the supplied ponit range.
//
//      Handles any non-zero number of X,Y or Y-only points.
//      For npts=0 always returns a zero value.
//      For npts=1 always returns the single pV[0] value.
//      For npts=2 or npts=3 does linear interpolation.
//      For npts>3 does four-point spline between 2nd and 2nd-to-last points.
//      For npts>3 does linear interpolation before 2nd and after 2nd-to-last.
//
//      Spline Algorithm: Calculate interpolation polynomial coefficients.
//      We derive a different cubic polynomial for each region between 2 points.
//      We assume that X=0 at the left point of the region & X=1 at the right.
//      We call y1 and y2 the Y values of the two points, between which we must
//      interpolate.  Also, y0 and y2 are the points just before  and after.
//
//      We assume that the slope at a point is the average of the slope between
//      it and the previous point and the slope between the next point and
//      itself. For equally spaced, these are slopes at left and right edges:
//          s1 = (y2-y0)/2
//          s2 = (y3-y1)/2
//
//      For X,Y data derive equivalent equally spaced Y0,Y3 on fly from slopes.
//          s1 = (((v1-v0)/(x1-x0)) + (y2-y1)/(x2-x1)) / 2
//          Y0 = y1 - s1*(x2-x1)
//          s2 = (((v2-v1)/(x2-x1)) + (y3-y2)/(x3-x2)) / 2
//          Y3 = y2 + s2*(x2-x1)
//
//      Remember that for each point our quadratic function is:
//          Y = f(x) = a*x^3 + b*x^2 + c*x + d
//
//      Calculus provides the slope function:
//          df/dx = 3*a*x^2 + 2*b*x + c
//
//      In solving for a,b,c,d we use four constraints.
//      The begin and end of function must intersect given pV values: y1,y2.
//      The begin and end slopes must be the s1,s2 average adjoining slopes.
//      At beginning x=0, and x=1 at end, which simplifies above functions.
//          f[0] = y1  =  d
//          f[1] = y2  =  a + b + c + d  =  a + b + c + y1
//          (df/dx)[0] =  s1  =  c
//          (df/dx)[1] =  s2  =  3*a + 2*b + c  = 3*a + 2*b + s1
//
//      Solving, we have:
//          a = 2*y1 - 2*y2 + s1 + s2
//          b = 3*y2 - 3*y1 - 2*s1 - s2
//          c = s1
//          d = y1
//
//      We return the interpolated value:
//          Y = a*X^3 + b*X^2 + c*X + d
//          Where X is the output point's 0-1 fraction between y1 and y2.
//
//  Arguments:
//      flx OSSpline (int mode, flx xval, flx* pV, flx* pX, int npts)
//      mode    0 = Linear interpolation
//              1 = Spline interpolation
//             +8 = Do not assume zero values one point outside of X limits
//      xval    X coordinate whose pV X or Y value is to be output.
//              If pX is NULL then this is a float array index (0=1st).
//      pV      Input Y or transformed X values between which we interpolate.
//      pX      Input X values for pV values or NULL if pV is equally spaced.
//              The xval gives the desired interpolation point in X,Y array.
//              For equally spaced pV values, xval must be point index (0=1st).
//      npts    Number of pV and pX input values.
//
//  Returns:
//      Spline interpolated pV value at given xval coordinate.
//*************************************************************************
    
CSTYLE flx xproc
OSSpline (int mode, flx xval, flx* pV, flx* pX, int npts) {
    flx Y0,Y1,Y2,Y3, YY,XX, X2,X3, S1,S2,S3, AA,BB, dX;
    int iv, iend, iem1;

    if (npts < 1) return(0);                    //no points assumes 0 output
    if (npts < 2) return(pV[0]);                //one point assumes flat value
    iend = npts - 1;
    iem1 = npts - 2;                            //index to last point
                                                //index to ponit before last
    // Handle X,Y data.

    if (pX) {                                   //X,Y?
        if (xval < pX[1]) {                     //inside or before first segment?
            dX = pX[1] - pX[0];                 //(no previous slope for spline)
            if (dX <= 0) dX= 1.0;
            if ((xval < pX[0])&&(!(mode & 8))) {//assume zero outside of limits?
                X2 = pX[0] - dX;
                if (xval < X2) return(0);       //zero one or more points outside
                YY = pV[0]*((xval - X2) / dX);
                return(YY);                     //interpolate to 0 less than that
            }
            YY = pV[0] + ((pV[1] - pV[0])*((xval - pX[0]) / dX));
            return(YY);                         //linear interpolate 1st seg
        }
        if (xval >= pX[iem1]) {                 //inside or after last segment?
            dX = pX[iend] - pX[iem1];           //(no next slope for spline)
            if (dX <= 0) dX= 1.0;
            if ((xval > pX[iend])&&(!(mode & 8))) {
                X2 = pX[iend] + dX;             //assume zero outside of limits?
                if (xval > X2) return(0);       //zero one or more points outside
                YY = pV[iend]*((X2 - xval) / dX);
                return(YY);                     //interpolate to 0 less than that
            }
            YY = pV[iem1] + ((pV[iend] - pV[iem1])*((xval - pX[iem1]) / dX));
            return(YY);                         //linear interpolate last seg
        }

        iv = 2;
        do {
            if (xval < pX[iv]) break;           //find index above
        } while (++iv < iend);     
        iv -= 1;                                //make index below
        if (iv >= iem1) {                       //should be impossible
            return(pV[iem1]);                   //(because covered >iemi above)
        }                                       //just in case
        Y1 = pV[iv];
        Y2 = pV[iv+1];

        dX = pX[iv+1] - pX[iv];
        if (dX <= 0) dX = 1.0;
        XX = (xval - pX[iv])/dX;                //fraction into Y1-Y2 

        if (!(mode & 1)) {                      //linear interpolation?
            return(Y1 + ((Y2 - Y1)*XX));
        }

        dX = pX[iv+1] - pX[iv];
        if (dX <= 0) dX = 1.0;
        S3 = (Y2 - Y1)/dX;
        dX = pX[iv] - pX[iv-1];
        if (dX <= 0) dX = 1.0;
        S1 = (Y1 - pV[iv-1])/dX;
        S1 = (S1 + S3) * 0.5;                   //average slope at Y1

        dX = pX[iv+2] - pX[iv+1];
        if (dX <= 0) dX = 1.0;
        S3 = (pV[iv+2] - Y2)/dX;
        dX = pX[iv+1] - pX[iv];
        if (dX <= 0) dX = 1.0;
        S2 = (Y2 - Y1)/dX;
        S2 = (S2 + S3) * 0.5;                   //average slope at Y2

    // Handle Y-only data where X is just index.

    } else {                                    //equally spaced w/o X values?
        if (xval < 1) {                         //before second point?
            if ((xval < 0)&&(!(mode & 8))) {    //assume zero outside of limits?
                if (xval < -1) return(0);       //zero one or more points outside
                YY = pV[0]*(xval + 1);          //interpolate to 0 less than that
                return(YY);
            }
            YY = pV[0] + ((pV[1] - pV[0])*xval);
            return(YY);                         //linear interpolate 1st seg
        }
        if (xval >= iem1) {                     //after second to last point?
            if ((xval > iend)&&(!(mode & 8))) { //assume zero outside of limits?
                if (xval > (iend+1)) return(0); //zero one or more points outside
                YY = pV[iend]*(xval - iend);    //interpolate to 0 less than that
                return(YY);
            }
            YY = pV[iem1] + ((pV[iend] - pV[iem1])*(xval - iem1));
            return(YY);                         //linear interpolate last seg
        }

        iv = (int) xval;                        //find index below
        if (iv < 1) {                           //should be impossible
            return(pV[0]);                      //ibecause <1 above covers
        }                                       //just in case
        if (iv >= iem1) {                       //should be impossible
            return(pV[iem1]);                   //just in case
        }
        Y1 = pV[iv];
        Y2 = pV[iv+1];
        XX = xval - int(iv);                    //fraction into Y1-Y2 

        if (!(mode & 1)) {                      //linear interpolation?
            return(Y1 + ((Y2 - Y1)*XX));
        }

        Y0 = pV[iv-1];
        Y3 = pV[iv+2];

        S1 = (Y2 - Y0)/2.0;
        S2 = (Y3 - Y1)/2.0;
    }

    // Now we have Y1 and Y2 begin and end values for xval's segment.
    // And we have S1 and S2 begin and end slopes for xval's segment.
    // And we have XX 0-1 fraction that xval is into the segment.
    // Solve for spline value given surrounding values and slopes.
    // a = 2*y1 - 2*y2 + s1 + s2
    // b = 3*y2 - 3*y1 - 2*s1 - s2
    // c = s1
    // d = y1

    AA = (2.0 * Y1) - (2.0 * Y2) + S1 + S2;
    BB = (3.0 * Y2) - (3.0 * Y1) - (2.0 * S1) - S2;
    //CC = S1;
    //DD = Y1;

    // Return spline function's value for 0-1 x value.
    // Y = a*x^3 + b*x^2 + c*x + d

    X2 = XX * XX;                               //x^2
    X3 = X2 * XX;                               //x^3
    YY = (AA * X3) + (BB * X2) + (S1 * XX) + Y1;

    return(YY);                                 //return interpolated value
}

//*************************************************************************
//  OSRaise:
//      Raises a floating number to the power of a given exponent.
//
//  Arguments:
//      flx OSRaise (flx base, flx exp)
//      base    Number to be raised to power.
//      exp     Exponent to be used.
//
//  Returns:
//      base ^ power
//*************************************************************************

CSTYLE flx xproc
OSRaise (flx base, flx exp) {

    #ifdef NOI86                                //-------
    return( pow(base, exp) );
    #else                                       //-------
    #ifdef ISWIN                                //===
    #ifdef ISX64                                //---
    return( pow(base, exp) );
    #else                                       //---
    ASM {
    fld     exp                 ;Exp
    fld     base                ;Base, Exp
    fyl2x                       ;P = Exp * log2(Base)
    fld     ST                  ;P,P
    frndint                     ;I,P    where I is an integer
    fsub    ST(1),ST            ;I,F    where -0.5 < F < +0.5
    fld1                        ;1,I,F
    fscale                      ;2^I, I, F
    fxch    ST(1)               ;I, 2^I, F
    fstp    ST                  ;2^I, F
    fxch    ST(1)               ;F, 2^I
    f2xm1                       ;(2^F)-1, 2^I
    fld1                        ;1, (2^F)-1, 2^I
    fadd                        ;2^F, 2^I
    fmul                        ;2^(F+I) = 2^P = 2^(Exp*log2(Base)) = Base^Exp
    fstp    base                ;Base = Base ^ Exp
    fwait                       ;
    }
    #endif                                      //---
    #else                                       //===
    asm ( "                                     \
    fldl    %2                  ;               \
    fldl    %1                  ;               \
    fyl2x                       ;               \
    fld     %%ST                ;               \
    frndint                     ;               \
    fsubr   %%ST,%%ST(1)        ;               \
    fld1                        ;               \
    fscale                      ;               \
    fxch    %%ST(1)             ;               \
    fstp    %%ST                ;               \
    fxch    %%ST(1)             ;               \
    f2xm1                       ;               \
    fld1                        ;               \
    faddp                       ;               \
    fmulp                       ;               \
    fstpl   %0                  ;               \
    fwait                       ;               \
          " : "=m"(base)
            : "m"(base), "m"(exp)
    );                                          //fsubr is correct!
    #endif                                      //===
    return(base);
    #endif      //-------
}

//*************************************************************************
//  OSLog10:
//      Gets the log base ten of a value.
//
//  Arguments:
//      flx OSLog10 (flx value)
//      value   Input value.
//
//  Returns:
//      Log10 of value.
//*************************************************************************

CSTYLE flx xproc
OSLog10 (flx value) {
    #ifdef NOI86                                //-------
    return( log10(value) );
    #else                                       //-------
    #ifdef ISX64                                //***
    return( log10(value) );
    #else                                       //***
    #ifdef ISWIN                                //===
    ASM {
    fldlg2          ;log10(2)
    fld     value   ;value, log10(2)
    fyl2x           ;log10(2)*log2(value)
    fstp    value   ;= log10(value)
    fwait           ;
    }
    #else                                       //===
    asm ( "                                     \
    fldlg2          ;log10(2)                   \
    fld     value   ;value, log10(2)            \
    fyl2x           ;log10(2)*log2(value)       \
    fstp    value   ;= log10(value)             \
    fwait           ;                           \
          " : "=m"(value)
            : "m"(value)
    );
    #endif                                      //===
    return(value);
    #endif                                      //-------
    #endif                                      //***
}

//*************************************************************************
//  OSDebug:
//      Allows console tty input and output for systems like Linux.
//      Note that OSPrint? below automatically uses OSDebug for Linux.
//
//  Arguments:
//      int OSDebug (int mode, text* pline)
//      mode    0 = Close console               (CIN_FINI)
//              1 = Open console                (CIN_OPEN)
//              2 = Read console line to pline  (CIN_READ)
//              3 = Print pline to console      (CIN_SEND)
//      pline   Zero terminated text to output or buffer for input.
//              Input buffer must be at least SZDISK.
//              Output should include LF if desired.
//              Not used for OS_OPEN or OS_FINI.
//
//  Returns:
//      Characters copied or negative if error.
//*************************************************************************

int s_tty = -1;

CSTYLE int xproc
OSDebug (int mode, text* pline) {
    #ifdef ISWIN                                //--- Windows
    switch (mode) {
    case CIN_FINI:
    case CIN_OPEN:
        return(0);

    case CIN_READ:
        printf(pline);
        return(OSTxtSize(pline)-1);

    case CIN_SEND:
        OSConGets(pline);
        return(OSTxtSize(pline)-1);
    }
    return(-2);

    #else                                       //--- Linux
    int done;

    switch (mode) {
    case CIN_FINI:
        if (s_tty == -1) return(0);
        close(s_tty);
        s_tty = -1;
        return(0);

    case CIN_OPEN:
        if (s_tty != -1) close(s_tty);
        s_tty = open("/dev/tty", O_RDWR);       //tty=CurrentTTY,console=SystemConsole
        return((s_tty == -1) ? -1 : 0);         //ty0=CurrentVirtualConsole

    case CIN_READ:
        if (s_tty == -1) {
            done = OSDebug(CIN_OPEN, NULL);
            if (done == -1) return(-3);
        }
        done = read(s_tty, pline, SZDISK-1);
        if (done < 0) return(-1);
        if (done > (SZDISK-1)) done = SZDISK-1;
        pline[done] = 0;                        //zero terminate returned line
        return(done);

    case CIN_SEND:
        if (s_tty == -1) {
            done = OSDebug(CIN_OPEN, NULL);
            if (done == -1) return(-3);
        }
        done = write(s_tty, pline, (OSTxtSize(pline) - 1));
        return(done);
    }
    return(-2);
    #endif                                      //---
}

//*************************************************************************
//  OSPrint:
//  OSPrintI:
//  OSPrintF:
//  OSPrintS:
//      Used by test functions for console and log file output.
//      These automatically go to OSDebug if it is open.
//      The OSPrint version normally shows a hex error value if non-zero.
//      The OSPrintI version normally shows an signed decimal value.
//      The OSPrintF version normally shows 2 floats, 1 decimal and 1 hex.
//      The OSPrintS version normally shows a text string and 1 hex.
//
//  Arguments:
//      void OSPrint  (int mode, text* pout, lint hex)
//      void OSPrintI (int mode, text* pout, lint dec)
//      void OSPrintF (int mode, text* pout, flt f1, flt f2, lint dec, lint hex)
//      void OSPrintS (int mode, text* pout, text* ptwo, lint hex)
//      mode   +2 = Verbose operation about each test beforehand.  (T_SEE)
//                  Otherwise only shows failures not hangs.
//             +4 = Will do other tests after failure.             (T_ALL) 
//             +8 = Add text output to log file.                   (T_LOG) 
//            +16 = Show errors in popup window not console.       (T_WIN)  
//           +256 = Failure message even if not verbose            (T_FAIL)
//           +512 = Show zero values too and show decimal not hex  (T_ZERO)
//                  OSPrintF with T_ZERO only shows f1 and non-zero
//          +1024 = Show pout as is and avoid four char indent     (T_ASIS)
//                  Also always shows popup in window environment
//          +1280 = Intended for non-debug normal errors           (T_ERR)
//              Note: Under Windows non-console forces T_WIN
//              Note: Using T_ALL means no Windows popup just log
//      pout    Text to output to display (SZDISK max)
//      value   Value to display after ptxt or 0 to ignore
//
//  Returns:
//      Nothing
//*************************************************************************

CSTYLE void xproc
OSPrint (int mode, text* pout, lint value) {
    text line[SZDISK+SZNAME+SZNAME];
    text* ptxt;

    #ifdef ISWIN                                //---
    mode |= T_WIN;
    #endif                                      //---

    if (!(mode & (T_SEE+T_FAIL))) {
        return;                                 //don't show verbose stuff?
    }
    ptxt = line;
    if (!(mode & T_ASIS)) {
        ptxt = OSTxtCopy(ptxt, (text*)((mode & T_FAIL) ? "** " : "    "));
    }
    ptxt = OSTxtCopy(ptxt, pout);               //preceed with !!: if error
    if (mode & T_ZERO) {                        //add number if needed
        *ptxt++ = ' ';
        ptxt = OSIntPut(value, ptxt);
    } else if (value) {
        *ptxt++ = ' ';
        *ptxt++ = '0';
        *ptxt++ = 'x';
        ptxt = OSHexPut(value, ptxt);
    }
    if (mode & T_LOG) {
        *ptxt = 0;                              //add to log file
        OSAddLog((0+8+256), NULL, line, 0, 0, NULL);
    }
    if ((mode & T_WIN)&&(mode & (T_FAIL|T_SEE))) {
        if (mode & T_ALL) {
            OSBeep(2);
            OSSleep(1000);
        } else if (!(s_First & TRUETWO)) {
            *ptxt = 0;
            OSMessageBox(0, "Error", line, NULL, NULL);
            if (!(mode & T_ASIS)) {
                s_First |= TRUETWO;             //stop endless popups
            }
        }
    }
    *ptxt++ = '\r';
    *ptxt++ = '\n';
    *ptxt = 0;
    #ifdef ISWIN                                //--- Windows
    OSConLine(line);
    #else                                       //--- Linux
    OSDebug(CIN_SEND, line);
    #endif                                      //---
    return;
}

CSTYLE void xproc
OSPrintI (int mode, text* pout, lint dec) {
    OSPrint((mode | (T_SEE+T_ZERO)), pout, dec);
    return;
}

CSTYLE void xproc
OSPrintF (int mode, text* pout, flt f1, flt f2, lint dec, lint hex) {
    text line[SZDISK+SZNAME];
    text* ptxt;

    ptxt = OSTxtCopy(line, pout);
    *ptxt++ = ' ';
    ptxt = OSFloatPut(f1, ptxt, 0, 0);
    if ((mode & T_ZERO)||(f2)) {
        *ptxt++ = ' ';
        ptxt = OSFloatPut(f2, ptxt, 0, 0);
    }
    if ((mode & T_ZERO)||(dec)) {
        *ptxt++ = ' ';
        ptxt = OSIntPut(dec, ptxt);
    }
    OSPrint((mode | T_SEE), line, hex);
    return;
}

CSTYLE void xproc
OSPrintS (int mode, text* pout, text* ptwo, lint hex) {
    text line[SZDISK+SZDISK];
    text* ptxt;

    ptxt = OSTxtCopy(line, pout);
    *ptxt++ = ' ';
    if (ptwo) {
        ptxt = OSTxtCopy(ptxt, ptwo);
    }
    OSPrint((mode | T_SEE), line, hex);
    return;
}

//*************************************************************************
//  OSBrowser:
//      Activates the default Internet browser to view a given URL.
//      See StmICache in OSFile.cpp to cache a file for browser use.
//      Can also be used to play any multimedia file (with wmplayer.exe).
//      Not supported unless compiled with /D DOGUI and returns 0.
//      Not supported by Linux and returns 0.
//
//  Arguments:
//      int OSBrowser (int mode, ftext* purl)
//      mode    0 = Allow in current browser window.
//              1 = Show in new default browser window.
//              2 = Show with default media player.
//             +4 = Add to mode 2 to force QuickTime player. 
//              Note that mode 0 failes for NT.
//              If mode 0 fails then this routine automatically uses mode 1.
//      purl    Complete URL (or local file name) to be viewed with the browser.
//
//  Returns:
//      Returns TRUE (1) if no error.
//      Returns FALSE (0) if canceled by user.
//*************************************************************************

#define OSEXT_MP4    '4PM'       //.MP4 used when when opening video clip
#define OSEXT_MOV    'VOM'       //.MOV used when when opening video clip
#define OSEXT_QT     'TQ'        //.QT  used when when opening video clip

CSTYLE int xproc
OSBrowser (int mode, ftext* purl) {
    #ifndef DOGUI                               //---
    return(FALSE);
    #else                                       //---
    #ifdef ISWIN                                //===
    text name[SZNOTE];
    text subkey[SZDISK];
    ftext* ptxt;
    ftext* pout;
    ftext* pend;
    HKEY key;
    lint err;
    lint cnt;
    int ok;
    errc e;

    while (*purl == ' ') purl += 1;
    if (*purl == NULL) return(TRUE);            //do nothing for blank URL

// This method uses the DOS START command which can look up and call the
// handler for any type of file.  It assumes the handler for the provided
// URL will be the default browser.  Unfortunately, it causes a DOS box
// to temporarily appear, but it quickly closes after the browser appears.
// By using SW_SHOWMINIMIZED rather than SW_SHOW with WinExec, the temporary
// DOS box only appears minimized (in the task bar) which generally looks OK.
// There is still one unfortunate feature: if the browser (at least IE) is
// already showing the specified URL it does not do anything to grab the
// focus and the focus tends to jump back to our application when the START
// DOS box closes.  If it fails (as it does on NT) we fall back on other way.

    if (mode == 0) {
        ptxt = OSTxtCopy(name, "start ");       //this method causes a DOS box
        *ptxt++ = '"';
        ptxt = OSTxtCopy(ptxt, purl);           //to temporarily flash on screen
        *ptxt++ = '"';
        *ptxt = 0;

        ok = WinExec(name, SW_SHOWMINIMIZED);
        if (ok >= 32) return(TRUE);
    }

// There are two methods for finding and calling the default browser.
// This second, formerly commented out, looks in the registry for the
// program handling the htmlfile type.  This method does not cause a
// DOS box to flash up, as does the "START..." method.  However, it brings
// up a new copy of the browser for each call and so has been made optional.
// However, it works on NT where executing "start ..." does not.

    subkey[0] = 0;
    ptxt = ".html";
    if (mode & 2) {
        ptxt = ".avi";
        err = OSFileExt(NULL, purl);
        if (  (mode & 4)
            ||(err == OSEXT_MOV)||(err == OSEXT_QT)||(err == OSEXT_MP4)  ) {
            ptxt = ".mov";
            mode |= 4;
        }
    }
    e = RegOpenKeyA(HKEY_CLASSES_ROOT, ptxt, &key);
    if ((e)&&(mode & 4)) {
        ptxt = ".avi";                          //Windows Media Play if no QTime
        e = RegOpenKeyA(HKEY_CLASSES_ROOT, ptxt, &key);
    }        
    if (e == 0) {
        cnt = SZDISK;                           //look for .html
        err = RegQueryValueA(key, NULL, subkey, &cnt);
        RegCloseKey(key);
        if (err) subkey[0] = 0;
    }
    if (subkey[0]) {
        ptxt = subkey + (OSTxtSize(subkey) - 1);
        pout = (mode & 2) ? "\\shell\\play" : "\\shell\\open";
        OSTxtCopy(ptxt, pout);             
    } else {                                    //try to not fail if no .html
        OSTxtCopy(subkey, "htmlfile\\shell\\open");
        pout = NULL;
    }
    e = RegOpenKeyA(HKEY_CLASSES_ROOT, subkey, &key);
    if ((e)&&(mode & 2)&&(pout)) {
        OSTxtCopy(ptxt, "\\shell\\open");       //don't insist on play
        e = RegOpenKeyA(HKEY_CLASSES_ROOT, subkey, &key);
    }
    if (e == 0) {
        cnt = SZDISK;                           //lookup corresponding file type
        err = RegQueryValueA(key, "command", subkey, &cnt);
        RegCloseKey(key);                       //and get its open command
        if (err == 0) {
            if (cnt > 1) {
                pout = name;
                ptxt = subkey;
                pend = subkey + (cnt - 1);
                while (ptxt < pend) {           //look for %1 in command
                    if (  (*ptxt == '%')
                        &&(  (*(ptxt+1) == '1')
                           ||(*(ptxt+1) == 'L')
                           ||(*(ptxt+1) == 'l')  )  ) {
                        ptxt += 2;
                        break;                  //if found, copy URL there
                    }
                    *pout++ = *ptxt++;          //copy stuff before %1
                }
                if (ptxt == pend) *pout++ = ' ';
                pout = OSTxtCopy(pout, purl);   //copy URL for %1 or at end
                while (ptxt < pend) {
                    *pout++ = *ptxt++;          //copy stuff after %1
                }
                *pout = 0;

                ok = WinExec(name, SW_SHOW);    //run browser with our URL
                if (ok >= 32) return(TRUE);
            }
        }
    }

// If second method fails and have not tried first, do so.

    if (mode == 1) {
        ptxt = OSTxtCopy(name, "start ");       //this method causes a DOS box
        *ptxt++ = '"';
        ptxt = OSTxtCopy(ptxt, purl);           //to temporarily flash on screen
        *ptxt++ = '"';
        *ptxt = 0;
        ok = WinExec(name, SW_SHOWMINIMIZED);
        if (ok >= 32) return(TRUE);

    } else if (mode & 2) {
        return(OSBrowser(1, purl));             //use browser if no media player
    }
    return(FALSE);
    #else                                       //===
    return(FALSE);                              //not supported by Linux
    #endif                                      //===
    #endif                                      //---
}

//*************************************************************************
//  OSAskFile:
//      Presents a File Open or File Save As dialog box and returns filename.
//      Not supported by Linux and returns 0.
//
//      NOTE: If using FLTK then Fl_Native_File_Chooser is more portable:
//       #include <FL/Fl_Native_File_Chooser.H>
//       [..]                                   //can be BROWSE_FILE for open
//       Fl_Native_File_Chooser fnfc;           //can be BROWSE_DIRECTORY
//       fnfc.title("Pick a file");             //can be BROWSE_MULTI_FILE
//       fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
//       fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
//       fnfc.filter("Text\t*.txt\n"
//                   "C Files\t*.{cxx,h,c}");
//       fnfc.directory("/var/tmp");            //default directory to use
//       switch ( fnfc.show() ) {
//         case -1: printf("ERROR: %s\n", fnfc.errmsg());    break; //ERROR
//         case  1: printf("CANCEL\n");                      break; //CANCEL
//         default: printf("PICKED: %s\n", fnfc.filename()); break; //FILE CHOSEN
//       }
//
//  Arguments:
//      int OSAskFile (int mode, ftext* ptitle, ftext* type, ftext* filter,
//                     ftext* pname, OSApp* papp)
//      mode    0 = Open
//              1 = SaveAs
//             +2 = No warning if open does not exist or save already exists.
//      ptitle  Caption text to use at top of dialog.
//              Use NULL to show default "Open" or "Save As".
//      type    First filter type name (such as "Site").
//              Use NULL if no type name.
//      filter  Pointer to rest of filter text using comma separators.
//              Should contain first pattern ("*.HTM" or "*.HTM;*.TXT")
//              followed by second type name followed by second pattern, etc.
//              (e.g. "*.MXS,HTML Document,*.HTM").  Use NULL for "*.*".
//              All filters combined must fit in 2*SZDISK characters.
//      pname   Pointer to buffer where selected path and file name is returned.
//              Returned text string is zero terminated.
//              Should be at least SZPATH characters.
//              On entry, this gives the default name initially shown.
//              Can optionally just give the default directory to show
//              if the initial name ends in a slash or colon.
//      papp    Owner's application or NULL if none or unknown.
//
//  Returns:
//      Returns FALSE (0) if canceled by user (or maybe error).
//      Returns TRUE (1) if no error for open.
//      Returns positive index of chosen filter file type for save (1=1st).
//*************************************************************************

CSTYLE int xproc
OSAskFile (int mode, ftext* ptitle, ftext* type, ftext* filter,
           ftext* pname, OSApp* papp) {
    #ifdef ISWIN                                //===
    OPENFILENAMEA os;
    text dir[SZDISK];
    text buf[2*SZDISK];
    ftext* ptxt;
    ftext* pext;
    text c;
    int ok;

    OSMemClear(&os, sizeof(OPENFILENAME));
    os.lStructSize = sizeof(OPENFILENAME);
    os.hwndOwner = (papp) ? papp->hwnd : NULL;

    ptxt = buf;
    if (type) {
        ptxt = OSTxtCopy(ptxt, type);
        *ptxt++ = 0;
    }
    pext = ptxt;
    if (filter == NULL) filter = "*.*";
    while (*filter) {
        c = *filter++;
        if (c == ',') c = 0;
        *ptxt++ = c;
    }
    *ptxt++ = 0;
    *ptxt++ = 0;
    *ptxt = 0;
    os.lpstrFilter = buf;
    os.nFilterIndex = 1;
    if ((*pext == '*')&&(*(pext+1) == '.')) {
        pext = pext + 2;
    }
    os.lpstrDefExt = pext;

    if (pname[0]) {
        ptxt = OSTxtStop(dir, pname, SZDISK);
        if (ptxt > dir) {
            c = *(ptxt-1);
            if ((c == '\\')||(c == '/')||(c == ':')) {
                *pname = 0;                     //Vista dislikes path+file in
                os.lpstrInitialDir = dir;       //lpstrInitialDir, just path ok
            }                                   //but all OS vers can use
        }                                       //path+file in lpstrFile
    }                                           //with NULL lpstrInitalDir
    os.lpstrFile = pname;                       //(use to separate before V66)
    os.nMaxFile = SZPATH;
    if (ptitle) os.lpstrTitle = ptitle;
    if (mode & 1) {                             //save as?
        os.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        if (mode & 2) os.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ok = GetSaveFileNameA(&os);
        if ((ok == FALSE)&&(CommDlgExtendedError() == FNERR_INVALIDFILENAME)) {
            *pname = 0;
            ok = GetSaveFileNameA(&os);
        }
        if (ok) {
            ok = os.nFilterIndex;               //return chosen file type index
            if (ok < 1) ok = 1;                 //just in case
        }
    } else {                                    //open?
        os.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        if (mode & 2) os.Flags = OFN_HIDEREADONLY;
        ok = GetOpenFileNameA(&os);
        if ((ok == FALSE)&&(CommDlgExtendedError() == FNERR_INVALIDFILENAME)) {
            *pname = 0;
            ok = GetOpenFileNameA(&os);
        }
    }
    return(ok);
    #else                                       //===
    return(0);                                  //not supported by Linux
    #endif                                      //===
}

//*************************************************************************
//  OSAskFont:
//      Allows the user to select font information.
//      Not supported unless compiled with /D DOGUI and returns 0.
//      Not supported by Linux and returns -1.
//
//  Arguments:
//      int OSAskFont (int mode, SFont* pfont, OSApp* papp)
//      mode    Reserved for future use and must be zero.
//      pfont   Returns various font information.
//              Also supplies default information on entry.
//              If zeroed on entry then provides reasonable defaults.
//      papp    Owner's application or NULL if none or unknown.
//
//  Returns:
//      Non-zero value if error or canceled.
//      Returns -2 if user aborted.
//*************************************************************************

CSTYLE int xproc
OSAskFont (int mode, SFont* pfont, OSApp* papp) {
    #ifndef DOGUI                               //---
    return(-1);
    #else                                       //---
    #ifdef ISWIN                                //===
    CHOOSEFONTA cf;
    LOGFONTA log;
    lint set, oset;
    int ok;

    OSMemClear(&cf, sizeof(CHOOSEFONT));        //set up default information.
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = (papp) ? papp->hwnd : NULL;
    cf.Flags = CF_SCREENFONTS | CF_SCALABLEONLY;
    cf.nFontType = SCREEN_FONTTYPE;

    OSMemClear(&log, sizeof(LOGFONTA));
    cf.lpLogFont = &log;
    cf.Flags |= CF_INITTOLOGFONTSTRUCT;

    log.lfWeight = (pfont->sfStyle & FFBOLD) ? FW_BOLD : FW_NORMAL;
    log.lfItalic = (pfont->sfStyle & FFITALIC) ? TRUE : FALSE;

    OSTxtCopy(log.lfFaceName, pfont->sfName);
    if (log.lfFaceName[0] == 0) OSTxtCopy(log.lfFaceName, "Arial");

    log.lfHeight = pfont->sfHeight;
    if (log.lfHeight > 0) log.lfHeight = -log.lfHeight;
    oset = log.lfHeight;                        //use negative 

    ok = ChooseFontA(&cf);                      //call the common dialog
    if (ok == FALSE) return(-2);      

    OSTxtCopy(pfont->sfName, log.lfFaceName);   //return the font formation
    pfont->sfStyle &= (~(FFBOLD+FFITALIC));
    pfont->sfStyle |= (log.lfWeight > FW_NORMAL) ? FFBOLD : 0;
    pfont->sfStyle |= (log.lfItalic) ? FFITALIC : 0;

    set = log.lfHeight;
    if (set > 0) set = -set;
    pfont->sfHeight = set;

    return(0);
    #else                                       //===
    return(-1);                                 //not supported by Linux
    #endif                                      //===
    #endif                                      //---
}

//*************************************************************************
//  OSAskColor:
//      Allows user to choose a color with common dialogs.
//      Shows the default color as the first of the custom colors.
//      Uses CSAGE - CSILVER (excluding CROSE) for other custom colors.
//      Keeps custom colors in static memory so custom colors defined
//      by the user in this dialog are persistent until application exits.
//      Not supported unless compiled with /D DOGUI and returns 0.
//      Not supported by Linux and returns -1.
//
//  Arguments:
//      errc OSAskColor (int mode, lint* pcolor, OSApp* papp)
//      mode    Reserved for future use and must be zero.
//      hdlg    Parent dialog or window or NULL if none.
//      pcolor  On entry gives default color to be selected in color dialog.
//              This must be a 0x00RRGGBB RGB value wihtout CDraw CRGB format.
//              Use DrawRGB in CDraw.cpp to convert to this simple RGB value.
//              On return, gives the chosen 0x00RRGGBB RGB color without CRGB.
//      papp    Owner's application or NULL if none or unknown.
//
//  Returns:
//      Non-zero value error or canceled.
//      Returns -2 if user aborted.
//*************************************************************************

dword WinCColors[16] = {0xFFFFFFFF};            //static so custom colors

CSTYLE int xproc
OSAskColor (int mode, lint* pcolor, OSApp* papp) {
    #ifndef DOGUI                               //---
    return(-1);
    #else                                       //---
    #ifdef ISWIN                                //===
    CHOOSECOLOR cc;
    fdword* pd;
    dword val, inc;
    int idx, ok;

    pd = WinCColors;
    if (*pd == 0xFFFFFFFF) {
        inc = 0x00101010;
        val = 0;
        for (idx=0; idx<16; idx++) {            //initialize custom colors with
            *pd++ = val;                        //shades of gray because we do
            val += inc;                         //not want to require CDraw here
        }
    }
    OSMemClear(&cc, sizeof(CHOOSECOLOR));
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.Flags = CC_RGBINIT+CC_FULLOPEN;          //remove CC_FULLOPEN for simpler
    cc.hwndOwner = (papp) ? papp->hwnd : NULL;
    cc.rgbResult = *pcolor & 0x00FFFFFF;
    cc.lpCustColors = WinCColors;
    WinCColors[0] = cc.rgbResult;
    ok = ChooseColor(&cc);
    if (!ok) return(-2);
    *pcolor = cc.rgbResult;
    return(0);
    #else                                       //===
    return(-1);                                 //not supported by Linux
    #endif                                      //===
    #endif                                      //---
}

//*************************************************************************
//  OSMessageBox:
//      Puts up an error message dialog.
//      Not supported by Linux and returns 0.
//
//  Arguments:
//      int OSMessageBox (int mode, ftext* ptitle, ftext* ptext, ftext* pname,
//                        OSApp* papp)
//      mode    Style of error box (as per Window's MessageBox).
//              Use 0 for default error icon (exclaimation mark) and OK button.
//              See various common TMB_? definitions in TriOS.h.
//      ptitle  Title text to display at top of dialog.
//              May be NULL if not needed (for default error title).
//      ptext   Error message text.
//              Use "\n" in text for force new line break.
//      pname   Pointer to a null-terminated string to add to the
//              message (usually a file name) separated by a colon and space
//              unless ptext ends with a \n linefeed character.
//              Must not exceed SZPATH characters.
//              May have hidden ending after SEP_BEL character.
//              May be NULL if no added text is desired.
//      papp    Owner's application or NULL if none or unknown.
//
//  Returns:
//      ID for pressed button as returned by Window's MessageBox.
//      Some of the common ID values are IDOK=1, IDCANCEL=2, IDYES=6, IDNO=7.
//*************************************************************************

CSTYLE int xproc
OSMessageBox (int mode, ftext* ptitle, ftext* ptext, ftext* pname, OSApp* papp) {
    #ifdef ISWIN                                //===
    text msg[SZNOTE];
    ftext* ptxt;
    HWND hwnd;

    ptxt = OSTxtCopy(msg, ptext);
    if (pname) {
        if ((ptxt > msg)&&(*(ptxt-1) != '\n')) {
            *ptxt++ = ':';
            *ptxt++ = ' ';
        }
        while ((*pname)&&(*pname != SEP_BEL)) {
            *ptxt++ = *pname++;                 //stop on unprintable ending
        }
        *ptxt = 0;
        ptext = msg;
    }

    if (mode == 0) mode = MB_OK | MB_ICONEXCLAMATION;
    hwnd = (papp) ? papp->hwnd : NULL;
    return( MessageBoxA(hwnd, ptext, ptitle, mode) );

    #else                                       //===
    if (pname) {                                //Use OSFlAsk for Linux
        text msg[SZHUGE];
        ptxt* ptxt = OSTxtCopy(msg, ptext);

        if ((ptxt > msg)&&(*(ptxt-1) != '\n')) {
            *ptxt++ = ':';
            *ptxt++ = ' ';
        }
        while ((*pname)&&(*pname != SEP_BEL)) {
            *ptxt++ = *pname++;                 //stop on unprintable ending
        }
        *ptxt = 0;
        ptext = msg;
    }
    if (mode == 0) mode = MB_OK | MB_ICONEXCLAMATION;
    Fl_Window* pflwin = (papp) ? papp->fwin : NULL;
    return( OSFlAsk(mode, ptext, pflwin) );
    #endif                                      //===
}

//*************************************************************************
//  OSFlAsk:
//      Presents an FLTK Yes / No / Cancel popup dialog.
//      Automatically sizes dialog for given text.
//      Requires that DOSASK be defined.
//      Used by OSMessageBox for Linux.
//
//  Arguments:
//      int OSFlAsk (int mode, text* pask, OSApp* papp)
//      mode    0 = Show Yes / No buttons                  (MB_YESNO)
//             +1 = Add Cancel button
//             +2 = Show Cancel Continue instead of Yes No (MB_CANCELCONTINUE)
//             +4 = Show OK button instead of Yes No       (MB_OK)
//             +8 = Left justify text and use wide dialog
//            +16 = Do not wait for popup dialog to finish
//            +32 = Do not make No/Continue button default
//            +64 = Show OK / Cancel instead of Yes / No buttons
//           2+64 = Show OK / Cancel                       (MB_OKCANCEL)
//      pask    Text string with question to ask.
//              May have \n newline characters to for separate lines.
//      papp    Parent window so popup can be centered or NULL.
//
//  Returns:
//      Postive OS_YES, OS_NO, OS_OK or OS_CAN for Cancel.
//      Note that mode 2 returns OS_YES for Cancel and OS_No for Continue.
//      Negative code if error.
//*************************************************************************

#ifdef  DOSASK                                  //=== DOSASK
#define FLA_BORD    10          //FlAsk pixel border
#define FLA_BTNY    24          //FlAsk dialog button standard height
#define FLA_BIGX   120          //FlAsk dialog big button width
#define FLA_SEPX     6          //FlAsk dialog standard pixels between buttons

#define FLA_YES      "&Yes"
#define FLA_NO       "&No"
#define FLA_OK       "&OK"
#define FLA_CANCEL   "&Cancel"
#define FLA_CONTINUE "&Cancel"

#include <FL/Fl.H>              //FlAsk includes
#include <FL/Fl_Draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

class FlAskYes {
public:
    FlAskYes (int mode, text* pask, Fl_Window* papp);
    ~FlAskYes ();

    text        m_line[SZHUGE]; //Question line
    int         m_retv;         //Value to return

    Fl_Window*  m_dial;         //Dialog window
};

#define FLA_WIDE ((FLA_BORD*2)+((FLA_BIGX+FLA_SEPX)*3))
#define FLA_LESS ((FLA_BORD*2)+((FLA_BIGX+FLA_SEPX)*2))
#define FLA_FONT 14                             //question font size (was 12)
#define FLA_LINE 16                             //question line size (was 15)

FlAskYes* fl_AskYes = NULL;

void FLA_Window (Fl_Widget* po, void* pd) {     //Dialog win callback on close
    FlAskYes* pobj = (FlAskYes*) pd;
    pobj->m_dial->hide();
    return;
}

void FLA_Cancel (Fl_Widget* po, void* pd) {     //Cancel
    FlAskYes* pobj = (FlAskYes*) pd;
    pobj->m_retv = OS_CAN;
    pobj->m_dial->hide();
    return;
}

void FLA_Yes (Fl_Widget* po, void* pd) {        //Yes
    FlAskYes* pobj = (FlAskYes*) pd;
    pobj->m_retv = OS_YES;
    pobj->m_dial->hide();
    return;
}

void FLA_No (Fl_Widget* po, void* pd) {         //No
    FlAskYes* pobj = (FlAskYes*) pd;
    pobj->m_retv = OS_NO;
    pobj->m_dial->hide();
    return;
}

void FLA_Ok (Fl_Widget* po, void* pd) {         //OK
    FlAskYes* pobj = (FlAskYes*) pd;
    pobj->m_retv = OS_OK;
    pobj->m_dial->hide();
    return;
}

FlAskYes::~FlAskYes () {                          //Destructor
    if (m_dial) m_dial->hide();
    fl_AskYes = NULL;
}

FlAskYes::FlAskYes (int mode, text* pask, Fl_Window* papp) {
    text* ptxt;
    Fl_Button* pbtn;
    Fl_Box* pbox;
    int xx, yy, wide, high, tall;

    if (fl_AskYes) {                            //dialog already exists?
        OSBeep(2);                              //should not happen
        if (fl_AskYes->m_dial) fl_AskYes->m_dial->hide();
        delete fl_AskYes;                       //remove old version first
    }
    fl_AskYes = this;

    m_line[0] = 0;
    if (pask) OSTxtStop(m_line, pask, SZHUGE);
    tall = 1;
    ptxt = m_line;
    while (*ptxt) {
        if (*ptxt == '\n') tall += 1;           //count lines
        ptxt += 1;
    }
    tall = (tall+1) * FLA_LINE;
    high = ((FLA_BORD*2)+FLA_BTNY+tall);
    wide = (mode & (1+8)) ? FLA_WIDE : FLA_LESS;

    m_dial = new Fl_Window(wide, high, "");
    if (m_dial == NULL) return;
    if (papp) {
        // Place in center of main window.
        xx = (papp->x() + (papp->w()/2)) - (wide/2);
        yy = (papp->y() + (papp->h()/2)) - (high/2);
        if (xx < papp->x()) xx = papp->x();
        if (yy < papp->y()) yy = papp->y();
        m_dial->position(xx, yy);
    }
    m_retv = 123;
    m_dial->callback(FLA_Window, this);

    m_dial->border(1);                          //0 for no title bar etc
    m_dial->set_modal();                        //popup dialog w/o X button
    m_dial->box(FL_FLAT_BOX);

    xx = FLA_BORD;
    yy = FLA_BORD;

    pbox = new Fl_Box(xx, yy, (wide - (FLA_BORD*2)), tall, m_line);
    pbox->align((mode & 8) ? (FL_ALIGN_TOP+FL_ALIGN_LEFT+FL_ALIGN_INSIDE) : (FL_ALIGN_TOP+FL_ALIGN_INSIDE));
    pbox->box(FL_FLAT_BOX);
    pbox->labelsize(FLA_FONT);

    yy += tall;

    if (!(mode & 4)) {                          //show Yes and No?
        ptxt = (mode & 2) ? FLA_CANCEL : FLA_YES;
        if (mode & 64) ptxt = FLA_OK;
        pbtn = new Fl_Button(xx, yy, FLA_BIGX, FLA_BTNY, ptxt);
        pbtn->callback(FLA_Yes, this);          //Yes or Cancel
        xx += FLA_BIGX+FLA_SEPX;

        ptxt = (mode & 2) ? FLA_CONTINUE : FLA_NO;
        if (mode & 64) ptxt = FLA_CANCEL;
        if (mode & 32) {
            pbtn = new Fl_Button(xx, yy, FLA_BIGX, FLA_BTNY, ptxt);
        } else {
            pbtn = new Fl_Return_Button(xx, yy, FLA_BIGX, FLA_BTNY, ptxt);
        }
        pbtn->callback(FLA_No, this);           //No or Continue
        xx += FLA_BIGX+FLA_SEPX;

    } else {                                    //just show OK?
        xx += FLA_BIGX+FLA_SEPX;
        pbtn = new Fl_Return_Button(xx, yy, FLA_BIGX, FLA_BTNY, FLA_OK);
        pbtn->callback(FLA_Ok, this);           //OK
        xx += FLA_BIGX+FLA_SEPX;
    }

    if (mode & 1) {                             //also show Cancel?
        pbtn = new Fl_Button(xx, yy, FLA_BIGX, FLA_BTNY, FLA_CANCEL);
        pbtn->callback(FLA_Cancel, this);       //Cancel
        xx += FLA_BIGX+FLA_SEPX;
    }

    m_dial->end();
    m_dial->show();
    if (!(mode & 16)) {                         //wait for dialog to finish?
        while (m_dial->shown()) Fl::wait();
    }
    return;
}

// OSFlAsk: Non-object function.
CSTYLE int xproc
OSFlAsk (int mode, text* pask, OSApp* papp) {
    Fl_Window* pwin = (papp) ? ((Fl_Window*)papp->fwin) : NULL;
    FlAskYes ask(mode, pask, pwin);
    return(ask.m_retv);
}
#else                                           //=== DOSASK
CSTYLE int xproc
OSFlAsk (int mode, text* pask, OSApp* papp) {
    return(0);
}
#endif                                          //=== DOSASK

//*************************************************************************
//  OSWideRich:
//      THIS IS A HELPER FUNCTION FOR WINDOWS AND NOT AN EXPORTED OS API!
//
//      Helps read & write UTF-8 Unicode text from a Rich Text Windows control.
//      Rich Text controls can edit and return Unicode without /D "UNICODE".
//      So the best way to have a non-Unicode application with byte text
//      allow user Unicode input and display of Unicode is to substitute a rich
//      text control for normal EDITTEXT controls (which require Unicode app).
//      For safety uses non-wide calls in case Windows does not support.
//
//      IMPORTANT: The application initialization must call: AfxInitRichEdit();
//                 Otherwise rich text will not work!
//
//      Not supported by Linux and returns 0.
//
//      CONTROL "",WD_TEXT,"RICHEDIT",ES_AUTOVSCROLL | WS_BORDER | 
//              WS_TABSTOP | ES_WANTRETURN | ES_MULTILINE, 10,15,240,104
//        
//  Arguments:
//      int OSWideRich (int mode, hand hctl, lptext* pptxt, uint min, uint max)
//      mode    1 to return wide text from control in pwtxt
//              2 to show text from pwtxt in control
//              3 to set the selection range to min - max
//            +32 added to mode 2 to keep previous undo information.
//            +64 added to modes 1 or 2 to just get or replace the selection.
//        +0x2000 added to modes 1 or 2 for right-to-left text
//      hctl    Window handle for rich text control.
//      pptxt   Gets or supplies UTF-8 unicode text.
//              For mode 1 may be NULL to have memory allocated.
//      min     Beginning selection character (0=1st) for mode 3.
//              Extra buffer added at end for mode 1 if NULL pptxt (normally 0).
//      max     Maximum buffer size for mode 1 if pptxt is not NULL.
//              Ending selection character (0=1st) for mode 3.
//
//  Returns:
//      Returns number of characters copied or needed including ending zero.
//      Returns zero if could not copy everything because buffer too small.
//      Also returns zero for mode 1 if UTF-8 text is malformed.
//*************************************************************************

CSTYLE int xproc
OSWideRich (int mode, hand hctl, lptext* pptxt, uint min, uint max) {
    #ifndef DO_WARNS                            //---
    return(0);
    #else                                       //---
    #ifdef ISWIN                                //===
    text bufa[SZPATH+4];
    wtxt bufr[SZPATH+4];
    wtxt* pwtxt;
    text* ptxt;
    text* puse;
    text* psel;
    text* pend;
    text* ptmp;
    text* ptwo;
    void* pmem;
    GETTEXTEX gets;
    SETTEXTEX sets;
    CHARRANGE rang;
    LRESULT res;
    HWND hwin;
    int smode, rest, cnt, num;
    lint need, used;
    uint beg, end;

    hwin = (HWND) hctl;
    smode = mode & 0xF;
    if (smode == 1) {                           //read text from control?
        ptxt = *pptxt;
        if (ptxt) {
            *ptxt = 0;                          //in case of error
        }
        need = GetWindowTextLength(hwin);
        used = (need + 1) * sizeof(wtxt);
        pmem = NULL;
        pwtxt = bufr;
        if (need > SZPATH) {
            pmem = OSAlloc(used);
            if (pmem == NULL) return(0);
            pwtxt = (wtxt*) pmem;
        }

        OSMemClear(&gets, sizeof(GETTEXTEX));
        gets.cb = used;                         //byte count
        gets.flags = GT_DEFAULT;                //GT_RAWTEXT?
        if (mode & 64) {
            gets.flags |= GT_SELECTION;
            need = 1;                           //don't punt when smaller
        }
        gets.codepage = 1200;                   //Unicode
        gets.lpDefaultChar = NULL;
        gets.lpUsedDefChar = NULL;
        res = SendMessageW(hwin, EM_GETTEXTEX, (WPARAM)&gets, (LPARAM)pwtxt);

        if (res < need) {                       //punt to non-wide way if error
            if (pmem) OSFree(pmem);             //this is expected for Win98
            cnt = SendMessage(hwin, WM_GETTEXTLENGTH, 0, 0);
            cnt += 1;                           //include ending zero
            pmem = NULL;
            puse = (text*) bufr;
            if (cnt >= SZPATH*2) {
                pmem = OSAlloc(cnt);
                if (pmem == NULL) return(0);
                puse = (text*) pmem;
            }                                   //get ansi text
            SendMessage(hwin, WM_GETTEXT, cnt, (uint)puse);
            if (mode & 64) {                    //just get selection?
                SendMessage(hwin, EM_FMTLINES, FALSE, 0);
                beg = 0;
                end = 0;
                SendMessage(hwin, EM_GETSEL, (uint)&beg, (uint)&end);
                if (end > beg) {                //if there is one
                    if (end < OSTxtSize(puse)) {
                        *(puse + end) = 0;
                    }
                    if (beg) {
                        OSTxtCopy(puse, (puse + beg));
                    }
                    cnt = OSTxtSize(puse);
                }
            }
            pwtxt = (wtxt*) OSAlloc(cnt * sizeof(wtxt));
            if (pwtxt == NULL) {
                if (pmem) OSFree(pmem);
                return(0);
            }
            MultiByteToWideChar(CP_ACP,0,puse,-1,(LPWSTR)pwtxt,cnt);
            if (pmem) OSFree(pmem);             //convert ansi to wide
            pmem = pwtxt;
        }
        if (ptxt == NULL) {                     //must allocate text?
            max = OSWideUTF8(0, pwtxt, NULL, -1, max);
            ptxt = (text*) OSAlloc(max+min);    //allocate min arg extra
            if (ptxt == NULL) return(0);        //so caller can insert inside
            *ptxt = 0;                          //just in case of error
            *pptxt = ptxt;
        }
        cnt = OSWideUTF8(0, pwtxt, ptxt, -1, max);
        if (pmem) OSFree(pmem);                 //convert to UTF-8
        return(cnt);

    } else if (smode == 2) {                    //write text to control?
        ptxt = *pptxt;
        if (ptxt == NULL) return(0);            //just in case
        cnt = OSWideUTF8(1, NULL, ptxt, -1, 0);
        pmem = NULL;
        pwtxt = bufr;
        if (cnt > SZPATH) {
            pmem = OSAlloc(cnt * sizeof(wtxt));
            if (pmem == NULL) return(0);
            pwtxt = (wtxt*) pmem;
        }                                       //convert to wide Unicode
        cnt = OSWideUTF8(1, pwtxt, ptxt, -1, cnt);

        OSMemClear(&sets, sizeof(SETTEXTEX));
        sets.flags = ST_DEFAULT;
        if (mode & 32) sets.flags |= ST_KEEPUNDO;
        if (mode & 64) sets.flags |= ST_SELECTION;
        sets.codepage = 1200;                   //Unicode
        res = SendMessageW(hwin, EM_SETTEXTEX, (WPARAM)&sets, (LPARAM)pwtxt);

        if (res == 0) {                         //punt and do without Unicode
            ptwo = NULL;                        //which is expected under Win98
            ptmp = NULL;
            cnt = WideCharToMultiByte(CP_ACP,0,(LPWSTR)pwtxt,-1,bufa,0,NULL,NULL);
            puse = bufa;                        //conver to Ansi text
            if (cnt > SZPATH) {
                ptwo = (text*) OSAlloc(cnt);
                if (ptwo == NULL) {
                    if (pwtxt) OSFree(pwtxt);
                    return(0);
                }
                puse = ptwo;
            }
            num =  WideCharToMultiByte(CP_ACP,0,(LPWSTR)pwtxt,-1,puse,cnt,NULL,NULL);
            if (num < cnt) puse = ptxt;         //punt and use UTF-8 directly?

            if (mode & 64) {                    //insert instead of selection?
                beg = 0;
                end = 0;
                SendMessage(hwin, EM_GETSEL, (uint)&beg, (uint)&end);
                rest = end - beg;               //number of chars selected
                cnt = OSTxtSize(puse) - 1;      //number of chars inserting
                if ((cnt)&&(rest >= 0)) {
                    SendMessage(hwin, EM_FMTLINES, FALSE, 0);
                    num = SendMessage(hwin, WM_GETTEXTLENGTH, 0, 0);
                    ptmp = (text*) OSAlloc(num + cnt + 1);
                    if (ptmp) {                 //OK over alloc if rest range
                        num = SendMessage(hwin, WM_GETTEXT, (num+1), (uint)ptmp);
                        if (rest > 0) {
                            psel = ptmp + beg;
                            pend = ptmp + end;  //remove old selection
                            end = num - end;
                            if (end) {
                                OSMemCopy(psel, pend, end);
                            }
                            num = num - rest;
                            if (num < 0) num = 0;
                        }
                        psel = ptmp + beg;
                        pend = psel + cnt;
                        end = num - beg;        //move down to make room
                        if (end) OSMemCopyR(pend, psel, end);
                        OSMemCopy(psel, puse, cnt);
                        ptmp[num + cnt] = 0;    //then insert
                        beg = beg + cnt;        //and move cursor to end of ins
                        puse = ptmp;
                    }
                }
            } 
            res = SendMessage(hwin, WM_SETTEXT, 0, (LPARAM)puse);
            if (ptmp) {
                OSFree(ptmp);                   //inserting over selection?
                SendMessage(hwin, EM_SETSEL, beg, beg);
                SendMessage(hwin, EM_SCROLLCARET, 0, 0);
            }
            if (ptwo) OSFree(ptwo);
        }
        if (pmem) OSFree(pmem);
        return((res) ? cnt : 0);

    } else if (smode == 3) {                    //set selection?
        rang.cpMin = min;                       //WARNING: never used or tested!
        rang.cpMax = max;
        res = SendMessageW(hwin, EM_EXSETSEL, 0, (LPARAM)&rang);
        return(res);
    }
    return(0);
    #else                                       //===
    return(0);                                  //not supported by Linux
    #endif                                      //===
    #endif                                      //---
}

//*************************************************************************
//  OSExec:   
//      Executes another application or command line process.
//      Normally waits until the program has completed before returning.
//      OSCommand: Was the old name for this under FxFoto.
//
//  Arguments:
//      int OSExec (int mode, text* papp, text* parg)
//      mode    0 = Wait for command to complete before returning.   (OS_EXWAIT)
//              1 = Return immediately after starting.               (OS_NOWAIT)
//             +4 = Start showing minimized (if windowed).          (+OS_EXMINI)
//             +8 = Add normal filename extension to papp name.     (+OS_ADDEXE)
//                  For example "Prog" becomes "Prog.exe" for Windows.
//              Note that TriOS.h defines symbols for these mode values.
//      papp    Program filename (either full pathname or name in current path).
//      parg    Optional argument text.
//              This is limited to SZHUGE characters and 256 spaced fields.
//              On some platforms (Linux) arguments surrounded by single or
//              double quotes are grouped as a single argv field string.
//              May be NULL if not needed.
//
//  Returns:
//      Returns the byte status code returned by called program (byte or less).
//      Returns 0 if mode 1 so did not wait for called program to return.
//      Returns -1 if error.
//*************************************************************************

CSTYLE int xproc
OSExec (int mode, text* papp, text* parg) {
    #ifdef ISWIN                                //=== Windows version
    text line[SZPATH+SZHUGE+SZNAME];
    PROCESS_INFORMATION process;
    STARTUPINFOA startup;
    HANDLE file;
    text name[SZPATH];
    text* ptxt;
    dword code;
    boolv mini;
    int ok;

    ptxt = OSTxtStop(line, papp, SZPATH);       //Windows combines app and arg
    if (mode & 8) ptxt = OSTxtCopy(ptxt, ".exe");
    if (parg) {
        *ptxt++ = ' ';
        OSTxtStop(ptxt, parg, SZHUGE);
    }

// NOTE: We formerly used SW_SHOW but it caused MFC apps to not maximize!
// Use WinExec if possible (for mode 1) since CreateProcess seems
// to slow down calling all other programs and help while program runs.

    if (mode & 1) {
        ok = WinExec(line, ((mode & 4) ? SW_MINIMIZE : SW_SHOWNORMAL));
        return((ok > 31) ? 0 : -1);
    }

// NOTE: We formerly used SW_SHOW but it caused MFC apps to not maximize!
// It seems that if the executable is a DOS program and it writes
// anything at all to the standard output stream (to the console)
// then Windows insists on showing the user the output before closing
// the console and the process never finishes until the user clicks
// on the close button for the console dialog.  To execut commands
// with console output without user intervention, we must redirect
// the output so that it does not go to the console.  Unfortunately,
// I can find no way to open the ">NUL" stream that works from the
// DOS command line (and placing ">NUL" at the end of the command line
// has no effect).  Therefore, we must create a real live file and
// send console output to it and then delete it when we are done.

    mini = (mode & 4) ? TRUE : FALSE;

    OSMemClear(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = (mini) ? SW_MINIMIZE : SW_SHOWNORMAL;

    file = INVALID_HANDLE_VALUE;
    if (!(mode & 1)) {                          //wait?
        code = GetTempPathA(SZPATH, name);
        ptxt = name + code;
        if (NO_PRESLASH(name,ptxt)) *ptxt++ = '\\';
        OSTxtCopy(ptxt, "TriErr.txt");
        file = CreateFileA(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_TEMPORARY, NULL);
        if (file == INVALID_HANDLE_VALUE) return(-1);
        startup.dwFlags = (mini) ? SW_MINIMIZE | STARTF_USESTDHANDLES
                                 : SW_SHOW | STARTF_USESTDHANDLES;
        startup.hStdOutput = file;
        startup.hStdError = file;
    }

    ok = CreateProcessA(NULL, line, NULL, NULL, TRUE,
                        DETACHED_PROCESS | HIGH_PRIORITY_CLASS,
                        NULL, NULL, &startup, &process);
    if (ok == FALSE) return(-1);                //run the command
    if (mode & 1) return(0);                    //do not wait for completion?

    code = 0;                                   //just in case
    do {                                        //wait for it to complete
        Sleep(100);
        ok = GetExitCodeProcess(process.hProcess, &code);
        if (ok == FALSE) return(-1);
    } while (code == STILL_ACTIVE);

    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        DeleteFileA(name);
    }
    return((int)code);

// Handle Linux.

    #else                                       //=== Linux version
    text args[SZHUGE];
    text base[SZDISK];
    text* argv[SZDISK];                         //maximum of 256 arguments
    text* ptxt;
    pid_t child;
    int ret, ii;
    int status;

    OSFileBase(base, papp, 1);                  //get base name
    argv[0] = base;
    ii = 1;
    if (parg) {
        OSTxtStop(args, parg, SZPATH);
        ptxt = args;
        while (*ptxt) {
            while ((*ptxt)&&((unsigned)*ptxt <= ' ')) ptxt += 1;
            if (*ptxt == 0) break;              //beginning of next arg
            if ((*ptxt == '"')||(*ptxt == SEP_QT1)) {
                ptxt += 1;                      //quoted argument?
                argv[ii] = ptxt;
                while ((*ptxt)&&(*ptxt != '"')&&(*ptxt != SEP_QT1)) ptxt += 1;
            } else {
                argv[ii] = ptxt;
                while ((*ptxt)&&((unsigned)*ptxt > ' ')) ptxt += 1;
            }
            ii += 1;
            if (ii == (SZDISK-1)) break;
            if (*ptxt == 0) break;
            *ptxt++ = 0;                        //zero terminate arg
        }
    }
    argv[ii] = 0;                               //terminate arg pointer vector

    child = vfork();                            //start child process
    if (child < 0) {                            //error?
        status = -1;

    } if (child == 0) {                         //we are child process?
        if (*papp == '/') {                     //execute application
            execv(papp, argv);                  //with absolute filename
        } else {
            execvp(papp, argv);                 //or find on our path
        }
        status = -2;                            //should never come here

    } else {                                    //parent process continues here
        if (mode & 1) {                         //don't wait?
            status = 0;                         //no status to return

        } else {                                //wait for completion?
            ret = waitpid(child, &status, 0);
            if (ret == -1) status = -1;
            else status = WEXITSTATUS(status);  //ignore other info
        }
    }
    return(status);
    #endif                                      //===
}

//*************************************************************************
//  OSExit:
//      Exits the program instead of returning from main().
//      This is for command-line apps and is not recommended for GUI programs.
//
//      When CDEBUG is defined, calls OSCleanup to check for memory leaks.
//      But skips memory leak check if exit code is 9999.
//      May be called with CRASH_EXIT or CRASH_REAL code for simulated
//      crashes when CDEBUG is defined. CRASH_REAL does a bad pointer exception.
//
//  Arguments:
//      void OSExit (uint code)
//      code    Status code returned to calling process.
//              If CDEBUG defined does exit crash if code 11  (CRASH_EXIT)
//              If CDEBUG defined does real crash if code 12  (CRASH_REAL)
//              If CDEBUG defined calls OSCleanup unless 9999 (NO_CLEANUP)
//              By convention we use code 2 to request reboot (DO_REBOOT)
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSExit (uint code) {
    #ifdef CDEBUG                               //-------
    byte* pmem;

    if (code != CRASH_EXIT) {
        if (code == CRASH_REAL) {
            pmem = NULL;
            *pmem = 0;                          //cause real exception

        #ifndef OSBASE                          //---
        } else if (code != NO_CLEANUP) {
            OSCleanup(0);                       //check for memory leaks
        #endif                                  //---
        }
    }
    #endif                                      //-------
    #ifdef ISWIN                                //===
    ExitProcess(code);
    #else                                       //===
    if (code == DO_REBOOT) {
        system("reboot");                       //same as shell: sudo reboot
    }
    exit(code);
    #endif                                      //===
}

//*************************************************************************
//  OSBeep:
//      Sounds the computer speaker.
//      Beware that only works under Linux if stdout is not redirected.
//      And under Linux all modes produce the same sound and write to console.
//
//  Arguments:
//      void OSBeep (int mode)
//      mode    0 = Error beep ("default sound")
//              1 = Exclamation beep ("exclamation sound")
//              2 = Stop beep ("hand sound")
//              3 = Question beep ("question sound")
//              4 = Completion beep ("system asterisk")
//              5 = Standard beep from computer's speaker (may not work).
//              The mode may be ignored for systems with only one sound.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSBeep (int mode) {
    #ifdef ISWIN                                //===
    if      (mode == 1) MessageBeep(MB_ICONEXCLAMATION);
    else if (mode == 2) MessageBeep(MB_ICONHAND);
    else if (mode == 3) MessageBeep(MB_ICONQUESTION);
    else if (mode == 4) MessageBeep(MB_ICONASTERISK);
    else if (mode == 5) MessageBeep(-1);
    else                MessageBeep(MB_OK);
    #else                                       //===
    printf("\a\n");                             //kludge good only for debugging
    #endif                                      //===
    return;
}

//*************************************************************************
//  OSTask:
//      Creates or destroys a task thread.
//
//      Example C/C++ code:
//
//      DEFSTRUCT(TaskInfo)                     //structure passes information
//      int      mode;                          //to and from the task
//      void*    hand; 
//      DEFENDING(TaskInfo)
//
//      OSTASK_RET Task (TaskInfo* pin) {
//          // Do something in the background here...
//          printf("Task mode=%i\n, mode);
//
//          // Task automatically terminates when it returns.
//          return OSTASK_0;
//      }
//
//      int StartTask (int mode, TaskInfo* pin) {
//          pin->mode = mode;                   //pass information to task
//          pin->hand = NULL;                   //task handle returned here
//
//          int rv = OSTask(OS_TASKNEW, &pin->hand, (LPFTASK)Task, pin);
//          return rv;                          //start Task function above
//      }                                       //running in background
//
//      WARNING: Killing a task from outside is not a good idea
//      in either Windows or Linux and task should normally kill itself.
//      It can do this by exiting its main function or using OS_TASKEND NULL.
//
//  Arguments:
//      int OSTask (int mode, lpvoid* pphand, LPFTASK pfunc, void* parm)
//      mode    0 = Stop (destroy) existing task.                   (OS_TASKEND)
//                  Stops another task with *pphand handle.
//                  WARNING: This is almost never completely safe!
//                  Stops calling task when pphand is NULL.
//              1 = Create a new task.                              (OS_TASKNEW)
//              2 = Return the calling task's handle.               (OS_TASKGET)
//              3 = Switch task priority (if possible).             (OS_TASKPRI)
//          +0x00 = Use default or normal priority
//          +0x10 = Switch to Low priority (if possible).          (+OS_TASKLOW) 
//          +0x20 = Switch to Normal priority (if possible).       (+OS_TASKSTD) 
//          +0x20 = Switch to High priority (if possible).         (+OS_TASKHI1)
//          +0x40 = Switch to Critical priority (if possible).     (+OS_TASKHI2)
//              Note that TriOS.h defines symbols for these mode values.
//              Under Linux setting any of the priority flags changes
//              from the default SCHED_OTHER round-robin scheduling
//              to SCHED_RR real-time round-robin so priories can work.
//              SCHED_FIFO could optionally be used instead below,
//              but it keeps the current task running forever until
//              it yields if there are no higher priority tasks.
//              SCHED_RR does round robin for same priority tasks.
//      pphand  Pointer to task's handle.
//              For modes 1 and 2 returns task's handle.
//              For other modes must give a previously returned handle.
//              For mode 0 exits current task if pphand is NULL.
//      pfunc   Pointer to task's entry-point routine.
//              This is a pointer to a function which takes one 32-bit argument
//              and returns a 32-bit value.
//              The task terminates if this function ever returns.
//              This argument is ignored except for mode 1.
//      parm    Argument passed to new thread's pfunc.
//              For mode 0 when pphand is NULL gives return status code.
//              This argument is ignored except for modes 1 and 0.
//
//  Returns:
//      Returns zero if no error.
//      Returns negative value if error.
//      Mode 3 always returns -2 in Linux, which cannot change priority.
//*************************************************************************

CSTYLE int xproc
OSTask (int mode, lpvoid* pphand, LPFTASK pfunc, void* parm) {
    #ifdef ISWIN                                //=== Windows
    HANDLE* phand;
    HANDLE handle;
    dword id;
    int smode;
    int priority;
    int ret;

    phand = (HANDLE*) pphand;
    smode = mode & 0xF;
    if (smode == 0) {                           //terminate task
        if (phand) {
            ret = TerminateThread(*phand, 0);
            return((ret) ? 0 : -1);
        } else {
            ExitThread(A32(parm));
            return(0);
        }
    } else if (smode == 1) {                    //start new task
        handle = CreateThread(NULL, 0, pfunc, parm, 0, &id);
        if (handle == NULL) return(-1);
        if (mode & OS_TASKPRM) {                //set priority?
            if      (mode & 0x10) priority = THREAD_PRIORITY_BELOW_NORMAL;
            else if (mode & 0x20) priority = THREAD_PRIORITY_NORMAL;
            else if (mode & 0x40) priority = THREAD_PRIORITY_ABOVE_NORMAL;
            else                  priority = THREAD_PRIORITY_TIME_CRITICAL;
            SetThreadPriority(handle, priority);
        }
        if (phand) {
            *phand = handle;                    //return handle
        }
        return(0);

    } else if (smode == 2) {                    //get task handle
        handle = GetCurrentThread();
        if (phand) {
            *phand = handle;
        }
        return(0);

    } else if (smode == 3) {                    //change priority
        if (phand == NULL) return(-1);
        if      (mode & 0x10) priority = THREAD_PRIORITY_BELOW_NORMAL;
        else if (mode & 0x20) priority = THREAD_PRIORITY_NORMAL;
        else if (mode & 0x40) priority = THREAD_PRIORITY_ABOVE_NORMAL;
        else if (mode & 0x80) priority = THREAD_PRIORITY_TIME_CRITICAL;
        else                  priority = THREAD_PRIORITY_NORMAL;
        SetThreadPriority(*phand, priority);
        return(0);
    }
    return(-2);

    #else                                       //=== Linux
    // Linux priorities: http://kothamasusatish.blogspot.com/2013/03/linux-thread-scheduling-policy-and.html
    struct sched_param pri;
    pthread_t* phand;
    pthread_t thread;
    int ret, smode;

    phand = (pthread_t*) pphand;
    smode = mode & 0xF;
    if (smode == 0) {                           //exit thread
        if (phand) {
            ret = pthread_cancel(*phand);
            return((ret == 0) ? 0 : -1);
        } else {
            pthread_exit((void*)parm);
            return(0);
        }

    } else if (smode == 1) {                    //create thread
        ret = pthread_create(&thread, NULL, pfunc, parm);
        if (ret != 0) return(-1);               //priorities not supported
        if (mode & OS_TASKPRM) {                //set priority?
            if      (mode & 0x10) pri.sched_priority =  1;
            else if (mode & 0x20) pri.sched_priority = 97;
            else if (mode & 0x40) pri.sched_priority = 98;
            else                  pri.sched_priority = 99;
            pthread_setschedparam(thread, SCHED_RR, &pri);
        }                                       //SCHED_FIFO prempts forever
        pthread_detach(thread);                 //caller never waits for thread
        if (phand) {
            *phand = thread;                    //return handle
        }
        return(0);

    } else if (smode == 2) {                    //get current thread's handle
        thread = pthread_self();
        if (phand) {
            *phand = thread;                    //return handle
        }
        return(0);

    } else if (smode == 3) {                    //change priority
        if (phand == NULL) return(-1);
        thread = *phand;
        if      (mode & 0x10) pri.sched_priority =  1;
        else if (mode & 0x20) pri.sched_priority = 97;
        else if (mode & 0x40) pri.sched_priority = 98;
        else                  pri.sched_priority = 99;
        pthread_setschedparam(thread, SCHED_RR, &pri);
        return(0);
    }
    return(-2);                                 //bad mode argument
    #endif                                      //===
}

//*************************************************************************
//  OSLock:
//      Implements a simple mutex lock (or critical section).
//      The lock operates on a per-task basis and a second attempt
//      to grab and then release the lock from the same task is ignored.
//      Currently the Linux version cannot be locked recursively from the same
//      task without deadlocking but is faster and more likely to run elsewhere.
//      Change the initializer as in the comment below for recursive operation.
//
//      Can also be used for a semaphore where a task can wait
//      until another task posts a signal to let the waiting task continue.
//
//      The OSLock_GRAB and OSLock_DONE macros can be used
//      instead of the OS_GRAB and OS_DONE calls for speed.
//
//  Arguments:
//      int OSLock (int mode, lpvoid* pplock)
//      mode    1 = Grab the mutex lock (enter critical section).      (OS_GRAB)
//                  Does not return until lock is available (or error).
//                  The *pplock pointer must be allocated by mode -1.
//                  Does nothing if *pplock is NULL.
//                  Must later call mode 2.
//                  Always succeeds.
//              2 = Release the mutex lock (leave critical section).   (OS_DONE)
//                  Must have previously called mode 1.
//                  Does nothing if *pplock is NULL.
//                  Always succeeds.
//             -1 = Initialize the mutex lock.                      (OS_LOCKNEW)
//                  This must be called first.
//                  It allocates memory and returns its pointer in *pplock.
//                  If it succeeds, mode -2 must be called to free.
//                  May return negative error.
//             -2 = Free mutex lock so that can no longer be used.  (OS_LOCKOFF)
//                  Frees the *pplock memory.                   
//
//             11 = Wait for semaphore.                             (OS_SEMWAIT)
//                  Decreases value by 1 as soon as it is not 0.
//             12 = Post to semaphore to let task run.              (OS_SEMPOST)
//                  Increases value by 1 to unblock waiting task.
//            -11 = Initialize semaphore and set value to 0.        (OS_SEMINIT)
//                  Must do mode 12 post before 11 wait continues.
//            -12 = Free semaphore.                                 (OS_SEMFREE)
//
//      pplock  Pointer to where lock memory pointer is stored.
//              This is initialized by mode -1.
//              It must be passed back for other modes.
//              It must not be changed by caller.
//
//  Returns:
//      Returns zero if no error.
//      Returns negative value if error.
//*************************************************************************

CSTYLE int xproc
OSLock (int mode, lpvoid* pplock) {
    #ifdef ISWIN                                //=== Windows
    LPCRITICAL_SECTION pcrit;
    HANDLE hsem;

    if (mode == 1) {
        pcrit = (LPCRITICAL_SECTION) *pplock;
        if (pcrit == NULL) return(-1);
        EnterCriticalSection(pcrit);
        return(0);

    } else if (mode == 2) {
        pcrit = (LPCRITICAL_SECTION) *pplock;
        if (pcrit == NULL) return(-1);
        LeaveCriticalSection(pcrit);
        return(0);

    } else if (mode == 11) {
        hsem = (HANDLE) *pplock;
        if (hsem == NULL) return(-1);
        WaitForSingleObject(hsem, INFINITE);
        return(0);

    } else if (mode == 12) {
        hsem = (HANDLE) *pplock;
        if (hsem == NULL) return(-1);
        ReleaseSemaphore(hsem, 1, NULL);
        return(0);

    } else if (mode == -1) {
        *pplock = NULL;                         //in case of error
        pcrit = (LPCRITICAL_SECTION) OSAlloc(sizeof(CRITICAL_SECTION));
        if (pcrit == NULL) return(-1);
        InitializeCriticalSection(pcrit);
        *pplock = pcrit;
        return(0);

    } else if (mode == -2) {
        pcrit = (LPCRITICAL_SECTION) *pplock;
        if (pcrit == NULL) return(-1);
        DeleteCriticalSection(pcrit);
        OSFree(pcrit);
        return(0);

    } else if (mode == -11) {
        *pplock = NULL;                         //in case of error
        hsem = CreateSemaphore(NULL, 0, 255, NULL);
        if (hsem == NULL) return(-1);
        *pplock = hsem;
        return(0);

    } else if (mode == -12) {
        hsem = (HANDLE) *pplock;
        if (hsem == NULL) return(-1);
        CloseHandle(hsem);
        return(0);
    }
    return(-2);

    #else                                       //=== Linux
    pthread_mutex_t* pmutex;
    sem_t* psem;
    int ok;

    if (mode == 1) {                            //lock mutex?
        pmutex = (pthread_mutex_t*) *pplock;
        if (pmutex == NULL) return(-1);
        pthread_mutex_lock(pmutex);
        return(0);
        
    } else if (mode == 2) {                     //unlock mutex?
        pmutex = (pthread_mutex_t*) *pplock;
        if (pmutex == NULL) return(-1);
        pthread_mutex_unlock(pmutex);
        return(0);

    } else if (mode == 11) {                    //wait for semaphore?
        psem = (sem_t*) *pplock;
        if (psem == NULL) return(-1);
        sem_wait(psem);
        return(0);

    } else if (mode == 12) {                    //post to semaphore?
        psem = (sem_t*) *pplock;
        if (psem == NULL) return(-1);
        sem_post(psem);
        return(0);

    } else if (mode == -1) {                    //create mutex?
        *pplock = NULL;                         //in case of error
        pmutex = (pthread_mutex_t*) OSAlloc(sizeof(pthread_mutex_t));
        if (pmutex == NULL) return(-1);
        pthread_mutex_init(pmutex, NULL);       //don't use PTHREAD_MUTEX_INITIALIZER
        *pplock = pmutex;                       //recursive is used in Windows
        return(0);                              //but slower and maybe less safe

    } else if (mode == -2) {                    //free mutex?
        pmutex = (pthread_mutex_t*) *pplock;
        if (pmutex == NULL) return(-1);
        pthread_mutex_destroy(pmutex);
        OSFree(pmutex);
        return(0);

    } else if (mode == -11) {                   //init semaphore?
        *pplock = NULL;                         //in case of error
        psem = (sem_t*) OSAlloc(sizeof(sem_t));
        if (psem == NULL) return(-1);
        ok = sem_init(psem, 0, 0);              //local to our process
        if (ok != 0) return(-2);
        *pplock = psem;
        return(0);
        
    } else if (mode == -12) {                   //free semaphore?
        psem = (sem_t*) *pplock;
        if (psem == NULL) return(-1);
        sem_destroy(psem);
        OSFree(psem);
        return(0);
    }
    return(-2);
    #endif                                      //===
}

//*************************************************************************
//  OSWait:
//      Allows a task to wait and another task to wake it up.
//      Can also allow multiple tasks to wait on one task to wake them all.
//
//      Should bracket both OS_WAIT and OS_WAKE with OS_GRAB and OS_DONE calls.
//      Beware that wake is ingored unless someone is already waiting.
//      So wait task must check and skip OS_WAIT if already has what it needs.
//      Also beware that missed wakes are possible, especially under Windows.
//      So wait task should set fairly short timeout and should check
//      whether what it was waiting for has arrived even if there is a timeout.
//
//      Before waiting, a task first OS_GRAB grabs a mutual exclusion lock
//      so that it can check whether it has additional work to do
//      and no other tasks can check for that work to avoid a race.
//      Then it either OS_DONE frees the mutual exclusion lock and keeps
//      going or it decides to go to OS_WAIT sleep and wait for another task
//      to signal or for an optional timeout to wake it up again.
//      The OS_WAIT wait call atomically releases the mutual exclusion lock,
//      but before it wakes up the mutex automatically is locked again,
//      allowing it to safely read data provided by the other task.
//      The data producing task normally uses OS_GRAB first to stop
//      the waiting consumer task from interfering while it passes the data
//      (which is often in a queue).  The OS_WAKE mode is non-blocking and
//      does not free the mutex, so the producer calls OS_DONE after OS_WAIT.
//
//      Cannot mix OSWait with OSLock calls since pwaits handles differ!
//      However the plock argument allows an OSLock mutex to be used
//      with this function instead of the pwaits built-in mutex.
//
//      Beware that the OS_WAIT may see spurious wakeups or another task
//      may wake unstead even if we starte waiting first. This is known to
//      be possible under Windows and it is safest to assume it under Linux.
//      Therefore the caller should use OS_WAIT in a loop that tests whether
//      what it was waiting for is really true before continuing.
//      And OS_WAIT should set the timeout short enough to be
//      able to recover if it misses the OS_WAKE signal.
//
//      Beware: The Windows implementation is primative!
//      See: http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
//      We used Section 3.2 version simplified to not include broadcast.
//      Note that Vista has SleepConditionVariableCS and WakeConditionVariable!
//
//      The OSWait_GRAB and OSWait_DONE macros can be used
//      instead of the OS_GRAB and OS_DONE calls for speed.
//
//  Arguments:
//      int OSWait (int mode, OSWAITS* pwaits, void* plock, lint timeout)
//      mode    1 = Grab mutual exclusing lock to check todo list.  (OS_GRAB)
//                  May optionally call OS_WAIT before OS_DONE.
//                  Must later call either OS_DONE.
//              2 = Release mutual exclusion lock without waiting.  (OS_DONE)
//                  This is used by waiting task after it wakes.
//                  OS_WAKE task can use OS_GRAB and OS_DONE too.
//
//              3 = Wait for wakeup.                                (OS_WAIT)
//                  The timeout gives max time to wait if not zero.
//                  Automatically frees mutex before waiting.
//                  Automatically grabs mutex again before waking.
//
//              4 = Wakeup another waiting task.                    (OS_WAKE)
//                  This is the signal task.
//                  Ignored if other task not already waiting!
//
//              5 = Wakeup all waiting tasks.                       (OS_WAKEALL)
//                  This is the signal broadcast task.
//                  Ignored if other task(s) not already waiting!
//
//             -1 = Intialize for later calls.                      (OS_LOCKNEW)
//                  This must be called first.
//                  It initializes the provided *pwaits structure.
//                  If it succeeds, mode -2 must be called to free.
//                  May return negative error.
//             -2 = Free mutex lock so that can no longer be used.  (OS_LOCKOFF)
//                  Frees the pwaits handles.                   
//
//      pwaits  Structure provided by caller to keep the wait state.
//              This is initialized by mode -1.
//              It must be passed back for other modes.
//              It must not be changed by caller.
//              The caller can read the pwaits->waits waiting task count
//              while the mutex is locked and can use pwaits->waits to
//              build non-tasked test versions that simulate waiting.
//
//      plock   Optional pointer to OSLock mutex to be used instead of pwaits.
//              Use NULL for the pwaits built-in mutex.
//              Otherwise this must be the pplock returned by OSLock -1.
//              If supplied, this function's OS_GRAB and OS_DONE use it instead.
//              If supplied, OS_WAIT and OS_WAKE assume it is grabbed first.
//              Actually it is ignored by OS_WAKE and can optionally be NULL
//              (because OS_WAKE assumes caller grabs and releases it).
//              This is ignored by -1 and -2 calls and should be NULL.
//
//      timeout Maximum milliseconds to wait or 0 if forever.
//              Should be 0 if not OS_WAIT mode.
//
//  Returns:
//      Returns zero if no error.
//      Returns negative value if error.
//      Returns positive TRUE=1 in OS_WAIT mode if timed out.
//*************************************************************************

CSTYLE int xproc
OSWait (int mode, OSWAITS* pwaits, void* plock, lint timeout) {
    #ifdef ISWIN                                //=== Windows
    LPCRITICAL_SECTION pmutex;
    lint count;
    dword rv;

    if (mode == OS_GRAB) {                      //lock mutex?
        if (plock) {
            EnterCriticalSection((LPCRITICAL_SECTION)plock);
            return(0);
        }
        if (pwaits->alloc != TRUE) return(0);
        EnterCriticalSection(&pwaits->mutex);
        return(0);

    } else if (mode == OS_DONE) {               //unlock mutex?
        if (plock) {
            LeaveCriticalSection((LPCRITICAL_SECTION)plock);
            return(0);
        }
        if (pwaits->alloc != TRUE) return(0);
        LeaveCriticalSection(&pwaits->mutex);
        return(0);

    } else if (mode == OS_WAIT) {               //wait for wakeup?
        if (pwaits->alloc != TRUE) return(0);
        pwaits->waits += 1;
        pwaits->wokes = TRUE;

        pmutex = (plock) ? ((LPCRITICAL_SECTION)plock) : (&pwaits->mutex);
        LeaveCriticalSection(pmutex);

        if (timeout == 0) timeout = INFINITE;   //ResetEvent not needed
        rv = WaitForSingleObject(pwaits->event, timeout);

        pwaits->waits -= 1;

        EnterCriticalSection(pmutex);
        if (rv == WAIT_TIMEOUT) return(1);      //TRUE for timeout
        return(0);

    } else if (mode == OS_WAKE) {               //wakeup waiting task?
        if (pwaits->alloc != TRUE) return(0);
        count = pwaits->waits;
        if (pwaits->wokes == FALSE) count = 0;  //stop second OS_WAKE
        pwaits->wokes = FALSE;
        if (count) {
            SetEvent(pwaits->event);
        }
        return(0);

    } else if (mode == OS_WAKEALL) {            //wakeup all waiting tasks?
        if (pwaits->alloc != TRUE) return(0);
        count = pwaits->waits;
        if (pwaits->wokes == FALSE) count = 0;  //stop second OS_WAKEANY
        pwaits->wokes = FALSE;
        while (count > 0) {
            count -= 1;
            SetEvent(pwaits->event);
        }
        return(0);

    } else if (mode == OS_LOCKNEW) {            //initalize?
        pwaits->alloc = FALSE;                  //in case of error
        pwaits->waits = 0;
        pwaits->wokes = FALSE;
        InitializeCriticalSection(&pwaits->mutex);
        pwaits->event = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pwaits->event == NULL) {            //first FALSE => auto ResetEvent
            return(-1);                         //second FALSE => not signaled
        }
        pwaits->alloc = TRUE;
        return(0);

    } else if (mode == OS_LOCKOFF) {            //cleanup?
        if (pwaits->alloc != TRUE) return(0);   //allows double destruct
        pwaits->alloc = FALSE;                  //without crashing
        DeleteCriticalSection(&pwaits->mutex);
        CloseHandle(pwaits->event);
        return(0);
    }
    return(-2);

    #else                                       //=== Linux
    pthread_mutex_t* pmutex;
    struct timespec time;
    lint count;
    int ok;

    if (mode == OS_GRAB) {                      //lock mutex?
        if (plock) {
            pthread_mutex_lock((pthread_mutex_t*)plock);
            return(0);
        }
        if (pwaits->alloc != TRUE) return(0);
        pthread_mutex_lock(&pwaits->mutex);
        return(0);
        
    } else if (mode == OS_DONE) {               //unlock mutex?
        if (plock) {
            pthread_mutex_unlock((pthread_mutex_t*)plock);
            return(0);
        }
        if (pwaits->alloc != TRUE) return(0);
        pthread_mutex_unlock(&pwaits->mutex);
        return(0);

    } else if (mode == OS_WAIT) {               //wait for wakeup?
        if (pwaits->alloc != TRUE) return(0);
        pwaits->waits += 1;
        pwaits->wokes = TRUE;
        pmutex = (plock) ? ((pthread_mutex_t*)plock) : (&pwaits->mutex);
        if (timeout) {
            clock_gettime(CLOCK_REALTIME, &time);
            time.tv_sec += timeout / 1000;      //future time to stop waiting
            time.tv_nsec += (timeout % 1000) * 1000000;
            if ((unsigned)time.tv_nsec >= 1000000000) {
                time.tv_nsec -= 1000000000;
                time.tv_sec += 1;
            }
            ok = pthread_cond_timedwait(&pwaits->cond, pmutex, &time);
        } else {
            ok = pthread_cond_wait(&pwaits->cond, pmutex);
        }
        pwaits->waits -= 1;
        if (ok == 0) return(0);
        if (ok == ETIMEDOUT) return(1);         //TRUE for timeout
        return(-3);

    } else if (mode == OS_WAKE) {               //wakeup waiting task?
        if (pwaits->alloc != TRUE) return(0);
        pwaits->wokes = FALSE;
        pthread_cond_signal(&pwaits->cond);
        return(0);

    } else if (mode == OS_WAKEALL) {            //wakeup all waiting tasks?
        if (pwaits->alloc != TRUE) return(0);
        count = pwaits->waits;
        if (pwaits->wokes == FALSE) count = 0;  //stop second OS_WAKEANY
        pwaits->wokes = FALSE;
        while (count > 0) {
            count -= 1;
            pthread_cond_signal(&pwaits->cond);
        }
        return(0);

    } else if (mode == OS_LOCKNEW) {            //initalize?
        pwaits->waits = 0;
        pwaits->wokes = FALSE;
        pthread_mutex_init(&pwaits->mutex,NULL);//don't use PTHREAD_MUTEX_INITIALIZER
        pthread_cond_init(&pwaits->cond,NULL);  //don't use PTHREAD_COND_INITIALIZER
        pwaits->alloc = TRUE;
        return(0);

    } else if (mode == OS_LOCKOFF) {            //cleanup?
        if (pwaits->alloc != TRUE) return(0);   //allows double destruct
        pwaits->alloc = FALSE;                  //without crashing
        pthread_mutex_destroy(&pwaits->mutex);
        pthread_cond_destroy(&pwaits->cond);
        return(0);
    }
    return(-2);
    #endif                                      //===
}

//*************************************************************************
//  OSMapMem:
//      Sets up a block of mapped memory that can communicate between apps.
//      Note that that under Cygwin Linux (for testing on top of Windows),
//      this routine uses shared memory and requires Cygserver.exe to run.
//
//  Arguments:
//      int OSMapMem (int mode, dword key, lpvoid* phand,
//                    lpvoid* ppmem, dword memsz)
//      mode   -1 = Close and destroy mapped memory.               (OS_MEMCLOSE)
//              0 = Unmap our memory copy without destroying.      (OS_MEMUNMAP)
//              1 = Open mapped memory for reading and writing.    (OS_MEMOPEN)
//              4 = Create mapped memory for reading and writing.  (OS_MEMCREAT)
//                  Mode 4 zeros the memory block.
//              Note that TriOS.h defines symbols for these mode values.
//      key     Unique numeric identifier of the mapped memory object.
//              Ignored for modes 0 and -1.
//      phand   Returns the memory map object handle for non-zero modes.
//              Supplies the memory map object handle for mode 0.
//      ppmem   Modes 4 and 1 return pointer to mapped memory or NULL if error.
//              May be NULL for mode 4 if not needed for now.
//              Modes 0 or -1 supply previously returned mapped memory pointer.
//              May be NULL for mode -1 if our copy is already unmapped.
//      memsz   Number of bytes of mapped memory to be used.
//              Ignored for modes 0 and -1.
//
//  Returns:
//      Returns zero if no error.
//      Returns negative value if error.
//*************************************************************************

CSTYLE int xproc
OSMapMem (int mode, dword key, lpvoid* phand, lpvoid* ppmem, dword memsz) {
    #ifdef ISWIN                                //=== Windows
    text name[SZNAME];
    HANDLE file;
    void* pmem;

    if (mode == 4) {                            //create?
        if (ppmem) *ppmem = NULL;               //in case of error
        OSHexPut(key, name);                    //use hex number as name
        file = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                  0, memsz, name);
        *phand = (HANDLE) file;
        if (file == NULL) return(-1);
        if (ppmem) {
            pmem = MapViewOfFile(file, FILE_MAP_WRITE, 0, 0, memsz);
            if (pmem == NULL) return(-2);
            OSMemClear(pmem, memsz);
            *ppmem = pmem;
        }
        return(0);
    }
    if (mode == 1) {                            //open previously created?
        *ppmem = NULL;                          //in case of error
        OSHexPut(key, name);                    //use hex number as name
        file = OpenFileMappingA(FILE_MAP_WRITE, FALSE, name);
        *phand = file;
        if (file == NULL) return(-1);
        pmem = MapViewOfFile(file, FILE_MAP_WRITE, 0, 0, memsz);
        if (pmem == NULL) return(-2);
        *ppmem = pmem;
        return(0);
    }
    if (mode == 0) {                            //unmap?
        pmem = *ppmem;
        UnmapViewOfFile(pmem);
        return(0);
    }
    if (mode == -1) {                           //close?
        if (ppmem) {
            pmem = *ppmem;
            UnmapViewOfFile(pmem);
        }
        file = (HANDLE) *phand;
        if (file == NULL) return(-1);
        CloseHandle(file);
        return(0);
    }
    return(-2);

    #else                                       //=== Linux
    void* pmem;
    int shmid;
    int ret;

    if (mode == 4) {                            //create?
        if (ppmem) *ppmem = NULL;               //in case of error
        shmid = shmget(key, memsz, (IPC_CREAT | 0666));
        *phand = (void*) shmid;
        if (shmid == -1) return(-1);
        pmem = shmat(shmid, NULL, 0);
        if ((pmem == NULL)||(((lint)pmem) == -1)) return(-2);
        OSMemClear(pmem, memsz);
        *ppmem = pmem;
        return(0);
    }
    if (mode == 1) {                            //open?
        *ppmem = NULL;                          //in case of error
        shmid = shmget(key, 0, 0);
        *phand = (void*) shmid;
        if (shmid == -1) return(-1);
        pmem = shmat(shmid, NULL, 0);
        if ((pmem == NULL)||(((lint)pmem) == -1)) return(-2);
        *ppmem = pmem;
        return(0);
    }
    if (mode == 0) {                            //unmap?
        ret = shmdt(*ppmem);        
        if (ret == -1) return(-1);
        return(0);
    }
    if (mode == -1) {                           //close?
        shmid = (int) *phand;
        ret = shmctl(shmid, IPC_RMID, NULL);
        if (ppmem) {
            pmem = *ppmem;
            shmdt(pmem);                        //must come after shmctt
        }
        if (ret == -1) return(-1);
        return(0);
    }
    return(-2);
    #endif                                      //===
}

//*************************************************************************
//  OSLastError:
//      Gets the last error's code number.
//
//  Arguments:
//      lint OSLastError (int mode)
//      mode    0 = Just return error code number.
//              1 = Also print code number and test message if possible.
//
//  Returns:
//      Returns OS dependent error code for the last error encountered.
//*************************************************************************

CSTYLE lint xproc
OSLastError (int mode) {
    #ifdef ISWIN                                //=== Windows
    text line[SZNAME];
    text* ptxt;
    errc e;
    e = GetLastError();
    if (mode) {
        ptxt = OSTxtCopy(line, "Last Error: ");
        OSHexPut(e, ptxt);
        OSConLine(line);
    }
    return( e );
    #else                                       //=== Linux
    if (mode) {
        printf("Last Error %i: %s\n", errno, strerror(errno));
    }
    return( errno );
    #endif                                      //===
}

//*************************************************************************
//  OSAddLog:
//      Writes a line to the application log file.
//      By default, uses GSSIrad.log file in the OS_LOCAPP AppData or exe dir.
//      If CDEBUG defined then shows application instance first date and time.
//      Can also return a given line at the end of the log file.
//
//  Arguments:
//      int OSAddLog (int mode, text* plog,
//                     text* pmsg, errc code, lint numb, text* pnam)
//      mode    0 = Add to existing log.
//              1 = Start a new log.
//             +2 = Always show (code,numb) even if zero. 
//             +4 = Do not add colon and space characters before pname.
//             +8 = Add plog base name to standard directory. 
//            +64 = Create log file in Data subdirectory.  
//           +128 = Do not add new instance line before first instance line.
//           +256 = Do not interrupt into debugger (info rather than an error).
//            512 = Just delete any existing log file.
//             -1 = Return last line of log file in pmsg SZDISK buffer.
//             -2 = Return next to last line of log file in pmsg SZDISK buffer.
//             -N = Return Nth line from end of log file in pmsg SZDISK buffer.
//      plog    Log filename.
//              If not full path then places in My Documents or /usr/doc.
//              May be NULL for default ErrorLog.txt
//      pmsg    Text message to add to log.
//              May be NULL if not needed.
//              For negative mode must be SZDISK buffer for line and newline.
//      code    Error code to add as (hex) after pmsg if non-zero.
//              May be zero if not needed.
//              Mode +2 adds even if zero.
//              Ignored for negative mode to get line and should be 0.
//      numb    Number to add as decimal after the code (code,numb) if non-zero.
//              Does not show if code is zero unless mode +2.
//              Mode +2 adds even if zero.
//              Ignored for negative mode to get line and should be 0.
//      pnam    Filename or other text to add at end of line after colon.
//              May be NULL if not needed.
//              Ignored for negative mode to get line and should be NULL.
//
//  Returns:
//      Returns zero if no error.
//      Returns negative value if error.
//*************************************************************************

CSTYLE int xproc
OSAddLog (int mode, text* plog, text* pmsg, int code, lint numb, text* pnam) {
    #ifdef ISWIN                                //===
    hand file;
    #else                                       //===
    int file;
    #endif                                      //===
    text bufr[SZNOTE];
    text line[SZDISK];
    text* ptxt;
    text* pend;
    text* pmem;
    dword done;
    aint fh;
    lint nb;
    int ret;
    int ee;

    #ifdef  CDEBUG                              //---
    #ifndef CNOBRK
    #ifdef  ISWIN
    if (!(mode & 256)) {                        //interrupt into debugger?
        BREAK3;                                 //allow on the fly debugging
    }
    mode &= (~256);
    #endif
    #endif
    #endif                                      //---

    if ((plog == NULL)||(mode & 8)) {
        ptxt = OSLocation(bufr, OS_LOCAPP);     //was OS_LOCDOC until V2.10
        if (mode & 64) {
            ptxt = OSTxtCopy(ptxt, "Data/");    //put in Data subdirectory?
        }
        if (plog == NULL) plog = OS_LOG;
        OSTxtCopy(ptxt, plog);                  //get complete filename
        plog = bufr;
    }
    if (mode == 512) {
        OSErase(plog);
        return(0);
    }

    if (mode < 0) {                             //read line from log?
        *pmsg = 0;                              //return blank in case of error
        fh = OSOpen(OS_READ, plog);
        if (fh == -1) return(ECNOOPEN);
        done = OSSeek(fh, 0, OS_END);
        if (done < 2) return(ECNOPTS);
        pmem = (text*) OSAlloc(done);
        if (pmem == NULL) return(ECMEMORY);
        OSSeek(fh, 0, OS_SET);
        done = OSRead(fh, pmem, done);          //read file into memory
        ptxt = &pmem[done];
        do {                                    //look backward from end
            pend = ptxt;
            if (ptxt <= pmem) break;
            if (*(ptxt-1) == '\n') ptxt -= 1;   //skip newline at end of a line
            if (*(ptxt-1) == '\r') ptxt -= 1;
            while (ptxt > pmem) {               //find begining of previous line
                if ((*(ptxt-1) == '\n')||(*(ptxt-1) == '\r')) break;
                ptxt -= 1;
            }
        } while (++mode);
        if (mode < -1) {
            OSFree(pmem);
            return(ECNOMATCH);                  //not enough lines?
        }
        done = (dword) (pend - ptxt);
        if (done > (SZDISK-1)) done = SZDISK-1; //truncate to fit if needed
        if (done) OSMemCopy(pmsg, ptxt, done);  //return line
        pmsg[done] = 0;                         //zero terminated
        OSFree(pmem);
        return(0);        
    }

    if (!(s_First & TRUE)) {                    //first time since prog started?
        s_First |= TRUE;                        //log new instance line
        if (!(mode & 128)) {
            ret = mode & 4;
            if (mode & 1) {                     //start a new log file?
                ret |= 1;
                mode &= (~1);
            }
            ret |= 256;                         //only debug break once
            ptxt = line;
            *ptxt++ = '[';
            ptxt = OSDateTime(ptxt, NULL, NULL, 4);
            OSTxtCopy(ptxt, "] *******");       //show first time for program
            OSAddLog(ret, plog, "\r\n******* New Instance", 0,0, line);
        }
    }

    #ifdef ISWIN                                //=== Windows
    ee = -1;
    file = NULL;                                //avoid compiler error
    if (!(mode & 1)) {                          //open log file
        file = CreateFileA(plog, (GENERIC_READ | GENERIC_WRITE),
                          (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        ee = (file == INVALID_HANDLE_VALUE) ? (-2) : 0;
    }
    if (ee) {
        file = CreateFileA(plog, (GENERIC_READ | GENERIC_WRITE), 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        ee = (file == INVALID_HANDLE_VALUE) ? (-3) : 0;
    }
    if (ee) {
        OSBeep(2);
        return(ee);
    }
    SetFilePointer(file, 0, NULL, FILE_END);    //add to end of log file

    #else                                       //=== Linux
    ee = -1;
    file = NULL;                                //avoid compiler error
    if (!(mode & 1)) {                          //open log file
        file = open(plog, (O_WRONLY | O_APPEND));
        ee = (file == -1) ? (-2) : 0;
    }
    if (ee) {
        file = open(plog, (O_WRONLY | O_APPEND | O_CREAT | O_TRUNC),
                    (S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH));
        ee = (file == -1) ? (-3) : 0;
    }
    if (ee) {
        OSBeep(2);
        return(ee);
    }
    lseek(file, 0, SEEK_END);                   //in case O_APPEND does not work
    #endif                                      //===

    ptxt = bufr;
    if (pmsg) {
        ptxt = OSTxtCopy(ptxt, pmsg);
    }
    if ((code)||(mode & 2)) {
        ptxt = OSTxtCopy(ptxt, " (");
        ptxt = OSHexPut(code, ptxt);
        if ((numb)||(mode & 2)) {
            ptxt = OSTxtCopy(ptxt, ", ");
            ptxt = OSIntPut(numb, ptxt);
        }
        ptxt = OSTxtCopy(ptxt, ")");
    }
    if (pnam) {
        if (!(mode & 4)) {
            ptxt = OSTxtCopy(ptxt, ": ");
        }
        ptxt = OSTxtCopy(ptxt, pnam);
    }
    if ((ptxt <= bufr)||(*(ptxt-1) != '\n')) {  //linefeed at end if needed
        ptxt = OSTxtCopy(ptxt, OS_ENDLINES);    //always use Windows standard
    }                                           //Linux usually is just newline
    nb = (OSTxtSize(bufr) - 1) * SZCHR;

    #ifdef ISWIN                                //===
    ret = WriteFile(file, bufr, nb, &done, NULL);
    CloseHandle(file);                          //write log line
    if (ret == FALSE) return(-1);
    #else                                       //===
    ret = write(file, bufr, nb);
    close(file);
    if (ret == -1) return(-1);
    #endif                                      //===

    return(0);
}

//*************************************************************************
//  OSOpen:
//      Opens a disk file.
//      Also see StmFile file handling class below.
//
//  Arguments:
//      aint OSOpen (int mode, text* pfilen)
//      mode    0 = Open for reading (others can read)             (OS_READ)
//              1 = Open for shared reading and writing (unlocked) (OS_SHARE)
//              2 = Open exclusively so only we can write and read (OS_WRITE)
//              3 = Open exclusively or create if does not exist   (OS_WRITE1)
//              4 = Create new file (exclusive mode assumed)       (OS_CREATE)
//
//           +256 = Create as read-only file                       (OS_NOWRT)
//           +512 = Create as non-executable file (read and write) (OS_NOEXE)
//           +768 = Truncate old file to initial length to rewrite (OS_TRUNC)
//                  Instead of both OS_WRITE | OS_TRUNC flags use: (OS_REWRITE)
//              Settings beginning with OS_NOWRT are mutually exclusive.
//
//      pfilen  Pointer to path and file name to open.
//              Either Ascii or UTF8 and is converted to Windows wide.
//              Must be SZPATH maximum characters.
//
//  Returns:
//      Opened file handle to be used with OSRead, OSWrite, OSSeek, OSClose.
//      Returns -1 if file error.
//      Returns -2 if UTF8 to Wide conversion error.
//*************************************************************************

CSTYLE aint xproc
OSOpen (int mode, text* pfilen) {
    #ifdef ISWIN                                //=== Windows
    dword access, share, flags;
    hand file;

    #ifndef CNWIDE                              //---
    wtxt wide[SZPATH];
    int num = OSWideUTF8(OS_2WIDE, wide, pfilen, -1, SZPATH);
    if (num <= 0) return(-2);                   //convert to wide chars
    #endif                                      //---

    if (mode & OS_WRITE+OS_CREATE) {
        access = GENERIC_READ | GENERIC_WRITE;  //open for exclusive writing
        share =  0;
    } else {
        access = GENERIC_READ;                  //or shared reading
        if (mode & OS_SHARE) access = GENERIC_READ | GENERIC_WRITE;
        share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
    flags = FILE_ATTRIBUTE_NORMAL;

    if (mode & OS_CREATE) {                      //create file if needed
        if ((mode & OS_MASKS) == OS_NOWRT) {
            flags |= FILE_READ_ONLY;
        }
        #ifndef CNWIDE                          //---
        file = CreateFileW((LPCWSTR)wide, access, share, NULL,
                           CREATE_ALWAYS, flags, NULL);
        #else                                   //---
        file = CreateFileA(pfilen, access, share, NULL,
                           CREATE_ALWAYS, flags, NULL);
        #endif                                  //---
        if (file == INVALID_HANDLE_VALUE) return(-1);
        return((aint)file);

    } else {                                    //open disk file
        #ifndef CNWIDE                          //---
        file = CreateFileW((LPCWSTR)wide, access, share, NULL,
                           OPEN_EXISTING, flags, NULL);
        #else                                   //---
        file = CreateFileA((pfilen, access, share, NULL,
                           OPEN_EXISTING, flags, NULL);
        #endif                                  //---
        if (file == INVALID_HANDLE_VALUE) {
            if ((mode & OS_RWMASK) == OS_WRITE1) {
                return( OSOpen((mode | OS_CREATE), pfilen) );
            }                                   //try creating
            return(-1);
        }

        if ((mode & OS_MASKS) == OS_TRUNC) {
            SetEndOfFile(file);
        }
        return((aint)file);
    }

    #else                                       //=== Linux
    int file, ok;

    if (mode & OS_CREATE) {
        ok = S_IRWXU | S_IRWXG | S_IRWXO;
        if        ((mode & OS_MASKS) == OS_NOWRT) {
            ok = S_IRUSR | S_IRGRP | S_IROTH;   //read only?
        } else if ((mode & OS_MASKS) == OS_NOEXE) {
            ok = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        }                                       //or not executable?
        file = open(pfilen, (O_CREAT|O_RDWR), ok);
        if (file == -1) return(-1);             //create and
        ok = flock(file, (LOCK_EX | LOCK_NB));  //lock for exclusive writing
        if (ok != -1) ftruncate(file, 0);       //Linux does not chop by default

    } else if (mode & OS_WRITE) {
        ok = O_RDWR;
        if ((mode & OS_MASKS) == OS_TRUNC) {
            ok = O_RDWR | O_TRUNC;
        }
        file = open(pfilen, ok);                //open for writing
        if (file == -1) {                       //cannot open?
            if ((mode & OS_RWMASK) == OS_WRITE1) {
                return( OSOpen((mode | OS_CREATE), pfilen) );
            }                                   //try creating
            return(-1);
        }
        ok = flock(file, (LOCK_EX | LOCK_NB));  //lock for exclusive writing

    } else if (mode & OS_SHARE) {
        ok = O_RDWR;
        if ((mode & OS_MASKS) == OS_TRUNC) {
            ok = O_RDWR | O_TRUNC;
        }
        file = open(pfilen, ok);                //shared reading and writing
        if (file == -1) return(-1);

    } else {
        file = open(pfilen, O_RDONLY);
        if (file == -1) return(-1);
        ok = flock(file, (LOCK_SH | LOCK_NB));  //or for shared reading
    }       
    if (ok == -1) {                             //could not lock the file?
        close(file);
        return(-1);
    }
    return((aint)file);
    #endif                                      //===
}

//*************************************************************************
//  OSSeek:
//      Seek to new location in file.
//      Beware that files can be larger than 2^32 bytes!
//      This works with huge files but only returns the position low dword.
//
//  Arguments:
//      lint OSSeek (aint file, lint move, int org)
//      file    File handle from OSOpen.
//      move    Position to move to relative to org.
//      org     OS_SET = 0 if move is offset from beginning of file.
//              OS_CUR = 1 if move is offset from current file pos.
//              OS_END = 2 if move is offset from end of file.
//
//  Returns:
//      New file position (low dword if huge file) or -1 if error.
//*************************************************************************

CSTYLE lint xproc
OSSeek (aint file, lint move, int org) {
    #ifdef ISWIN                                //=== Windows
    dword whence, off;

    if      (org == OS_SET) whence = FILE_BEGIN;
    else if (org == OS_CUR) whence = FILE_CURRENT;
    else if (org == OS_END) whence = FILE_END;
    else return(-1);

    off = SetFilePointer((HANDLE)file, move, NULL, whence);
    if (off == EMASK) {
        return(-1);
    }
    return(off);

    #else                                       //=== Linux
    int whence;
    off_t newoff;

    if      (org == OS_SET) whence = SEEK_SET;
    else if (org == OS_CUR) whence = SEEK_CUR;
    else if (org == OS_END) whence = SEEK_END;
    else return(-1);

    newoff = lseek((int)file, (off_t)move, whence);
    return(newoff);
    #endif                                      //===
}

//*************************************************************************
//  OSHuge:
//      Same as OSSeek but works with huge files exceeding 4GB.
//      Seek to new location in file.
//
//  Arguments:
//      qint OSHuge (aint file, qint move, int org)
//      file    File handle from OSOpen.
//      move    Position to move to relative to org.
//      org     OS_SET = 0 if move is offset from beginning of file.
//              OS_CUR = 1 if move is offset from current file pos.
//              OS_END = 2 if move is offset from end of file.
//              These definitions must match Seek in TriStm.cpp.
//
//  Returns:
//      New file position or -1 if error.
//*************************************************************************

CSTYLE qint xproc
OSHuge (aint file, qint move, int org) {
    #ifdef ISWIN                                //=== Windows
    dword whence;
    QDW qdw;

    if      (org == OS_SET) whence = FILE_BEGIN;
    else if (org == OS_CUR) whence = FILE_CURRENT;
    else if (org == OS_END) whence = FILE_END;
    else return(-1);

    qdw.qw = (qword) move;
    qdw.lo = SetFilePointer((HANDLE)file, qdw.lo, (PLONG)&qdw.hi, whence);
    if (qdw.lo == EMASK) {
        if (OSLastError(0) != 0) {
            return(-1);
        }
    }
    return((qint)qdw.qw);

    #else                                       //=== Linux
    int whence;
    off_t newoff;

    if      (org == OS_SET) whence = SEEK_SET;
    else if (org == OS_CUR) whence = SEEK_CUR;
    else if (org == OS_END) whence = SEEK_END;
    else return(-1);

    newoff = lseek((int)file, (off_t)move, whence);
    return((qint)newoff);
    #endif                                      //===
}

//*************************************************************************
//  OSRead:
//      Reads from a file.
//
//  Arguments:
//      lint OSRead (aint file, void* pv, dword cb)
//      file    File handle from OSOpen.
//      pv      Pointer to memory buffer to read to.
//      cb      Number of bytes to read.
//
//  Returns:
//      Number of bytes read or -1 if error.
//*************************************************************************

CSTYLE lint xproc
OSRead (aint file, void* pv, dword cb) {
    #ifdef ISWIN                                //=== Windows
    dword done;
    int ok;

    ok = ReadFile((HANDLE)file, pv, cb, &done, NULL);
    return((ok) ? done : -1);

    #else                                       //=== Linux
    lint done;

    done = (lint) read((int)file, pv, cb);
    return((done >= 0) ? done : -1);
    #endif                                      //===
}

//*************************************************************************
//  OSWrite:
//      Writes to file.
//
//  Arguments:
//      lint OSWrite (aint file, const void* pv, dword cb)
//      file    File handle from OSOpen.
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//
//  Returns:
//      Number of bytes written or -1 if error.
//*************************************************************************

CSTYLE lint xproc
OSWrite (aint file, void* pv, dword cb) {
    #ifdef ISWIN                                //=== Windows
    dword done;
    int ok;

    ok = WriteFile((HANDLE)file, pv, cb, &done, NULL);
    return((ok) ? done : -1);

    #else                                       //=== Linux
    lint done;

    done = (lint) write((int)file, pv, cb);
    return((done >= 0) ? done : -1);
    #endif                                      //===
}

//*************************************************************************
//  OSClose:
//      Closes an open file.
//
//  Arguments:
//      int OSClose (aint file)
//      file    File handle from OSOpen.
//
//  Returns:
//      0 for success or -1 for error.
//*************************************************************************

CSTYLE int xproc
OSClose (aint file) {
    int ok;

    #ifdef ISWIN                                //=== Windows
    if ((HANDLE)file == INVALID_HANDLE_VALUE) return(0);//already closed?
    ok = CloseHandle((HANDLE)file);
    return((ok) ? 0 : -1);

    #else                                       //=== Linux
    if (file == -1) return(0);                  //already closed?
    flock((int)file, (LOCK_UN | LOCK_NB));      //unlock exclusive or read
    ok = close((int)file);                      //and close
    return(ok);
    #endif                                      //===
}

//*************************************************************************
//  OSSync:
//      Used to flush cached file buffers to disk.
//
//  Arguments:
//      int OSSync (aint file)
//      file    File handle from OSOpen.
//
//  Returns:
//      0 for success or -1 for error.
//*************************************************************************

CSTYLE int xproc
OSSync (aint file) {
    #ifdef ISWIN                                //=== Windows
    int ok;
    ok = FlushFileBuffers((HANDLE)file);
    return((ok) ? 0 : -1);

    #else                                       //=== Linux
    int rv;
    rv = fsync((int)file);
    return(rv);
    #endif                                      //===
}

//*************************************************************************
//  OSChop:
//      Truncates a disk file at the current seek position.
//      Deletes the rest of the file after the current seek point.
//
//  Arguments:
//      errc OSChop (aint file)
//      file    File handle from OSOpen.
//
//  Returns:
//      0 for success or -1 for error.
//*************************************************************************

CSTYLE int xproc
OSChop (aint file) {
    int ok;

    #ifdef ISWIN                                //=== Windows
    ok = SetEndOfFile((HANDLE)file);            //truncate disk file
    return((ok) ? 0 : -1);

    #else                                       //=== Linux
    off_t pos;

    pos = lseek((int)file, 0, SEEK_CUR);
    ok = ftruncate((aint)file, pos);
    return(ok);
    #endif                                      //===
}

//*************************************************************************
//  OSErase:
//      Erase a closed file.
//
//  Arguments:
//      int OSErase (text* pfilen)
//      pfilen  Pointer to path and file name text for file to erase.
//              Filenames are absolute and cannot be relative to current dir.
//
//  Returns:
//      TRUE normally or FALSE if error.
//*************************************************************************

CSTYLE int xproc
OSErase (text* pfilen) {
    int ok;

    #ifdef ISWIN                                //=== Windows
    #ifndef CNWIDE                              //---
    wtxt wide[SZPATH];
    int num = OSWideUTF8(OS_2WIDE, wide, pfilen, -1, SZPATH);
    if (num <= 0) return(FALSE);                //convert to wide chars
    ok = DeleteFileW((LPCWSTR)wide);
    #else                                       //---
    ok = DeleteFileA(pfilen);
    #endif                                      //---
    return(ok);

    #else                                       //=== Linux
    ok = unlink(pfilen);
    return((ok == -1) ? FALSE : TRUE);
    #endif                                      //===
}

//*************************************************************************
//  OSRename:
//      Rename a disk file.
//
//  Arguments:
//      int OSRename (text* poldn, text* pnewn)
//      poldn   Pointer to path and file name of exiting file to be renamed.
//              Filenames are absolute and cannot be relative to current dir.
//      pnewn   Pointer to new file name.
//              Filenames are absolute and cannot be relative to current dir.
//              This may be in a different directory of the same disk volume.
//              This file may not already exist. 
//
//  Returns:
//      TRUE normally or FALSE if error.
//*************************************************************************

CSTYLE int xproc
OSRename (text* poldn, text* pnewn) {
    int ok;

    #ifdef ISWIN                                //=== Windows
    #ifndef CNWIDE                              //---
    wtxt wold[SZPATH];
    wtxt wnew[SZPATH];
    int num;
    num = OSWideUTF8(OS_2WIDE, wold, poldn, -1, SZPATH);
    if (num <= 0) return(FALSE);                //convert to wide chars
    num = OSWideUTF8(OS_2WIDE, wnew, pnewn, -1, SZPATH);
    if (num <= 0) return(FALSE);
    ok = MoveFileExW((LPCWSTR)wold, (LPCWSTR)wnew, MOVEFILE_COPY_ALLOWED);
    #else                                       //---
    ok = MoveFileExA(poldn, pnewn, MOVEFILE_COPY_ALLOWED);
    #endif                                      //---
    return(ok);

    #else                                       //=== Linux
    ok = rename(poldn, pnewn);
    return((ok == -1) ? FALSE : TRUE);
    #endif                                      //===
}

//*************************************************************************
//  OSDirNew:
//      Creates a new directory.
//      Can create multiple directories along given path.
//
//  Arguments:
//      int OSDirNew (int mode, text* pdir)
//      mode    0 = Just create directory at end of path if needed.
//             +1 = Create all directories in pdir path if needed.    (OS_PATH)
//                  This returns TRUENEG only if ending didn't exist.
//      pdir    Pointer to path and directory name such as "C:\Temp\SubDir".
//              Path names are absolute and cannot be relative to current dir.
//              May optionally have ending backslash as in "C:\Temp\SubDir\".
//
//  Returns:
//      TRUE if newly created, TRUENEG if already existed, or FALSE if error.
//*************************************************************************

CSTYLE int xproc
OSDirNew (int mode, text* pdir) {
    text dir[SZPATH];
    text* ptxt;
    int ok;

// Make sure all directories along the path exist.

    if (mode & OS_PATH) {                       //create all dirs on the path?
        ptxt = pdir;
        if (  ((*ptxt == '\\')&&(*(ptxt+1) == '\\'))
            ||((*ptxt == '/' )&&(*(ptxt+1) == '/' ))  ) {
            ptxt += 2;                          //skip over //server/
            while ((*ptxt)&&(*ptxt != '\\')&&(*ptxt != '/')) ptxt += 1;
        }
        if ((*ptxt)&&(*(ptxt+1) == ':')) {
            ptxt += 2;                          //skip over C:
        }
        if ((*ptxt == '\\')||(*ptxt == '/')) {
            ptxt += 1;                          //skip over root slash
        }
        ok = O32(ptxt - pdir);
        ptxt = dir;
        while (ok--) {
            *ptxt++ = *pdir++;                  //copy up to first directory
        }
        *ptxt = 0;

        ok = TRUENEG;                           //assume nothing to create
        while (*pdir) {                         //go through path directories
            while ((*pdir)&&(*pdir != '/')&&(*pdir != '\\')) {
                *ptxt++ = *pdir++;              //add directory name to path
            }
            if ((*pdir == '/')||(*pdir == '\\')) {
                *ptxt++ = *pdir++;              //add ending slash
            }
            *ptxt = 0;

            ok = OSDirNew(0, dir);              //recurse to create directory
            if (ok == FALSE) return(FALSE);
        }
        return(ok);                             //finished
    }

// Create single directory at the end of the path.

    #ifdef ISWIN                                //=== Windows
    SECURITY_ATTRIBUTES secure;

    ptxt = OSTxtStop(dir, pdir, SZPATH);        //copy so adding slash is ok
    NO_ENDSLASH(dir, ptxt);                     //get rid of ending backslash

    OSMemClear(&secure, sizeof(secure));        //default security descriptor
    secure.nLength = sizeof(secure);
    secure.lpSecurityDescriptor = NULL;
    secure.bInheritHandle = FALSE;

    #ifndef CNWIDE                              //---
    wtxt wide[SZPATH];
    int num = OSWideUTF8(OS_2WIDE, wide, pdir, -1, SZPATH);
    if (num <= 0) return(FALSE);                //convert to wide chars
    ok = CreateDirectoryW((LPCWSTR)wide, &secure);
    #else                                       //---
    ok = CreateDirectoryA(pdir, &secure);       //create the directory
    #endif                                      //---
    if (ok) return(TRUE);
    return((GetLastError() == ERROR_ALREADY_EXISTS) ? TRUENEG : FALSE);

    #else                                       //=== Linux
    struct stat info;

    ptxt = OSTxtStop(dir, pdir, SZPATH);        //copy so adding slash is ok
    NO_ENDSLASH(dir, ptxt);                     //get rid of ending backslash
    ok = stat(dir, &info);                      //get infomation about the file
    if ((ok != -1)&&(info.st_mode & S_IFDIR)) {
        return(TRUENEG);                        //directory already exists?
    }
    ok = mkdir(dir, (S_IRWXU|S_IRWXG|S_IRWXO)); //create the directory
    if (ok == -1) return(FALSE);
    return(TRUE);
    #endif                                      //===
}

//*************************************************************************
//  OSRegIni:
//      Handles reading and setting values in a text *.ini file like this:
//      This is derived from ComReg in another Triscape project.
//
//        # Ignores comment lines beginning with pound sign.
//        [SECTION1]
//        KEY1=value1
//        KEY2=value2
//        # This second section has independent keys.
//        [SECTION2]
//        KEY3=value3
//        KEY1=value4
//
//      It is also possible to use *.ini files with one unnamed section:
//
//        # Traditional Unix config file without sections
//        KEY1=value1
//        KEY2=value2
//
//      Section and key names are upper and lower case independent.
//      These are similar to Windows INI files.
//
//  Arguments:
//      errc OSRegIni (int mode, IReg* preg, text* psec, text* pkey, text* pval)
//      mode        Sum of various CREG_? flags as explained below.
//
//      CREG_LOAD = Load pval .ini filename and return preg memory.  (CREG_LOAD)
//                  Opens file for reading and closes immedately.
//                  Can optionally start new .ini in memory
//                  for later saving using NULL for pval.
//                  Ignores psec and pkey which should be NULL.
//                  Must call CREG_FREE mode to close.
//      CREG_OPEN = Open pval .ini filename and return preg memory.  (CREG_OPEN)
//                  Opens for writing and keeps locked open
//                  with open file handle returned in preg->ipFil.
//                  Ignores psec and pkey which should be NULL.
//                  Must call CREG_FREE mode to close.
//      CREG_SAVE = Save .ini filename from preg memory.             (CREG_SAVE)
//                  Saves to CREG_OPEN file ifpreg->ipFil if NULL.
//                  Otherwise creates pval filename and saves.
//                  No save if used CREG_LOAD and psav is NULL.
//                  Ignores psec and pkey which should be NULL.
//                  Normally combine +CREG_FREE to with CREG_CLOSE.
//      CREG_FREE = Close preg file and free memory if needed.       (CREG_FREE)
//                  Must be called after CREG_LOAD or CREG_OPEN.
//                  CREG_CLOSE includes CREG_SAVE and CREG_FREE.
//     CREG_CLOSE = Normally use CREG_CLOSE to both save and free.  (CREG_CLOSE)
//
//       CREG_SET = Set or add pkey = pval existing value in psec.    (CREG_SET)
//                  If no existing pkey value inserts a new
//                  value to beginning of the given psec.
//                  This is used to set modifiy an old value.
//                  The same as using CREG_GET, CREG_DEL, CREG_ADD
//                  and then using CREG_CSEC1 then CREG_ADD if new.
//                  This is an easier version of CREG_ADD below.
//
//       CREG_GET = Get pkey in psec and return value in pval.        (CREG_GET)
//                  Requires psec and pkey point to desired names.
//                  If psec is NULL assumes one unnamed section.
//                  Returns the corresponding pval value.
//                  The pval buffer must be SZDISK characters.
//                  If +CREG_HUGE mode, then pval is SZHUGE.
//                  If +CREG_SIZE just returns desired buffer size.
//                  Found line becomes the new current line.
//                  Returns ECNOMATCH and *pval = 0 if cannot find.
//
//       CREG_ADD = Add pkey = pval as new line.                      CREG_ADD)
//                  If pkey is blank then just adds pval as line.
//                  Must use after CREG_GET,SEC1,NEXT,ADD,DEL,NEW
//                  which determine the [section] and place to add.
//                  Ignores psec which should be NULL.
//                  Adds line after previous current line.
//                  Or if +CREG_BFOR adds line before current one.
//                  Added line becomes the new current line.
//                  It is often easier to use CREG_SET.
//
//      CREG_SEC1 = Find psec and prepare to use CREG_NEXT.          (CREG_SEC1)
//                  Does not return any value just preg->ipCur.
//                  The ipkey and ipval args should be NULL.
//                  May use +CREG_MORE to combine with CREG_ADD.
//                  Returns ECNOMATCH if cannot find.
//      CREG_NEXT = Return next pkey and pval in section.            (CREG_NEXT)
//                  Returns any text before the = in pkey.
//                  Returns any text after the = in pval.
//                  If there is no = then returns *pkey=0 and
//                  returns the whole line in pval with ECNOMATCH.
//                  Skips over # comment lines and blank lines.
//                  Can use after CREG_GET,SEC1,NEXT,ADD,DEL,NEW.
//                  Requires preg->ipCur for beginning of line
//                  which is set by CREG_GET,SEC1,NEXT,ADD,DEL,NEW.
//                  Returns both pkey and pval text values.
//                  The pkey buffer must be SZDISK characters
//                  The pval buffer must be SZDISK characters.
//                  If +CREG_HUGE mode, then pval size is unlimited.
//                  If +CREG_SIZE just returns desired buffer size.
//                  Ignores the psec arg which should be NULL.
//                  Returns ECNOMATCH and blanks if cannot find.
//
//       CREG_NEW = Add psec new [section] to end of file.           (CREG_NEW)
//                  Initially has no lines so use CREG_ADD next.
//                  Can add CREG_BFOR for blank line before.
//                  Ignores pkey and pval which should be NULL.
//                  End of file becomes the new current line.
//
//       CREG_DEL = Delete current line.                             (CREG_DEL)
//                  Can use after CREG_GET,SEC1,NEXT,ADD,DEL,NEW.
//                  Ignores psec, pkey, pval which should be NULL.
//                  The next line becomes the new current line.
//
//        +0x1000 = Add new line before current line and not after   (CREG_BFOR)
//                  when adding a new value with CREG_ADD.
//                  CREG_BFOR can also be added to CREG_NEW to
//                  put an extra blank line before the new section.
//        +0x2000 = Add with Windows CR and LF and not just newline. (CREG_CRLF)
//                  By default this uses Unix style just newlines.
//        +0x4000 = Assume unlimited buffer and not just SZDISK.     (CREG_HUGE)
//                  Can be use dwith CREG_GET and CREG_NEXT.
//        +0x8000 = Return postive pval byte size with ending zero   (CREG_SIZE)
//                  but does not set the value and pval can be NULL.
//                  Can be use dwith CREG_GET and CREG_NEXT.
//          +0x80 = Add to CREG_GET,SEC1,DEL to do more modes.       (CREG_MORE)
//
//      preg    Set up by CREG_LOAD or CREG_OPEN and must be
//              supplied to subsequent calls. Use CREG_SAVE to free.
//              The caller normally need not initialize or use this.
//              The ipBeg and ipSiz define in-memory .ini file image.
//              The ipCur is offset in ipBeg memory to current line.
//              The ipFil is open file handle for write locked .ini.
//
//      psec    Supplies the desired [SECTION] name without [] brackets.
//              Name matching is not case dependent.
//              For example "Servers" would match [SERVERS].
//              Use NULL for CREG_GET or SEC1 if just one unnamed section.
//              Names are normally limited to SZNAME=32 chars with end zero.
//              Used for CREG_GET, CREG_SEC1, and CREG_NEW.
//              Should be NULL for other modes.
//
//      pkey    Supplies or returns the desired KEY= name without the =.
//              Name matching is not case dependent.
//              For example "Win" would match WIN=value
//              Names are normally limited to SZNAME=32 chars with end zero.
//              Should be NULL for CREG_SEC1, CREG_NEW, CREG_DEL, CREG_LOAD.
//
//      pval    Supplies new zero-term value text for CREG_SET or CREG_ADD mode.
//              Returns the desired KEY=VALUE text value for CREG_GET, etc.
//              Get vals are normally limited to SZDISK=264 chars with end zero.
//              If CREG_SIZE mode flag then pval is ignored for CREG_GET, etc.
//              The CREG_HUGE mode flag can be added for unlimited buffer.
//              The CREG_LOAD, CREG_OPEN and CREG_SAVE modes uses a filename
//              which by convention ends with an .ini extension.
//              CREG_SAVE and CREG_CLOSE may use NULL to save to CREG_OPEN.
//              Not required for CREG_DEL and should be NULL.
//              Used as the file path name for CREG_LOAD.
//
//  Returns:
//      Non-zero code if error.
//      If CREG_GET+CREG_GHUGE or CREG_NEXT+CREG_HUGE returns positive
//      byte size required to hold returned pval string including ending 0
//      or returns negative code if error.
//*************************************************************************

CSTYLE int xproc
OSRegIni (int mode, IReg* preg, text* psec, text* pkey, text* pval) {
    text buf[SZPATH+SZNAME+SZTINY];
    text* ptxt;
    text* pbeg;
    text* pend;
    text* puse;
    text* plin;
    text* pall;
    int rv, crlf, issec, nosec, smode, again;
    lint nc, max;
    dword all;
    aint file;

    pall = preg->irBeg;
    ptxt = pall;
    pend = ptxt + preg->irSiz;
    again = FALSE;

    if (mode & (CREG_GET+CREG_SEC1)) {          //find value or section?
        if (pall == NULL) {                     //in case just created in mem
            if ((mode & CREG_GET)&&(pval)&&(!(mode & CREG_MORE))) *pval = 0;
            return(-1);
        }
        nosec = ((psec)&&(*psec)) ? FALSE : TRUE;
        issec = nosec;

        while (TRUE) {                          //come here for each line
            plin = ptxt;
            if (ptxt >= pend) {                 //end of file?
                if (mode & CREG_MORE) {
                    if (again) return(-1);
                    again = TRUE;               //if could not find
                    pall = preg->irBeg;         //then add to section
                    ptxt = pall;
                    pend = ptxt + preg->irSiz;
                    issec = nosec;
                    crlf = mode & CREG_CRLF;
                    if (nosec) {
                        mode = (CREG_ADD+CREG_BFOR) | crlf;
                        break;                  //add if no sections?
                    }
                    mode = (CREG_ADD+CREG_SEC1+CREG_MORE+CREG_BFOR) | crlf;
                    continue;
                }
                if ((mode & CREG_GET)&&(pval)) *pval = 0;
                return(-1);                     //could not find sec or key
            }
            while (  (ptxt < pend)&&((unsigned)*ptxt <= ' ')
                   &&(*ptxt != '\n')&&(*ptxt != '\r')  ) {
                ptxt += 1;                      //skip beginning blanks
            }
            if (*ptxt == '[') {                 //found [section] name?
                if (issec) {                    //end of desired section?
                    if (mode & CREG_MORE) {
                        if (again) return(-1);
                        again = TRUE;           //if could not find
                        pall = preg->irBeg;     //then add to section
                        ptxt = pall;
                        pend = ptxt + preg->irSiz;
                        issec = nosec;
                        crlf = mode & CREG_CRLF;
                        if (nosec) {
                            mode = (CREG_ADD+CREG_BFOR) | crlf;
                            break;
                        }
                        mode = (CREG_ADD+CREG_SEC1+CREG_MORE+CREG_BFOR) | crlf;
                        continue;
                    }
                    if (pval) *pval = 0;        //could not find value?
                    return(-1);                 //stop looking
                }
                ptxt += 1;
                while (  (ptxt < pend)&&((unsigned)*ptxt <= ' ')
                       &&(*ptxt != '\n')&&(*ptxt != '\r')  ) {
                    ptxt += 1;
                }
                pbeg = ptxt;
                while ((ptxt < pend)
                    &&(*ptxt != ']')
                    &&(*ptxt != '\n')
                    &&(*ptxt != '\r')  ) {
                    ptxt += 1;                  //find end of section name
                }
                while ((pbeg < ptxt)&&((unsigned)*(ptxt-1) <= ' ')) {
                    ptxt -= 1;                  //get rid of ending blanks
                }
                nc = PTROFF(pbeg, ptxt);
                if (nc >= SZDISK) nc = SZDISK-1;
                if (nc) OSMemCopy(buf, pbeg, nc);
                pbeg = ADDOFF(text*, buf, nc);
                *pbeg = 0;                      //get zero terminated version
                if (OSTxtCmp(psec, buf) == 0) { //found desired section name?
                    issec = TRUE;
                }
                while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                    ptxt += 1;                  //skip to end of line
                }

            } else if ((issec == FALSE)||(*ptxt == '#')||(pkey == NULL)) {
                while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                    ptxt += 1;                  //skip line in wrong section
                }                               //and skip line with #comment

            } else {                            //otherwise look for key=value
                pbeg = ptxt;                    //in matching section
                while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                    if (*ptxt != '=') {
                        ptxt += 1;              //keep looking for =
                        continue;
                    }                           //found =
                    puse = ptxt;
                    while ((pbeg < puse)&&((unsigned)*(puse-1) <= ' ')) {
                        puse -= 1;              //get rid of ending blanks
                    }
                    nc = PTROFF(pbeg, puse);
                    if (nc >= SZDISK) nc = SZDISK-1;
                    if (nc) OSMemCopy(buf, pbeg, nc);
                    pbeg = ADDOFF(text*, buf, nc);
                    *pbeg = 0;                  //get zero terminated key name
                    if (OSTxtCmp(pkey, buf) != 0) {
                        break;                  //key did not match?
                    }
                    ptxt += 1;                  //found matching key?
                    while (  (ptxt < pend)&&((unsigned)*ptxt <= ' ')
                           &&(*ptxt != '\n')&&(*ptxt != '\r')  ) {
                        ptxt += 1;              //skip any blanks before value
                    }
                    pbeg = ptxt;
                    while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                        ptxt += 1;              //skip to newline at end of val
                    }
                    preg->irCur = PTROFF(pall, plin);
                    if (mode & CREG_MORE) {     //continue to another mode?
                        again = TRUENEG;
                        break;
                    }
                    nc = PTROFF(pbeg, ptxt);
                    if (mode & CREG_SIZE) {
                        return(nc+1);           //just return required size?
                    }
                    if ((nc >= SZDISK)&&(!(mode & CREG_HUGE))) {
                        return(-2);             //will not fit?
                    }
                    if (nc) OSMemCopy(pval, pbeg, nc);
                    pbeg = ADDOFF(text*, pval, nc);
                    *pbeg = 0;                  //return zero terminated value
                    return(0);                  //return matching line too
                }
                if (again == TRUENEG) break;    //continue to another mode?
                while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                    ptxt += 1;                  //did not find matching key=
                }                               //so skip this line
            }                                   //skip end of line
            if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
            if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;

            if ((issec)&&(mode & CREG_SEC1)) {  //return first line of section?
                while ((*ptxt == '#')||(*ptxt == '\r')) {
                    while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')  ) { 
                        ptxt += 1;              //skip blank and # comment lines
                    }
                    if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
                    if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
                }
                preg->irCur = PTROFF(pall, ptxt);
                if (mode & CREG_MORE) break;    //continue to another mode?
                return(0);
            }
        }
    }

    if (mode & CREG_NEXT) {                     //find next line key and value
        ptxt = ADDOFF(text*, pall, preg->irCur);//get desired line pointer
        while ((ptxt < pend)&&((unsigned)*ptxt <= ' ')) {
            ptxt += 1;                          //skip blanks and blank lines
        }
        while (*ptxt == '#') {                  //ignore comment line?
            while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')  ) { 
                ptxt += 1;
            }
            if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
            if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
            while ((ptxt < pend)&&((unsigned)*ptxt <= ' ')) ptxt += 1;
        }
        plin = ptxt;                            //begin line after any blanks
        if (*ptxt == '[') {                     //ran into next section?
            *pkey = 0;
            if (pval) *pval = 0;
            return(-3);
        }

        pbeg = ptxt;
        puse = NULL;
        while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
            if (((unsigned)*ptxt <= ' ')&&(puse == NULL)) {
                puse = ptxt;                    //remember end of key if space
            }
            if (*ptxt != '=') {
                ptxt += 1;                      //keep looking for =
                continue;
            }
            if (puse == NULL) puse = ptxt;      //found =
            nc = PTROFF(pbeg, puse);
            if (nc >= SZDISK) nc = SZDISK-1;
            if (nc) OSMemCopy(pkey, pbeg, nc);
            pbeg = ADDOFF(text*, pkey, nc);
            *pbeg = 0;                          //return zero terminated key

            ptxt += 1;                          //skip to after the =
            while (  (ptxt < pend)&&((unsigned)*ptxt <= ' ')  
                   &&(*ptxt != '\n')&&(*ptxt != '\r')  ) {
                ptxt += 1;                      //and any blanks before value
            }
            pbeg = ptxt;
            while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                ptxt += 1;                      //find end of line
            }
            nc = PTROFF(pbeg, ptxt);
            if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
            if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
            preg->irCur = PTROFF(pall, ptxt);
            if (mode & CREG_SIZE) {
                return(nc+1);                   //just return required size?
            }
            if ((nc >= SZDISK)&&(!(mode & CREG_HUGE))) {
                return(ECNOROOM);               //will not fit?
            }
            if (nc) OSMemCopy(pval, pbeg, nc);
            pbeg = ADDOFF(text*, pval, nc);
            *pbeg = 0;                          //return zero terminated value

            return(0);                          //return next line too
        }

        *pkey = 0;                              //if no = found return blank key
        nc = PTROFF(plin, ptxt);
        if (nc) OSMemCopy(pval, plin, nc);      //and return whole line in pval
        pbeg = ADDOFF(text*, pval, nc);
        *pbeg = 0;
        if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
        if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
        preg->irCur = PTROFF(pall, ptxt);       //return end of section line
        return(-4);                             //no more lines in section?
    }

    if (mode & CREG_DEL) {                      //delete last found line?
        plin = ADDOFF(text*, pall, preg->irCur);
        ptxt = plin;                            //get desired line pointer
        while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
            ptxt += 1;                          //skip to end of line
        }
        if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
        if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
        max = PTROFF(plin, ptxt);
        nc = PTROFF(ptxt, pend);
        if (nc > 0) {
            OSMemCopy(plin, ptxt, nc);          //remove line
        }        
        preg->irSiz -= max;                     //irCur stays same for next line
        if (preg->irSiz < 0) preg->irSiz = 0;   //just in case
        if (!(mode & CREG_MORE)) {              //no continue to another mode?
            return(0);
        }
    }

    if (mode & CREG_NEW) {                      //add new section at end of file
        ptxt = buf;
        if (preg->irSiz) {
            puse = ADDOFF(text*, pall, (preg->irSiz - 1));
            if ((*puse != '\n')||(mode & CREG_BFOR)) { 
                if (mode & CREG_CRLF) *ptxt++ = '\r';
                *ptxt++ = '\n';                 //preceed with newline
            }                                   //in case last value had none
        }
        *ptxt++ = '[';
        ptxt = OSTxtStop(ptxt, psec, SZDISK);
        *ptxt++ = ']';
        if (mode & CREG_CRLF) *ptxt++ = '\r';
        *ptxt++ = '\n';
        *ptxt = 0;
        max = PTROFF(buf, ptxt);
        all = preg->irSiz + max;
        if (all > preg->irAll) {                //grow buffer?
            all = ((all + (1 + (IR_BLOCK-1))) >> IR_BSHFT) << IR_BSHFT;
            pall = (text*) OSAlloc(all);        //allocate more memory
            if (pall == NULL) return(-5);       //expand memory buffer for line
            if (preg->irBeg) {
                OSMemCopy(pall, preg->irBeg, preg->irSiz);
                OSFree(preg->irBeg);
            }
            preg->irBeg = pall;
            preg->irAll = all;
        }
        plin = ADDOFF(text*, preg->irBeg, preg->irSiz);
        OSMemCopy(plin, buf, max);              //don't bother to keep end 0
        preg->irSiz += max;
        preg->irCur = preg->irSiz;              //return end of line too
        return(0);
    }

    if (mode & CREG_ADD) {                      //add after last found line?
        ptxt = buf;                             //construct key=value line
        if ((pkey)&&(*pkey)) {
            ptxt = OSTxtCopy(ptxt, pkey);
            *ptxt++ = '=';
        }
        ptxt = OSTxtStop(ptxt, pval, SZDISK);
        if (mode & CREG_CRLF) *ptxt++ = '\r';
        *ptxt++ = '\n';
        *ptxt = 0;
        max = PTROFF(buf, ptxt);
        all = preg->irSiz + max;
        if (all > preg->irAll) {                //grow buffer?
            all = ((all + (1 + (IR_BLOCK-1))) >> IR_BSHFT) << IR_BSHFT;
            pall = (text*) OSAlloc(all);        //expand memory buffer for line
            if (pall == NULL) return(-5);
            if (preg->irBeg) {
                OSMemCopy(pall, preg->irBeg, preg->irSiz);
                OSFree(preg->irBeg);
            }
            pend = ADDOFF(text*, pall, preg->irSiz);
            preg->irBeg = pall;
            preg->irAll = all;
        }
        preg->irSiz += max;
        plin = ADDOFF(text*, preg->irBeg, preg->irCur);
        ptxt = plin;                            //get desired line pointer
        if (!(mode & CREG_BFOR)) {              //add after and not before?
            while ((ptxt < pend)&&((unsigned)*ptxt <= ' ')) ptxt += 1;
            if (*ptxt == '[') {                 //end of section?
                ptxt = plin;                    //add to end of section
            } else {
                while ((ptxt < pend)&&(*ptxt != '\n')&&(*ptxt != '\r')) {
                    ptxt += 1;                  //skip to end of old line
                }
                if ((ptxt < pend)&&(*ptxt == '\r')) ptxt += 1;
                if ((ptxt < pend)&&(*ptxt == '\n')) ptxt += 1;
            }
        }
        plin = ptxt;
        puse = ADDOFF(text*, plin, max);
        nc = PTROFF(plin, pend);
        if (nc > 0) {
            OSMemCopyR(puse, plin, nc);         //move rest down to make room
        }
        OSMemCopy(plin, buf, max);              //and install our new line
        preg->irCur = PTROFF(pall, plin);       //becomes new current line
        return(0);                              //don't bother to keep end 0
    }

    if (mode & (CREG_LOAD+CREG_OPEN)) {         //load from file
        OSMemClear(preg, sizeof(IReg));         //set up information
        preg->irFil = -1;
        if (pval == NULL) {
            return(0);                          //start without file?
        }
        smode = (mode & CREG_OPEN) ? OS_WRITE : OS_READ;
        file = OSOpen(smode, pval);             //open file
        if (file == -1) return(-6);
        max = OSSeek(file, 0, OS_END);          //get file size
        OSSeek(file, 0, OS_SET);                //back to beginning
        all = ((max + (1 + (IR_BLOCK-1))) >> IR_BSHFT) << IR_BSHFT;

        if (preg->irBeg) OSFree(preg->irBeg);
        pall = (text*) OSAlloc(all);            //allocate memory for file
        if (pall == NULL) {
            OSClose(file);
            return(-5);
        }
        preg->irBeg = pall;
        preg->irSiz = max;
        preg->irAll = all;

        nc = OSRead(file, pall, max);           //load into memory
        pall[nc] = 0;                           //zero terminate for debugging
        if (mode & CREG_LOAD) {
            OSClose(file);
        } else {
            preg->irFil = file;
        }
        if (nc < max) return(-7);
        return(0);
    }

    if (mode & CREG_SAVE) {                     //save to file
        file = preg->irFil;
        if (pval) {
            if (file != -1) OSClose(file);      //pval overrides old CREG_OPEN
            file = OSOpen(OS_CREATE, pval);     //create new file?
            if (file == -1) return(-8);
            preg->irFil = file;
        }

        rv = 0;
        pall = preg->irBeg;
        max = preg->irSiz;
        if ((pall)&&(max)) {
            nc = OSWrite(file, pall, max);      //save to file
            if ((dword)nc < preg->irSiz) rv = -9;
        }
        if (!(mode & CREG_FREE)) {              //do not also free?
            return(rv);
        }
    }

    if (mode & CREG_FREE) {                     //just close?
        file = preg->irFil;
        if (file != -1) {
            OSClose(file);
            preg->irFil = -1;
        }
        pall = preg->irBeg;
        if (pall) {
            OSFree(pall);
        }
        preg->irBeg = NULL;
        preg->irSiz = 0;
        preg->irAll = 0;
        preg->irCur = 0;
        preg->irFil = -1;
        return(0);
    }
    return(ECARGUMENT);                         //bad mode
}

//*************************************************************************
//  OSReg:
//      Portable string storage using Windows Registry or Linux INI File.
//
//  Arguments:
//      int OSReg (int mode, text* file, text* key, text* name, text* pvalue)
//      mode   0 = Get value.       (OS_READ)
//                 Text value returned in *pvalue.
//                 Blank string returned if error.
//             2 = Set new value.   (OS_WRITE)
//                 Text should not exceed SZDISK chars.
//      file    Path and filename of ini file (ends with .ini by convention).
//              This is currently only used in Linux version.
//      key     Name of subkey in HKEY_LOCAL_MACHIN (eg "Software\Triscape").
//              Uses only the last part in INI file (e.g. "Triscape").
//      name    Name of value within the key to be set.
//      pvalue  Text value to be assocated with key and name to put.
//              SZDISK buffer to receive text value if REG_GET.
//                  
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE int xproc
OSReg (int mode, text* pfile, text* pkey, text* pname, text* pvalue) {
    #ifdef ISWIN
    HKEY root;
    HKEY key;
    dword cnt;
    long fail;
    #else          
    text* ptxt;
    IReg ini;
    int rv;
    #endif

    if (mode & OS_WRITE) {                      //put?
        #ifdef ISWIN                            //--- Windows
        root = HKEY_LOCAL_MACHINE;
        fail = RegCreateKeyA(root, pkey, &key);
        if (fail) return(ECSYS);

        cnt = OSTxtSize(pvalue) - 1;
        fail = RegSetValueExA(key, pname, NULL, REG_SZ, (fbyte*)pvalue, cnt);
        RegCloseKey(key);
        if (fail) return(ECSYS);
        return(0);
        
        #else                                   //--- Linux
        OSMemClear(&ini, sizeof(ini));
        ptxt = pkey;                            //no error if no ini file yet
        while (*ptxt) {                         //use only end of registry path
            if (*ptxt == '\\') break;
            ptxt += 1;
        }
        ptxt = (*ptxt == '\\') ? (ptxt+1) : pkey;
        rv = OSRegIni(CREG_LOAD, &ini, NULL, NULL, pfile);
        if (rv) {
            rv = OSRegIni(CREG_NEW, &ini, ptxt, NULL, NULL);
            if (rv == 0) {
                rv = OSRegIni(CREG_ADD, &ini, ptxt, pname, pvalue);
            }
        } else {
            rv = OSRegIni(CREG_SET, &ini, ptxt, pname, pvalue);
        }
        if (rv == 0) {
            rv = OSRegIni(CREG_SAVE, &ini, NULL, NULL, pfile);
        }                                       //also save to registry
        OSRegIni(CREG_FREE, &ini, NULL, NULL, NULL);
        return(rv);
        #endif                                  //---

    } else {                                    //get?
        pvalue[0] = 0;                          //try reading from registry

        #ifdef ISWIN                            //--- Windows
        root = HKEY_LOCAL_MACHINE;
        fail = RegOpenKeyA(root, pkey, &key);
        if (fail) return(ECSYS);

        cnt = SZDISK;
        *pvalue = 0;
        fail = RegQueryValueExA(key, pname, NULL, NULL, (fbyte*)pvalue, &cnt);
        RegCloseKey(key);
        if (fail) return(ECSYS);
        return(0);

        #else                                   //--- Linux
        OSMemClear(&ini, sizeof(ini));          //try reading from file
        rv = OSRegIni(CREG_LOAD, &ini, NULL, NULL, pfile);
        if (rv == 0) {
            ptxt = pkey;                        //use only end of registry path
            while (*ptxt) {
                if (*ptxt == '\\') break;
                ptxt += 1;
            }
            ptxt = (*ptxt == '\\') ? (ptxt+1) : pkey;
            rv = OSRegIni(CREG_GET, &ini, ptxt, pname, pvalue);
            OSRegIni(CREG_FREE, &ini, NULL, NULL, NULL);
        }
        return(rv);
        #endif                                  //---
    }
}

//*************************************************************************
//  OSExists:
//      Works like OSFindFile except just tests one file without changing name.
//
//  Arguments:
//      int OSExists (int mode, text* pname)
//      mode    Reserved for future use and must be 0.
//      pname   Gives path and file name to check.
//              Filenames are absolute and cannot be relative to current dir.
//
//  Returns:
//      Returns 0 if the file exists.
//      Returns -1 if file does not exist.
//*************************************************************************

CSTYLE int xproc
OSExists (int mode, text* pname) {
    #ifdef ISWIN                                //=== Windows
    HANDLE hand;

    #ifndef CNWIDE                              //---
    WIN32_FIND_DATAW data;
    wtxt wide[SZPATH];
    int num = OSWideUTF8(OS_2WIDE, wide, pname, -1, SZPATH);
    if (num <= 0) return(-1);                   //convert to wide chars
    hand = FindFirstFileW((LPWSTR)wide, &data);
    #else                                       //---
    WIN32_FIND_DATAA data;
    hand = FindFirstFileA(pname, &data);
    #endif                                      //---
    if (hand == INVALID_HANDLE_VALUE) {
        return(-1);                             //does not exist
    }
    FindClose(hand);
    return(0);                                  //file exists

    #else                                       //=== Linux
    struct stat info;
    int rv;

    rv = stat(pname, &info);                    //get infomation about the file
    if ((rv == -1)||(!(info.st_mode & S_IFREG))) {
        return(-1);                             //does not exist
    }
    return(0);                                  //file exists
    #endif                                      //===
}

//*************************************************************************
//  OSFindFile:
//      Finds the first or next file matching a file specification.
//      Works with local disk files or over the Internet via FTP.
//
//  Arguments:
//      int OSFindFile (int mode, text* pname, FFHand* phand)
//      mode    1 = Find first file match the pname pattern.        (FF_FIRST)
//              2 = Find next matching file.                        (FF_NEXT)
//              0 = Done with the phand handle.                     (FF_DONE)
//      pname   Gives path and file name for mode 1 (may use * wildcards).
//              For example: C:\Dir\File*.* or C:\Dir\*.txt or C:\Dir\File.abc*
//              Filenames are absolute and cannot be relative to current dir.
//              We only allow limited wildcards in Linux implementation.
//              In Linux ? is not supported and there can be a most two * chars,
//              one at the end of the base name and one ending the extension.
//              The matching is case independent even under Linux.
//              Gives previously returned path (for directory) for mode 2.
//              On return for modes 1 and 2, gives the exact file name found.
//              On return the file name includes the full path.
//              Must point to a buffer of at least SZPATH characters for return.
//              Ignored for mode 0.
//      phand   Handle returned by mode 1 operation.
//              Must be supplied without modification for modes 2 and 0.
//              Structure returns info on the file found for modes 1 and 2.
//              The phand->ffAttrib returns: AT_DIRECTORY,AT_NORMAL,AT_READONLY
//              The other phand->ffAttrib flags are used for Windows only.
//              The ffCreate and ffModify return file date and times.
//              The ffNewms and ffModms return date and times in ms.
//              The ffBytes returns file byte size.
//
//  Returns:
//      Returns 0 and non-blank pname if match found for modes 1 and 2.
//      Non-zero code if error.
//      Returns -1 if no more files match the pattern.
//      Returns -2 if unanticipated system error.
//*************************************************************************

CSTYLE int xproc
OSFindFile (int mode, text* pname, FFHand* phand) {
    #ifdef ISWIN                                //=== Windows
    text dir[SZPATH];
    HANDLE hand;
    QDW quad;
    text* ptxt;
    int ok;

// Find first file.

    if (mode & FF_FIRST) {
        ptxt = pname;
        phand->ffAttrib = 0;

        #ifndef CNWIDE                          //---
        WIN32_FIND_DATAW data;
        wtxt wide[SZPATH];
        int num = OSWideUTF8(OS_2WIDE, wide, pname, -1, SZPATH);
        if (num <= 0) goto nomatch;             //convert to wide chars
        hand = FindFirstFileW((LPCWSTR)wide, &data);
        phand->ffHandle = (fvoid*) hand;
        if (hand == INVALID_HANDLE_VALUE) goto nomatch;
        quad.LowPart  = data.nFileSizeLow;
        quad.HighPart = data.nFileSizeHigh;
        phand->ffBytes = quad.QuadPart;
        phand->ffAttrib = data.dwFileAttributes;
        quad.LowPart  = data.ftCreationTime.dwLowDateTime;
        quad.HighPart = data.ftCreationTime.dwHighDateTime;
        phand->ffNewms = quad.qw / 10000;       //ms per 100ns
        quad.LowPart  = data.ftLastWriteTime.dwLowDateTime;
        quad.HighPart = data.ftLastWriteTime.dwHighDateTime;
        phand->ffModms = quad.qw / 10000;
        OSDateTime(NULL, &phand->ffCreate, (qdtime*)&data.ftCreationTime, OS_DTFILE);
        OSDateTime(NULL, &phand->ffModify, (qdtime*)&data.ftLastWriteTime, OS_DTFILE);
        OSFileDir(dir, pname);
        ptxt = OSTxtCopy(pname, dir);
        num = OSWideUTF8(OS_2UTF8, (wtxt*)data.cFileName, ptxt, -1, MAX_PATH);
        if (num <= 0) goto nomatch;

        #else                                   //---
        WIN32_FIND_DATAA data;
        hand = FindFirstFileA(ptxt, &data);
        phand->ffHandle = (fvoid*) hand;
        if (hand == INVALID_HANDLE_VALUE) goto nomatch;
        quad.LowPart  = data.nFileSizeLow;
        quad.HighPart = data.nFileSizeHigh;
        phand->ffBytes = quad.QuadPart;
        phand->ffAttrib = data.dwFileAttributes;
        quad.LowPart  = data.ftCreationTime.dwLowDateTime;
        quad.HighPart = data.ftCreationTime.dwHighDateTime;
        phand->ffNewms = quad.qw / 10000;       //ms per 100ns
        quad.LowPart  = data.ftLastWriteTime.dwLowDateTime;
        quad.HighPart = data.ftLastWriteTime.dwHighDateTime;
        phand->ffModms = quad.qw / 10000;
        OSDateTime(NULL, &phand->ffCreate, (qdtime*)&data.ftCreationTime, OS_DTFILE);
        OSDateTime(NULL, &phand->ffModify, (qdtime*)&data.ftLastWriteTime, OS_DTFILE);
        OSFileDir(dir, pname);
        ptxt = OSTxtCopy(pname, dir);
        OSTxtCopy(ptxt, data.cFileName);
        #endif                                  //---
        return(0);

// Find next file.

    } else if (mode & FF_NEXT) {
        if ((HANDLE)phand->ffHandle == INVALID_HANDLE_VALUE) goto nomatch;

        #ifndef CNWIDE                          //---
        WIN32_FIND_DATAW data;
        ok = FindNextFileW((HANDLE)phand->ffHandle, &data);
        if (ok == FALSE) goto nomatch;
        quad.LowPart  = data.nFileSizeLow;
        quad.HighPart = data.nFileSizeHigh;
        phand->ffBytes = quad.QuadPart;
        phand->ffAttrib = data.dwFileAttributes;
        quad.LowPart  = data.ftCreationTime.dwLowDateTime;
        quad.HighPart = data.ftCreationTime.dwHighDateTime;
        phand->ffNewms = quad.qw / 10000;       //ms per 100ns
        quad.LowPart  = data.ftLastWriteTime.dwLowDateTime;
        quad.HighPart = data.ftLastWriteTime.dwHighDateTime;
        phand->ffModms = quad.qw / 10000;
        OSDateTime(NULL, &phand->ffCreate, (qdtime*)&data.ftCreationTime, OS_DTFILE);
        OSDateTime(NULL, &phand->ffModify, (qdtime*)&data.ftLastWriteTime, OS_DTFILE);
        OSFileDir(dir, pname);
        ptxt = OSTxtCopy(pname, dir);
        int num = OSWideUTF8(OS_2UTF8, (wtxt*)data.cFileName, ptxt, -1, MAX_PATH);
        if (num <= 0) goto nomatch;

        #else                                   //---
        WIN32_FIND_DATAA data;
        ok = FindNextFileA((HANDLE)phand->ffHandle, &data);
        if (ok == FALSE) goto nomatch;
        quad.LowPart  = data.nFileSizeLow;
        quad.HighPart = data.nFileSizeHigh;
        phand->ffBytes = quad.QuadPart;
        phand->ffAttrib = data.dwFileAttributes;
        quad.LowPart  = data.ftCreationTime.dwLowDateTime;
        quad.HighPart = data.ftCreationTime.dwHighDateTime;
        phand->ffNewms = quad.qw / 10000;       //ms per 100ns
        quad.LowPart  = data.ftLastWriteTime.dwLowDateTime;
        quad.HighPart = data.ftLastWriteTime.dwHighDateTime;
        phand->ffModms = quad.qw / 10000;
        OSDateTime(NULL, &phand->ffCreate, (qdtime*)&data.ftCreationTime, OS_DTFILE);
        OSDateTime(NULL, &phand->ffModify, (qdtime*)&data.ftLastWriteTime, OS_DTFILE);
        OSFileDir(dir, pname);
        ptxt = OSTxtCopy(pname, dir);
        OSTxtCopy(ptxt, data.cFileName);
        #endif                                  //---
        return(0);

// Done finding files.

    } else {
        if ((HANDLE)phand->ffHandle == INVALID_HANDLE_VALUE) return(0);
        FindClose((HANDLE)phand->ffHandle);
        phand->ffHandle = (fvoid*) INVALID_HANDLE_VALUE;
        return(0);
    }

nomatch:
    *pname = 0;
    return(-1);

// **************
// Linux version.

    #else                                       //=== Linux
    text dir[SZPATH];
    text ext[SZPATH];
    text* ptxt;
    text* pend;
    DIR* pdir;
    struct dirent* pent;
    struct stat info;
    int num, ok;
        
    if (!(mode & (FF_FIRST+FF_NEXT))) {
        pdir = (DIR*) phand->ffHandle;          //close?
        if (pdir == NULL) return(0);
        closedir(pdir);
        phand->ffHandle = NULL;
        return(0);
    }
    if (mode & FF_FIRST) {                      //first?
        OSFileDir(dir, pname);
        pdir = opendir(dir);                    //open directory
        if (pdir == NULL) {
            *pname = 0;
            return(-1);
        }
        phand->ffHandle = pdir;
        phand->ffMatchFlg = 0;
        phand->ffMatchNam[0] = 0;
        phand->ffMatchExt[0] = 0;

        OSFileBase(phand->ffMatchNam, pname, OS_NOEXT);
        ptxt = phand->ffMatchNam;
        num = 255;
        do {                                    //set up for pattern matching
            if (*ptxt == 0) break;
            if (*ptxt == '*') {
                phand->ffMatchFlg |= 1;
                break;
            }
            ptxt += 1;
        } while (--num);
        *ptxt = 0;

        OSFileExt(ext, pname);
        ptxt = ext;
        if (*ptxt == '.') ptxt += 1;            //omit dot from extension
        OSTxtStop(phand->ffMatchExt, ptxt, 256);
        ptxt = phand->ffMatchExt;
        if (*ptxt) {
            OSTxtStop(phand->ffMatchExt, ptxt, 255);
            ptxt = phand->ffMatchExt;
            num = 255;
            do {
                if (*ptxt == 0) break;
                if (*ptxt == '*') {
                    phand->ffMatchFlg |= 2;
                    break;
                }
                ptxt += 1;
            } while (--num);
            *ptxt = 0;
        }
    }   

    pdir = (DIR*) phand->ffHandle;              //find next (modes 1 or 2)
    if (pdir == NULL) {
        *pname = 0;
        return(-1);
    }
    pend = dir + OSFileDir(dir, pname);
    while (TRUE) {
        pent = readdir(pdir);                   //get next directory entry
        if (pent == NULL) {
            if (errno == 0) break;
            *pname = 0;
            return(-1);
        }
        OSFileBase(ext, pent->d_name, OS_NOEXT);//get base name
        ok = OSTxtCmp(phand->ffMatchNam, ext);  //match base name
        if ((phand->ffMatchFlg & 1)&&(ok == -2)) {
            ok = 0;                             //matched base beginning ok?
        }
        if (ok == 0) {
            OSFileExt(ext, pent->d_name);       //if so, get and match extension
            ptxt = ext;                         //don't match dots so base.*
            if (*ptxt == '.') ptxt += 1;        //matches: base  base.  base.ext
            ok = OSTxtCmp(phand->ffMatchExt, ptxt);
            if ((phand->ffMatchFlg & 2)&&(ok == -2)) {
                ok = 0;                         //matched extension beginning?
            }
        }
        if (ok == 0) {                          //matched desired pattern?
            OSTxtStop(pend, pent->d_name, SZDISK);
            ok = stat(dir, &info);              //get information about the file
            if (ok == -1) {
                *pname = 0;
                return(-2);
            }
            phand->ffBytes = (qword) info.st_size;
            phand->ffAttrib = 0;
            if (info.st_mode & S_IFREG) {
                phand->ffAttrib |= AT_NORMAL;
                if ((info.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
                    phand->ffAttrib |= AT_READONLY;
                }
            } else if (info.st_mode & S_IFDIR) {
                phand->ffAttrib |= AT_DIRECTORY;
            } else {
                ok = TRUENEG;                   //no return if not file or dir
            }
        }
        if (ok == 0) {                          //only return files or directory
            phand->ffNewms = info.st_ctime * 1000;
            phand->ffModms = info.st_mtime * 1000;
            OSDateTime(NULL, &phand->ffCreate, (qdtime*)&info.st_ctime, 
                       (OS_DTFILE+OS_DTGMT));
            OSDateTime(NULL, &phand->ffModify, (qdtime*)&info.st_mtime,
                       (OS_DTFILE+OS_DTGMT));
            OSTxtCopy(pname, dir);              //return matching filename
            return(0);
        }
    }
    *pname = 0;
    return(-1);
    #endif                                      //===
}

//*************************************************************************
//  OSClipboard:
//      Reads (pastes) and writes (copies) clipboard text.
//      Currently works only for Windows.
//
//  Arguments:
//      int OSClipboard (int mode, text* ptxt, int ntxt, hand hwin)
//      mode    1 = Paste text from clipboard to *ptxt buffer.         OS_PASTE
//                  The ntxt is the max char size of the ptxt buffer.
//              2 = Copy text to clipboard from *ptxt,                 OS_COPY
//                  using ntxt as the string size including terminating zero,
//                  or ntxt = 0 automatically sizes to ptxt string.
//      ptxt    Text string to put to or get from clipboard.
//      ntxt    Size of ptxt string including terminating zero.
//              If zero then assumes BLKSIZE buffer for mode 1.
//              If zero then uses actual ptxt size for mode 2.
//              Must be at least 3 for mode 1 because returns -N if need more.
//      hwin    Window handle to use or NULL if none.
//
//  Returns:
//      Returns number of characters.
//      Returns -1 for clipboard error.
//      Returns -2 for memory allocation error.
//      Returns -N if ntxt buffer is not large enough and N chars are needed.
//*************************************************************************

CSTYLE int xproc
OSClipboard(int mode, text* ptxt, int ntxt, hand hwin) {
    #ifdef ISWIN                                //=== Windows
    HGLOBAL mem;
    HANDLE clip;
    int rv;

    HWND desk = (hwin) ? ((HWND)hwin) : GetDesktopWindow();
    rv = OpenClipboard(desk);
    if (rv == 0) return(-1);

    if (mode & OS_COPY) {                       //put text on clipboard?
        if (ntxt <= 0) ntxt = OSTxtSize(ptxt);  //handle utf8 unicode
        #ifndef CNWIDE                          //---
        mem = GlobalAlloc(GMEM_MOVEABLE, (ntxt*2)+4);
        if (mem == NULL) {                      //assume nwide <= 2*chars
            CloseClipboard();
            return(-2);
        }
        wtxt* puni = (wtxt*) GlobalLock(mem);
        if (puni == NULL) {
            CloseClipboard();
            return(-2);
        }
        int num = OSWideUTF8(OS_2WIDE, puni, ptxt, -1, ntxt);
        if (num <= 0) *ptxt = 0;                //convert from wide chars
        rv = EmptyClipboard();
        GlobalUnlock(mem);
        clip = SetClipboardData(CF_UNICODETEXT, mem);
        CloseClipboard();
        GlobalFree(mem);
        if (clip == NULL) return(-1);
        CloseClipboard();
        return(num-1);
        #else                                   //---
        rv = EmptyClipboard();                  //handle ascii
        OSMemCopy(GlobalLock(mem), ptxt, ntxt); //copy caller's text to clipboard
        GlobalUnlock(mem);
        clip = SetClipboardData(CF_TEXT, mem);
        CloseClipboard();
        GlobalFree(mem);
        if (clip == NULL) return(-1);
        CloseClipboard();
        return(ntxt);
        #endif                                  //---

    } else {                                    //get text from clipboard?
        #ifndef CNWIDE                          //---
        clip = GetClipboardData(CF_UNICODETEXT);//handle utf8 unicode
        if (clip == NULL) return(-1);
        mem = GlobalLock(clip);
        if (mem == NULL) return(-2);
        wtxt* puni = (wtxt*) mem;
        wtxt* puse = puni;
        if (ntxt <= 0) ntxt = BLKSIZE;
        int num = OSWideUTF8(OS_2UTF8, puni, ptxt, -1, ntxt);
        if (num <= 0) *ptxt = 0;                //convert from wide chars
        CloseClipboard();
        return(num - 1);                        //return size w/o end zero
        #else                                   //---
        clip = GetClipboardData(CF_TEXT);       //handle ascii
        if (clip == NULL) return(-1);
        mem = GlobalLock(clip);
        if (mem == NULL) return(-2);
        text* puse = (char*) mem;
        int need = OSTxtSize(puse);
        if (ntxt <= 0) ntxt = BLKSIZE;
        if (need > ntxt) return(-need);
        OSTxtCopy(ptxt, puse);                  //copy text to caller
        CloseClipboard();
        return(need - 1);                       //return size w/o end zero
        #endif                                  //---
    }
    
    # else                                      //=== Linux
    return(ECCANNOT);
    #endif                                      //===
}

//*************************************************************************
//  OSTemporary:
//      Gets a new filename which does not conflict with existing files.
//      Regardless of the name, this has nothing to do with streams.
//
//  Arguments:
//      int OSTemporary (int mode, ftext* pname, 
//                       text* pdir, text* pbase, text* pext)
//      mode    0 = Use "TRI" as beginning of file name if ppre is NULL.
//              1 = Use "LOG" as beginning of file name if ppre is NULL;
//             +2 = Only add a 2 digit hex number rather than 4.
//      pname   Where the unique path and file name is returned.
//              Provided buffer must be at least SZPATH characters.
//      pdir    Directory where file is to be placed.
//              For mode 2 must point to the base name to be used.
//              May be NULL to use the Window's temp directory.
//      pbase   Base name prefix used insrtead of "TRI" or "LOG".
//              May be NULL to use "TRI" or "LOG" depending on mode.
//      pext    File name extension to be used (including period).
//              May be NULL to use ".tmp" default.
//
//  Returns:
//      Returns 0 and non-blank pname if worked.
//      Returns -1 and a blank name if error.
//*************************************************************************

#define TEMP_TRIES 1024;                        //maximum file names tried

CSTYLE int xproc
OSTemporary (int mode, ftext* pname, text* pdir, text* pbase, text* pext) {
    text path[SZPATH];
    ftext* ptxt;
    ftext* pend;
    dword mask, num, max;
    errc ret;

    *pname = 0;                                 //in case of erro
    if (pbase == NULL) {
        pbase = (text*) ((mode & 1) ? "LOG" : "TRI");
    }
    mask = 0xFFFF;
    if (mode & 2) {
        mask = 0xFF;
    }
    if (pext == NULL) pext = ".tmp";            //force pext case for Linux
    if (pdir) {
        ptxt = OSTxtCopy(path, pdir);
    } else {
        ptxt = OSLocation(path, OS_LOCTMP);     //use temp or tmp directory
    }
    NO_ENDSLASH(path,ptxt);                     //get rid of ending slash

    pend = path + (OSTxtSize(path) - 1);
    *pend++ = OS_SLASH;
    num = OSTickCount() & mask;                 //trial hex number to add
    max = TEMP_TRIES;
    do {
        if ((max--) == 0) {
            *pname = 0;
            return(-1);                         //don't keep trying forever
        }
        ptxt = OSTxtCopy(pend, pbase);          //build trial file name
        ptxt = OSHexPut(num++, ptxt);           //increment number for next time
        OSTxtCopy(ptxt, pext);
        OSTxtCopy(pname, path);                 //return trial filename
        ret = OSExists(1, path);                //and see if already exists
    } while (ret == 0);                         //keep trying until find new one
    return(0);
}

//*************************************************************************
//  OSDiskDrive:
//      Finds next available disk drive root of given type.
//      Allows caller to look for files on mounted flash drives.
//      Not supported by Linux and returns -1.
//
//  Arguments:
//      int OSDiskDrive (int mode, text* proot)
//      mode    0 = Include all sorts of disk drives
//             +1 = Exclude fixed drives
//             +2 = Exclude removable drives
//             +4 = Exclude CD-ROM drives
//             +8 = Exclude ramdisk drives
//            +16 = Exclude network (remote) drives
//            +32 = Exclude floppy drives (A: and B:)
//            +64 = Do not check whether drive has disk ready
//           +128 = Exclude fixed drives with more than 5GB (pseudo removable)
//             30 = Just get fixed local drives
//             52 = Just get large local drives
//             61 = Just get removable devices     (OS_NOFIX)
//            188 = Just get probable flash drives (OS_FLASH)
//              Note that +128 must also specify +64 and must not specify +1!
//      proot   On entry gives last returned drive or blank if first.
//              On entry may give path based on last drive (like "C:\*.*").
//              On return gives path for drive (like "C:\") or blank if none.
//              Returns short zero-terminated string. Should be at least SZNAME.
//
//  Returns:
//      Non-zero code if nothing found (and blank *proot).
//*************************************************************************

CSTYLE int xproc
OSDiskDrive (int mode, text* proot) {
    #ifdef ISWIN                                //=== Windows
    text disk;
    dword list, mask;
    dword secs, pers, frees, useds;
    uint type;
    flx dv;
    int ok;

    disk = *proot;
    if ((disk == '\\')||(disk == '/')) disk = 0;//just in case
    if (disk) {                                 //start with drive after last
        disk = (CAPITAL(disk) - 'A') + 1;
        mask = 1 << disk;
    } else {                                    //start with A: drive?
        disk = 0;
        mask = 1;
    }
    list = GetLogicalDrives();
    if (mode & 32) list = list & (~(1+2));      //ignore A: and B:?
    while (disk < 32) {
        if (mask & list) {                      //disk is avaialble?
            proot[0] = disk + 'A';              //get path name for disk's root
            proot[1] = ':';
            proot[2] = '\\';
            proot[3] = 0;
            type = GetDriveTypeA(proot);        //get disk's type
            if (  ((!(mode &  1))&&(type == DRIVE_FIXED))
                ||((!(mode &  2))&&(type == DRIVE_REMOVABLE))
                ||((!(mode &  4))&&(type == DRIVE_CDROM))
                ||((!(mode &  8))&&(type == DRIVE_RAMDISK))
                ||((!(mode & 16))&&(type == DRIVE_REMOTE))  ) {
                if (mode & 64) return(0);       //found a disk of correct type?
                useds = 0;
                ok = GetDiskFreeSpaceA(proot, &secs, &pers, &frees, &useds);
                if ((ok)&&(useds)) {            //disk seems to be inserted?
                    if (!(mode & 128)) {        //not small "fixed" spec?
                        return(0);
                    } else if (type != DRIVE_FIXED) {
                        return(0);              //not fixed
                    } else if (disk > 3) {      //small "fixed" must not be C:
                        dv = (flx)useds * (flx)pers * (flx)secs;
                        if (dv < 5000000000.0) {
                            return(0);          //small "fixed" must be < 5GB
                        }
                    }
                }
            }
        }
        disk += 1;                              //try next disk letter
        mask = mask << 1;
    }
    *proot = 0;
    return(-2);

#else                                           //=== Linux
    return(-1);
#endif                                          //===
}

//*************************************************************************
//  OSDiskSpace:
//      Determines how much free disk space is available.
//
//  Arguments:
//      qword OSDiskSpace (int mode, ftext* pdisk)
//      mode    Reserved for future use and must be zero.
//      pdisk   Name of the root directory for the disk (eg "e:\" or \).
//              May be NULL to use C: for Windows or root for Linux.
//
//  Returns:
//      Returns the number of free bytes of disk space.
//      Returns zero if error.
//*************************************************************************

#ifdef ISWIN
typedef BOOL (WINAPI * PSPACEEX)(text* pdir, PULARGE_INTEGER pb, PULARGE_INTEGER pt, PULARGE_INTEGER pn);
#endif

CSTYLE qword xproc
OSDiskSpace (int mode, ftext* pdisk) {
    #ifdef ISWIN                                //=== Windows
    HMODULE hmod;
    PSPACEEX pfun;
    ULARGE_INTEGER lb, lt, ln;
    text disk[SZPATH];
    dword sectors, bytes, free, used;
    qword total;
    int ok;

    if (pdisk) OSFileRoot(disk, pdisk);         //get root name
          else OSTxtCopy(disk, "C:\\");

    hmod = LoadLibraryA("KERNEL32.dll");
    if (hmod) {
        pfun = (PSPACEEX) GetProcAddress(hmod, "GetDiskFreeSpaceExA");
        if (pfun) {
            ok = (*pfun)(disk, &lb, &lt, &ln);  //use new function if available
            if (ok) {
                FreeLibrary(hmod);
                return(lb.QuadPart);
            }
        }
        FreeLibrary(hmod);
    }

    ok = GetDiskFreeSpaceA(disk, &sectors, &bytes, &free, &used);
    if (ok == FALSE) return(0);
    total = (qword)free * (qword)bytes * (qword)sectors;
    return(total);

#else                                           //=== Linux
    struct statfs stats;
    qword total;
    int rv;

    if (pdisk == NULL) pdisk = "/";
    rv = statfs(pdisk, &stats);
    if (rv == -1) return(0);
    total = (qword)stats.f_bfree * (qword)stats.f_bsize;
    return(total);
#endif                                          //===
}

//*************************************************************************
//  OSComPort:
//      Handles COM Serial I/O.
//
//  Arguments:
//      errc OSComPort (int mode, int numb, byte* ptxt, ComInfo* pcom)
//      mode    0 = Close COM port                              (CIN_FINI)
//              1 = Open COM port #numb                         (CIN_OPEN)
//              2 = Read num bytes from open COM port           (CIN_READ)
//              4 = Write num bytes to open COM port            (CIN_SEND)
//          +0x10 = Open 9600 N-8-1                             (CIN_9600)
//                  Used only by CIN_OPEN mode.
//          +0x20 = Open 115200 N-8-1 (default if no baud set)  (CIN_115200)
//                  Used only by CIN_OPEN mode.
//          +0x40 = Open pcom->cin_baud N-8-1                   (CIN_BAUD)
//                  Supported: 110,300,600,1200,2400,4800,9600,
//                  14400,19200,38400,57600,115200.
//                  Used only by CIN_OPEN mode.
//          +0x80 = Allows data bits, parity and stop bits      (CIN_PLUS)
//                  add into high pcom->cin_baud above baud.
//                  Supported: C_7DAT,C_NPAR,C_OPAR,C_2STP
//                  Compatible with AT91C uart control but
//                  not all possible settings are supported
//                  Used only by CIN_OPEN mode.
//         +0x100 = Packetize reads by stopping where timed     (CIN_TIME)
//                  packets end. Must add CIN_TIME to CIN_OPEN
//                  so CIN_READ returns fewer bytes and stops
//                  at the end of a packet where the next byte
//                  received after packet arrived >5ms later
//                  Caller may set pcom->cin_time to ms timeout
//                  after CIN_OPEN returns or leave it at 5ms
//                  Used only by CIN_OPEN mode.
//                  Set CIN_TO too for >49ms and delay read returns.
//        +0x1000 = Allow c_callback to be used if not NULL     (CIN_CALL)
//                  Used only by CIN_OPEN mode.
//        +0x2000 = Do not use cin_lock read task mutex locking (CIN_NLOK)
//                  Used only by CIN_OPEN mode.
//        +0x4000 = Read as much as can and no zero terminate   (CIN_NZ)
//                  Used only by CIN_READ mode.
//        +0x8000 = Stop read on \n linefeed                    (CIN_LF)
//                  Used only by CIN_READ.
//       +0x10000 = Timeout before read return.                 (CIN_TO)
//                  Used only by CIN_OPEN.
//      numb    COM? port number for CIN_OPEN
//              Use 0 for Linux tty console
//              Use 1 for ttyO1, 2 for ttyO2, 3 for ttyO3 Linux UARTs.
//              Use -1 for ttyUSB0 Linux FTDI connection.
//              Bytes to read for CIN_READ or 0 to \n linefeed
//              Bytes to write if CIN_READ or 0 if zero terminated
//      ptxt    Buffer to read to or bytes to send
//              Ignored unless CIN_READ or CIN_SEND mode
//              CIN_READ must leave room for terminating zero byte
//      pcom    Pointer to ComInfo structure used for open port
//              If pcom->c_callback is not NULL then called with c_callparm
//              and a second info pointer to a zero terminated string for
//              each \n line or timeout, which happens instead of CIN_READ.
//              Callback limited to SZDISK-1 bytes passed in info string.
//              Callback must operate quickly and cannot CIN_READ.
//              CIN_READ buffer is flushed after callback.
//
//  Returns:
//      Negative code if error.
//      CIN_READ leaves caller's ptxt buffer unchanged if error.
//      CIN_READ mode returns ECPENDING if have not read all requested bytes yet.
//      Returns zero or positive number of bytes read or written.
//*************************************************************************

// *******
// ComTask: Task used by OSComPort to read from COM port without blocking.
// We allow out-of-band packet end informaton by having our c_in buffer bytes
// use an 0xFF "escape" followed by a second "command" byte which is either
// 0xFF for a real 0xFF byte or 0 for the "end of packet" out-of-band message.
// Currently CIN_ESC protocol is not used unless CIN_TIME was added to CIN_OPEN.

#define CIN_ESC 0xFF                            //buffer escape byte
#define CIN_END 0                               //end-of-packet escape command

OSTASK_RET OSComTask (void* parm) {
    text line[SZDISK];
    text* ptxt;
    byte* pins;
    byte* puse;
    lint time, last;
    dword did;
    byte cc;
    
    ComInfo* pcom = (ComInfo*) parm;
    if (pcom == NULL) return(OSTASK_0);
    last = OSTickCount();
    ptxt = line;

    while (TRUE) {
        did = OSRead(pcom->cin_hand, &cc, 1);   //wait and read one byte
        OSLock_GRAB(pcom->cin_lock);            //=== Grab Mutex
        if (did != 1) {
            pcom->cin_errs = ECREAD;
            OSLock_DONE(pcom->cin_lock);
            break;
        }
        pins = ADDOFF(byte*, pcom->c_in, pcom->cin_head);
        puse = pins++;
        if (pins == &pcom->c_in[CIN_INS]) {
            pins = pcom->c_in;                  //wrap around buffer
        }
        while (pins == &pcom->c_in[pcom->cin_tail]) {
            OSLock_DONE(pcom->cin_lock);
            OSSleep(10);                        //buffer full?
            OSLock_GRAB(pcom->cin_lock);
        }                                       //give other tasks a chance
        *puse = cc;                             //add byte we read to buffer
        if (pcom->c_callparm) {                 //keep copy string
            *ptxt++ = cc;                       //for callback
            if ((ptxt - line) >= SZDISK) ptxt -= 1;
        }
        if (pcom->cin_mode & CIN_TIME) {        //packetize?
            time = OSTickCount();
            if ((signed)(time - last) > (signed)pcom->cin_time) {
                cc = *puse;                     //over 5ms since last byte read?
                *puse = CIN_ESC;                //send out-of-band escape
                puse = pins++;
                *puse = CIN_END;                //send end-of-packet command
                if (pins == &pcom->c_in[CIN_INS]) {
                    pins = pcom->c_in;          //wrap around buffer
                }
                puse = pins++;
                *puse = cc;                     //and then the byte just read
                if (pins == &pcom->c_in[CIN_INS]) {
                    pins = pcom->c_in;          //wrap around buffer
                }
                if (pcom->c_callback) {         //callback when reach timeout?
                    *ptxt = 0;
                    ptxt = line;
                    (*pcom->c_callback) (pcom->c_callparm, line);
                    pcom->cin_have = 0;         //flush circular buffer after call
                    pcom->cin_tail = PTROFF(pcom->c_in, pins);
                }
            }
            last = time;
            pcom->cin_last = time;
            if (*puse == CIN_ESC) {             //read byte cannot be escape
                puse = pins++;
                *puse = CIN_ESC;                //so send CIN_ESC,0xFF
                if (pins == &pcom->c_in[CIN_INS]) {
                    pins = pcom->c_in;          //wrap around buffer
                }
            }
        }
        pcom->cin_have += 1;                    //count available chars
        if (*puse == '\n') {                    //and flag if linefeed seen
            pcom->c_lfhave += 1;
            if (pcom->c_callback) {             //callback when reach newline?
                *ptxt = 0;
                ptxt = line;
                (*pcom->c_callback) (pcom->c_callparm, line);
                pcom->cin_have = 0;             //flush circular buffer after call
                pcom->cin_tail = PTROFF(pcom->c_in, pins);
            }
        }
        pcom->cin_head = PTROFF(pcom->c_in, pins);
        OSLock_DONE(pcom->cin_lock);            //=== Free Mutex
    }
    return(OSTASK_0);                           //should never happen
}

// *********
// OSComPort:

CSTYLE int xproc
OSComPort (int mode, int numb, byte* pdat, ComInfo* pcom) {
    text name[SZNAME];
    text* ptxt;
    byte* pbuf;
    byte* pend;
    byte cc;
    dword ret, done, have, feed, baud;
    aint save;
    int e;
    #ifdef ISWIN
    HANDLE hand;
    DCB dcb;
    #else
    struct termios tty;
    int file;
    #endif

    if (mode == CIN_FINI) {                     //close port?
        if (pcom->cin_task) {
            OSTask(OS_TASKEND, &pcom->cin_task, NULL, NULL);
            pcom->cin_task = NULL;
        }                                       //kill our task
        if (pcom->cin_hand) {
            OSClose(pcom->cin_hand);            //close serial port handle
            pcom->cin_hand = 0;
        }
        if (pcom->cin_lock) {
            OSLock(OS_LOCKOFF, &pcom->cin_lock);//free mutex
            pcom->cin_lock = NULL;
        }
        pcom->cin_errs = 0;
        pcom->cin_mode = 0;
        pcom->cin_head = 0;
        pcom->cin_tail = 0;
        pcom->cin_have = 0;
        pcom->cin_used = 0;
        pcom->c_lfhave = 0;
        pcom->c_lfused = 0;
        return(0);
    }
    if (mode & CIN_READ) {                      //read from port?
        OSLock_GRAB(pcom->cin_lock);            //=== Grab Mutex
        if (pcom->cin_errs) {                   //error from task?
            e = pcom->cin_errs;                 //(OS read did not work?)
            pcom->cin_errs = 0;
            save = pcom->cin_hand;
            OSMemClear(pcom, sizeof(ComInfo));  //clear everything already read
            pcom->cin_hand = save;              //(not that it will do much good)
            OSLock_DONE(pcom->cin_lock);
            return(e);
        }
        if (numb == 0) {                        //task has read enough for us?
            mode |= CIN_LF;                     //stop on linefeed only?
            numb = EMAXI;
        }
        have = pcom->cin_have - pcom->cin_used; //characters available
        feed = pcom->c_lfhave - pcom->c_lfused; //linefeeds available
        if (!(mode & CIN_NZ)) {                 //must read all requested?
            if (!( ((dword)numb <= have) || ((mode & CIN_LF)&&(feed)) )) {
                OSLock_DONE(pcom->cin_lock);
                return(ECPENDING);              //have not read enough yet?
            }
        }
        pbuf = ADDOFF(byte*, pcom->c_in, pcom->cin_tail);
        pend = ADDOFF(byte*, pcom->c_in, pcom->cin_head);
        if (pcom->c_callback) {
            pend = pbuf;                        //we cannot read if get callback!
        }
        if (  (pcom->cin_mode & CIN_TO)         //wait for timeout to return?
            &&((signed)(OSTickCount() - pcom->cin_last) < (signed)pcom->cin_time)  ) {
            ret = FALSE;
            done = 0;
            while (pbuf != pend) {              //be sure how many available
                done += 1;
                cc = *pbuf++;
                if (pbuf == &pcom->c_in[CIN_INS]) {
                    pbuf = pcom->c_in;          //wrap at end of buffer
                }
                if (cc == CIN_ESC) {            //out-of-band escape?
                    cc = *pbuf++;
                    if (pbuf == &pcom->c_in[CIN_INS]) {
                        pbuf = pcom->c_in;      //wrap at end of buffer
                    }
                    if (cc == CIN_END) {        //end of packet command?
                        done -= 1;              //not a received byte
                        if (done > 0) ret = TRUE;
                    }                           //see if have stuff before timeout?
                }
                if (numb == (int)done) {        //stop on desired byte count?
                    break;
                }
                if (ret == FALSE) {
                    if (!(mode & CIN_NZ)) *pdat = 0;
                    OSLock_DONE(pcom->cin_lock);
                    return(0);                  //return nothing yet?
                }                               //can return what we have?
            }                                   //restore pbuf for below
            pbuf = ADDOFF(byte*, pcom->c_in, pcom->cin_tail);
        }
        done = 0;
        while (pbuf != pend) {                  //grab as many bytes as we can
            pcom->cin_used += 1;
            done += 1;
            cc = *pbuf++;
            if (pbuf == &pcom->c_in[CIN_INS]) {
                pbuf = pcom->c_in;              //wrap at end of buffer
            }
            if (pcom->cin_mode & CIN_TIME) {    //packetizing?
                if (cc == CIN_ESC) {            //out-of-band escape?
                    cc = *pbuf++;
                    if (pbuf == &pcom->c_in[CIN_INS]) {
                        pbuf = pcom->c_in;      //wrap at end of buffer
                    }
                    if (cc == CIN_END) {        //end of packet command?
                        done -= 1;              //not a received byte
                        pcom->cin_used -= 1;
                        if (done == 0) {        //nothing was read so far?
                            continue;           //continue reading next packet
                        }
                        break;                  //return here at end of packet
                    }
                }
            }
            *pdat++ = cc;
            if (cc == '\n') {
                pcom->c_lfused += 1;
                if (mode & CIN_LF) {            //stop on line feed?
                    break;
                }
            }
            if (numb == (int)done) {            //stop on desired byte count?
                break;
            }
        }
        if (!(mode & CIN_NZ)) {
            *pdat = 0;                          //return with terminating zero
        }
        pcom->cin_tail = PTROFF(pcom->c_in, pbuf);
        OSLock_DONE(pcom->cin_lock);            //=== Free Mutex
        return(done);
    }
    if (mode & CIN_SEND) {                      //write to port?
            if (numb == 0) {
                pend = pdat;
                while (*pend++) numb += 1;
            }
            done = OSWrite(pcom->cin_hand, pdat, numb);
            if ((int)done != numb) return(ECWRITE);
            return(done);
    }
    if (mode & CIN_OPEN) {                      //open port?
        OSComPort(CIN_FINI, 0, NULL, pcom);     //close if open and clear pcom
        if (mode & CIN_TIME) {
            pcom->cin_time = CIN_TOUT;
            if (mode & CIN_TO) {
                pcom->cin_time = CIN_TRET;      //longer wait-until timeout?
                pcom->cin_last = OSTickCount();
            }
        }
        pcom->cin_mode = mode;

        #ifdef ISWIN                            //--- Windows
        ptxt = OSTxtCopy(name, "\\\\.\\COM");   //The \\.\COMn required if n>9
        OSIntPut(numb, ptxt);
        hand = CreateFileA((LPCSTR)name,        //COMn as text to open port
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,       //FILE_FLAG_OVERLAPPED |
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                           NULL);
        if (hand == INVALID_HANDLE_VALUE) return(ECOPEN);
        pcom->cin_hand = (aint) hand;

        OSMemClear(&dcb, sizeof(dcb));
        //GetCommState(hand, &dcb);
        dcb.DCBlength = sizeof(dcb);
        dcb.BaudRate = (mode & CIN_9600) ? CBR_9600 : CBR_115200;
        if (mode & CIN_BAUD) {
            baud = pcom->cin_baud & C_BAUD;
            if      (baud == 115200) dcb.BaudRate = CBR_115200;
            else if (baud ==  57600) dcb.BaudRate =  CBR_57600;
            else if (baud ==  38400) dcb.BaudRate =  CBR_38400;
            else if (baud ==  19200) dcb.BaudRate =  CBR_19200;
            else if (baud ==  14400) dcb.BaudRate =  CBR_14400;
            else if (baud ==   9600) dcb.BaudRate =   CBR_9600;
            else if (baud ==   4800) dcb.BaudRate =   CBR_4800;
            else if (baud ==   2400) dcb.BaudRate =   CBR_2400;
            else if (baud ==   1200) dcb.BaudRate =   CBR_1200;
            else if (baud ==    600) dcb.BaudRate =    CBR_600;
            else if (baud ==    300) dcb.BaudRate =    CBR_300;
            else if (baud ==    110) dcb.BaudRate =    CBR_110;
        }
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = 0;
        dcb.fParity = FALSE;
        dcb.fBinary = TRUE;
        if (mode & CIN_PLUS) {
            baud = pcom->cin_baud & C_BITS;     //7 instead of 8 data bits
            if (baud == (2 << C_SHFT)) dcb.ByteSize = 7;
            baud = pcom->cin_baud & C_PARI;     //even or odd parity
            if (baud == (0 << (C_SHFT+2))) dcb.Parity = EVENPARITY;
            if (baud == (1 << (C_SHFT+2))) dcb.Parity = ODDPARITY;
            baud = pcom->cin_baud & C_STOP;     //1.5 or 2 stop bits
            if (baud == (1 << (C_SHFT+2+3))) dcb.StopBits = ONE5STOPBITS;
            if (baud == (2 << (C_SHFT+2+3))) dcb.StopBits = TWOSTOPBITS;
        }
        ret = SetCommState(hand, &dcb);         //set baud rate and such
        if (ret == 0) return(ECRANGE);
        //Another way to set baud rate.
        //if (!(BuildDCB("115200,n,8,1", &dcb)) return(ECRANGE);

        #else                                   //--- Linux
        // ttyO0, ttyO1 etc are usually physical UARTS.
        // tty=CurrentTTY, console=SystemConsole, tty0=CurrentVirutalConsole
        ptxt = OSTxtCopy(name, "/dev/tty");     //tty is usually console
        if (numb) {
            if (numb >= 0) {
                *ptxt++ = 'O';                  //ttyO1 is usually UART1
            } else {
                ptxt = OSTxtCopy(ptxt, "USB");  //ttyUSB0 is usually FTDI cable
                numb = -numb - 1;
            }
            OSIntPut(numb, ptxt);
        }
        file = open(name, O_RDWR|O_NOCTTY|O_SYNC);
        if (file < 0) return(ECOPEN);
        pcom->cin_hand = (aint) file;

        OSMemClear(&tty, sizeof(tty));
        if (tcgetattr(file, &tty) != 0) {
            return(ECSYS);
        }

        e = (mode & CIN_9600) ? B9600 : B115200;
        if (mode & CIN_BAUD) {
            baud = pcom->cin_baud & C_BAUD;
            if      (baud == 115200) e = B115200;
            else if (baud ==  57600) e =  B57600;
            else if (baud ==  38400) e =  B38400;
            else if (baud ==  19200) e =  B19200;
            else if (baud ==   9600) e =   B9600;
            else if (baud ==   4800) e =   B4800;
            else if (baud ==   2400) e =   B2400;
            else if (baud ==   1200) e =   B1200;
            else if (baud ==    600) e =    B600;
            else if (baud ==    300) e =    B300;
            else if (baud ==    110) e =    B110;
        }
        cfsetospeed(&tty, e);
        cfsetispeed(&tty, e);

        // disable IGNBRK for mismatched speed tests
        // otherwise receive break as \000.
        tty.c_iflag |= IGNBRK;                  //disable break processing
        tty.c_lflag = 0;                        //no signaling chars, no echo, etc
        tty.c_oflag = 0;                        //no remapping, no delays
        tty.c_cc[VMIN]  = 1;                    //1=blocking 0=non-blocking read
        tty.c_cc[VTIME] = 0;                    //0.5 seconds read timeout (0=block)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); //shut off xon/xoff ctrl
        tty.c_cflag |= (CLOCAL | CREAD);        //ignore modem controls, enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      //no parity by default
        tty.c_cflag &= ~CSTOPB;                 //no stop bits by default
        tty.c_cflag &= ~CRTSCTS;

        tty.c_cflag = (tty.c_cflag & (~CSIZE)) | CS8;
        if (mode & CIN_PLUS) {                  //8-bit chars by default
            baud = pcom->cin_baud & C_BITS;     //extra flags for bits,parity,stop?
            if (baud == (2 << C_SHFT)) tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
            if (baud == (0 << (C_SHFT+2))) tty.c_cflag |= PARENB;
            if (baud == (1 << (C_SHFT+2))) tty.c_cflag |= PARENB + PARODD;
            baud = pcom->cin_baud & C_STOP;
            if (baud == (2 << (C_SHFT+2+3))) tty.c_cflag |= CSTOPB;
            //1 for 1.5 stop bits is not supported
        }

        if (tcsetattr(file, TCSANOW, &tty) != 0) {
            return(ECSYS);                      //set baud rate and such
        }
        #endif                                  //---

        pcom->cin_lock = NULL;
        if (!(mode & CIN_NLOK)) {
            e = OSLock(OS_LOCKNEW, &pcom->cin_lock);
            if (e) return(e);                   //setup read task mutex
        }
        if (!(mode & CIN_CALL)) {               //do not allow cin_callback?
            pcom->c_callback = NULL;
            pcom->c_callparm = NULL;
        }

        ret = OSTask(OS_TASKNEW+OS_TASKHI1, &pcom->cin_task, OSComTask, pcom);
        if (ret) return(ECCOMM);                //high priority input task
        return(0);
    }
    return(ECARGUMENT);
}

#ifndef OSBASE      //--- (Omit because needs special link libraries?)
//*************************************************************************
//  OSMMap:
//      Allows memory mapped hardware I/O reading and writing physical memory.
//      Currently does not work with Windows.
//
//  Arguments:
//      dword OSMMap (int mode, dword addr, dword used, CMMap* pmap)
//      mode    0 = Close memory mapping                        (MM_FINI)
//              1 = Open a memory mapping                       (MM_OPEN)
//              2 = Read dword at addr offset in mapped memory  (MM_READ)
//              4 = Write dword at addr offset in mapped memory (MM_WRITE)
//              8 = Write byte at addr offset in mapped memory  (MM_BYTE)
//      addr    Phyical memory address for beginning of mapped block for open.
//              Byte offset into block for read/write.
//              Ignored for close.
//      used    Bytes in memory block for open. Uses BLKSIZE if 0.
//              Value to write for write dword or byte.
//              Value to return if error for read.
//              Ignored for close.
//      pmap    Pointer to ComInfo structure used for open port.
//      
//  Returns:
//      Negative code if error except for read which returns value.
//*************************************************************************

CSTYLE dword xproc
OSMMap (int mode, dword addr, dword used, CMMap* pmap) {
    #ifdef ISWIN                                //--- Windows
    return(ECCANNOT);
    #else                                       //--- Linux
    dword* pdw;
    byte* pbv;
    void* ptr;

    if (mode & MM_READ) {                       //read from maped memory?
        if (addr >= pmap->mm_size) return(used);
        pdw = ADDOFF(dword*, pmap->mm_base, addr);
        return(*pdw);
    }
    if (mode & MM_WRITE) {                      //write dword to mapped memory?
        if (addr >= pmap->mm_size) return(ECRANGE);
        pdw = ADDOFF(dword*, pmap->mm_base, addr);
        *pdw = used;
        return(0);
    }
    if (mode & MM_BYTE) {                       //write byte to mapped memory?
        if (addr >= pmap->mm_size) return(ECRANGE);
        pbv = ADDOFF(byte*, pmap->mm_base, addr);
        *pbv = (byte) used;
        return(0);
    }
    if (mode & MM_OPEN) {                       //open mapped memory block
        pmap->mm_hand = open("/dev/mem", O_RDWR);
        if (pmap->mm_hand == -1) return(ECOPEN);
        if (used == 0) used = BLKSIZE;          //just in case
        ptr = mmap(0, used, PROT_READ|PROT_WRITE, MAP_SHARED, (int)pmap->mm_hand, addr);
        if (ptr == MAP_FAILED) return(ECNULL);
        pmap->mm_base = (byte*) ptr;
        pmap->mm_addr = addr;
        pmap->mm_size = used;
        return(0);
    }
    if (pmap->mm_hand == -1) return(0);
    close(pmap->mm_hand);                       //close mapped memory block
    pmap->mm_hand = -1;
    return(0);
    #endif                                      //---
}

//*************************************************************************
//  OSConnect:
//      Opens a socket for TCP/IP or UDP/IP communication.
//
//      Note that a server must use mode 1=SOCK_SERVER to get a bound socket
//      on which it can latter accept mode 2=SOCK_ACCEPT connections
//      which each return a separate connection separate handle.
//
//      UDP is not connected and only uses mode 1+256=SOCK_USEUDP.
//
//      TCP server connections require modes 1=SOCK_SERVER then 2=SOCK_ACCEPT.
//      There can be mulitple connections per 1=SOCK_SERVER.
//      All 1=SOCK_SERVER, 2=SOCK_ACCEPT, 3=SOCK_CLIENT  handles must be closed.
//
//      Note that for Windows Winsock mode 4=SOCK_APPINIT and 5=SOCK_APPEXIT
//      are required and call WSAStartup but do nothing for Linux.
//
//      Warning: It may be dangerous to return 64-bit Windows handles as int
//      since may be truncated, but in practice handles seem to be low offsets.
//      If we are incorrect, then return value should be aint rather than int!
//      Or we could use the library open which does return an int handle.
//
//  Arguments:
//      aint OSConnect (int mode, aint sock, text* paddr, int port, ConEx* pex)
//
//      mode    0 = Close sock handle from modes 1, 2 or 3.        (SOCK_CLOSE)
//                  May add SOCK_SHUT to shutdown futher writes.   (SOCK_SHUT)
//                  May add SOCK_NOWAIT to not linger for pending. (SOCK_NOWAIT)
//
//              1 = TCP: Prepare to accept server connections      (SOCK_SERVER)
//                  Returns handle for future mode 2 calls.
//                  Optional maximum waiting connections can be
//                  specified by pex->maxcons (5 is the default).
//                  Do not use handle with OSConSend and Recv!
//                  TCP is stream oriented and OSConRecv returns
//                  available bytes and not whole OSConSend messages.
//                  Must supply port to use but usually no paddr.
//
//                  UDP: 1+256 sets for non-connected datagrams.   (SOCK_USEUDP)
//                  Returns handle for OSConSend and OSConRecv.
//                  1+256 UDP must use OSConSend with pex providing
//                  the target computer's conaddr and conport.
//                  1+256 UDP may use OSConRecv with pex to learn
//                  sending computer's conaddr and conport.
//                  UDP is message oriented and OSConRecv returns
//                  just one whole OSConSend message.
//                  Must supply receive port but usually no paddr.
//
//                  RAW: 1+32 sets for raw ethernet packets.       (SOCK_USERAW)
//
//              2 = Connect as TCP server and return new socket    (SOCK_ACCEPT)
//                  Must have previously called mode 1.
//                  Ignores the paddr and port arguments.
//                  Returns handle for OSConSend and OSConRecv.
//
//              3 = Connect as TCP client.                         (SOCK_CLIENT)
//                  Unlike server, no previous call needed.
//                  Must supply server's paddr and port.
//                  Returns handle for OSConSend and OSConRecv.
//
//                  Modes 1,2,3 return handle closed with mode 0.
//                  TCP server connections require modes 1 then 2.
//                  TCP client connections require mode 3.
//
//             11 = Application startup (not needed for Linux)    (SOCK_APPINIT)
//             12 = Application exiting (not needed for Linux)    (SOCK_APPDONE)
//                  Modes 11 and 12 calls must be balanced.
//                  Modes 11 and 12 do nothing if not Windows.
//                  But call them anyway for portability.
//                  Does not return handle.
//
//            +32 = Add for Raw Ethernet packets                     (SOCK_RAWS)
//                  Must include pex argument to use WinPcap.
//                  Use with SOCK_USERAW (1+32) to open raw socket.
//                  Add SOCK_NOWAIT to use 1ms instead of 10ms timeout.
//
//           +128 = Use IPv6 address (?:?:?:?:?:?:?:?)                (SOCK_IV6)
//                  128 = 8x16 bits with hex word values
//
//           +256 = Add for UDP instead of TCP connection             (SOCK_UDP)
//                  Add +256 only to mode 1.
//                  Use SOCK_USEUDP mode instead of 1+256.
//
//          +2048 = Add to zero all pex parameters.                   (SOCK_DEF)
//          +4096 = Return raw MAC address in paddr.                  (SOCK_MAC)
//                  SOCK_MAC does not use paddr on entry, only port.
//                  Returns 0 if not compiled with DO_MACAD.
//
//      sock    Handle from prior OSConnect for modes 0 and 3.
//              Ignored for other modes and should be zero.
//
//      paddr   IP address (or maybe domain name for mode 3).
//              IP address is in dotted string form like "127.0.0.1"
//              where each number is decimal 0-255 byte value.
//              The local loopback to same computer is often 127.0.0.1
//              IPv6 has 8 hex words: 2001:db8:85a3:0:0:8a2e:370:7334
//              Mode 3 may optionally use domain like "www.triscape.com"
//              Required for mode 3, optional for mode 1, NULL for others.
//              For mode 3 TCP clients gives server computer's address.
//              For mode 1 TCP or UDP may give server binding used to
//              receive messages but is normally NULL to use any connection.
//              For mode 1 may not be a domain name.
//              For mode 1 RAW may be ethernet port name or NULL to use default.
//
//      port    Port number 1-65535.  Ports below 1024 require root.
//              Custom applications should use ports greater than 1024.
//              Required for mode 3, optional for mode 1, 0 for others.
//              For mode 3 TCP client gives server computer's port.
//              For mode 1 TCP server gives binding used to receive.
//              For mode 1 UDP must give server binding to receive.
//              For mode 1 UDP may optionally be 0 if just sending.
//              For mode 1 RAW gives ethernet port number (1=1st) or 0 default.
//
//      pex     Extra information or use NULL for defaults.
//              If pex supplied then pex->error returns error.
//              In addition to sockets codes, see our local SOCKERR_? errors.
//              TCP mode 2 returns pex->conaddr and pex->conport
//              describing the other end of the connection.
//              TCP modes 2 and 3 may set CI_NODELAY to turn of Nagle algorithm
//              which allows short message to send without normal 200 ms
//              delay to see whether the IP packet can be filled.
//              CI_NODELAY is faster but can cause more network congestion.
//              CI_REUSEAD sets SO_REUSEADDR socketopt so that a server
//              restart can grab a port without waiting two minutes.
//              CI_REUSEAD may be necessary to avoid SOCK_USEUDP error,
//              especially if using 127.0.0.1 loopback address.
//              Can set CI_REUSEAD and maxcons in the mode 1=SOCK_SERVER call.
//              Set CI_NODELAY,sendbuf,recvbuf in 2=SOCK_ACCEPT, 3=SOCK_CLIENT.
//              Set CI_NOBLOCK to make all socket operations non-blocking.
//              Use CI_NOBLOCK for connect instead of CI_DONTWAIT for receive.
//              Use CI_CANCAST to allow UDP broadcast send and/or receive.
//              Add SOCK_DEF to mode to zero for all default values.
//              If 1+256 UDP mode, sets CI_USEUDP in conflag on return.
//              If 1+256 RAW mode, sets CI_USERAW in conflag on return.
//              
//  Returns:
//      Socket connection handle for future OSConSend,OSConRecv,OSConnect calls.
//      Modes 0, 11 and 12 return 0 if no error.
//      Returns -1 (SOCK_ERR) if error (and pex->error may return SOCKERR_?).
//*************************************************************************

CSTYLE aint xproc
OSConnect (int mode, aint sock, text* paddr, int port, ConEx* pex) {
    text* ptxt;
    text* pmac;
    struct hostent* phost;
    struct sockaddr_in addr;
    int modes, domain, proto, type, off, err, ok;
    aint hcon;
    #ifdef ISWIN                                //---
    int len;
    #else                                       //---
    socklen_t len;
    #endif                                      //---

    modes = mode & 0xF;
    if (pex) {
        pex->error = 0;
        pex->rawcode = 0;
        pex->conaddr[0] = 0;
        pex->conport = 0;
        pex->ourport = 0;
        pex->ipserve = 0;
        pex->winpcap = NULL;
        if (mode & SOCK_DEF) {
            pex->conflag = 0;
            pex->maxcons = 0;
            pex->sendbuf = 0;
            pex->recvbuf = 0;
            pex->lingers = 0;
        }
    }

    if (modes == SOCK_CLOSE) {                  //close socket
        if (mode & SOCK_SHUT) {                 //shutdown further writes?
            #ifdef ISWIN                        //---
            shutdown((SOCKH)sock, 0x01);        //SD_SEND in WinSock2.h
            #else                               //---
            shutdown((SOCKH)sock, SHUT_WR);
            #endif                              //---
            return(0);
        }
        if (mode & SOCK_NOWAIT) {               //do not linger for pending
            struct linger ling;
            ling.l_onoff = 1;
            ling.l_linger = (short) 0;
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));
        }
        #ifdef ISWIN                            //===
        #ifdef DOPCAP                           //---
        if ((pex)&&(pex->winpcap)) {
            pcap_close((pcap_t*)pex->winpcap);
            pex->winpcap = NULL;
        }
        #endif                                  //---
        closesocket((SOCKH)sock);
        #else                                   //===
        close((SOCKH)sock);
        #endif                                  //===
        return(0);
    }

    if (modes >= SOCK_APPINIT) {                //infrequent calls?
        if (modes == SOCK_APPINIT) {            //initialize
            #ifdef ISWIN                        //===
            WSADATA WsaDat;
            WSAStartup(MAKEWORD(2,2),&WsaDat);  //1996 version 2.2 is new enough
            #endif                              //===
            return(0);

        } else if (modes == SOCK_APPDONE) {     //exiting
            #ifdef ISWIN                        //===
            WSACleanup(); 
            #endif                              //===
            return(0);
        }
        if (pex) pex->error = SOCKERR_MODE;
        return(SOCK_ERR);                       //bad mode
    }

    if (modes == SOCK_ACCEPT) {                 //accept new server connection
        len = sizeof(addr);
        hcon = accept((SOCKH)sock, (struct sockaddr*)&addr, &len);
        if (hcon == -1) {
            if (pex) {
                err = 0;
                len = sizeof(err);
                ok = getsockopt((SOCKH)sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                pex->rawcode = err;
                if (err) err = SOCKERR_STOP;
                if (ok < 0) err = SOCKERR_NONE;
                pex->error = err;
            }
            return(SOCK_ERR);
        }
        if (pex) {
            ptxt = inet_ntoa(addr.sin_addr);
            if (ptxt) OSTxtStop(pex->conaddr, ptxt, SZDISK);
            pex->conport = ntohs(addr.sin_port);//return client's info
            pex->error = 0;
        }
        return(hcon);                           //return connection socket
    }

    domain = AF_INET;
    if (mode & SOCK_IV6) {
        #ifndef AF_INET6                        //---
        if (pex) pex->error = SOCKERR_IV6;
        return(SOCK_ERR);
        #else                                   //---
        domain = AF_INET6;
        #endif                                  //---
    }
    type = (mode & SOCK_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    proto = 0;                                  //IPPROTO_IP=0
    #ifndef ISWIN                               //===
    if (mode & SOCK_RAWS) {
        domain = AF_PACKET;
        type = SOCK_RAW;
        proto = htons(ETH_P_ALL);               //IPPROTO_RAW IPPROTO_IP fail to receive
    }
    #endif                                      //===

    sock = O32( socket(domain, type, proto) );  //get a new socket handle
    if (sock == -1) {
        if (pex) pex->error = SOCKERR_NULL;
        return(SOCK_ERR);
    }

    if (pex) {                                  //non-standard settings?
        if (pex->sendbuf) {                     //set non-default buffer sizes
            off = pex->sendbuf;
            len = sizeof(pex->sendbuf);
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_SNDBUF, (char*)&off, len);
        }
        if (pex->recvbuf) {
            off = pex->recvbuf;
            len = sizeof(pex->recvbuf);
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_RCVBUF, (char*)&off, len);
        }
        if (pex->lingers) {
            struct linger ling;
            ling.l_onoff = 1;
            ling.l_linger = (short) ((pex->lingers < 0) ? 0 : pex->lingers);
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));
        }
        if (pex->conflag & CI_NODELAY) {
            off = 1;                            //no Nagle algorithm?
            len = sizeof(off);
            setsockopt((SOCKH)sock, IPPROTO_TCP, TCP_NODELAY, (char*)&off, len);
        }
        if (pex->conflag & CI_REUSEAD) {
            off = 1;                            //server restart reuse address?
            len = sizeof(off);
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_REUSEADDR, (char*)&off, len);
        }
        if (pex->conflag & CI_CANCAST) {
            off = 1;                            //allow broadcasting?
            len = sizeof(off);
            setsockopt((SOCKH)sock, SOL_SOCKET, SO_BROADCAST, (char*)&off, len);
        }
        if ((pex->conflag & CI_NOBLOCK)&&(modes != SOCK_CLIENT)) {
            off = 1;                            //make socket non-blocking
            #ifdef ISWIN                        //===
            ioctlsocket(sock, FIONBIO, (u_long*)&off);
            #else                               //===
            ioctl(sock, FIONBIO, &off);
            #endif                              //===
        }
    }

    OSMemClear(&addr, sizeof(addr));            //prepare connection address
    addr.sin_family = AF_INET;                  //only support IP sockets

    if (modes == SOCK_CLIENT) {                 //open client socket
        if (paddr == NULL) return(ECARGUMENT);
        addr.sin_port = htons(port);            //desired server address
        addr.sin_addr.s_addr = inet_addr(paddr);

        if (addr.sin_addr.s_addr == 0xFFFFFFFF) {
            phost = (struct hostent*) gethostbyname(paddr);
            if ((phost == NULL)||(phost->h_addr_list[0] == 0)) {
                if (pex) {
                    pex->error = SOCKERR_DNS;   //DNS domain lookup?
                }
                return(SOCK_ERR);               //cannot resolve
            }
            OSMemCopy(&addr.sin_addr.s_addr, phost->h_addr_list[0], sizeof(addr.sin_addr.s_addr));

        } else if (pex) {                       //remember server's IP
           OSTxt2MIP(2+4, (byte*)&pex->ipserve, paddr);
        }

        ok = connect((SOCKH)sock, (struct sockaddr*)&addr, sizeof(addr));
        if (ok == -1) {                         //connect socket
            if (pex) {
                err = 0;
                len = sizeof(err);
                ok = getsockopt((SOCKH)sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                pex->rawcode = err;
                if (err) err = SOCKERR_STOP;
                if (ok < 0) err = SOCKERR_NONE;
                pex->error = err;
            }
            #ifdef ISWIN                        //===
            closesocket((SOCKH)sock);
            #else                               //===
            close((SOCKH)sock);
            #endif                              //===
            return(SOCK_ERR);
        }
        if (pex->conflag & CI_NOBLOCK) {        //make socket non-blocking?
            off = 1;                            //must do after connect
            #ifdef ISWIN                        //===
            ioctlsocket(sock, FIONBIO, (u_long*)&off);
            #else                               //===
            ioctl(sock, FIONBIO, &off);
            #endif                              //===
        }
        return(sock);
    }

    if (modes == SOCK_SERVER) {                 //open server socket
        if ((paddr)&&(paddr[0] == 0)) paddr = NULL;

        if (mode & SOCK_RAWS) {                 //Raw Ethernet?
            if (pex == NULL) return(ECARGUMENT);//requires pex for handle or index
            pex->conflag |= CI_USERAW;

            pmac = NULL;
            if (mode & SOCK_MAC) {
                pmac = paddr;                   //for raw can
                paddr = NULL;                   //return MAC instead of supply name?
                pmac[0] = 0;
                pmac[1] = 0;
                pmac[2] = 0;
                pmac[3] = 0;
                pmac[4] = 0;
                pmac[5] = 0;
            }

            #ifdef ISWIN                        //=== Windows RAW
            #ifndef DOPCAP                      //---
            return(ECCANNOT);                   //raw requires WinPcap if Windows
            #else                               //---

            text errbuf[PCAP_ERRBUF_SIZE];      //WinPcap version
            pcap_if_t* alldevs;
            pcap_t* hcap;

            if (paddr == NULL) {
                len = pcap_findalldevs(&alldevs, errbuf);
                if (len == -1) return(ECMISSING);
                if (port <= 0) port = 1;
                while (alldevs) {
                    if (--port == 0) break;     //find by index number
                    if (alldevs->next == NULL) break;
                    alldevs = alldevs->next;
                }
                if (alldevs == NULL) return(ECMISSING);
                paddr = alldevs->name;          //and get corresponding name
            }
            #ifdef DO_MACAD                     //---
            if (pmac) {                         //return our MAC address?
                IP_ADAPTER_INFO* info;
                IP_ADAPTER_INFO* pinf;
                dword bytes;
                info = NULL;
                GetAdaptersInfo(info, &bytes);  //get structure size needed
                info = (IP_ADAPTER_INFO*) OSAlloc(bytes);
                if (info == NULL) return(ECMEMORY);
                ptxt = paddr;
                while (*ptxt != '{') ptxt += 1; //find {....} part of name
                GetAdaptersInfo(info, &bytes);  //Windows info about all ports
                pinf = info;
                while (pinf) {                  
                    if (OSTxtCmp(pinf->AdapterName, ptxt) == 0) {
                        OSMemCopy(pmac, pinf->Address, 6);
                        break;                  //return our matching MAC address
                    }
                    pinf = pinf->Next;
                }
                OSFree(info);
            }
            #endif                              //---

            len = (mode & SOCK_NOWAIT) ? 1 : 10;//msec timeout
            hcap = pcap_open_live(paddr,        //name of the device
                                65536,          //portion of packets to capture
                                1,              //promiscuous mode if non-zero
                                len,            //read timeout
                                errbuf);        //error buffer
            if (hcap == NULL) return(ECCOMM);
            pex->winpcap = hcap;                //remember WinPcap handle
            return(sock);
            #endif                              //---

            #else                               //=== Linux RAW
            struct ifreq ifr;
            OSMemClear(&ifr, sizeof(ifreq));
            if (paddr == NULL) {
                paddr = "eth0";
                switch (port) {                 //we only allow four Linux adapters
                case 2:  paddr = "eth1"; break;
                case 3:  paddr = "eth2"; break;
                case 4:  paddr = "eth3"; break;
                }
            }
            len = OSTxtSize(paddr);
            if (len > sizeof(ifr.ifr_name)) return(SOCK_ERR);
            memcpy(ifr.ifr_name, paddr, len);
            len = ioctl(sock, SIOCGIFINDEX, &ifr);
            if ((signed)len < 0) return(SOCK_ERR);
            pex->maxcons = ifr.ifr_ifindex;     //remember port index

            #ifdef DO_MACAD                     //---
            struct ifreq mac;
            if (pmac) {
                OSMemClear(&mac, sizeof(struct ifreq));
                OSTxtStop(if_mac.ifr_name, paddr, IFNAMSIZ-1);
                len = ioctl(sockfd, SIOCGIFHWADDR, &mac);
                if ((signed)len < 0) return(SOCK_ERR);
                OSMemCopy(pmac, mac.ifr_hwaddr.sa_data, 6);
            }                                   //return our matching MAC address
            #endif                              //---
            return(sock);            
            #endif                              //===
        }

        if ((mode & SOCK_UDP)&&(port == 0)&&(paddr == NULL)) {
            if (pex) pex->conflag |= CI_USEUDP;
            return(sock);                       //UDP client without binding?
        }          

        if ((paddr)&&(*paddr)) {
            addr.sin_addr.s_addr = inet_addr(paddr);
        } else {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        addr.sin_port = htons(port);            //our desired binding

        ok = bind((SOCKH)sock, (struct sockaddr*)&addr, sizeof(addr));
        if (ok == -1) {                         //bind to our own server address
            if (pex) {
                err = 0;
                len = sizeof(err);
                //ok = getsockopt((SOCKH)sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                err = WSAGetLastError();        //maybe better than SO_ERROR
                pex->rawcode = err;
                if (err) err = SOCKERR_STOP;
                if (ok < 0) err = SOCKERR_NONE;
                pex->error = err;
            }
            #ifdef ISWIN                        //===
            closesocket((SOCKH)sock);
            #else                               //===
            close((SOCKH)sock);
            #endif                              //===
            return(SOCK_ERR);
        }
        
        if (mode & SOCK_UDP) {                  //if UDP
            if (pex) {
                pex->conflag |= CI_USEUDP;
                pex->ourport = port;            //return our receive port
                pex->conport = port;            //just in case
            }
            return(sock);                       //return handle for send & recv
        }

        off = 5;                                //default max waiting connects
        if ((pex)&&(pex->maxcons)) off = pex->maxcons;

        ok = listen((SOCKH)sock, off);
        if (ok == -1) {
            if (pex) {
                err = 0;
                len = sizeof(err);
                ok = getsockopt((SOCKH)sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                pex->rawcode = err;
                if (err) err = SOCKERR_STOP;
                if (ok < 0) err = SOCKERR_NONE;
                pex->error = err;
            }
            #ifdef ISWIN                        //===
            closesocket((SOCKH)sock);
            #else                               //===
            close((SOCKH)sock);
            #endif                              //===
            return(SOCK_ERR);
        }
        return(sock);                           //handle for future accept calls
    }

    if (pex) pex->error = SOCKERR_MODE;         //bad mode argument
    return(SOCK_ERR);                           //return -1 error
}

//*************************************************************************
//  OSConSend:
//      Sends TCP/IP data or a UDP/IP message.
//      Beware that TCP may not send all requested bytes
//      and caller must try again for unsent ending.
//
//  Arguments:
//      int OSConSend (aint sock, void* pv, dword cv, ConEx* pex)
//      sock    Handle from prior OSConnect.
//
//      pv      Pointer to memory buffer to write from.
//
//      cb      Number of bytes to write.
//
//      pex     For TCP must be NULL or not have CI_USEUDP in conflag.
//              For RAW the pv packet must already have the target MAC address.
//              For UDP pex is required and supplies target address and port.
//              For UDP pex->conflag must have CI_USEUDP bit set from OSConnect.
//              For UDP must supply conaddr and conport target address.
//              If supplied and conflag does not have CI_SKIPERR set
//              then returns sockets error code if not all bytes could be sent.
//              If pex supplied and conflag has CI_DONTWAIT then returns
//              without sending all cv bytes if they will not fit in buffer
//              but sends as many bytes as possible and returns number sent.
//              Beware that CI_DONTWAIT does nothing on many systems!
//              TCP/UDP with CI_RAWPACK skips sending 12 or 14 byte raw header
//              but includes the protocol word and following bytes in the packet 
//              and takes the target ip and port from the supplied raw header.
//              TCP with CI_TCPSIZE sends whole message before returning, but
//              CI_TCPSIZE does not require protocol+sizerest first when sending.
//              Instead TCP with CI_TCPSIZE uses cb as the size to send.
//              Note CI_RAWPACK ip and port ints are LSB first NOT NETWORK ORDER!
//      
//  Returns:
//      Number of bytes sent or -1 if error.
//      Non-zero code returned in pin->error if error.
//      Returns pin->error SOCKERR_DONE if no socket seems to be closed.
//*************************************************************************

// ========
// Thse control CI_RAWPACK send and receive to skip either 12 or 14 bytes
// of the 14 byte raw ethernet header and start the UDP/TCP message data
// with either the prototype+sizerest words or with just the sizerest word.
// Use CON_RAW 12 to begin with prototype in sent UDP/TCP data (dword aligned).
// Use CON_RAW 14 to begin with sizerest in sent UDP/TCP data (not dword aligned).

#define CON_RAW    12                           //12 or 14 without/with proto skip
#define CON_BEG   (16-CON_RAW)                  //4  or 2  with/without proto sent

// ========
// ConError: Checks socket send/recv/sendto/recvfrom errors.
// Used by both OSConSend and OSConRecv.

#define CON_MSG_MS 10000                        //max ms to finish send/recv msg (10sec)

errc ConError () {
    errc e;
    #ifdef ISWIN
    e = (errc) WSAGetLastError();
    if ((e == WSAEWOULDBLOCK)||(e == WSAEINTR)||(e == WSAENOTSOCK)) e = 0;
    #else
    e = (errc) errno;
    if ((e == EWOULDBLOCK)||(e == EINTR)||(e == EAGAIN)) e = 0;
    #endif
    if (e != 0) {
        e = -1;                                 //connection lost error?
    }
    return(e);
}
// ========

CSTYLE int xproc
OSConSend (aint sock, void* pv, dword cv, ConEx* pex) {
    byte* pbuf;
    struct sockaddr_in addr;
    int timeout, done, flag, err, raw, ok;
    lint conflag, ipaddr, began;
    #ifdef ISWIN                                //---
    #ifdef DOPCAP
    int len;
    #endif                                      //---
    #endif                                      //---

    timeout = FALSE;
    conflag = (pex) ? pex->conflag : 0;

    if (conflag & CI_USERAW) {                  //send RAW ethernet?
        pex->error = 0;

        #ifdef ISWIN                            //=== Windows RAW
        #ifndef DOPCAP                          //---
        return(-1);
        #else                                   //---
        pcap_t* hcap = (pcap_t*) pex->winpcap;
        if (hcap == NULL) return(-1);

        len = pcap_sendpacket(hcap, (byte*)pv, cv);
        if (len == 0) return(cv);
        pex->error = SOCKERR_NONE;
        pex->rawcode = len;
        return(-1);
        #endif                                  //---

        #else                                   //=== Linux RAW
        struct sockaddr_ll raws;

        OSMemClear(&raws, sizeof(raws));
        raws.sll_ifindex  = pex->maxcons;       //OSConnect saves index here
        raws.sll_halen    = ETH_ALEN;     
        raws.sll_family   = PF_PACKET;          //maybe not needed
        raws.sll_protocol = htons(ETH_P_ALL);   //maybe not needed
        //raws.sll_hatype   = ARPHRD_ETHER;     //maybe not needed
        //raws.sll_pkttype  = PACKET_OTHERHOST; //maybe not needed
        OSMemCopy(raws.sll_addr, pv, 6);        //copy destination MAC address

        done = sendto((SOCKH)sock, (char*)pv, cv, 0, (struct sockaddr*)&raws, sizeof(raws));
        if (done == (int)cv) {
            pex->error = 0;
            return(done);
        }
        #endif                                  //===
    }

    if (conflag & CI_USEUDP) {                  //send UDP datagram?
        OSMemClear(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        flag = 0;
        #ifndef ISWIN                           //---
        if (pex->conflag & CI_DONTWAIT) flag = MSG_DONTWAIT;
        #endif                                  //---

        if (!(pex->conflag & CI_RAWPACK)) {     //send the message
            addr.sin_port = htons((word)pex->conport);
            addr.sin_addr.s_addr = inet_addr(pex->conaddr);
            done = sendto((SOCKH)sock, (char*)pv, cv, flag, (struct sockaddr*)&addr, sizeof(addr));
        } else {                                //simulate raw packet from UDP?
            pbuf = (byte*) pv;                  //IP address and port come from raw
            ipaddr = *((dword*)(pbuf+0));       //header before what we really send
            pex->conport = *((word*)(pbuf+12));
            addr.sin_port = htons((word)pex->conport);
            OSTxt2MIP(1+4, (byte*)&ipaddr, pex->conaddr);
            addr.sin_addr.s_addr = inet_addr(pex->conaddr);
            done = sendto((SOCKH)sock, (char*)(pbuf+CON_RAW), (cv-CON_RAW), flag, (struct sockaddr*)&addr, sizeof(addr));
            if (done >= 0) done += CON_RAW;     //do not send raw ethernet mac addresses
        }
        if (done == (int)cv) {
            pex->error = 0;
            return(done);
        }
                                                //send TCP stream data?
    } else {
        flag = 0;
        if (pex) {
            #ifndef ISWIN                       //---
            flag = MSG_NOSIGNAL;                //stop lost connection from killing app!
            if (pex->conflag & CI_DONTWAIT) flag |= MSG_DONTWAIT;
            #endif                              //---
            pex->error = 0;

            done = 0;
            if (pex->conflag & CI_RAWPACK) {    //simulate raw packet from TCP?
                done = CON_RAW;
                pv = ADDOFF(void*, pv, CON_RAW);//skip over raw header mac addresses
                cv -= CON_RAW;
            }

            if (pex->conflag & CI_TCPSIZE) {    //send whole message at once?
                pbuf = (byte*) pv;
                err = 0;
                ok = cv;
                began = OSTickCount();
                while (ok > 0) {                //send in pieces if need
                    err = send((SOCKH)sock, (char*)pbuf, ok, flag);
                    if (err < 0) {
                        err = ConError();
                        if (err < 0) break;
                        if (pex->conflag & (CI_NOBLOCK+CI_DONTWAIT)) {
                            if (((lint)OSTickCount() - began) >= CON_MSG_MS) {
                                timeout = TRUE; //timeout waiting to send?
                                if (ok < (int)cv) {
                                    err = -1;   //avoid hanging forever
                                }               //error if timeout in middle
                                break;          //of the message    
                            }
                        }
                    }
                    done += err;
                    pbuf += err;
                    ok -= err;
                }
                if (err < 0) {
                    done = err;                 //return error below
                }

            } else {
                done = send((SOCKH)sock, (char*)pv, cv, flag);
            }                                   //regular TCP send

        } else {
            done = send((SOCKH)sock, (char*)pv, cv, flag);
        }                                       //regular TCP send
        if (done > 0) {
            return(done);
        }
    }

    if (!(pex->conflag & CI_SKIPERR)) {
        raw = 0;
        err = 0;
        if (done < 0) {
            done = ConError();                  //see if probably diconnect?
            if (done < 0) {
                err = SOCKERR_DONE;
                raw = errno;
            }
        }
        if (timeout) {
            err = SOCKERR_TIM;                  //timeout error
            raw = 70;                           //linux comm error on send
        }
        pex->rawcode = raw;
        pex->error = err;
    }
    return((done >= 0) ? done : -1);
}

//*************************************************************************
//  OSConRecv:
//      Receives TCP/IP data or a UDP/IP message.
//      Beware that TCP may not receive all requested bytes
//      and caller must try again for ending portion.
//
//  Arguments:
//      int OSConRecv (aint sock, void* pv, dword cv, ConEx* pex)
//      sock    Handle from prior OSConnect.
//
//      pv      Pointer to memory buffer to read to.
//
//      cb      Number of bytes to read.
//              UDP/RAW can be larger than message size and just reads message.
//
//      pex     For TCP must be NULL or not have CI_USEUDP in conflag.
//              For UDP pex->conflag must have CI_USEUDP bit set from OSConnect.
//              For UDP returns sender's conport and conaddr for messages.
//              If supplied and conflag does not have CI_SKIPERR set
//              then returns sockets error code if not all bytes could be read.
//              If pex supplied and conflag has CI_WAITALL then waits for
//              all cv bytes to be received before returning.
//              Beware that CI_DONTWAIT works only for RAW under Windows!
//              But using CI_NOBLOCK on open makes this non-blocking.
//              Beware that CI_WAITALL does nothing on Windows!
//              Instead use CI_NOBLOCK for OSConnect to not block for receive.
//              TCP with CI_TCPSIZE reads protocol+size words first and then reads
//              just that number of bytes for the message and disregards cv argument.
//              UDP with CI_RAWPACK flag returns message after 12 or 14 byte raw head
//              MAC's with the source instead of the first four bytes of source MAC
//              and the port instead of the two protocol bytes. The rest is zeroed.
//              TCP with CI_RAWPACK does the same but also requires CI_TCPSIZE.
//              Note the CI_RAWPACK ip and port ints are LSB first NOT NETWORK ORDER!
//              For RAW pex->conflag must have CI_USERAW bit set from OSConnect.
//              For WinPcap RAW CI_DONTWAIT stops after timeout.
//
//  Returns:
//      Number of bytes received or -1 if error.
//      Non-zero code returned in pin->error if error.
//      Returns pin->error SOCKERR_DONE if no socket seems to be closed.
//      Returns zero for non-blocking socket if nothing has been received.
//*************************************************************************

#ifdef ISWIN                                    //--- Windows
#define SOCKERR_WAIT    WSAEWOULDBLOCK
#else                                           //--- Linux
#define SOCKERR_WAIT    EWOULDBLOCK
#endif

CSTYLE int xproc
OSConRecv (aint sock, void* pv, dword cv, ConEx* pex) {
    text* ptxt;
    byte* pbuf;
    struct sockaddr_in addr;
    int timeout, done, flag, raws, err, raw, ok;
    lint conflag, ipaddr, began;
    #ifdef ISWIN                                //---
    int len;
    #else                                       //---
    socklen_t len;
    #endif                                      //---

    timeout = FALSE;
    conflag = (pex) ? pex->conflag : 0;

    if (conflag & CI_USERAW) {                  //receive RAW ethernet?
        pex->error = 0;
        #ifdef ISWIN                            //=== Windows RAW
        #ifndef DOPCAP                          //---
        return(-1);
        #else                                   //---

        byte* pdata;
        struct pcap_pkthdr* phead;
        pcap_t* hcap = (pcap_t*) pex->winpcap;
        if (hcap == NULL) return(-1);

        if (conflag & CI_DONTWAIT) {
            len = pcap_next_ex(hcap, &phead, (const byte**)&pdata);
        } else {
            do {                                //get next packet
                len = pcap_next_ex(hcap, &phead, (const byte**)&pdata);
            } while (len == 0);                 //ignore timeouts
        }
        if (len > 0) {
            if (phead->caplen < cv) cv = phead->caplen;
            OSMemCopy(pv, pdata, cv);           //copy packet to caller
            return(phead->caplen);              //and return
        } else if (len == -1) {
            pex->error = SOCKERR_NONE;
            pex->rawcode = len;
            return(-1);
        }
        return(0);
        #endif                                  //---

        #else                                   //=== Linux RAW?
        flag = (pex->conflag & CI_PEEK) ? MSG_PEEK : 0;
        if (pex->conflag & CI_DONTWAIT) flag |= MSG_DONTWAIT;
        if (pex->conflag & CI_WAITALL)  flag |= MSG_WAITALL;
        done = recvfrom((SOCKH)sock, (char*)pv, cv, flag, NULL, NULL);
        if (done > 0) {
            return(done);
        }
        #endif                                  //===
    }

    if (conflag & CI_USEUDP) {                  //recieve UDP datagram?
        flag = (pex->conflag & CI_PEEK) ? MSG_PEEK : 0;
        #ifndef ISWIN                           //---
        if (pex->conflag & CI_DONTWAIT) flag |= MSG_DONTWAIT;
        if (pex->conflag & CI_WAITALL)  flag |= MSG_WAITALL;
        #endif                                  //---
        OSMemClear(&addr, sizeof(addr));
        len = sizeof(addr);

        if (!(pex->conflag & CI_RAWPACK)) {     //receive the message
            done = recvfrom((SOCKH)sock, (char*)pv, cv, flag, (struct sockaddr*)&addr, &len);

            pex->conport = ntohs(addr.sin_port);//return sender's address
            ptxt = inet_ntoa(addr.sin_addr);
            OSTxtStop(pex->conaddr, ptxt, SZNAME);

        } else {
            pbuf = (byte*) pv;                  //simulate raw packet from UDP?
            done = recvfrom((SOCKH)sock, (char*)(pbuf+CON_RAW), cv, flag, (struct sockaddr*)&addr, &len);
            if (done > 0) {                     //received a message?
                done += CON_RAW;                //add ethernet header before
                OSMemClear(pbuf, 12);           //sender IP instead of source MAC
                ptxt = inet_ntoa(addr.sin_addr);//Windows sin_addr lsb 1st so use ntoa!
                OSTxtStop(pex->conaddr, ptxt, SZNAME);
                OSTxt2MIP(2+4, (byte*)&ipaddr, pex->conaddr);
                *((dword*)(pbuf+6)) = ipaddr;
                pex->conport = ntohs(addr.sin_port);
                *((word*)(pbuf+12)) = (word) pex->conport;
            }                                   //and leave target MAC zero
        }

        if (done > 0) {
            return(done);
        }

    } else {                                    //receive TCP stream data?
        raws = 0;                               //assume no raw header added size
        flag = 0;
        if (pex) {
            if (pex->conflag & CI_PEEK) flag = MSG_PEEK;
            #ifndef ISWIN                       //---
            if (pex->conflag & CI_DONTWAIT) flag |= MSG_DONTWAIT;
            if (pex->conflag & CI_WAITALL)  flag |= MSG_WAITALL;
            #endif                              //---

            pex->error = 0;
            if (pex->conflag & CI_RAWPACK) {    //fake raw header first?
                pbuf = (byte*) pv;
                OSMemClear(pbuf, 6);            //zero destination mac
                *((dword*)(pbuf+6)) = pex->ipserve;
                *((word*)(pbuf+12)) = (word) pex->conport;
                pv = ADDOFF(void*, pv, CON_RAW);
                raws = CON_RAW;
            }
                
            if (pex->conflag & CI_TCPSIZE) {    //read protocol+size words first?
                began = OSTickCount();
                pbuf = (byte*) pv;
                done = recv((SOCKH)sock, (char*)pbuf, CON_BEG, flag);
                if (done > 0) {                 //read 4 or 2 bytes to get sizerest
                    ok = done;
                    while (ok < CON_BEG) {
                        done = recv((SOCKH)sock, (char*)(pbuf+ok), (CON_BEG-ok), flag);
                        if (done > 0) {
                            ok += done;
                        } else if (done < 0) {
                            done = ConError();
                            if (done < 0) break;
                            if (pex->conflag & (CI_NOBLOCK+CI_DONTWAIT)) {
                                if (((lint)OSTickCount() - began) >= CON_MSG_MS) {
                                    done = -1;
                                    break;      //avoid hanging forever
                                }
                            }
                        }
                    }
                    if (ok != CON_BEG) {
                        pex->rawcode = ECBADNPTS;
                        pex->error = SOCKERR_STOP;
                        return(-1);
                    }
                    pbuf += CON_BEG-2;          //skip over protocol word if needed
                    ok = (int) *((word*)pbuf);  //size in second two bytes
                    if ((ok < 0)||(ok > 1400)) {
                        pex->rawcode = ECBADNPTS;
                        pex->error = SOCKERR_STOP;
                        return(-1);             //bad sizerest, msg garbled?
                    }
                    pbuf += 2;                  //skip over sizerest word
                    err = 0;
                    cv = ok;
                    while (ok > 0) {            //read rest of msg before returning
                        err = recv((SOCKH)sock, (char*)pbuf, ok, flag);
                        if (err < 0) {          //read in pieces if needed
                            err = ConError();
                            if (err < 0) break; 
                            if (pex->conflag & (CI_NOBLOCK+CI_DONTWAIT)) {
                                timeout = TRUE; //timeout waiting to read?
                                if (((lint)OSTickCount() - began) >= CON_MSG_MS) {
                                    if (ok < (int)cv) {
                                        err = -1;
                                    }           //avoid hanging forever
                                    break;      //error if timeout in middle
                                }
                            }
                        }
                        done += err;
                        pbuf += err;
                        ok -= err;
                    }
                    if (err < 0) {              //error reading whole message?
                        done = err;             //return error below
                    }
                }

            } else {                            //normal TCP read without size first
                done = recv((SOCKH)sock, (char*)pv, cv, flag);
            }

        } else {                                //normal TCP read without size first
            done = recv((SOCKH)sock, (char*)pv, cv, flag);
        }
        if (done > 0) {                         //read something?
            return(done+raws);
        }
    }

    if (!(pex->conflag & CI_SKIPERR)) {         //investigate and return errors?
        raw = 0;
        err = 0;
        if (done < 0) {
            done = ConError();                  //see if probably disconect?
            if (done < 0) {
                err = SOCKERR_DONE;
                raw = errno;
            }
        }
        if (timeout) {
            err = SOCKERR_TIM;                  //timeout error
            raw = 62;                           //linux timer expired
        }
        pex->rawcode = raw;
        pex->error = err;
    }
    return((done >= 0) ? done : -1);
}

#ifdef __cplusplus                              //***
//*************************************************************************
//  OSReadWeb:
//      Reads a file over the Internet and
//      saves it to an StmFile file or memory file object.
//      Currently only works for Windows.
//
//  Arguments:
//      errc OSReadWeb (int mode, StmFile* pfile, text* purl)
//      mode    Reserved for future use and must be zero.
//      pfile   Target file to save the purl web file.
//              Caller must create as a disk or memory file beforhand
//              using the StmOpen method or StmMemory function.
//      purl    File's web address, such as: http://www.domain.com/dir/file.ext
//
//  Returns:
//      Number of bytes in *ppmem file image or zero if error.
//*************************************************************************

int xproc OSReadWeb (int mode, StmFile* pfile, text* purl) {
    #ifdef ISWIN                                //=== Windows
    byte buf[BLKSIZE];
    HINTERNET hnet, hcon;
    dword reads, dones;
    int bytes;
    int ok;

    hnet = InternetOpenA(FILE_AGENT, 0, NULL, NULL, 0);
    if (hnet == NULL) return(0);                //start Internet session

    hcon = InternetOpenUrlA(hnet, purl, NULL, 0, 0, NULL);
    if (hcon == NULL) {                         //open the file
        InternetCloseHandle(hnet);
        return(0);
    }
    bytes = 0;

    while (TRUE) {                              //read in chunks
        ok = InternetReadFile(hcon, buf, BLKSIZE, &reads);
        if (ok == FALSE) {                      //read file by blocks until done
            InternetCloseHandle(hcon);
            InternetCloseHandle(hnet);
            return(0);
        }
        if (reads == 0) break;                  //all done?
        bytes += reads;
        pfile->Write(buf, reads, &dones);
        if (dones != reads) return(0);
    }

    InternetCloseHandle(hcon);
    InternetCloseHandle(hnet);
    return(bytes);

    #else                                       //=== Linux
    return(0);
    #endif                                      //===
}
#endif                                          //***

//*************************************************************************
//  OSipcPipe:
//      Creates or closes an interprocess communication pipe.
//      Use OSipcSend and OSipcRecv to write and read using created pipe.
//      This works similarly to OSConnect but locally and not over the internet.
//      For Linux optionally allows non-local internet connections.
//      Note that we set so OSipcRecv times out in PIPE_TIMEO (normally 60 sec).
//
//  Arguments:
//      aint OSipcPipe (int mode, aint hpipe, text* pname)
//      mode    0 = Close sock handle from modes 1, 2 or 3.        (SOCK_CLOSE)
//              1 = Prepare to accept server connections           (SOCK_SERVER)
//                  Returns handle for future mode 2 calls.
//              2 = Connect as pipe server and return new socket   (SOCK_ACCEPT)
//                  Must have previously called mode 1.
//              3 = Connect as pipe client.                        (SOCK_CLIENT)
//                  Unlike server, no previous call needed.
//      hpipe   Handle from prior OSipcPipe for modes 0 and 2.
//              Ignored for other modes and should be zero.
//      pname   Pipe name (excluding the standard pipe name prefix).
//              Note that multiple independent pipes can be
//              accepted and operate with the same pipe name.
//              For local pipes, use a short text name without a @
//              char and must not exceed SZNAME (32) characters.
//              For Linux, may be a non-local internet address
//              in the form [port]@[ip] where [port] is the port
//              number or blank for the default 1112 port number
//              and ip is the IP address like "127.0.0.1" or
//              optionally the ip can be a domain name for a client
//              and ip can be blank for a server's default IP address.
//              For example, "@127.0.0.1" for client or just "@" for server.
//              Windows version does not currently support internet pipes.
//              Unlike OSConnect pname must also be given for mode 2.
//
//  Returns:
//      Pipe connection handle for future OSipcSend,OSipcRecv,OSipcPipe calls.
//      Returns -1 (SOCK_ERR) if error.
//*************************************************************************

#define WIN_PIPEPATH "\\\\.\\pipe\\"            //Standard Windows pipe prefix
#define LIN_PIPEPATH "/tmp/tripipe__"           //Standard Linux pipe prefix

CSTYLE aint xproc
OSipcPipe (int mode, aint hpipe, text* pname) {
    #ifdef ISWIN                                //=== Windows
    text path[SZPATH];
    text* ptxt;
    int ok;
    
    if (mode == SOCK_CLOSE) {
        if (hpipe == SOCK_NOT) return(0);       //\.\pipe\name
        //Should only be used for server and probably not needed
        //DisconnectNamedPipe((HANDLE)hpipe);
        CloseHandle((HANDLE)hpipe);
        return(0);
    }
    if (mode == SOCK_SERVER) {                  //this step not used in Windows
        return(SOCK_NOT);
    }
    if (mode == SOCK_ACCEPT) {
        ptxt = OSTxtCopy(path, WIN_PIPEPATH);
        OSTxtStop(ptxt, pname, SZDISK);

        hpipe = (aint) CreateNamedPipeA(path, PIPE_ACCESS_DUPLEX, 0, 
                                        PIPE_UNLIMITED_INSTANCES, 
                                        BLKSIZE, BLKSIZE, PIPE_TIMEO*1000, NULL);
        if ((HANDLE)hpipe == INVALID_HANDLE_VALUE) return(SOCK_ERR);

        ok = ConnectNamedPipe((HANDLE)hpipe, NULL);
        if (ok == FALSE) return(-1);
        return(hpipe);
    }
    if (mode == SOCK_CLIENT) {
        ptxt = OSTxtCopy(path, WIN_PIPEPATH);
        OSTxtStop(ptxt, pname, SZNAME);
        hpipe = (aint) CreateFileA(path, 
                                   (GENERIC_READ | GENERIC_WRITE),
                                   0, NULL, OPEN_EXISTING, 0, NULL);
        while ((HANDLE)hpipe == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_PIPE_BUSY) {
                ok = WaitNamedPipeA(path, 0);
                if (ok) {
                    hpipe = (aint) CreateFileA(path, 
                                               (GENERIC_READ | GENERIC_WRITE),
                                               0, NULL, OPEN_EXISTING, 0, NULL);
                    continue;
                }
            }
            return(SOCK_ERR);
        }
        return(hpipe);
    }
    return(ECARGUMENT);

    #else                                       //=== Linux
    text* ptxt;
    struct hostent* phost;
    struct sockaddr_un addr;
    struct sockaddr_in inet;
    struct timeval tv;
    socklen_t len;
    int ok, port;

    if (mode == SOCK_CLOSE) {
        close((SOCKH)hpipe);
    }
    if (mode == SOCK_SERVER) {
        hpipe = socket(AF_UNIX, SOCK_STREAM, 0);
        if (hpipe == -1) return(SOCK_ERR);

        tv.tv_sec = PIPE_TIMEO;
        setsockopt((SOCKH)hpipe, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval));

        port = OSIntGet(pname, &ptxt);
        while (*ptxt == ' ') ptxt += 1;
        if (*ptxt != '@') {                     //not [port]@ip so local pipe?
            addr.sun_family = AF_UNIX;
            ptxt = OSTxtCopy(addr.sun_path, LIN_PIPEPATH);
            OSTxtStop(ptxt, pname, SZNAME);
            len = OSTxtSize(addr.sun_path) + sizeof(addr.sun_family);
            unlink(addr.sun_path);              //not clear why this is needed!
            ok = bind(hpipe, (struct sockaddr*)&addr, len);

        } else {                                //[port]@[ip] for internet pipe?
            ptxt += 1;                          //skip @ character
            while (*ptxt == ' ') ptxt += 1;     //allow extra blanks after @
            if (port == 0) port = PIPE_PORT;    //def 1112 port if 0 or blank
            if (ptxt[0]) {
                inet.sin_addr.s_addr = inet_addr(ptxt);
            } else {                            //blank after @ for our ip
                inet.sin_addr.s_addr = htonl(INADDR_ANY);
            }
            inet.sin_port = htons(port);        //our desired binding
            ok = bind((SOCKH)hpipe, (struct sockaddr*)&inet, sizeof(inet));
        }
        if (ok == -1) {
            close((SOCKH)hpipe);
            return(SOCK_ERR);
        }
        ok = listen((SOCKH)hpipe, 8);
        if (ok == -1) {
            close((SOCKH)hpipe);
            return(SOCK_ERR);
        }
        return(hpipe);
    }
    if (mode == SOCK_ACCEPT) {
        len = sizeof(addr);
        hpipe = accept((SOCKH)hpipe, (struct sockaddr*)&addr, &len);
        if (hpipe == -1) return(SOCK_ERR);
        return(hpipe);
    }
    if (mode == SOCK_CLIENT) {
        hpipe = socket(AF_UNIX, SOCK_STREAM, 0);
        if (hpipe == -1) return(SOCK_ERR);

        tv.tv_sec = PIPE_TIMEO;
        setsockopt((SOCKH)hpipe, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval));

        port = OSIntGet(pname, &ptxt);
        while (*ptxt == ' ') ptxt += 1;
        if (*ptxt != '@') {                     //not [port]@ip so local pipe?
            addr.sun_family = AF_UNIX;
            ptxt = OSTxtCopy(addr.sun_path, LIN_PIPEPATH);
            OSTxtStop(ptxt, pname, SZNAME);
            len = OSTxtSize(addr.sun_path) + sizeof(addr.sun_family);
            ok = connect((SOCKH)hpipe, (struct sockaddr*)&addr, len);

        } else {                                //[port]@ip for internet pipe?
            ptxt += 1;                          //skip @ character
            while (*ptxt == ' ') ptxt += 1;     //allow extra blanks after @
            if (port == 0) port = PIPE_PORT;    //def 1112 port if 0 or blank
            OSMemClear(&inet, sizeof(inet));
            inet.sin_family = AF_INET;
            inet.sin_port = htons(port);
            inet.sin_addr.s_addr = inet_addr(ptxt);
            if (inet.sin_addr.s_addr == 0xFFFFFFFF) {
                phost = (struct hostent*) gethostbyname(ptxt);
                if ((phost == NULL)||(phost->h_addr_list[0] == 0)) {
                    return(SOCK_ERR);           //domain name instead of ip?
                }
                OSMemCopy(&inet.sin_addr.s_addr, phost->h_addr_list[0], sizeof(inet.sin_addr.s_addr));
            }
            ok = connect((SOCKH)hpipe, (struct sockaddr*)&inet, sizeof(inet));
        }
        if (ok == -1) {                         //connect socket
            close((SOCKH)hpipe);
            return(SOCK_ERR);
        }
        return(hpipe);
    }
    return(ECARGUMENT);
    #endif                                      //===
}

//*************************************************************************
//  OSipcSend:
//      Sends data over an IPC pipe.
//      Even though our IPC pipe is stream oriented,
//      this function always sends all bytes before returning
//      unless it returns an error. We also flush and sleep to speed delivery.
//
//  Arguments:
//      int OSipcSend (aint hpipe, void* pv, dword cb)
//      hpipe   Handle from prior OSipcPipe.
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//      
//  Returns:
//      Number of bytes sent or -1 if error.
//*************************************************************************

CSTYLE int xproc
OSipcSend (aint hpipe, void* pv, dword cv) {
    dword done, left;
    #ifdef ISWIN                                //===
    int ok;

    left = cv;
    while (left) {
        ok = WriteFile((HANDLE)hpipe, pv, left, &done, NULL);
        if ((ok == FALSE)||(done <= 0)) return(-1);
        pv = ADDOFF(void*, pv, done);
        left -= done;
    }
    //This seems to hang tests.
    //FlushFileBuffers((HANDLE)hpipe);
    OSSleep(0);
    return(cv);

    #else                                       //===
    left = cv;
    while (left) {
        done = send((SOCKH)hpipe, (char*)pv, left, MSG_NOSIGNAL);
        if (done <= 0) return(-1);
        pv = ADDOFF(void*, pv, done);
        left -= done;
    }
    fsync((int)hpipe);
    OSSleep(0);
    return(cv);
    #endif                                      //===
}

//*************************************************************************
//  OSipcRecv:
//      Reads data from an IPC pipe.
//      Can also check available bytes to be read without reading (peek).
//      But this peek operation is available only in Windows!
//
//  Arguments:
//      int OSipcRecv (aint hpipe, void* pv, dword cb)
//      hpipe   Handle from prior OSipcPipe.
//      pv      Pointer to memory buffer to read to.
//              Use NULL for Windows peek when cb is zero.
//      cb      Number of bytes to read.
//              May be 0 in Windows to peek available bytes without reading.
//      
//  Returns:
//      Number of bytes read (or available if cp is 0) or -1 if error.
//*************************************************************************

CSTYLE int xproc
OSipcRecv (aint hpipe, void* pv, dword cv) {
    #ifdef ISWIN                                //===
    dword done;
    int ok;

    if ((cv == 0)&&(pv == NULL)) {              //peek?
        ok = PeekNamedPipe((HANDLE)hpipe, NULL,0,NULL,&done,NULL);
        return((ok) ? done : 0);                //return available bytes
    }
    ok = ReadFile((HANDLE)hpipe, pv, cv, &done, NULL);
    return((ok) ? done : -1);

    #else                                       //===
    if ((cv == 0)&&(pv == NULL)) return(-1);    //Linux does not allow peek
    int done = recv((SOCKH)hpipe, (char*)pv, cv, 0);
    if (done != (int)cv) return(-1);
    return(done);
    #endif                                      //===
}
#endif              //--- OSBASE

//*************************************************************************
//  OSRandom:
//      Uses a xorshift algothm to produce pseudo random numbers.
//      See: https://en.wikipedia.org/wiki/Xorshift
//      Has a period of 2^32 - 1.
//
//  Arguments:
//      dword OSRandom (dword seed)
//      seed    Initial seed number or previous random number.
//
//  Returns:
//      Next random number in the sequence.
//      Returned numbers are evenly distributed beween 0 and 0xFFFFFFFF,
//*************************************************************************

CSTYLE dword xproc
OSRandom (dword seed) {
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
    return(seed);
}

//*************************************************************************
//  OSCipher:
//      Encrypts or decrypts using TEA Tiny Encryption Algorithm.
//      This code uses the "New Variant" also called "XTEA".
//      Requires data in multiples of 8 bytes (two dwords).
//
//      See http://www.simonshepherd.supanet.com/tea.htm
//      New http://143.53.36.235:8080/tea.htm
//      "Please feel free to use any of this code in your applications.
//      The TEA algorithm (including new-variant TEA) has been placed
//      in the public domain, as have my assembly language implementations." 
//
//      Always encrypts to little endian output.
//      Always decripts from little endian input.
//
//  Arguments:
//      void OSCipher (lint mode, dword* pkey, void* pdata, lint bytes)
//      mode    1 = Encrypt                                 (OS_ENCRYPT)
//              2 = Decrypt (unless mode 256).              (OS_DECRYPT)
//             +8 = Use 8 rather than 32 iterations (much faster).
//            +16 = Use 16 rather than 32 iterations (faster).
//            +32 = Use just single dword 32-bit key. 
//            +64 = Use just double dword 64-bit key.
//           +128 = Update *pkey first key with random version of old key.
//                  This is recommended for +256 stream mode for
//                  sending multiple streams based on the same key.
//            256 = Use stream Cipher of 1 to 256*4 bytes.  (OS_ENBYTES)
//                  Must use mode 256 for both encrypting and decryption!
//                  Can encrypt or decrypt any number of bytes up to 256.
//                  Possibly less secure than non-stream encryption mode,
//                  especially if you send multiple streams with same key.
//                  Also use +128 mode for better security if send multiple.
//      pkey    Points to four dwords of key value (128 bits).
//              If mode +32 then only one dword *pkey is used.
//      pdata   Data to be encrypted or decrypted in place.
//              Must have space for up to seven extra bytes if not qwords.
//              Keeps encrypted data in little-endian byte order.
//              Keeps decrypted original data in local byte order.
//      bytes   Bytes size of pdata.
//              Should be multiple of eight bytes unless +128 mode.
//              Otherwise does next higher multiple of eight bytes
//              and the pdata buffer must be large enough for padded bytes.
//              If zero then assumes zero terminated text string.
//              If +256 stream mode then bytes is limited to 256 max.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSCipher (lint mode, dword* pkey, void* pdata, lint bytes) {
    byte* pb;
    byte* ps;
    dword* pd;
    dword k[4];
    dword y, z, sum, delta, origs, n;
    dword osum, on;
    
    if (bytes == 0) {
        bytes = OSTxtSize((text*)pdata);
    }

    if (mode & 256) {                           //stream cipher
        dword iv[256];

        mode = OS_ENCRYPT + (mode & (8+16+32+64+128));
        if (bytes > 256*4) bytes = 256*4;
        origs = bytes;
        bytes = (bytes + 7) & (~7);             //must be multiple of eight

        iv[0] = OSRandom(*pkey);                //derive initial vector
        n = bytes >> 2;                         //number of dwords
        z = 1;
        while (z < n) {
            iv[z] = OSRandom(iv[z-1]);          //from key (not other key dw)
            z += 1;
        }
        if (mode & 128) {
            *pkey = OSRandom(iv[z-1]);          //update first key randomly
        }
        OSCipher(mode, pkey, iv, bytes);        //get random stream inital value
        pb = (byte*) pdata;                     //which can be even qwords
        ps = (byte*) iv;                        //because too large is okay
        do {
            *pb = *pb ^ *ps++;                  //xor works either for
            pb += 1;                            //encrypt or decrypt
        } while (--origs);
        return;                                 //with zeros after orig bytes
    }

    bytes = (bytes + 7) & (~7);                 //must be multiple of eight
    on = (mode & (8+16)) ? ((mode & 8) ? 8 : 16) : 32;
    osum = 0x9E3779B9 * on;                     //32=ample, 16=sufficient, 8=ok
    pd = (dword*) pdata;                        //get data to be used

    k[0] = *pkey++;                             //get key value
    if (mode & 32)  {                           //just use 32 bit key?
        k[1] = (k[0] * 2053) + 13849;           //psuedo-random key changes
        k[2] = (k[1] * 2053) + 13849;
        k[3] = (k[2] * 2053) + 13849;
    } else if (mode & 64) {                     //just use 64 bit key?
        k[1] = *pkey++;                         //use old pseudo-random than
        k[2] = (k[0] * 2053) + 13849;           //OSRandom for back compatible
        k[3] = (k[1] * 2053) + 13849;           //and also to make different
    } else {                                    //than +128 mode initial vector
        k[1] = *pkey++;
        k[2] = *pkey++;
        k[3] = *pkey;
    }
    if (mode & 128) {
        *pkey = OSRandom(k[3]);                 //update first key randomly
    }

    if (mode & OS_ENCRYPT) {                    //handle encrypt
        while (bytes > 0) {                     //go through data
            n = on;                             //32=ample, 16=sufficient, 8=ok

            y = *pd;                            //get 64 bits of input in y,z
            z = *(pd+1);
            bytes -= 8;

            sum = 0;
            delta = 0x9E3779B9;
            do {
                y += (((z << 4) ^ (z >> 5)) + z) ^ (sum + k[sum & 3]);
                sum += delta;
                z += (((y << 4) ^ (y >> 5)) + y) ^ (sum + k[(sum >> 11) & 3]);
            } while (--n);

            #ifndef CBIG_ENDIAN                 //=== Intel
            *pd++ = y;                          //set 64 bits of output from y,z
            *pd++ = z;
            #else                               //=== Big Endian
            *pd++ = DSWAP(y);
            *pd++ = DSWAP(z);
            #endif                              //===
        }
        return;

    } else {                                    //handle decrypt
        while (bytes > 0) {                     //go through data
            n = on;                             //32=ample, 16=sufficient, 8=ok

            #ifndef CBIG_ENDIAN                 //=== Intel
            y = *pd;                            //get 64 bits of input in y,z
            z = *(pd+1);
            #else                               //=== Big Endian
            y = DSWAP(*pd);
            z = DSWAP(*(pd+1));
            #endif                              //===
            bytes -= 8;

            sum = osum;                         //0xC6EF3720 for n=32
            delta = 0x9E3779B9;                 //sum = delta * n
            do {
                z -= (((y << 4) ^ (y >> 5)) + y) ^ (sum + k[(sum >> 11) & 3]);
                sum -= delta;
                y -= (((z << 4) ^ (z >> 5)) + z) ^ (sum + k[sum & 3]);
            } while (--n);

            *pd++ = y;                          //set 64 bits of output from y,z
            *pd++ = z;
        }
        return;
    }
}

//*************************************************************************
//  OSPassword:
//      Sets new 128 bit encryption key using text password.
//
//      Can also be used to build a 128 bit hash of the password
//      which can be kept in insecure storage for comparison with
//      user-entered passwords without giving away the real password.
//      Beware that the current hash implementation uses XTEA encryption
//      and TEA has been claimed not to be bad as a cryptographic hash function.
//      But it is not clear whether the three duplicate key problem is in XTEA.
//      To allow for SHA hashing in future, we use 192 bits or six dwords and
//      not just four in the hashed password storage because SHA uses 160 bits.
//      It is okay to just store the first five pcry->crkey dwords or 160 bits.
//
//  Arguments:
//      void OSPassword (int mode, dword* pkey, text* puse)
//      mode    0 = Set 128 bit pkey using pass password text        (PASS_INIT)
//             +4 = Set pkey to 192 bit hash of password             (PASS_HASH)
//      pkey    Must be dword[OS_KEY] array for password hash value.
//              PASS_INIT sets pkey four dwords for pass argument.
//              PASS_HASH sets all six pkey dwords.
//      pass    Encryption password to use for mode 0 (text*).
//              May be blank for default key.
//              May be NULL to leave old key.
//              Must not be NULL for mode 4.
//
//  Returns:
//      Nothing except in pcry->cruse and maybe pcry->crkey.
//*************************************************************************

CSTYLE void xproc
OSPassword (int mode, dword* pkey, text* pass) {
    text* ptxt;
    text* puse;
    int bytes, done, maxs;
    dword cruse[OS_KEY];

    if (pass) {                                 //build new key from password?
        pkey[0] = TEA_KEY0;                     //start with default key
        pkey[1] = TEA_KEY1;
        pkey[2] = TEA_KEY2;
        pkey[3] = TEA_KEY3;
        pkey[4] = TEA_KEY4;                     //in case needed by PASS_HASH
        pkey[5] = TEA_KEY5;

        maxs = (mode & PASS_HASH) ? (6*4) : (4*4);
        done = 0;                               //use five dwords for hash
        bytes = 0;                              //but only four for XTEA key
        if (*pass == 0) {
            done = maxs;                        //just use default key?
        }
        while (done < maxs) {                   //distribute password text
            ptxt = pass;                        //amoung the key's four dwords
            puse = (text*) &pkey[0];            //avoid compiler error
            while (*ptxt) {                     //and keep going until use all
                if (bytes == 0) {
                    bytes = maxs;
                    puse = (text*) &pkey[0];
                }
                *puse++ ^= *ptxt++;
                bytes -= 1;
                done += 1;
            }
        }   

        if (mode & PASS_HASH) {                 //create password hash?
            cruse[0] = pkey[0] ^ pkey[4];
            cruse[1] = pkey[1] ^ TEA_HASH;
            cruse[2] = pkey[2] ^ pkey[5];
            cruse[3] = pkey[3] ^ TEA_HASH;
            OSCipher(OS_ENCRYPT, cruse, pkey, maxs);
            return;
        }
    }
    return;
}

#ifndef OSBASE      //--- (Omit because Linux may give prinf warning)
//*************************************************************************
//  OSInputArg:
//      Works like our OSConsole macro to show a prompt and get a line of input.
//      But can optionally use a series of command-line arguments instead,
//      which allows the code to run automatically without prompted user input.
//      Gets an input line either from the console
//      or from a supplied list of command line arguments.
//
//  Arguments:
//      void OSInputArg (text* prompt, text* line, lptext** pargv)
//      prompt  Input prompt to be shown in console.
//              Use NULL to never ask if pargv does not have value.
//      line    Returns input line and should be a SZDISK buffer.
//      pargv   Pointer to a pointer into the array of arguments pointers.
//              This is similar to the main argv array of pointers,
//              but it MUST be terminated with a NULL pointer!
//              Caller's array pointer is increment to the next argument.
//              If pargv is NULL then just uses OSConsole to ask the user.
//              If the next pointer in the array is NULL then uses OSConsole.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE void xproc
OSInputArg (text* prompt, text* line, lptext** pargv) {
    lptext* argv;
    text* ptxt;

    ptxt = NULL;
    if (pargv) {
        argv = *pargv;
        if (argv) {
            ptxt = *argv;
            if (ptxt) {
                argv += 1;
                *pargv = argv;
            }
        }
    }
    if (ptxt) {
        OSTxtStop(line, ptxt, SZDISK);
    } else if (prompt) {
        OSConsole(prompt, line)
    }
    return;
}
#endif                                          //---

#ifdef __cplusplus                              //===
//*************************************************************************
//  StmFile::StmFile:
//  StmFile::~StmFile:
//      Constructor and destructor for StmFile objects.
//
//  Arguments:
//      None
//
//  Returns:
//      Nothing
//*************************************************************************

StmFile::StmFile () {
    m_used = 0;
    m_delete = FALSE;
    m_changed = FALSE;
    m_file = -1;
    m_mem = NULL;
    m_lock = NULL;
    m_posn = 0;
    m_size = 0;
    m_left = 0;
    return;
}

StmFile::~StmFile () {
    StmClose();
    return;
}

//*************************************************************************
//  StmFile::StmOpen:
//      Opens a disk file for an StmFile object which encapulates the handle.
//
//  Arguments:
//      errc StmOpen (int mode, text* pfilen)
//      mode    0 = Open for reading (others can read)             (OS_READ)
//              1 = Open for shared reading and writing (unlocked) (OS_SHARE)
//              2 = Open exclusively so only we can write and read (OS_WRITE)
//              3 = Open exclusively or create if does not exist   (OS_WRITE1)
//              4 = Create new file (exclusive mode assumed)       (OS_CREATE)
//
//           +256 = Create as read-only file                       (OS_NOWRT)
//           +512 = Create as non-executable file (read and write) (OS_NOEXE)
//           +768 = Truncate old file to initial length to rewrite (OS_TRUNC)
//                  Instead of both OS_WRITE | OS_TRUNC flags use: (OS_REWRITE)
//          +1024 = Load entire file into memory for speed.        (OS_INMEM)
//                  Beware that writing only modifies memory
//                  and does nto automatically save back to disk.
//              Settings beginning with OS_NOWRT are mutually exclusive.
//
//      pfilen  Pointer to path and file name to open.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc StmFile::StmOpen (int mode, text* pfilen) {
    byte* pmem;
    dword bytes;
    errc e;

    StmClose();
    m_file = OSOpen(mode, pfilen);
    if (m_file == -1) return(ECOPEN);
    if ((mode & OS_MASKS) == OS_INMEM) {
        e = StmLoad(this, &pmem, &bytes);
        if (e) return(e);
        m_size = bytes;
        m_mem = pmem;
    }
    m_used = 1;
    return(0);
}

//*************************************************************************
//  StmFile::StmClose:
//      Closes the open file, if any.
//
//  Arguments:
//      errc StmClose ()
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc StmFile::StmClose () {
    void* pmem;
    int rv;

    if (m_mem) {
        pmem = m_mem;
        m_mem = NULL;
        m_posn = 0;
        m_size = 0;
        m_left = 0;
        OSFree(pmem);                           //release memory file
    }

    if (m_file != -1) {
        rv = OSClose(m_file);                   //close disk file
        if (rv == -1) return(ECSYS);
        m_file = -1;
    }

    m_used = 0;
    m_changed = FALSE;
    if (m_lock) {
        OSLock(OS_DONE, &m_lock);
        OSLock(OS_LOCKOFF, &m_lock);
        m_lock = NULL;
    }
    return(0);
}

//*************************************************************************
//  StmFile::StmChop:
//      Truncates a disk file at the current seek position.
//      This object's file must be open for writing.
//      Deletes the rest of the file after the current seek point.
//      Note also that StmOpen allows DOTRUNC to chop entire old file.
//      But if you want to read the old file first, then use this method.
//      This method does nothing for Internet files.
//
//  Arguments:
//      errc StmChop (int mode)
//      mode    Reserved for future use and must be zero.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECNOTDONE if not a disk file.
//*************************************************************************

isproc StmFile::StmChop (int mode) {
    int rv;

    if ((m_mem)&&(m_posn < (m_size - 1))) {
        m_posn = m_size - 1;                    //truncate memory file
        return(0);
    }

    rv = OSChop(m_file);
    if (rv == -1) return(ECSYS);
    return(0);
}

//*************************************************************************
//  StmFile::Read:
//      Reads from stream file.
//
//  Arguments:
//      errc Read (void* pv, dword cb, dword* pcbRead)
//      pv      Pointer to memory buffer to read to.
//      cb      Number of bytes to read.
//      pcbRead Pointer to where number of bytes read is returned.
//              May be NULL if not needed.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc StmFile::Read (void* pv, dword cb, dword* pcbRead) {
    byte* pmem;
    dword done, rest;
    lint nb;    
    
    if (m_mem) {                                //handle memory file
        if (m_lock) OSLock(1, &m_lock);
        pmem = m_mem + m_posn;
        rest = m_size - m_posn;
        done = (cb > rest) ? rest : cb;
        OSMemCopy(pv, pmem, done);
        m_posn += done;
        if (m_lock) OSLock(2, &m_lock);
        if (pcbRead) *pcbRead = done;
        return(0);
    }

    nb = OSRead(m_file, pv, cb);                //handle disk file
    if (nb == -1) return(ECREAD);
    if (pcbRead) {
        *pcbRead = (dword) nb;
    }
    return(0);
}

//*************************************************************************
//  StmFile::Write:
//      Writes to stream file.
//
//  Arguments:
//      errc Write (void* pv, dword cb, dword* pcbDone)
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//      pcbDone Pointer to where number of bytes written is returned.
//              May be NULL if not needed.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc StmFile::Write (void* pv, dword cb, dword* pcbDone) {
    byte* pmem;
    dword rest, need;
    lint nb;

    m_changed = TRUE;
    if (m_mem) {                                //handle memory file
        if (m_lock) OSLock(1, &m_lock);
        rest = m_size - m_posn;
        if (cb > rest) {
            need = cb - rest;
            if (need > m_left) {
                rest = (need + m_size + BLKMASK) & (~BLKMASK);
                pmem = (fbyte*) OSReAlloc(m_mem, rest);
                if (pmem) m_mem = pmem;
                if (pmem == NULL) {
                    if (m_lock) OSLock(2, &m_lock);
                    return(ECMEMORY);
                }
                m_left = rest - m_size;
            }
            m_left -= need;
        }
        pmem = m_mem + m_posn;
        OSMemCopy(pmem, (fvoid*)pv, cb);
        m_posn += cb;
        if (m_posn > m_size) m_size = m_posn;
        if (m_lock) OSLock(2, &m_lock);
        if (pcbDone) *pcbDone = cb;
        return(0);
    }

    nb = OSWrite(m_file, pv, cb);               //handle disk file
    if (nb == -1) return(ECWRITE);
    if (pcbDone) {
        *pcbDone = (dword) nb;
    }
    return(0);
}

//*************************************************************************
//  StmFile::Seek:
//      Seek to new location in stream file.
//
//  Arguments:
//      errc Seek (qint move, int org, qword* pnewpos)
//      move    Position to move to relative to org.
//      org     OS_SET = 0 if move is offset from beginning of file.
//              OS_CUR = 1 if move is offset from current file pos.
//              OS_END = 2 if move is offset from end of file.
//      pnewpos Pointer to where new position from file begining is returned.
//              May be NULL if not needed.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc StmFile::Seek (qint move, int org, qword* pnewpos) {
    qint pos;

    if (m_mem) {                                //handle memory file
        pos = (qint) m_posn;
        if (org == OS_SET) {
            pos = move;
        } else if (org == OS_CUR) {
            pos = pos + move;
        } else if (org == OS_END) {
            pos = (qint)m_size + move;
        }
        if (pos > (qint) m_size) {
            pos = (qint) m_size;
        } else if (pos < 0) {
            pos = 0;
        }
        m_posn = (dword) pos;
        if (pnewpos) {
            *pnewpos = (qword) m_posn;
        }
        return(0);
    }

    pos = OSHuge(m_file, move, org);            //handle disk file
    if (pos == -1) return(ECSEEK);
    if (pnewpos) {
        *pnewpos = (qword) pos;
    }
    return(0);
}

//*************************************************************************
//  StmFile::AddRef:
//      Increase object's usage count.
//      This should not be called when object is first opened with StmOpen.
//      Sets up m_lock mutex lock which caller may use if multitasking.
//
//  Arguments:
//      See below.
//
//  Returns:
//      New usage count.
//*************************************************************************

isproc StmFile::AddRef () {
    dword cnt;

    if (m_lock == NULL) OSLock(OS_LOCKNEW, &m_lock);
    if (m_lock) OSLock(OS_GRAB, &m_lock);
    m_used += 1;
    cnt = m_used;
    if (m_lock) OSLock(OS_DONE, &m_lock);
    return(cnt);
}

//*************************************************************************
//  StmFile::Release:
//      Decrease object's usage count.
//      Closes file when count reaches zero.
//      Used in lieu of StmClose when multiple file users.
//
//  Arguments:
//      See below.
//
//  Returns:
//      New usage count.
//*************************************************************************

isproc StmFile::Release () {
    dword cnt;

    if (m_lock) OSLock(OS_GRAB, &m_lock);
    if (m_used == 0) {
        if (m_lock) OSLock(OS_DONE, &m_lock);
        return(0);
    }
    m_used -= 1;
    cnt = m_used;
    if (cnt == 0) {
        StmClose();
        if (m_delete) {
            m_delete = 0;
            delete this;
        }
    }
    if (m_lock) OSLock(OS_DONE, &m_lock);
    return (cnt);
}

//*************************************************************************
//  StmRead:
//      Helper function to read from stream.
//      Returns error if not all bytes are read.
//      Also does nothing if reading zero bytes
//      (since some IStream implementations return a warning for this).
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      errc StmRead (fStm* pstream, fvoid* pv, dword cb)
//      pstream Pointer to IStream object.
//      pv      Pointer to memory buffer to read to.
//      cb      Number of bytes to read.
//
//  Returns:
//      Returns zero if no error.
//      Returns ECREAD if error.
//*************************************************************************

csproc StmRead (fStm* pstream, fvoid* pv, dword cb) {
    errc hres;
    dword done;

    if (cb == 0) return(0);
    hres = pstream->Read(pv, cb, &done);
    if ((hres)||(done < cb)) return(ECREAD);
    return(0);
}

//*************************************************************************
//  StmWrite:
//      Helper function to write to stream.
//      Returns error if not all bytes are written.
//      Also does nothing if writing zero bytes
//      (since some IStream implementations return a warning for this).
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      errc StmWrite (fStm* pstream, fvoid* pv, dword cb)
//      pstream Pointer to IStream object.
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//
//  Returns:
//      Returns zero if no error.
//      Returns ECWRITE if error.
//*************************************************************************

csproc StmWrite (fStm* pstream, fvoid* pv, dword cb) {
    errc hres;
    dword done;

    if (cb == 0) return(0);
    hres = pstream->Write(pv, cb, &done);
    if ((hres)||(done < cb)) return(ECWRITE);
    return(0);
}

//*************************************************************************
//  StmSeek:
//      Helper function to seek to a new stream position.
//      Uses 32-bit dword (or long) position rather than LARGE_INTEGER.
//      Returns error if final position is incorrect for type 0.
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      errc StmSeek (fStm* pstream, long pos, int org, dword* poff)
//      pstream Pointer to IStream object.
//      pos     Desired file byte position relative to the type argument.
//      org     0 => pos is relative to beginning of file       OS_SET
//              1 => pos is relative to the current position    OS_CUR
//              2 => pos is relative to the end of the file     OS_END
//      poff    If not NULL, returns final file byte offset from beginning.
//
//  Returns:
//      Returns zero if no error.
//      Returns ECSEEK if error (or ECARGUMENT if type is invalid).
//*************************************************************************

csproc StmSeek (fStm* pstream, long pos, int org, dword* poff) {
    qword newpos;
    errc e;

    e = pstream->Seek(pos, org, &newpos);
    if (poff) *poff = (dword) newpos;
    if ((e) || ((org == OS_SET)&&(newpos != (qword)((dword)pos)))) {
        return(ECSEEK);
    }
    return(0);
}

//*************************************************************************
//  StmLoad:
//      Helper funtion to load the entire contents of a StmFile into memory.
//      Must still close the file afterward.
//      Must also free the memory allocated and returned by this routine.
//      Always adds one extra zero character to the end of memory.
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      errc StmLoad (fStm* pstream, fvoid* ppmem, fdword* pbytes)
//      pstream Pointer to StmFile object.
//      ppmem   Pointer to where OSAlloc memory with stream image is returned.
//              This is a pointer to a long pointer.
//              But it is defined as a pointer to void to avoid call casting.
//              Caller must latter use OSFree(*ppmem).
//      pbytes  Pointer to where the byte size of the image is returned.
//              This size does not include the extra zero character at end.
//              May be NULL if not needed.
//
//  Returns:
//      Non-zero code if error.
//      If *ppmem is not NULL then allocated memory must
//      be freed by caller (with OSFree) even if error.
//*************************************************************************

csproc StmLoad (fStm* pstream, fvoid* ppmem, fdword* pbytes) {
    fvoid* pmem;
    ftext* pend;
    dword bytes;
    errc e;

    *((lpvoid*)ppmem) = NULL;
    e = StmSeek(pstream, 0, 2, &bytes);
    if (e) return(e);
    e = StmSeek(pstream, 0, 0, NULL);
    if (e) return(e);
    pmem = OSAlloc(bytes + SZCHR);
    if (pmem == NULL) return(ECMEMORY);
    *((lpvoid*)ppmem) = pmem;
    if (pbytes) *pbytes = bytes;
    e = StmRead(pstream, pmem, bytes);
    pend = ADDOFF(ftext*, pmem, bytes);
    *pend = 0;
    return(e);
}

//*************************************************************************
//  StmMemory:
//      Helper function to create a StmFile memory file from a block of memory.
//      Leaves stream positioned at its beginning.
//
//  Arguments:
//      errc StmMemory (lpStm* ppstm, void* pmem, lint cmem)
//      ppstm   Where new StmFile object pointer is returned.
//              Set to NULL if error.
//      pmem    Pointer to memory to be copied into the new stream.
//              A copy is made and caller still owns this memory on return.
//              The copy is automatically freed when the stream is released.
//              No copy is made if cmem is negative (and caller cannot free).
//              May be NULL to create an new empty stream with no contents.
//      cmem    Number of bytes in pmem memory block.
//              Use negative if pmem is an OSAlloc memory block to be used
//              as initial file and owned by the new StmFile without copying
//              and also to use *ppstm object on entry and not use new.
//              Use zero if pmem is an OSAlloc memory block to be used
//              ss initial free memory with zero file size to start,
//              which allows growth with caller-supplied memory.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc StmMemory (lpStm* ppstm, void* pmem, lint cmem) {
    fStm* pstm;
    void* pcopy;
    dword left;

    pstm = NULL;
    pcopy = pmem;
    if ((pmem)&&(cmem > 0)) {
        pcopy = OSAlloc(cmem);                  //allocate memory copy?
        if (pcopy == NULL) {
            *ppstm = NULL;
            return(ECMEMORY);
        }
        OSMemCopy(pcopy, pmem, cmem);
        left = 0;

    } else if (pmem) {                          //use caller's memory?
        cmem = -cmem;                           //size is negative
        pcopy = pmem;
        left = 0;
        pstm = *ppstm;
        if (pstm) {                             //caller's object too?
            pstm->m_used = 0;                   //initialize as if new
            pstm->m_delete = FALSE;
            pstm->m_changed = FALSE;
            pstm->m_file = -1;
            pstm->m_lock = NULL;
            if (cmem == 0) {                    //caller's supplied free space
                left = pstm->m_left;            //with zero initial file size?
            }
        }

    } else {
        pcopy = OSAlloc(BLKSIZE);               //create empty memory block
        if (pcopy == NULL) {
            *ppstm = NULL;
            return(ECMEMORY);
        }
        cmem = 0;
        left = BLKSIZE;
    }
    if (pstm == NULL) {
        pstm = new StmFile;                     //allocate object if needed
    }
    if (pstm == NULL) {
        *ppstm = NULL;
        return(ECMEMORY);
    }
    pstm->m_mem = (byte*) pcopy;                //give it the memory file image
    pstm->m_size = cmem;
    pstm->m_left = left;
    pstm->m_posn = 0;
    *ppstm = pstm;
    return(0);
}

//*************************************************************************
//  StmErrors:
//      Checks for uncommon errors such as out of disk space.
//
//  Arguments:
//      errc StmErrors (int mode, int ecode)
//      mode    1 = Return best error code.
//                  May change ecode to ECDISKFULL or ECPROTECT.
//             +2 = Do not use GetLastError to check uncommon errors.
//      ecode   Default error code to return if GetLastError() has no known error.
//
//  Returns:
//      Best error code.
//*************************************************************************

csproc StmErrors (int mode, int ecode) {
    #ifdef ISWIN                                //===
    errc elast;
    #endif                                      //===
    
    if ((!(mode & 2))&&(ecode != (int)ECPENDING)) {
        #ifdef ISWIN                            //===
        elast = GetLastError();
        if ((elast == ERROR_DISK_FULL)||(elast == ERROR_HANDLE_DISK_FULL)) {
            ecode = ECDISKFULL;
        } else if (elast == ERROR_ACCESS_DENIED) {
            ecode = ECDENIED;
        } else if (  (elast == ERROR_WRITE_PROTECT)
                   ||(elast == ERROR_INVALID_DRIVE)
                   ||(elast == ERROR_NOT_READY)  ) {
            ecode = ECPROTECT;
        } else if ((elast == ERROR_PATH_NOT_FOUND)||(elast == ERROR_INVALID_NAME)) {
            ecode = ECBADNAME;
        } else if ((elast == ERROR_SHARING_VIOLATION)||(elast == ERROR_LOCK_VIOLATION)) {
            ecode = ECSHARING;
        }   
        #else                                   //===
        if (errno == ENOSPC) {
            ecode = ECDISKFULL;
        }
        #endif                                  //===
    }
    return(ecode);
}

//*************************************************************************
//  StmFOpen:
//      Helper funtion to open a file similarly to fopen().
//      Uses new and returns the StmFile object pointer as the pseudo FILE ptr.
//      Use the returned pointer with StmRead, StmWrite, StmSeek
//      or use StmFRead and StmFWrite which are similar to fread and fwrite.
//      Must later use StmFClose to close file and remove StmFile object.
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      fStm* StmFOpen (int mode, ftext* pfilen)
//      mode    0 = Open for reading (others can read)             (OS_READ)
//              1 = Open for shared reading and writing (unlocked) (OS_SHARE)
//              2 = Open exclusively so only we can write and read (OS_WRITE)
//              3 = Open exclusively or create if does not exist   (OS_WRITE1)
//              4 = Create new file (exclusive mode assumed)       (OS_CREATE)
//      pfilen  Pointer to path and file name text.
//
//  Returns:
//      Returns non-NULL pointer "handle" if no error.
//      Note that the STMFILE "handle" is really just a StmFile* pointer.
//      StmFClose must later be called to close this "handle".
//*************************************************************************

ctproc(fStm*) StmFOpen (int mode, ftext* pfilen) {
    StmFile* pfile;
    errc e;

    pfile = new StmFile;
    if (pfile == NULL) return(NULL);
    e = pfile->StmOpen(mode, pfilen);
    if (e == 0) return(pfile);
    delete pfile;
    return(NULL);
}

//*************************************************************************
//  StmFClose:
//      Helper funtion to close a file similarly to fclose().
//      Closes the file and deletes the StmFile object.
//      Should have used StmFOpen to open the file.
//      This is NOT part of the StmFile class.
//
//  Arguments:
//      errc StmFClose (fStm* file)
//      file    Pointer value previously returned by StmFOpen().
//              The file "handle" is really just a StmFile* pointer.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc StmFClose (fStm* file) {
    errc e;
    e = file->StmClose();
    delete file;
    return(e);
}

//*************************************************************************
//  StmFRead:
//      Helper function to read from stream similarly to fread().
//      Unlike StmRead, returns number of bytes read rather than an error code.
//      Also does nothing if reading zero bytes
//      (since some IStream implementations return a warning for this).
//
//      This is NOT part of the StmFile class.
//      Instead it simulates a standard fread using a StmFile object.
//      We use it inside the JPEG library by redefining JFREAD to StmFRead.
//
//  Arguments:
//      int StmFRead (fStm* file, void* pv, int cb)
//      file    Pointer to StmFile object.
//              This should be returned from StmFOpen.
//      pv      Pointer to memory buffer to read to.
//      cb      Number of bytes to read.
//
//  Returns:
//      Number of bytes read.
//*************************************************************************

CSTYLE int StmFRead (void* file, void* pv, int cb) {
    dword done;

    if (cb == 0) return(0);
    ((fStm*)file)->Read(pv, cb, &done);
    return(done);
}

//*************************************************************************
//  StmFWrite:
//      Helper function to write to stream similarly to fwrite().
//      Unlike StmWrite, returns number of bytes read rather than an error code.
//      Also does nothing if writing zero bytes
//      (since some IStream implementations return a warning for this).
//
//      This is NOT part of the StmFile class.
//      Instead it simulates a standard fwrite using a StmFile object.
//      We use it inside the JPEG library by redefining JFWRITE to StmFWrite.
//
//  Arguments:
//      int StmFWrite (fStm* file, void* pv, int cb)
//      file    Pointer to StmFile object.
//              This should be returned from StmFOpen.
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//
//  Returns:
//      Number of bytes written.
//*************************************************************************

CSTYLE int StmFWrite (void* file, void* pv, int cb) {
    dword done;

    if (cb == 0) return(0);
    ((fStm*)file)->Write(pv, cb, &done);
    return((int)done);
}
#endif                                          //=== __cplusplus

//*************************************************************************
//  FOSAlloc:
//  FOSFree:
//      Versions of OSAlloc and OSFree for use by external libraries.
//      Use int instead of aword for size.
//*************************************************************************

CSTYLE void* FOSAlloc (int bytes) {
    return(OSAlloc(bytes));
}

CSTYLE void* FOSFree (void* mem) {
    return(OSFree(mem));
}

//*************************************************************************
//  Similar:
//      Compares two 32-bit integer arrays to see if almost the same.
//
//  Arguments:
//      int Similar (lint* pone, lint* ptwo, lint npts, flt frac)
//      pone    First data array
//      ptwo    Second data array
//      npts    Number of values in both data arrays
//      frac    Fraction of maximum value for maximum difference
//              between corresponding points in the two arrays.
//              If the pone max absolute value is less than 1000,
//              then frac*1000 is the maximum difference.
//              Use .01 to allow up to a 1% difference.
//              Use -1 for the default of .001.
//
//  Returns:
//      Returns index to first mismatch not within given frac tolerance.
//      Returns npts if arrays match within given frac tolerance.
//*************************************************************************

CSTYLE int xproc
Similar (lint* pone, lint* ptwo, lint npts, flt frac) {
    lint max, cnt, val;
    lint* puse;
    FLTTEMP(temp);
        
    max = 0;
    if (!(FLTZERO(frac))) {
        puse = pone;
        cnt = npts;
        do {
            val = *puse++;
            val = ABS(val);
            if (val > max) max = val;           //find maximum absolute value
        } while (--cnt);
    }

    if (frac < 0) frac = 0.001f;                //.1% default if negative
    if (max < 1000) max = 1000;
    frac *= (flt) max;
    FLT2INT(max, frac, temp);                   //maximum delta

    cnt = 0;
    do {
        val = (*pone++) - (*ptwo++);            //compare corresponding points
        val = ABS(val);
        if (val > max) break;                   //mismatch?
        cnt += 1;
    } while (cnt < npts);
    return(cnt);
}

//*************************************************************************
//  OSTest:
//      Test for TriOS.cpp functions.
//      This is a general prototype for function-driven module tests.
//      Test functions should use the same parameters.
//      Compile with /D OSBASE to exclude this function.
//
//      This tests most but not all TriOS functions.
//      It does not exhaustively test each function.
//      It cannot test UI functions.
//
//  Arguments:
//      int OSTest (int mode, text* pdir)
//      mode    0 = Run tests
//             +1 = Create test data if needed before running test. (T_NEW)
//                  When tests match with saved result files,
//                  Useful for test working legacy code when ported.
//             +2 = Verbose operation about each test beforehand.   (T_SEE)
//                  Otherwise only shows failures not hangs.
//             +4 = Continue tests after a failure.                 (T_ALL)
//                  For multiple tests does not stop on failure.
//             +8 = Add test output to log file.                    (T_LOG) 
//            +32 = Skip OSConnect UDP network test with loopback.  (T_DIF) 
//      pdir    Folder path name ending in slash where test data exists.
//              May be NULL to use current working directory.
//
//  Returns:
//      Non-zero code if failure.
//*************************************************************************

#ifndef OSBASE                                  //---
OSTASK_RET OSTestTask (void* parm) {            //OSTestTask:
    lpvoid* pparms = (lpvoid*) parm;            //parm must point to an arry
    void* plock = *pparms++;                    //of three pointers:
    lint* pival = (lint*) *pparms++;            //pparms[0] = lock mutex handle
    OSWAITS* pwaits = (OSWAITS*) *pparms++;     //pparms[1] = our 1234 return value
    OSWait(OS_WAIT, pwaits, NULL, 5000);        //pparms[2] = OSWAITS pointer
    OSLock(OS_GRAB, &plock);                    //pparms[3] = our task handle
    OSSleep(100);                               
    *pival = 1234;                              //return check value
    void* pmap = NULL;
    void* pmem = NULL;
    OSMapMem(OS_MEMOPEN, 0x12345678, &pmap, &pmem, 8);
    if (  (pmem == NULL)                        //check mapped memory value
        ||(((lint*)pmem)[0] != 0xABCD)
        ||(((lint*)pmem)[1] != 0xEF01)  ) {
        *pival = -1234;
    }
    OSMapMem(OS_MEMCLOSE, 0x12345678, &pmap, &pmem, 0);
    OSLock(OS_DONE, &plock);
    return(0);
}

OSTASK_RET OSTestTCP (void* parm) {             //OSTestTCP:
    text line[SZNAME];
    lint ii;
    ConEx cex;
    aint file, sock;
    lpvoid* pparms = (lpvoid*) parm;            //parm must point to an arry
    lint* pival = (lint*) *pparms++;            //pparms[0] = return value
    file = OSConnect(SOCK_APPINIT, 0, NULL, 0, NULL);
    if (file == -1) {
        *pival = -1;
        return(0);
    }
    OSMemClear(&cex, sizeof(ConEx));
    cex.conflag = CI_REUSEAD;                   //just in case
    file = OSConnect(SOCK_SERVER, 0, "127.0.0.1", 5000, &cex);
    if (file == -1) {
        *pival = -2;
        return(0);
    }
    sock = OSConnect(SOCK_ACCEPT, file, "127.0.0.1", 5000, &cex);
    if (sock == -1) {
        *pival = -3;
        return(0);
    }
    ii = OSConRecv(sock, line, SZNAME, NULL);
    if ((ii != SZNAME)||(OSTxtCmpS(line, "This is a test."))) {
        *pival = -5;
        return(0);
    }
    OSTxtCopy(line, "This good test.");
    ii = OSConSend(sock, line, SZNAME, NULL);
    if (ii != SZNAME) {
        *pival = -6;
        return(0);
    }
    sock = OSConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
    if (sock == -1) {
        *pival = -7;
        return(0);
    }
    file = OSConnect(SOCK_CLOSE, file, NULL, 0, NULL);
    if (file == -1) {
        *pival = -8;
        return(0);
    }
    *pival = 0;                                 //good return
    return(0);
}

CSTYLE int xproc
OSTest (int mode, text* pdir) {
    text name[SZPATH];
    text line[SZPATH];
    text* pend;
    text* puse;
    text* ptxt;
    void* pmap;
    void* pmem;
    FFHand find;
    OSWAITS waits;
    void* parms[4];
    flt fdif, fuse, fval;
    aint ival, ii;
    aint file;
    qint qi;
    errc ee;
    #ifdef __cplusplus
    StmFile stm;
    dword pos;
    #endif

    OSPrint((mode|T_ASIS), "OSTest: TriOS.cpp Testing", 0);
    ee = 0;
    if (pdir) {
        pend = OSTxtCopy(name, pdir);
    } else {
        pend = OSLocation(name, OS_CURDIR);     //get current working directory
    }

    // Test OSMemClear, OSMemFill, OSMemCopy, OSMemMatch, OSCipher.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSMemClear, OSMemFill, OSMemCopy, OSMemMatch, OSCipher...", 0);
        ee = ECERROR;
        OSMemClear(line, SZDISK);
        for (ii=0; ii<SZDISK; ++ii) if (line[ii]) break;
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSMemClear error", ii);
            break;
        }
        line[124] = 123;
        ii = OSMemFindB(123, line, SZDISK);
        if (ii != 124) {
            OSPrint(mode+T_FAIL, "OSMemFindB error", ii);
            break;
        }
        ii = OSMemFindD(123, line, SZDISK);
        if (ii != (124>>2)) {
            OSPrint(mode+T_FAIL, "OSMemFindD error", ii);
            break;
        }
        ii = OSMemFindD(125, line, SZDISK);
        if (ii != -1) {
            OSPrint(mode+T_FAIL, "OSMemFindD false positive error", ii);
            break;
        }
        ii = OSMemFindND(0, line, SZDISK);
        if (ii != (124>>2)) {
            OSPrint(mode+T_FAIL, "OSMemFindND error", ii);
            break;
        }
        OSTxtCopy(&line[124], "Stacey Simonoff");
        OSTxtCopy(&line[100], "Sta");
        ptxt = line;
        file = SZDISK;
        ii = OSMemSearch((lpbyte*)&ptxt, (aword*)&file, (byte*)"Stacey ", 7, 6);
        if ((ii != 7)||(OSTxtCmpS(ptxt, "Stacey Simonoff"))) {
            OSPrint(mode+T_FAIL, "OSMemSearch error", ii);
            break;
        }
        ptxt = OSTxtFind("STACEY", line, &line[SZDISK], 1);
        if (OSTxtCmpS(ptxt, "Stacey Simonoff")) {
            OSPrint(mode+T_FAIL, "OSTxtFind error", ii);
            break;
        }
        OSMemFillB(line, 0x12, SZDISK>>1);
        OSMemFillD(&line[SZDISK>>1], 0x12121212, SZDISK>>1);
        for (ii=0; ii<SZDISK; ++ii) if (line[ii] != 0x12) break;
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSMemFill error", ii);
            break;
        }
        OSMemCopy(&line[SZDISK], line, SZDISK);
        for (ii=0; ii<SZDISK; ++ii) if (line[SZDISK+ii] != 0x12) break;
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSMemCopy error", ii);
            break;
        }
        OSMemCopyR(&line[SZDISK], line, SZDISK);
        for (ii=0; ii<SZDISK; ++ii) if (line[SZDISK+ii] != 0x12) break;
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSMemCopyR error", ii);
            break;
        }
        ii = OSMemMatch(&line[SZDISK], line, SZDISK);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSMemMatch error", ii);
            break;
        }
        ii = OSMemDiffs(&line[SZDISK], line, SZDISK);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSMemDiffs error", ii);
            break;
        }
        ival = 0xABCD4321;
        OSCipher(1+32, (dword*)&ival, line, SZDISK);
        OSCipher(2+32, (dword*)&ival, line, SZDISK);
        ii = OSMemMatch(line, &line[SZDISK], SZDISK);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSCript error", ii);
            break;
        }
        ee = 0;
        break;
    }

    // Test OSTxtSize, OSTxtCopy, etc functions.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSTxtSize, OSTxtCopy, OSTxtCmp...", 0);
        ee = ECERROR;
        ii = OSTxtSize("Testing");
        if (ii != 8) {
            OSPrint(mode+T_FAIL, "OSTxtSize error", ii);
            break;
        }
        puse = "More testing...";
        ptxt = OSTxtCopy(line, puse);
        ii = OSTxtCmpS(line, puse);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTxtCmpS error", ii);
            break;
        }
        line[1] = 'O';
        ii = OSTxtCmp(line, puse);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTxtCmp error", ii);
            break;
        }
        ptxt = OSTxtStop(line, puse, 5);
        ii = OSTxtCmpS(line, "More");
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTxtArop error", ii);
            break;
        }
        ptxt = NULL;
        OSTxtSafe(&ptxt, puse, 0);
        ii = OSTxtCmpS(ptxt, puse);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTxtSafe error", ii);
            break;
        }
        ptxt = (text*) OSFree(ptxt);
        if (ptxt != NULL) {
            OSPrint(mode+T_FAIL, "OSTxtSafe OSFree error", 0);
            break;
        }
        ee = 0;
        break;
    }

    // Test text number get/put functions.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSIntGet, OSIntPut, etc...", 0);
        ee = ECERROR;
        ptxt = line;
        ptxt = OSIntPut(0, ptxt);
        *ptxt++ = ',';
        ptxt = OSIntPut(123456, ptxt);
        *ptxt++ = ',';
        ptxt = OSIntPut(-123, ptxt);
        *ptxt++ = ',';
        ptxt = OSHexPut(0x1234ABCD, ptxt);
        *ptxt++ = ',';
        ptxt = OSFloatPut(1.23, ptxt, 0, 0);
        *ptxt++ = ',';
        ptxt = OSFloatPut(-1.23, ptxt, 0, 0);
        ii = OSTxtCmpS(line, "0,123456,-123,1234ABCD,1.23,-1.23");
        if (ii != 0) {
            OSPrintS(mode+T_FAIL, "Text Number I/O mismatch", line, ii);
            break;
        }
        ii = OSIntGet(line, &ptxt);
        ptxt += 1;
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSIntPut+OSIntGet mismatch", ii);
            break;
        }
        ptxt = line;
        ii = OSIntGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSIntPut+OSIntGet mismatch 0", ii);
            break;
        }
        ii = OSIntGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != 123456) {
            OSPrint(mode+T_FAIL, "OSIntPut+OSIntGet mismatch 123456", ii);
            break;
        }
        ii = OSIntGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != -123) {
            OSPrint(mode+T_FAIL, "OSIntPut+OSIntGet mismatch -123", ii);
            break;
        }
        ii = OSHexGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != 0x1234ABCD) {
            OSPrint(mode+T_FAIL, "OSHexPut+OSHexGet mismatch 12345678", ii);
            break;
        }
        fval = (flt) OSFloatGet(ptxt, &ptxt);
        ptxt += 1;
        if (fval != 1.23f) {
            OSPrintF(mode+T_FAIL, "OSFloatPut+OSFloatGet mismatch", 1.23f, fval, 0,0);
            break;
        }
        fval = (flt) OSFloatGet(ptxt, &ptxt);
        ptxt += 1;
        if (fval != -1.23f) {
            OSPrintF(mode+T_FAIL, "OSFloatPut+OSFloatGet mismatch", -1.23f, fval, 0,0);
            break;
        }
        OSTxtCopy(line, "0x123,-123,123");
        ptxt = line;
        ii = OSNumGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != 0x123) {
            OSPrint(mode+T_FAIL, "OSNumGet error 0x123", ii);
            break;
        }
        ii = OSNumGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != -123) {
            OSPrint(mode+T_FAIL, "OSNumGet error 123", ii);
            break;
        }
        ii = OSNumGet(ptxt, &ptxt);
        ptxt += 1;
        if (ii != 123) {
            OSPrint(mode+T_FAIL, "OSNumGet error -123", ii);
            break;
        }
        ee = 0;
        break;
    }

    // Test text filename parsing functions.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing filename parse functions...", 0);
        ee = ECERROR;
        OSTxtCopy(line, "C:/Folder\\Filename.ext");
        ptxt = line;
        ii = OSFileRoot(&line[512], line);
        if ((ii != 3)||(OSTxtCmpS(&line[512], "C:/"))) {
            OSPrintS(mode+T_FAIL, "OSFileRoot error", &line[512], ii);
            break;
        }
        ii = OSFileDir(&line[512], line);
        if ((ii != 10)||(OSTxtCmpS(&line[512], "C:/Folder\\"))) {
            OSPrintS(mode+T_FAIL, "OSFileDir error", &line[512], ii);
            break;
        }
        ii = OSFileBase(&line[512], line, OS_NOEXT);
        if ((ii != 8)||(OSTxtCmpS(&line[512], "Filename"))) {
            OSPrintS(mode+T_FAIL, "OSFileBase error", &line[512], ii);
            break;
        }
        ii = OSFileBase(&line[512], line, OS_EXTN);
        if ((ii != 12)||(OSTxtCmpS(&line[512], "Filename.ext"))) {
            OSPrintS(mode+T_FAIL, "OSFileBase error", &line[512], ii);
            break;
        }
        ii = OSFileExt(&line[512], line);
        if ((ii != 0x545845)||(OSTxtCmpS(&line[512], ".ext"))) {
            OSPrintS(mode+T_FAIL, "OSFileExt error", &line[512], ii);
            break;
        }
        ii = OSNameInt("ext", NULL);
        if (ii != 0x545845) {
            OSPrint(mode+T_FAIL, "OSNameInt error", ii);
            break;
        }
        ii = OSFileType(&line[512], line, ".new");
        if ((ii != 22)||(OSTxtCmpS(&line[512], "C:/Folder\\Filename.new"))) {
            OSPrintS(mode+T_FAIL, "OSFileType error", &line[512], ii);
            break;
        }
        ee = 0;
        break;
    }

    // Test transendental math functions.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSSqrt OSCos OSSin etc...", 0);
        ee = ECERROR;
        fval = (flt) OSSqrt(123.45);
        fuse = 11.1108f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSSqrt error", fval, fuse, 0, 0);
            break;
        }
        fval = (flt) OSCos(123.45);
        fuse = -0.59952686f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSCos error", fval, fuse, 0, 0);
            break;
        }
        fval = (flt) OSSin(123.45);
        fuse = -0.8003546f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSSin error", fval, fuse, 0, 0);
            break;
        }
        fval = (flt) OSArcTan(123.45);
        fuse = 1.562696f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSArcTan error", fval, fuse, 0, 0);
            break;
        }
        fval = (flt) OSRaise(123.45, 2.3);
        fuse = 64629.30963f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSRaise error", fval, fuse, 0, 0);
            break;
        }
        fval = (flt) OSLog10(123.45);
        fuse = 2.0914910f;
        fdif = fval - fuse;
        fdif = ABS(fdif);
        if (fdif > T_ALMOST) {
            OSPrintF(mode+T_FAIL, "OSArcTan error", fval, fuse, 0, 0);
            break;
        }
        ee = 0;
        break;
    }

    // Test OSAlloc and OSFree memory functions
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSAlloc OSFree memory allocation...", 0);
        ee = ECERROR;
        pmem = OSAlloc(1024);
        if (pmem == NULL) {
            OSPrint(mode+T_FAIL, "OSAlloc error", OSLastError(0));
            break;
        }
        OSMemClear(pmem, 1024);
        pmem = OSReAlloc(pmem, 2048);
        if (pmem == NULL) {
            OSPrint(mode+T_FAIL, "OSReAlloc error", OSLastError(0));
            break;
        }
        OSMemClear(pmem, 1024);
        pmem = OSFree(pmem);
        if (pmem != NULL) {
            OSPrint(mode+T_FAIL, "OSFree error", OSLastError(0));
            break;
        }
        ee = 0;
        break;
    }

    // Test OSOpen and other File I/O.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSOpen and other File I/Os...", 0);
        ee = ECERROR;
        OSTxtCopy(pend, "Test.bin");
        file = OSOpen(OS_CREATE, name);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "OSOpen create error", (lint)file);
            break;
        }
        OSMemFillD(line, 0x12345678, SZDISK);
        ii = OSWrite(file, line, SZDISK);
        if (ii < SZDISK) {
            OSPrint(mode+T_FAIL, "OSWrite error", ii);
            break;
        }
        ii = OSSeek(file, 2, OS_SET);
        if (ii != 2) {
            OSPrint(mode+T_FAIL, "OSSeek error", ii);
            break;
        }
        ii = OSRead(file, &ival, 4);
        if ((ii != 4)||(ival != 0x56781234)) {
            OSPrint(mode+T_FAIL, "OSRead error", ii);
            break;
        }
        qi = OSHuge(file, 2, OS_SET);
        if (qi != 2) {
            OSPrint(mode+T_FAIL, "OSHuge error", ii);
            break;
        }
        ii = OSRead(file, &ival, 4);
        if ((ii != 4)||(ival != 0x56781234)) {
            OSPrint(mode+T_FAIL, "OSRead second error", ii);
            break;
        }
        ii = OSClose(file);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSClose error", ii);
            break;
        }
        file = OSOpen(OS_READ, name);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "OSOpen read error", (lint)file);
            break;
        }
        ii = OSRead(file, &line[SZDISK], SZDISK);
        if (ii < SZDISK) {
            OSPrint(mode+T_FAIL, "OSRead error", ii);
            break;
        }
        ii = OSMemMatch(line, &line[SZDISK], SZDISK);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSRead OSMemMatch error", ii);
            break;
        }
        OSClose(file);
        ptxt = line + OSFileDir(line, name);
        ptxt = ptxt + OSFileBase(ptxt, name, OS_NOEXT);
        OSTxtCopy(ptxt, "2.bin");
        file = OSOpen(OS_CREATE, line);         //want Test.bin and Test2.bin
        if (file == -1) {
            OSPrint(mode+T_FAIL, "Create Test2/bom error", (lint)file);
            break;
        }
        OSWrite(file, (void*)"Test", 4);
        OSClose(file);
        *ptxt++ = '*';
        OSFileExt(ptxt, name);
        ii = OSFindFile(FF_FIRST, line, &find);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSFindFile first error", ii);
            break;
        }
        ii = OSTxtCmpS(line, name);
        if (ii != 0) {
            OSPrintS(mode+T_FAIL, "OSFindFile name error", line, ii);
            break;
        }
        ii = OSFindFile(FF_NEXT, line, &find);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSFindFile next error", ii);
            break;
        }
        OSFileBase(&line[2000], line, OS_EXTN);
        ii = OSTxtCmpS(&line[2000], "Test2.bin");
        if (ii != 0) {
            OSPrintS(mode+T_FAIL, "OSFindFile name2 error", line, ii);
            break;
        }
        ii = OSFindFile(FF_NEXT, line, &find);
        if (ii != -1) {
            OSPrintS(mode+T_FAIL, "OSFindFile last error", line, ii);
            break;
        }
        ii = OSFindFile(FF_DONE, NULL, &find);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSFindFile done error", ii);
            break;
        }
        ii = OSExists(0, name);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSExists false negative error", ii);
            break;
        }
        line[0] = '!';
        ii = OSExists(0, line);
        if (ii != -1) {
            OSPrint(mode+T_FAIL, "OSExists false positive error", ii);
            break;
        }
        ee = 0;
        break;
    }

    #ifdef __cplusplus                          //===
    // Test StmFile File I/O class.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing StmFile I/Os...", 0);
        ee = ECERROR;
        OSTxtCopy(pend, "Test.bin");

        ee = stm.StmOpen(OS_CREATE, name);
        if (ee) {
            OSPrint(mode+T_FAIL, "StmOpen create error", (lint)file);
            break;
        }
        OSMemFillD(line, 0x12345678, SZDISK);
        ee = stm.Write(line, SZDISK, &pos);
        if ((ee)||(pos < SZDISK)) {
            OSPrint(mode+T_FAIL, "StmWrite error", ii);
            if (ee == 0) ee = ECERROR;
            break;
        }
        ee = StmSeek(&stm, 2, OS_SET, &pos);
        if ((ee)||(pos != 2)) {
            OSPrint(mode+T_FAIL, "StmSeek error", ii);
            if (ee == 0) ee = ECERROR;
            break;
        }
        ee = stm.Read(&ival, 4, &pos);
        if ((ee)||(ival != 0x56781234)||(pos != 4)) {
            OSPrint(mode+T_FAIL, "StmRead error", ii);
            if (ee == 0) ee = ECERROR;
            break;
        }
        ee = stm.StmClose();
        if (ee) {
            OSPrint(mode+T_FAIL, "StmClose error", ii);
            break;
        }
        ee = stm.StmOpen(OS_READ, name);
        if (ee) {
            OSPrint(mode+T_FAIL, "StmOpen read error", (lint)file);
            break;
        }
        ee = StmRead(&stm, &line[SZDISK], SZDISK);
        if (ee) {
            OSPrint(mode+T_FAIL, "StmRead error", ii);
            break;
        }
        ii = OSMemMatch(line, &line[SZDISK], SZDISK);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "StmRead OSMemMatch error", ii);
            break;
        }
        stm.StmClose();
        ee = 0;
        break;
    }
    #endif                                      //===

    // Test OSTask, OSLock, OSWait, OSMapMem.
    while (ee == 0) {                           //pseudo loop for error breaks
        OSPrint(mode, "Testing OSTask, OSLock, OSWait, OSMapMem...", 0);
        ee = ECERROR;
        pmem = NULL;
        pmap = NULL;
        ii = OSMapMem(OS_MEMCREAT, 0x12345678, &pmap, &pmem, 8);
        if ((ii != 0)||(pmem == NULL)) {
            OSPrint(mode+T_FAIL, "OSMapMem create error", ii);
            break;
        }
        ((lint*)pmem)[0] = 0xABCD;              //write values into mapped memory
        ((lint*)pmem)[1] = 0xEF01;
        ii = OSLock(OS_LOCKNEW, &parms[0]);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSLock create error", ii);
            break;
        }
        ii = OSLock(OS_GRAB, &parms[0]);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSLock Grab error", ii);
            break;
        }
        parms[2] = &waits;
        ii = OSWait(OS_LOCKNEW, &waits, NULL, 0);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSWait create error", ii);
            break;
        }
        ival = 0;
        parms[1] = (void*) &ival;
        ii = OSTask(OS_TASKNEW, &parms[3], OSTestTask, parms);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTask error", ii);
            break;
        }
        OSSleep(100);
        ii = OSWait(OS_WAKE, &waits, NULL, 0);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OS_WAKE error", ii);
            break;
        }
        ii = OSLock(OS_DONE, &parms[0]);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSLock Done error", ii);
            break;
        }
        OSSleep(100);
        OSLock(OS_GRAB, &parms[0]);
        if (ival != 1234) {
            OSPrintI(mode+T_FAIL, "Task return value error", ival);
            break;
        }
        OSLock(OS_DONE, &parms[0]);
        ii = OSLock(OS_LOCKOFF, &parms[0]);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSLock destroy error", ii);
            break;
        }
        ii = OSWait(OS_LOCKOFF, &waits, NULL, 0);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSWait destroy error", ii);
            break;
        }
        ii = OSMapMem(OS_MEMCLOSE, 0x12345678, &pmap, &pmem, 0);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSMapMem close error", ii);
            break;
        }
        ee = 0;
        break;
    }

    #ifndef OSBASE                              //===
    // Test OSConnect OSConSend OSConRecv TCP+UDP with loopback.
    while ((ee == 0)&&(!(mode & T_DIF)))  {     //pseudo loop for error breaks
        ConEx cex;
        OSPrint(mode, "Testing OSConnect OSConSend OSConRecv TCP+UDP with loopback...", 0);
        ee = ECERROR;
        file = OSConnect(SOCK_APPINIT, 0, NULL, 0, NULL);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "SOCK_APINIT error", file);
            break;
        }

        ival = 1;
        parms[0] = (void*) &ival;
        ii = OSTask(OS_TASKNEW, &parms[3], OSTestTCP, parms);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTask error", ii);
            break;
        }
        OSSleep(100);
        if (ival < 0) {
            OSPrint(mode+T_FAIL, "OSTestTCP task preliminary error", ival);
            break;
        }
        OSMemClear(&cex, sizeof(ConEx));
        cex.conflag = CI_REUSEAD;               //just in case
        file = OSConnect(SOCK_CLIENT, 0, "127.0.0.1", 5000, &cex);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "SOCK_CLIENT connect error", file);
            break;
        }
        OSMemClear(line, SZNAME);
        OSTxtCopy(line, "This is a test.");
        ii = OSConSend(file, line, SZNAME, NULL);
        if (ii != SZNAME) {
            OSPrint(mode+T_FAIL, "OSConSend TCP error", file);
            break;
        }
        ii = OSConRecv(file, line, SZNAME, NULL);
        if ((ii != SZNAME)||(OSTxtCmpS(line, "This good test."))) {
            OSPrintS(mode+T_FAIL, "OSConRecv TCP error", line, file);
            break;
        }
        OSSleep(200);
        if (ival != 0) {
            OSPrint(mode+T_FAIL, "OSTestTCP task error", ival);
            break;
        }
        file = OSConnect(SOCK_CLOSE, file, NULL, 0, NULL);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "SOCK_CLOSE TCP error", file);
            break;
        }

        OSMemClear(&cex, sizeof(ConEx));
        cex.conflag = CI_REUSEAD;               //omit 127.0.0.1 for Linux
        file = OSConnect(SOCK_USEUDP, 0, "127.0.0.1", 5000, &cex);
        if (file == -1) {                       //the IP is not really needed
            OSPrint(mode+T_FAIL, "SOCK_USEUDP error", file);
            break;
        }
        OSTxtCopy(cex.conaddr, "127.0.0.1");
        cex.conport = 5000;
        for (ii=0; ii<SZDISK; ++ii) if (line[ii]) break;
        ii = OSConSend(file, line, SZDISK, &cex);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSConSend error", ii);
            break;
        }
        OSSleep(100);
        ii = OSConRecv(file, &line[512], SZDISK, &cex);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSConRecv error", ii);
            break;
        }
        ii = OSMemMatch(line, &line[512], SZDISK);
        if (ii != SZDISK) {
            OSPrint(mode+T_FAIL, "OSConRecv OSMemMatch error", ii);
            break;
        }
        file = OSConnect(SOCK_CLOSE, file, NULL, 0, NULL);
        if (file == -1) {
            OSPrint(mode+T_FAIL, "SOCK_CLOSE UDP error", file);
            break;
        }
        ee = 0;
        break;
    }
    #endif                                      //===

    // Skip testing OSIntGet, OSIntPut, OSFloatGet, OSFloatPut, OSFileBase, etc.
    // These have been working for years and are not OS dependent.
    return(ee);
}
#endif                                          //---