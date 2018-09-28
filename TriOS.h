//*************************************************************************
//  FILENAME:   TriOS.h
//  AUTHOR:     SCSimonoff 10-1-96
//
//      Portable OS Utility Functions.
//      Includes CDefs.h and CError.h
//
//  This software was created 1996-2018 by SCSimonoff.
//  It may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _TriOS          //Only define once if include multiple times
#define _TriOS
#include <stdio.h>                              //allows for debugging printf
#ifdef WIN32                                    //===
#define  NOMINMAX                               //avoid min,max conflict with std
#include <Windows.h>
#else                                           //===
#include <unistd.h>
#include <pthread.h>
#endif                                          //===
#include "CDefs.h"
#include "CError.h"

// Define OSTask task function pointer and structures used as function args.

#ifdef WIN32                                    //===
typedef dword (stdcall sfar* LPFTASK) (void* parm);
#define OSTASK_RET dword stdcall
#define OSTASK_RT dword
#define OSTASK_0 0
#else                                           //===
typedef void* (*LPFTASK) (void* parm);
#define OSTASK_RET void*
#define OSTASK_RT void*
#define OSTASK_0 NULL
#endif                                          //===

// Define OSWAITS structure used with OSWait.
// Note that the waits member is the count of waiting tasks and is public for
// reading.  The other members should not normally be used except by OSWait.

DEFSTRUCT(OSWAITS)                              //OSWAITS: holds mutex+cond
#ifdef WIN32                                    //=== Windows
lint               alloc;
lint               waits;
lint               wokes;
CRITICAL_SECTION   mutex;
HANDLE             event;
#else                                           //=== Linux
lint               alloc;
lint               waits;
lint               wokes;
pthread_mutex_t    mutex;
pthread_cond_t     cond;
#endif                                          //===
DEFENDING(OSWAITS)
typedef OSWAITS* lpOSWAITS;

// Define FFHand structure used with OSFindFile.

typedef struct  {           //FFHand:
qword   ffBytes;            //Byte size of the file found
qword   ffNewms;            //Time created in epoch milliseconds
qword   ffModms;            //Time modified in epoch milliseconds
qdtime  ffCreate;           //OSDateTime style date & time when created
qdtime  ffModify;           //OSDateTime style date & time when last modified
void*   ffHandle;           //Handle (used internally)
dword   ffAttrib;           //Attributes (see AT_... definitions below)
dword   ffMatchFlg;         //Matching flags: 1=base, 2=Ext (used internally)
dword   ffExtension;        //Available for use by caller
text    ffMatchNam[256];    //Base filename beginning to match (used internally)
text    ffMatchExt[256];    //Extension beginning to match (used internally)
} FFHand;
typedef FFHand sfar fFFHand;
typedef fFFHand* sfar lpFFHand;

#define AT_READONLY         0x00000001
#define AT_HIDDEN           0x00000002
#define AT_SYSTEM           0x00000004
#define AT_DIRECTORY        0x00000010
#define AT_ARCHIVE          0x00000020
#define AT_NORMAL           0x00000080
#define AT_TEMPORARY        0x00000100

// OSConnect sockets extra information structure.
// Beware that Windows does not support CI_WAITALL and CI_DONTWAIT!
// Note that maxcons, sendbuf and recvbuf must be set for OSConnect.
// If lingers is 0 then does not set SO_LINGER and close usually aborts
// any pending sends. If lingers is -1 close definately aborts pending sends.

typedef struct {        //ConEx: Structure passed to OSConnect
lint    conflag;        //Flags defined below
lint    maxcons;        //Maximum waiting server connections (0 for default)
lint    sendbuf;        //Send buffer bytes (bytes/sec * delay, 0 for default)
lint    recvbuf;        //Receive buffer bytes (0 for default)
lint    lingers;        //Secs to finish sending on close (0=default,-1=none)
void*   winpcap;        //Used for WinPcap handle raw ethernet under Windows
lint    ourport;        //Keeps our receive port
lint    conport;        //Returns client's port or gives OSConSend port (LSB 1st)
text    conaddr[SZDISK];//Returns client's IP text or supplies OSConSend address
lint    ipserve;        //Server IP4 for TCP accept or connected socket (LSB 1st)
lint    rawcode;        //Returns raw error code (which varies with OS) or 0
lint    error;          //Returns SOCKERR_? error code or 0
} ConEx;
typedef ConEx* lpConEx;

#define CI_PEEK     1   //Set in conflag to not consume OSConRecv data
#define CI_WAITALL  2   //Set in conflag to not wait in OSConRecv for all bytes
#define CI_DONTWAIT 4   //Set in conflag to not wait to OSConSend/Recv for bytes
#define CI_NODELAY  8   //Set in conflag to disable Nagle algorithm for TCP send
#define CI_REUSEAD  16  //Set in conflag to restart server without 2 min wait
#define CI_USERAW   32  //Set in conflag to use Raw Ethernet packets
#define CI_SKIPERR  128 //Set in conflag to not return rawcode and error values
#define CI_USEUDP   256 //Set in conflag to use UDP OSConSend & OSConRecv
#define CI_NOBLOCK  512 //Set in conflag for OSConnect to make all I/O non-blocking
#define CI_RAWPACK 1024 //Set in conflag for UDP OSonRecv/Send raw ethernet header
#define CI_TCPSIZE 2048 //Set in conflag for TCP msg protocol and size words first
#define CI_CANCAST 4096 //Set in conflag to allow broadcast send and receive

// Structure used for OSComPort serial communications.

#define CIN_INS 4096    //Input buffer size

typedef struct {        //ComInfo:
aint    cin_hand;       //Serial port handle
void*   cin_task;       //Open task handle
int     cin_errs;       //Non-zero code if recent errors
int     cin_baud;       //Baud rate to use if CIN_BAUD mode
int     cin_mode;       //Open mode with CIN_TIME bit if packetizing
void*   cin_lock;       //Read task mutex may be used by c_callback task
dword   cin_head;       //Offset to head of ins buffer where new data goes
dword   cin_tail;       //Offset to tail of ins buffer where data is read
dword   cin_time;       //Packet timeout ms (0 = 5ms default)
dword   cin_last;       //Last character arrival time (ms)
dword   cin_have;       //Number of characters read by task
dword   cin_used;       //Number of characters used by API
dword   c_lfhave;       //Number of linefeeds read by task
dword   c_lfused;       //Number of linefeeds used by API
void*   c_callparm;     //First argument passed to c_callback
LPBACK  c_callback;     //Callback passed each ...\n info string or NULL
byte    c_in[CIN_INS];  //Input circular buffer
} ComInfo;
typedef ComInfo* lpComInfo;

#define CIN_FINI   0x00 //Mode to close port
#define CIN_OPEN   0x01 //Mode to open port
#define CIN_READ   0x02 //Mode to read input
#define CIN_SEND   0x04 //Mode to write output
#define CIN_9600   0x10 //Mode for 9600 baud
#define CIN_115200 0x20 //Mode for 115200 baud
#define CIN_BAUD   0x40 //Mode for pcom->cin_baud
#define CIN_PLUS   0x80 //Mode for pcom->cin_baud C_BITS+C_PARI+C_STOP
#define CIN_TIME  0x100 //Mode for timeout packets instead of stream
#define CIN_CALL 0x1000 //Mode to allow cin_callback to be used if not NULL
#define CIN_NLOK 0x2000 //Mode to not use cin_lock read task mutex lock
#define CIN_NZ   0x4000 //Mode to read as much as can and not zero terminate
#define CIN_LF   0x8000 //Mode to stop read on linefeed
#define CIN_TO  0x10000 //Mode to return read after timeout

#define CIN_TOUT      5 //Default packet timeout ms
#define CIN_TRET     49 //Default packet timeout ms if CIN_TO

#define C_BAUD 0x00FFFFF //cin_baud mask for baud
#define C_BITS 0x0300000 //cin_baud mask for data bits 5,6,7,8
#define C_PARI 0x1C00000 //cin_baud mask for parity E=0,O=1,Space,Mark,N=4
#define C_STOP 0x6000000 //cin_baud mask for stop bits 1=0, 1.5=1 (not linux), 2=2
#define C_SHFT  20       //cin_baud shift for 0x03 data + 0x1C parity + 0x20 stop
#define C_AT91 (20-6)    //cin_baud shift for 0xC0 data + 0x700 parity + 0x800 stop

#define C_7DAT 0x0200000 //cin_baud added for 7 data bits
#define C_NPAR 0x1000000 //cin_baud added for no parity
#define C_OPAR 0x0800000 //cin_baud added for odd parity
#define C_2STP 0x4000000 //cin_baud added for 2 stop bits

// Structure used for OSMMap Hardware I/O.
// For speed, caller may use returned mm_base to directly read and write.

typedef struct {        //CMMap:
aint    mm_hand;        //Open map handle
dword   mm_addr;        //Base hardware address
dword   mm_size;        //Byte size of mapped memory
byte*   mm_base;        //Base mapped pointer
} CMMap;
typedef CMMap* lpCMMap;

#define MM_FINI    0x00 //Mode to close memory map
#define MM_OPEN    0x01 //Mode to open memory map
#define MM_READ    0x02 //Mode to read dword at given map offset
#define MM_WRITE   0x04 //Mode to write dword at given map address
#define MM_BYTE    0x08 //Mode to write byte at given map address

// Windows uses pointer handles while Linux uses int for files and sockets.
// SOCKH allows type casts to vary with the OS.

#ifdef ISWIN
#define SOCKH SOCKET
#else
#define SOCKH int
#endif

// Structure encapsulating OS-dependent window information.

typedef struct {        //OSApp:
void*     fwin;         //FLTK window or NULL
#ifdef WIN32            //===
HINSTANCE hins;         //Windows app instance
HWND      hwnd;         //Windows app main window
HDC       hdc;          //Draw handle
#else                   //===
void*     hins;         //Windows app instance
void*     hwnd;         //Windows app main window
void*     hdc;          //Draw handle
#endif                  //===
} OSApp;
typedef OSApp* lpOSApp;

// Structure used to pass font information.

typedef struct {        //SFont:
lint    sfHeight;       //Height (as in LOGFONT's lfHeight)
lint    sfStyle;        //Style flags (see below)
lint    sfColor;        //Foreground Font Color (CSTD,CRGB,CBGR in CDraw.h)
text    sfName[SZNAME]; //Font Name (if FFNAMED flag)
} SFont;
typedef SFont* lpSFont;

#define FFBOLD      1       //Bold ifStyle flag (700 weight not 400)
#define FFITALIC    2       //Italic ifStyle flag
#define FFUNDER     4       //Underlined ifStyle flag
#define FFSTRIKE    8       //Strikeout ifStyle flag
#define FFNAMED     128     //Use sfName font name (not standard names)

// Define structure for OSRegIni registry information.

typedef struct _IReg {          //IReg: Registry memory block
text*   irBeg;                  //Pointer to beginning of registry text
dword   irSiz;                  //Bytes of registry text
dword   irCur;                  //Byte offset to current line of registry text
dword   irAll;                  //Allocated bytes in irBeg memory buffer
aint    irFil;                  //Open file handle
} IReg;
typedef IReg* lpIReg;

#define IR_BLOCK    256         //irAll block size increment (power of two)
#define IR_BSHFT      8         //Block size as power of two

#define CREG_SEC1   1           //OSRegIni Find first line of a section
#define CREG_NEXT   2           //OSRegIni Get key and value for next sec line
#define CREG_GET    4           //OSRegIni Get key value for given section
#define CREG_NEW    16          //OSRegIni Add new section at end of file
#define CREG_ADD    32          //OSRegIni Add new key and value line
#define CREG_DEL    64          //OSRegIni Delete current key and value line
#define CREG_MORE   128         //OSRegIni Keep going and do additional modes
#define CREG_LOAD   0x100       //OSRegIni Load .ini file for read only
#define CREG_OPEN   0x200       //OSRegIni Load .ini and keep file open
#define CREG_SAVE   0x400       //OSRegIni Save .ini file
#define CREG_FREE   0x800       //OSRegIni Free memory
#define CREG_BFOR   0x1000      //OSRegIni Add line before current line not after
#define CREG_CRLF   0x2000      //OSRegIni Add lines end with Windows CR and LF
#define CREG_HUGE   0x4000      //OSRegIni Assume unlimited pval buffer size
#define CREG_SIZE   0x8000      //OSRegIni Return required pval buffer size
#define CREG_CLOSE  (CREG_SAVE+CREG_FREE)
#define CREG_SET    (CREG_ADD+CREG_GET+CREG_DEL+CREG_MORE+CREG_BFOR)

// Define the StmFile class (only available to C++ code).

#ifdef __cplusplus                              //---
class StmFile {                                 //StmFile:
public:
    StmFile();
    ~StmFile();

    isproc      StmOpen (int mode, ftext* pfilen);
    isproc      StmClose ();
    isproc      StmChop (int mode);

    isproc      Read (void* pv, dword cb, dword* pcbRead);
    isproc      Write (void* pv, dword cb, dword* pcbDone);
    isproc      Seek (qint move, int org, qword* pnewpos);

    isproc      AddRef ();
    isproc      Release ();

    dword       m_used;     //usage count
    int         m_changed;  //non-zero if have written to file since opened
    int         m_delete;   //non-zero if should delete object upon Release()

    byte*       m_mem;      //memory data pointer or NULL
    dword       m_posn;     //memory data seek position
    dword       m_size;     //byte size of m_mem memory data file
    dword       m_left;     //free bytes available at end of m_mem buffer
    void*       m_lock;     //critical section lock for m_mem when not m_pend

    aint        m_file;     //open disk file handle
};
#endif                                          //---

// Define TriOS functions.

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
text* xproc     OSTxtCopy (ftext* pdest, text* psource);
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
flx xproc       OSRaise (flx base, flx exp);
flx xproc       OSLog10 (flx value);

int xproc       OSBrowser (int mode, ftext* purl);
int xproc       OSAskFile (int mode, ftext* ptitle, ftext* type, ftext* filter, ftext* pname, OSApp* papp);
int xproc       OSAskFont (int mode, SFont* pfont, OSApp* papp);
int xproc       OSAskColor (int mode, lint* pcolor, OSApp* papp);
int xproc       OSMessageBox (int mode, ftext* ptitle, ftext* ptext, ftext* pname, OSApp* papp);
int xproc       OSFlAsk (int mode, text* pask, OSApp* papp);

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

int xproc       OSDebug  (int mode, text* pline);
void xproc      OSPrint  (int mode, text* pout, lint value);
void xproc      OSPrintI (int mode, text* pout, lint dec);
void xproc      OSPrintF (int mode, text* pout, flt f1, flt f2, lint dec, lint hex);
void xproc      OSPrintS (int mode, text* pout, text* ptwo, lint hex);

int  xproc      Similar (lint* pone, lint* ptwo, lint npts, flt frac);
int  xproc      OSTest (int mode, text* pdir);
CSTYLE_END

// =======
// Definitions for mode arguments used by some functions.

#define OS_DTDEF    0           //OSDateTime default text date and time format
#define OS_DTDATE   1           //OSDateTime add in for output date only
#define OS_DTTIME   2           //OSDateTime add in for output time only
#define OS_DTYMD    4           //OSDateTime YYYY/MM/DD HH:MM format
#define OS_DTMDY    8           //OSDateTime Mon DD, YYYY HH:MM format
#define OS_DTDOW    12          //OSDateTime Day, Month DD, YYYY HH:MM format
#define OS_DTFILE   32          //OSDateTime input file time format from puse
#define OS_DTFOUT   64          //OSDateTime output file time format to pout
#define OS_DTFSEC   65          //OSDateTime output seconds since 2000 to pout
#define OS_DTGMT    128         //OSDateTime use GMT for current or file time
#define OS_DTCMP    256         //OSDateTime compare pout and puse times
#define OS_DTADD    512         //OSDateTime increment pout time by puse
#define OS_DTLOC    1024        //OSDateTime convert GMT puse to local time
#define OS_DTGET    2048        //OSDateTime get date time dword from text
#define OS_DTSEC    4096        //OSDateTime get seconds since beginning of time
#define OS_DT2000   8192        //OSDateTime to use Year-2000 so not negative

#define OS_NOEXT    0           //OSFileBase without extension
#define OS_EXTN     1           //OSFileBase with extension
#define OS_CAP1     4           //OSFileBase add to capitalize first letter
#define OS_DIRN     8           //OSFileBase add to check for dir with end slash

#define OS_LOCBIN   0           //OSLocation OS binaries folder name
#define OS_LOCEXE   1           //OSLocation running executable's path and file
#define OS_LOCDOC   2           //OSLocation user documents folder name
#define OS_LOCAPP   3           //OSLocation application data folder name
#define OS_LOCALL   4           //OSLocation all users data folder (\ProgramData)
#define OS_LOCTMP   5           //OSLocation temporary folder name
#define OS_CURDIR   6           //OSLocation current directory
#define OS_EXEDIR   7           //OSLocation executable's directory
#define OS_DOCEXE   8           //OSLocation windows documens or linux exe dir
#define OS_VERNUM   20          //OSLocation OS version number: OS.Maj.Min.Build
#define OS_USER     21          //OSLocation for user name

#define OSN_WINDOWS 1           //OSLocation OS_VERNUM OS code for Windows
#define OSN_LINUX   2           //OSLocation OS_VERNUM OS code for Linux

#define OS_EXWAIT   0           //OSExec wait for completion
#define OS_NOWAIT   1           //OSExec do not wait for completion
#define OS_EXMINI   4           //OSExec show minimized
#define OS_ADDEXE   8           //OSExec add normal extension (like .exe)

#define OS_TASKEND  0           //OSTask stop
#define OS_TASKNEW  1           //OSTask create task
#define OS_TASKGET  2           //OSTask return callers task handle
#define OS_TASKPRI  3           //OSTask switch task priority if possible
#define OS_TASKDEF  0x00        //OSTask add for default priority
#define OS_TASKLOW  0x10        //OSTask add for low priority
#define OS_TASKSTD  0x20        //OSTask add for normal priority
#define OS_TASKHI1  0x40        //OSTask add for high priority
#define OS_TASKHI2  0x80        //OSTask add for critical priority
#define OS_TASKPRM  0xF0        //OSTask mask for priority

#define OS_GRAB     1           //OSWait/OSLock enter critical section mutex
#define OS_DONE     2           //OSWait/OSLock leave critical section mutex
#define OS_WAIT     3           //OSWait only   wait for wakeup
#define OS_WAKE     4           //OSWait only   wake waiting task
#define OS_WAKEALL  5           //OSWait only   wake any waiting tasks
#define OS_DONEISR  6           //Reserved for future use with FreeRTOS
#define OS_LOCKNEW  -1          //OSWait/OSLock create mutex lock
#define OS_LOCKOFF  -2          //OSWait/OSLock destroy lock
#define OS_SEMNEW   -3          //OSLock create semaphore lock
#define OS_SEMOFF   -4          //OSLock destroy semaphore lock

#define OS_SEMWAIT  11          //OSLock wait for semaphore
#define OS_SEMPOST  12          //OSLock post to semaphore
#define OS_SEMINIT  -11         //OSLock create semaphore
#define OS_SEMFREE  -12         //OSLock destroy semaphore

#define OS_MEMCLOSE -1          //OSMemMap close and destory shared memory
#define OS_MEMUNMAP 0           //OSMemMap unmap local copy of shared memory
#define OS_MEMOPEN  1           //OSMemMap open shared memory
#define OS_MEMCREAT 4           //OSMemMap create shared memory

#define OS_READ     0x00        //OSOpen mode for shared reading (must be zero)
#define OS_SHARE    0x01        //OSOpen mode for shared reading and writing
#define OS_WRITE    0x02        //OSOpen mode for exclusive read and write
#define OS_WRITE1   0x03        //OSOpen mode for write open or create if error
#define OS_CREATE   0x04        //OSOpen mode for create exclusive read/write
#define OS_VIACOM   0x05        //OSOpen mode for opening a com serial port

#define OS_RWMASK   0x07        //OSOpen mask for read, write, create mode
#define OS_RESERVE  0xF0        //OSOpen mask reserved for caller use

#define OS_NOWRT    0x100       //OSOpen mode for created read-only file
#define OS_NOEXE    0x200       //OSOpen mode for created non-execute under unix
#define OS_TRUNC    0x300       //OSOpen mode to truncate file to zero length
#define OS_INMEM    0x400       //StmOpen mode to load file into memory
#define OS_MASKS    0xF00       //OSOpen mode mask for DONOWRT,DONOEXE,DOTRUNC
#define OS_REWRITE   (OS_WRITE+OS_TRUNC)

#define OS_SET      0           //OSSeek position relative to beginning of file
#define OS_CUR      1           //OSSeek position relative to current file pos
#define OS_END      2           //OSSeek position relative to end of file

#define OS_PATH     1           //OSDirNew mode to create all dirs on the path
#define OS_NOFIX    61          //OSDiskDrive mode to find removable drives
#define OS_FLASH    188         //OSDiskDrive mode to find probably flash drives

#define OS_2UTF8    0           //OSWideUTF8 convert to UTF8
#define OS_2WIDE    1           //OSWideUTF8 convert to wide

#define OS_PASTE    1           //OSClipboard paste from clipboard
#define OS_COPY     2           //OSClipboard copy to clipboard

#define FF_FIRST    1           //OSFindFile find first file mode
#define FF_NEXT     2           //OSFindFile find next file mode
#define FF_DONE     0           //OSFindFile done mode

#define SOCK_CLOSE     0        //OSConnect close socket
#define SOCK_SERVER    1        //OSConnect open server socket
#define SOCK_ACCEPT    2        //OSConnect accept server connection socket
#define SOCK_CLIENT    3        //OSConnect open client socket

#define SOCK_APPINIT   11       //OSConnect initialize sockets for application
#define SOCK_APPDONE   12       //OSConnect shut down sockets for application
#define SOCK_RAWS      32       //OSConnect for raw eithernet packets
#define SOCK_IV6       64       //OSConnect use IPv6
#define SOCK_UDP       256      //OSConnect for UDP instead of TCP
#define SOCK_SHUT      512      //OSConnect shutdown socket for further writes
#define SOCK_NOWAIT    1024     //OSConnect do not linger for socket close
#define SOCK_DEF       2048     //OSConnect to zero provided ConEx
#define SOCK_MAC       4096     //OSConnect to return MAC address
#define SOCK_USERAW (SOCK_SERVER+SOCK_RAWS)
#define SOCK_USEUDP (SOCK_SERVER+SOCK_UDP)

#define SOCK_NOT       -3       //Impossible handle value
#define SOCK_APP       -2       //Fake connect handle (real handle always >= 0)
#define SOCK_ERR       -1       //OSConnect error sock handle return value
#define SOCKERR_DONE 0xFFFFFFC1 //OSConnect pex->error for socket closed
#define SOCKERR_STOP 0xFFFFFFC2 //OSConnect pex->error for other socket errors
#define SOCKERR_MODE 0xFFFFFFC3 //OSConnect pex->error for bad mode argument
#define SOCKERR_NULL 0xFFFFFFC4 //OSConnect pex->error for socket create error
#define SOCKERR_NONE 0xFFFFFFC5 //OSConnect pex->error for getsockopt error
#define SOCKERR_IV6  0xFFFFFFC6 //OSConnect pex->error if IPv6 not supported
#define SOCKERR_DNS  0xFFFFFFC7 //OSConnect pex->error for DNS lookup error
#define SOCKERR_TIM  0xFFFFFFC8 //OSConnect pex->error for send/recv timeout

#define FILE_AGENT  "Mozilla/2.0 (compatible; SporTraq; Windows)"

#define CRASH_EXIT  11          //OSExit code for simulated crash exit
#define CRASH_REAL  12          //OSExit code for realistic crash simulation
#define NO_CLEANUP  9999        //OSExit code for no OSCleanup memory leak test
#define DO_REBOOT   2           //OSExit code to request reboot
#define PIPE_TIMEO  60          //OSipcPipe receive timeout in sec (was 5)
#define PIPE_PORT   1112        //OSipcPipe default port number for @ip name

#define OS_LOG "GSSIrad.log"    //OSAddLog filename

#define OS_ENCRYPT  1           //OSCipher to encrypt (must be 1)
#define OS_DECRYPT  2           //OSCipher to decrypt
#define OS_ENBYTES  256         //OSCipher to encrypt or decrypt stream bytes

#define OS_KEY      4           //Number of dwords in OSCipher key
#define OS_PWH      6           //Number of dwords in OSPassword hash

#define PASS_INIT   0           //Password mode for password key
#define PASS_SEND   1           //Password mode for send encryption
#define PASS_RECV   2           //Password mode for receive encryption
#define PASS_HASH   4           //Password mode to make encrypted password hash
#define PASS_SALT   8           //Password mode to initialize nonce salt

#define TEA_KEY0    0x01234567  //Default key value 
#define TEA_KEY1    0x31415FAD
#define TEA_KEY2    0xFEDCBA98
#define TEA_KEY3    0xBADFEB08
#define TEA_KEY4    0xCCCCCCCC  //only needed by PASS_HASH
#define TEA_KEY5    0x55555555  //only needed by PASS_HASH
#define TEA_HASH    0xC12B4AB8

#define T_NEW            1      //Test mode added to create new test data
#define T_SEE            2      //Test mode added for verbose output
#define T_ALL            4      //Test mode added for tests even after failure
#define T_LOG            8      //Test mode added to output to log file
#define T_WIN           16      //Test mode added to popup errors in window
#define T_DIF           32      //Test mode added for different tests than normal

#define T_FAIL         256      //Test print mode for failure message
#define T_ZERO         512      //Test print mode to show zero values
#define T_ASIS        1024      //Test print mode to avoid four char indent
#define T_ERR   (T_FAIL+T_ASIS) //Application error message print mode

#define T_NEARLY      0.001f    //Similar less tolerance fraction
#define T_ALMOST      0.0001f   //Similar more tolerance fraction

typedef struct _CLogin {        //CLogin: Login User info transfered to server
text    name[SZNAME];           //User name
dword   hash[OS_PWH];           //Password hash value
} CLogin;
typedef CLogin* lpCLogin;

// =======
// Define shorter versions of common OSMessageBox formats.

#ifndef WIN32                   //---
#define MB_YESNO            0   //FlAsk modes
#define MB_YESNOCANCEL      1
#define MB_CANCELCONTINUE   2
#define MB_OKCANCEL         68
#define MB_OK               4
#define MB_ICONINFORMATION  8   //FlAsk wide left justified
#define MB_ICONEXCLAMATION  8
#define MB_ICONQUESTION     0   //FlAsk small centered
#define MB_STOP             0
#define MB_DEFBUTTON2       32  //FlAsk has no default button options
#endif                          //---

#define TMB_QYESNO      (MB_YESNO       | MB_ICONQUESTION)
#define TMB_QYESNOCAN   (MB_YESNOCANCEL | MB_ICONQUESTION)
#define TMB_IYESNO      (MB_YESNO       | MB_ICONINFORMATION)
#define TMB_IYESNOCAN   (MB_YESNOCANCEL | MB_ICONINFORMATION)
#define TMB_EYESNO      (MB_YESNO       | MB_ICONEXCLAMATION | MB_DEFBUTTON2)
#define TMB_EYESNOCAN   (MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2)
#define TMB_SYESNO      (MB_YESNO       | MB_ICONSTOP | MB_DEFBUTTON2)
#define TMB_SYESNOCAN   (MB_YESNOCANCEL | MB_ICONSTOP | MB_DEFBUTTON2)
#define TMB_NYESNO      (MB_YESNO       | MB_ICONQUESTION | MB_DEFBUTTON2)
#define TMB_NYESNOCAN   (MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2)

#define TMB_QOKCAN      (MB_OKCANCEL    | MB_ICONQUESTION)
#define TMB_IOKCAN      (MB_OKCANCEL    | MB_ICONINFORMATION)
#define TMB_EOKCAN      (MB_OKCANCEL    | MB_ICONEXCLAMATION | MB_DEFBUTTON2)
#define TMB_WOKCAN      (MB_OKCANCEL    | MB_ICONEXCLAMATION)

#define TMB_IOK         (MB_OK          | MB_ICONINFORMATION)
#define TMB_EOK         (MB_OK          | MB_ICONEXCLAMATION)

#define OS_OK  1        //OK pressed
#define OS_CAN 2        //Cancel pressed
#define OS_YES 6        //Yes pressed
#define OS_NO  7        //No pressed

// =======
// Triscape qdtime (qword or double dword) date and time format.

#define DT_YEARMASK 0xFFF00000  //MSW Mask for datetime year only (year+2000)
#define DT_MNTHMASK 0x000F0000  //MSW Mask for datetime month only (1=Jan)
#define DT_DAYSMASK 0x0000F800  //MSW Mask for datetime dword day (1=1st)
#define DT_HOURMASK 0x000007C0  //MSW Mask for datetime dword hour only (0-23)
#define DT_MINSMASK 0x0000003F  //MSW Mask for datetime dword minute only (0-59)

#define DT_DATEMASK 0xFFFFF800  //MSW Mask for datetime dword YMD date-only fields
#define DT_TIMEMASK 0x000007FF  //MSW Mask for datetime dword HH:MM time fields

#define DT_YEARUNIT 0x00100000  //Unit 1 year value
#define DT_MNTHUNIT 0x00010000  //Unit 1 month value
#define DT_DAYSUNIT 0x00000800  //Unit 1 day value
#define DT_HOURUNIT 0x00000040  //Unit 1 hour value
#define DT_MINSUNIT 0x00000001  //Unit 1 minute value

#define DT_YEARSHFT 20          //Shift to convert year to integer
#define DT_MNTHSHFT 16          //Shift to convert month to integer
#define DT_DAYSSHFT 11          //Shift to convert month to integer
#define DT_HOURSHFT 6           //Shift to convert hour to integer

#define DT_SECSMASK 0xFC000000  //LSW Mask for datetime dword seconds (0-59)
#define DT_MSECMASK 0x03FF0000  //LSW Mask for datetime dword milliseconds
#define DT_RESVMASK 0x0000FFFF  //LSW Mask for datetime dword reserved

#define DT_SECSSHFT 26          //Shift to convert seconds to integer
#define DT_MSECSHFT 16          //Shift to convert milliseconds to integer

#define DT_SECSUNIT 0x04000000  //Unit 1 second value
#define DT_MSECUNIT 0x00010000  //Unit 1 millisecond value

#define DT_YEARO    2000        //Optionally add to TD_YEARMASK so not negative

#ifdef ISWIN                    //===
#define DT_FTSEC    10000000    //Number of Windows FILETIME units per second
#else                           //===
#define DT_FTSEC    1           //Number of Linux time_t units per second
#endif                          //===

typedef struct _SYSTIME {       //SYSTIME:
    word wYear;
    word wMonth;
    word wDayOfWeek;
    word wDay;
    word wHour;
    word wMinute;
    word wSecond;
    word wMilliseconds;
} SYSTIME;

// =======
// OSConsole: Macro to do console line input. (See the UserInput function too.)
// The prompt may be NULL if not needed and should not have newline.
// The line buffer must have at least SZDISK chars.
// text line[SZDISK];
// OSConsole("Input this line: ", line);
// printf("This was typed: %s", line);
// You may need to include <stdio.h> to use this.

#ifdef WIN32            //===
#ifndef VISUALC6        //---
#define OSConsole(prompt, line)                         \
        line[0] = 0;                                    \
        if ((prompt)&&(prompt[0])) printf(prompt);      \
        gets_s(line, SZDISK);
#else                   //---
#define OSConsole(prompt, line)                         \
        line[0] = 0;                                    \
        if ((prompt)&&(prompt[0])) printf(prompt);      \
        gets(line);
#endif                  //---
#else                   //===
#define OSConsole(prompt, line)                         \
        line[0] = 0;                                    \
        if ((prompt)&&(prompt[0])) printf(prompt);      \
        fgets(line,SZDISK,stdin);                       \
        if (line[0] == '\n') line[0] = 0;
#endif                  //===

// =======
// OSConLine: Macro to do console line output.
// The prompt need not have newline to allow next to add to line.
// You may need to include <stdio.h> to use this.

#define OSConLine(prompt)                               \
        printf(prompt);

// =======
// OSConGets: Macro to do console line input.
// The line buffer must have at least SZDISK chars.

#ifdef WIN32            //===
#ifndef VISUALC6        //---
#define OSConGets(line)                                 \
        line[0] = 0;                                    \
        gets_s(line, SZDISK);
#else                   //---
#define OSConGets(line)                                 \
        line[0] = 0;                                    \
        gets(line);
#endif                  //---
#else                   //===
#define OSConGets(line)                                 \
        line[0] = 0;                                    \
        fgets(line,SZDISK,stdin);                       \
        if (line[0] == '\n') line[0] = 0;
#endif                  //===

// =======
// OSKeyHit: Macro to read console key or return 0 if none pressed
// You may need to include <conio.h> to use this.

#ifdef WIN32            //===
#define OSKeyHit() (_kbhit()) ? _getch() : 0
#else                   //===
#define OSKeyHit() (kbhit()) ? getchar() : 0
#endif                  //===

// =======
// OSSleep: Macro used instead of Windows Sleep for portability.
// Suspends current thread for specified number of milliseconds.
// Use 0 to reschedule task with the minimum wait.

#ifdef WIN32            //===
#define OSSleep(msec)   Sleep(msec)
#else                   //===
#define OSSleep(msec)   usleep(msec * 1000);
#endif                  //===

// =======
// Macros for faster mutex lock and unlock.
// OSLock_GRAB: OSLock_GRAB(plock) replaces OSLock(OS_GRAB, &plock)
// OSLock_DONE: OSLock_DONE(plock) replaces OSLock(OS_DONE, &plock)
// OSWait_GRAB: OSWait_GRAB(pwait) replaces OSWait(OS_GRAB,&pwait,NULL,0)
// OSWait_DONE: OSWait_DONE(pwait) replaces OSWait(OS_DONE,&pwait,NULL,0)

#ifdef ISWIN                                    //===
#define OSLock_GRAB(pl) if (pl) EnterCriticalSection((LPCRITICAL_SECTION)(pl));
#define OSLock_DONE(pl) if (pl) LeaveCriticalSection((LPCRITICAL_SECTION)(pl));
#define OSWait_GRAB(pw) if (pw.alloc) EnterCriticalSection(&((pw).mutex));
#define OSWait_DONE(pw) if (pw.alloc) LeaveCriticalSection(&((pw).mutex));
#else                                           //===
#define OSLock_GRAB(pl) if (pl) pthread_mutex_lock((pthread_mutex_t*)(pl));
#define OSLock_DONE(pl) if (pl) pthread_mutex_unlock((pthread_mutex_t*)(pl));
#define OSWait_GRAB(pw) if (pw.alloc) pthread_mutex_lock(&((pw).mutex));
#define OSWait_DONE(pw) if (pw.alloc) pthread_mutex_unlock(&((pw).mutex));
#endif                                          //===

// =======
// OSCOPY: Macros to do inplace assembly block copy if possible.
// OSCOPYD: Version requires that bytes be a multiple of four.
// The arguments must be single variables and not computed expressions.
// Note that passing memory pointers rather than arrays avoids Linux warnings.

#ifndef NOI86           //-------
#ifdef ISX64                                    //--- 64-bit
#define OSCOPY(pdest, psource, nbytes)                  \
        OSMemCopy(pdest, psource, (dword)nbytes)
#define OSCOPYD(pdest, psource, nbytes)                 \
        OSMemCopy(pdest, psource, (dword)nbytes)
#else                                           //--- 32-bit

#ifdef WIN32            //===
#define OSCOPY(pdest, psource, nbytes)                  \
__asm   mov     esi,psource                             \
__asm   mov     edi,pdest                               \
__asm   mov     ecx,nbytes                              \
__asm   cld                                             \
__asm   mov     eax,ecx                                 \
__asm   shr     ecx,2                                   \
__asm   rep     movs dword ptr[edi],dword ptr[esi]      \
__asm   mov     ecx,eax                                 \
__asm   and     ecx,3                                   \
__asm   rep     movs byte ptr[edi],byte ptr[esi]

#define OSCOPYD(pdest, psource, nbytes)                 \
__asm   mov     esi,psource                             \
__asm   mov     edi,pdest                               \
__asm   mov     ecx,nbytes                              \
__asm   cld                                             \
__asm   shr     ecx,2                                   \
__asm   rep     movs dword ptr[edi],dword ptr[esi]

#else                   //===
#define OSCOPY(pdest, psource, nbytes)                  \
    asm ( "                                             \
        movl    %0, %%esi               ;               \
        movl    %1, %%edi               ;               \
        movl    %2, %%ecx               ;               \
        cld                             ;               \
        mov     %%ecx, %%eax            ;               \
        shr     $2, %%ecx               ;               \
        rep     movsl (%%esi), (%%edi)  ;               \
        mov     %%eax, %%ecx            ;               \
        and     $3, %%ecx               ;               \
        rep     movsb (%%esi), (%%edi)  ;               \
          " :                                           \
            : "m"(psource), "m"(pdest), "m"(nbytes)     \
            : "%eax", "%ecx", "%esi", "%edi"            \
    );
#define OSCOPYD(pdest, psource, nbytes)                 \
    asm ( "                                             \
        movl    %0, %%esi               ;               \
        movl    %1, %%edi               ;               \
        movl    %2, %%ecx               ;               \
        cld                             ;               \
        shr     $2, %%ecx               ;               \
        rep     movsl (%%esi), (%%edi)  ;               \
          " :                                           \
            : "m"(psource), "m"(pdest), "m"(nbytes)     \
            : "%ecx", "%esi", "%edi"                    \
    );
#endif                  //===
#endif                                          //---

#else                   //-------
#define OSCOPY(pdest, psource, nbytes)                  \
        OSMemCopy(pdest, psource, (dword)nbytes)
#define OSCOPYD(pdest, psource, nbytes)                 \
        OSMemCopy(pdest, psource, (dword)nbytes)
#endif                  //-------

// =======
// Macros to do inplace assembly block fill if possible.
// OSFILLB: Fills with bytes
// OSFILLD: Fills with (nbytes/4) dwords.
// The arguments must be single variables and not computed expressions.

#ifndef NOI86           //-------
#ifdef ISX64                                    //--- 64-bit
#define OSFILLB(pdest, value, nbytes)                   \
        OSMemFillB(pdest, (byte)value, (dword)nbytes)
#define OSFILLD(pdest, value, nbytes)                   \
        OSMemFillD(pdest, (dword)value, (dword)nbytes)
#else                                           //--- 32-bit
#ifdef WIN32            //===
#define OSFILLB(pdest, value, nbytes)                   \
__asm   mov     edi,pdest                               \
__asm   mov     al,byte ptr value                       \
__asm   mov     ecx,nbytes                              \
__asm   cld                                             \
__asm   rep     stos byte ptr[edi]

#define OSFILLD(pdest, value, nbytes)                   \
__asm   mov     edi,pdest                               \
__asm   mov     eax,dword ptr value                     \
__asm   mov     ecx,nbytes                              \
__asm   shr     ecx,2                                   \
__asm   cld                                             \
__asm   rep     stos dword ptr[edi]

#else                   //===
#define OSFILLB(pdest, value, nbytes)                   \
    asm ( "                                             \
        movl    %0, %%edi       ;                       \
        movb    %1, %%al        ;                       \
        movl    %2, %%ecx       ;                       \
        cld                     ;                       \
        rep     stosb (%%edi)   ;                       \
          " :                                           \
            : "m"(pdest), "m"(value), "m"(nbytes)       \
            : "%eax", "%ecx", "%edi"                    \
    );
#define OSFILLD(pdest, value, nbytes)                   \
    asm ( "                                             \
        movl    %0, %%edi       ;                       \
        movl    %1, %%eax       ;                       \
        movl    %2, %%ecx       ;                       \
        shr     $2, %%ecx       ;                       \
        cld                     ;                       \
        rep     stosl (%%edi)   ;                       \
          " :                                           \
            : "m"(pdest), "m"(value), "m"(nbytes)       \
            : "%eax", "%ecx", "%edi"                    \
    );
#endif                  //===
#endif                                          //---

#else                   //-------
#define OSFILLB(pdest, value, nbytes)                   \
        OSMemFillB(pdest, (byte)value, (dword)nbytes)
#define OSFILLD(pdest, value, nbytes)                   \
        OSMemFillD(pdest, (dword)value, (dword)nbytes)
#endif                  //-------

// Macros to help handle path file names.
// OS_ESC: Special escape for special filename features.
// OS_SLASH: OS dependent path slash for Linux or backslash for Windows.
// OS_ENDLINES: String used at end of text file lines.
// IS_RELATIVE: is true if the pfn is a relative path file name.
// NO_SAFEPATH: is true if the ptxt points to unsafe ../ parent directory.
// Should use NO_SAFEPATH in loop checking each ptxt char in the filename.
                       
#define OS_ESC '^'

#ifndef CWINIX                                  //---
#ifdef  ISWIN                                   //===
#define OS_SLASH '\\'
#define OS_ENDLINES "\r\n"
#else                                           //===
#define OS_SLASH '/'
#define OS_ENDLINES "\n"
#endif                                          //===
#else                                           //---
#define OS_SLASH '/'
#define OS_ENDLINES "\n"
#endif                                          //---

#define IS_RELATIVE(pfn)                                \
        ((*pfn != '/')&&(*pfn != '\\')&&((*pfn == 0)||(*(pfn+1) != ':')))

#define NO_SAFEPATH(ptxt)                               \
        ((*ptxt == '.')&&(*(ptxt+1) == '.')&&((*(ptxt+2) == '/')||(*(ptxt+2) == '\\')))

// Macros to help handle ending directory name slash characters.
// NO_PRESLASH: is true if the previous character is not a backslash or slash.
// IS_PRESLASH: is true if the previous character is a backslash or slash.
// NO_ENDSLASH: makes sure that ending slash is removed from string if needed.
// IS_ENDSLASH: makes sure that an ending slash is added to string if needed.
// The slash macros assume that ptxt is at final zero of pbuf string.

#define NO_PRESLASH(pbuf,ptxt)                          \
        ((ptxt > pbuf)&&(*(ptxt-1) != '\\')&&(*(ptxt-1) != '/'))

#define IS_PRESLASH(pbuf,ptxt)                          \
        ((ptxt > pbuf)&&((*(ptxt-1) == '\\')||(*(ptxt-1) == '/')))

#define NO_ENDSLASH(pbuf,ptxt)                          \
        if (IS_PRESLASH(pbuf,ptxt)) {                   \
            *(ptxt-1) = 0;                              \
        }

#define IS_ENDSLASH(pbuf,ptxt)                          \
        if (NO_PRESLASH(pbuf,ptxt)) {                   \
            *ptxt++ = OS_SLASH;                         \
            *ptxt = 0;                                  \
        }

// Macros to help convert utf8 or ascii text strings to OS standard.
// OS_STR makes a "string" constant wide if Windows Unicode.
// OS_TXT makes txt zero-terminated string variable wide if Windows Unicode.
// OS_TXT requires wtext buf[SZPATH] buffer to be used with 512 wide words.
// OS_TXT returns pointer to normal or wide Unicode string.
// OS_TXN is the same as OS_TXT but specifies the nc number of txt characters.
// OS_TXW casts our wtxt wide word text to Windows LPWSTR if needed.
// OS_TXTCOPY copies a txt zero terminated string to normal or wide target.
// OS_TXTCOPY does not return anything meaninful unlike OSTxtCopy.
// OS_UTF returns an ascii utf8 string given a wide string if Windows Unicode.
// OS_UTF requires text buf[SZPATH] buffer (or maybe wtxt) with 512 char bytes.

#ifndef _UNICODE                                //--- NORMAL ASCII or UTF8
#define OS_STR(quotes)      quotes              //leave byte strings
#define OS_TXT(txt,buf)     (txt)
#define OS_TXN(txt,nc,buf)  (txt)  
#define OS_TXW(wst)         (wst)
#define OS_TXTCOPY(des,txt) OSTxtCopy(des,txt)
#define OS_UTF(wst,buf)     (wst)

#else                                           //--- WINDOWS UNICODE
#define OS_STR(quotes)      L##quotes           //define wide string constant
#define OS_TXT(txt,buf)     (OSWideUTF8(1,(wtxt*)buf,txt,-1,SZPATH),(LPWSTR)buf)
#define OS_TXN(txt,nc,buf)  (OSWideUTF8(1,(wtxt*)buf,txt,nc,SZPATH),(LPWSTR)buf)
#define OS_TXW(wst)         ((LPWSTR)wst)
#define OS_TXTCOPY(des,txt) OSWideUTF8(1,(wtxt*)des,txt,OSTxtSize(txt),SZHUGE)
#define OS_UTF(wst,buf)     (OSWideUTF8(0,(wtxt*)wst,(text*)buf,-1,SZPATH),(text*)buf)
#endif                                          //---

#ifdef __cplusplus                              //---
typedef StmFile  fStmFile;
typedef StmFile  fStm;      //fStm* was formerly LPSTREAM
typedef StmFile* lpStmFile;
typedef fStm*    lpStm;
typedef StmFile* STMFILE;   //sometimes use all caps version for pointer

int xproc       OSReadWeb (int mode, StmFile* pfile, text* purl);

csproc          StmRead (fStm* pstream, fvoid* pv, dword cb);
csproc          StmWrite (fStm* pstream, fvoid* pv, dword cb);
csproc          StmSeek (fStm* pstream, long pos, int org, dword* poff);
csproc          StmLoad (fStm* pstream, fvoid* ppmem, fdword* pbytes);
csproc          StmMemory (lpStm* ppstm, void* pmem, lint cmem);
csproc          StmErrors (int mode, int ecode);

ctproc(fStm*)   StmFOpen (int mode, ftext* pfilen);
csproc          StmFClose (fStm* file);

CSTYLE int      StmFRead (void* file, void* pv, int cb);
CSTYLE int      StmFWrite (void* file, void* pv, int cb);
#endif                                          //---

CSTYLE void*    FOSAlloc (int bytes);
CSTYLE void*    FOSFree (void* mem);

#endif                  //_TriOS

