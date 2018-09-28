//*************************************************************************
//  FILENAME:   TriEx.h
//  AUTHOR:     SCSimonoff    12-4-13
//  MRU:
//
//  Definitions for TriEx.cpp
//
//  This software was created 2012-2018 by SCSimonoff for.
//  It may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _TriEx                   //Only define once if include multiple times
#define _TriEx

// Serialization definitions.

typedef struct {                //SerIn: Serialization information
text*   mem;                    //Serialization memory block
lint    end;                    //Offset in mem to current end
lint    use;                    //Offset in mem to raw bytes end
lint    siz;                    //Available bytes in mem
text    was[SZPATH];            //Original serialization file's directory
text    now[SZPATH];            //New serialization file's directory
text*   ofn;                    //Override name for copied clip file or NULL
text*   ost;                    //Override name for copied ST data file or NULL
} SerIn;
typedef SerIn* sfar lpSerIn;

#define SER_HEAD   "Parameters" //Serialization file check word

#define SER_SEP     SEP_TAB     //Ends all serialization value fields
#define SER_REC     SEP_LF      //Ends serialization record
#define SER_SEC     SEP_FF      //Ends serialization of section of records
#define SER_ESC     SEP_ESC     //Ends serialization of series of objects

#define ALT_SEP     0x1         //Alternate serialization value term   (was \t)
#define ALT_REC     0x2         //Alternate serialization record       (was \n)
#define ALT_SEC     0x3         //Alternate serialization section      (was \f)
#define ALT_ESC     0x4         //Alternate serialization series      (was esc)

#define SER_BLK    (8*1024)     //Serialization allocation block size
#define SER_INT     10          //Byte space needed for integer value
#define SER_FLT     12          //Byte space needed for flt value
#define SER_FLX     24          //Byte space needed for flx value

#define SER_GET          1      //Mode to read from serialization
#define SER_PUT          2      //Mode to write to serialization

#define SER_ABS          0      //Mode to get absolute path name
#define SER_REL          4      //Mode to get relative path name
#define SER_ANY          8      //Mode to get either absolute or relative

#define SER_APP     0x0010      //Global application information
#define SER_OBJ     0x0020      //Reserved for objects
#define SER_RAW     0x0040      //Raw serial bytes (not text) with SerRAWS
#define SER_END     0x0080      //End of objects list
#define SER_MSK     0x00F0      //Mask for types

#define RAW_ODD     0x0100      //SerRaws does not align end on dword value
#define RAW_WRD     0x0200      //SerRAWS low-endian word values
#define RAW_DWD     0x0400      //SerRAWS low-endian dword values
#define RAW_QWD     0x0800      //SerRAWS low-endian qword values
#define RAW_MSK     0x0F00      //Mask for endian flags

#define SER_REP     0x8000      //Repeat write just beginning, serialize part 1

#define RAW_BYTE(m,s,v)   SerRAWS((m|SER_RAW)        ,s,v,1)
#define RAW_BYTS(m,s,v,c) SerRAWS((m|SER_RAW)        ,s,v,c)
#define RAW_BODD(m,s,v)   SerRAWS((m|SER_RAW|RAW_ODD),s,v,1)
#define RAW_WODD(m,s,v)   SerRAWS((m|SER_RAW|RAW_WRD|RAW_ODD),s,v,1)
#define RAW_WORD(m,s,v)   SerRAWS((m|SER_RAW|RAW_WRD),s,v,2)
#define RAW_WRDS(m,s,v,c) SerRAWS((m|SER_RAW|RAW_WRD),s,v,(c*2))
#define RAW_DWRD(m,s,v)   SerRAWS((m|SER_RAW|RAW_DWD),s,v,4)
#define RAW_DWDS(m,s,v,c) SerRAWS((m|SER_RAW|RAW_DWD),s,v,(c*4))
#define RAW_QWRD(m,s,v)   SerRAWS((m|SER_RAW|RAW_QWD),s,v,8)
#define RAW_QWDS(m,s,v,c) SerRAWS((m|SER_RAW|RAW_QWD),s,v,(c*8))
#define RAW_TEXT(m,s,v,c) SerRTXT((m|SER_RAW),        s,v,c);

// Prepares byte array pointer for RAW_BYTS macro.
// Caller must provide a memer: label for ECMEMORY error.
#define RAW_BALL(m,s,v,n) if ((m & SER_GET)&&(n)) {     \
                            if (v) OSFree(v);           \
                            v=(double*)OSAlloc(n);      \
                            if (v==NULL) goto memer; }

// Prepares word array pointer for RAW_WRDS macro.
// Caller must provide a memer: label for ECMEMORY error.
#define RAW_WALL(m,s,v,n) if ((m & SER_GET)&&(n)) {     \
                            if (v) OSFree(v);           \
                            v=(double*)OSAlloc(n*2);    \
                            if (v==NULL) goto memer; }

// Prepares dword array pointer for RAW_DWDS macro.
// Caller must provide a memer: label for ECMEMORY error.
#define RAW_DALL(m,s,v,n) if ((m & SER_GET)&&(n)) {     \
                            if (v) OSFree(v);           \
                            v=(double*)OSAlloc(n*4);    \
                            if (v==NULL) goto memer; }

// Prepares qword array pointer for RAW_QWDS macro.
// Caller must provide a memer: label for ECMEMORY error.
#define RAW_QALL(m,s,v,n) if ((m & SER_GET)&&(n)) {     \
                            if (v) OSFree(v);           \
                            v=(double*)OSAlloc(n*8);    \
                            if (v==NULL) goto memer; }

// Prepares text string pointer for RAW_TEXT macro.
// Caller must declare lint n to hold size being saved.
// Caller must provide a memer: label for ECMEMORY error.
// Not needed for non-allocated fixed-size text buffers.
#define RAW_TALL(m,s,v,n) if (m & SER_GET) {                     \
                            SerRAWS((m|SER_RAW|RAW_DWD),s,&n,4); \
                            if (v) OSFree(v);                    \
                            v=(char*)((n>0)?(OSAlloc(n)):NULL);  \
                            if ((n)&&(v==NULL)) goto memer;      \
                          } else {                               \
                            n = (v) ? OSTxtSize(v) : 0;          \
                            SerRAWS((m|SER_RAW|RAW_DWD),s,&n,4); \
                          }

#define CF_UNIQUE        1      //CopyFile mode to make unique target name
#define CF_OVERWT        2      //CopyFile mode to overwrite target file
#define CF_TONAME        4      //CopyFile mode to copy to pdir path and name
#define CF_STNAME        8      //CopyFile mode to copy to pdir name with CF_EXT
#define CF_EXT          ".dat"  //CopyFile CF_STNAME extenstion to use

#define PROG_KB         10      //Right shift to convert bytes to KB

#define TF_PATH          0      //TargetFile mode to convert to path name
#define TF_BASE          1      //TargetFile mode to convert to base name
#define TF_READ          2      //TargetFile mode to read without increment name
#define TF_OVER          4      //TargetFile mode to allow file overwrite w/o increment
#define TF_DATA          8      //TargetFile mode to insist on Data folder.
#define TF_LAST         16      //TargetFile mode to find last existing filename
#define TF_REDO         32      //TargetFile mode to have TF_PATH redo source path
#define TF_SUB         "Data"   //Data subfolder name

#define HELP_LOC         1      //ShowHelp local viewer mode
#define HELP_WEB         0      //ShowHelp web viewer mode

#define USER_INT         0      //UserInput mode for lint integer value
#define USER_FLT         1      //UserInput mode for flt floating value
#define USER_TXT         2      //UserInput mode for text string value
#define USER_BYT         3      //UserInput mode for signed byte value
#define USER_HEX         4      //UserInput mode for hexidecmial dword value
#define USER_TYP       0x7      //UserInput mask for basic type
#define USER_NEW        16      //UserInput added to mode to not show old value
#define USER_ASK        32      //UserInput added to ask for -flag if not found
#define USER_BEG        64      //UserInput added to match with -flag beginning

#define USER_AV1        -1      //UserInput mode to remember utf8 command line
#define USER_AV2        -2      //UserInput mode to remember wide command line

#define USER_RAW         1      //UserOutput mode to not add linefeed
#define USER_OPT         2      //UserOutput mode to not show if quiet mode
#define USER_LOG         4      //UserOutput mode to also write to log file
#define USER_ALL         8      //UserOutput mode for decimal value even if zero
#define USER_SET     (16+2)     //UserOutput mode to set quiet mode to value
#define USER_RET        32      //UserOutput mode to just return text

#define SHOW_INT        10      //Character width of interger output

#define TR_2BIN 1               //TextReg mode to decrypt bytes
#define TR_2TXT 2               //TextReg mode to encrypt text
#define TR_ADD1 16              //TextReg mode if 1st sec 1 more char
#define TR_SEP3 32              //TextReg mode if dash every 3 chars
#define TR_SEP4 64              //TextReg mode if dash every 4 chars
#define TR_SEP5 96              //TextReg mode if dash every 5 chars
#define TR_SFT3 5               //TextReg shift to TR_SEP3 mode bit

#define REG_CLASS   1           //Registry HKEY_CLASSES_ROOT  (Shared Classes)
#define REG_USER    2           //Registry HKEY_CURRENT_USER  (Current User)
#define REG_LOCAL   3           //Registry HKEY_LOCAL_MACHINE (All Users)
#define REG_ROOT    0xF         //Registry Mask for root key value

#define REG_PUT     0x00        //Registry Put key value (must be zero)
#define REG_GET     0x10        //Registry Get key value
#define REG_DEL     0x20        //Registry Delete key value

#define REG_SZDISK  0x100       //Registry if buffer is only SZDISK (must be 256!)
#define REG_SZSIZE  0x200       //Registry to just return required get size
#define REG_SZHUGE  0x400       //Registry if buffer guaranteed big enough
#define REG_GOREAL  0x800       //Registry force using real PC registry even if MyDev
#define REG_GOHIDE 0x1000       //Registry to just hide the FxFoto.ini file
#define REG_SZITEM 0x2000       //Registry if buffer is only SZITEM
#define REG_NOMIX  0x8000       //RegInfo added to skip unscramble

#ifdef __cplusplus                              //---
// QFifo Queue class.
// Simple FIFO with fixed-size messages which requires not mutex locking.

#define QF_DEEP (256)           //Default max number of messages
#define QF_SIZE (512*4)         //Default message byte size
#define QF_VN   (16)            //Default max messages for IniV version

class QFifo {                   //QFifo:
public:
    QFifo();
    virtual ~QFifo();

    isproc      Init (int mode, dword nmsg, dword msiz);
    isproc      IniV (lint* pmem, dword nmsg);
    isptr       Put1 ();
    isvoid      Put2 ();
    isproc      PutV (lint val);
    isptr       Get1 ();
    isvoid      Get2 ();
    isproc      GetV ();
    isptr       Get1X (lint* puse);

    byte*       m_fifo;         //FIFO buffer pointer
    byte*       m_free;         //Buffer to be freed when destroyed
    dword       m_nmsg;         //Maximum pending messages
    dword       m_msiz;         //Bytes per message
    dword       m_iput;         //Index in m_fifo to next message put to head
    dword       m_oput;         //Byte offset in m_fifo to next message put
    dword       m_iget;         //Index in m_fifo to next message get from tail
    dword       m_oget;         //Byte offset in m_fifo to next message get
    errc        m_errct;        //May be set non-zero by Put task for overrun
    lint        m_extra;        //May be set by Get task to acknowledge new value
    void*       m_lock;         //Optional mutex lock or NULL if not used
};

// BFifo Queue class.
// Each block stored on fifo begins with a dword byte size
// followed by the data bytes which need not be an even number of dwords.
// The next block begins on an even dword after end padding if needed.
// Blocks can be split by wrap-around from beginning back to end of buffer.

#define BF_SIZE (BLKSIZE*4)     //Default buffer byte size

class BFifo {                   //BFifo:
public:
    BFifo();
    virtual ~BFifo();

    isproc      Set (int mode, dword bytes);
    isproc      Put (void* pblock, dword bytes);
    isproc      Get (void* pblock, dword* pbytes);

    dword*      m_fifo;         //FIFO buffer pointer
    dword       m_bytes;        //Total bytes in fifo (must be even dwords)
    dword       m_input;        //Byte offset in fifo for next block put
    dword       m_output;       //Byte offset in fifo for next block get
    errc        m_error;        //May be set non-zero by Put task for overrun
    void*       m_lock;         //FIFO queue mutex lock or NULL if not used
};
#endif                                          //---

// Structure used by MFifo non-C++ version of QFifo.
// This has the state information structure immediately followed by ring buffer.
// This PFifo structure MUST be duplicated in PruRTS.p code!
// This allows the sender to be PRU assembly language.

DEFSTRUCT(PFifo)                //PFifo:
    dword       m_nmsg;         //Maximum pending messages
    dword       m_msiz;         //Bytes per message
    dword       m_iput;         //Index in m_fifo to next message put to head
    dword       m_iget;         //Index in m_fifo to next message get from tail
    byte        m_data[4];      //Beginning of fifo data buffer
DEFENDING(PFifo)
TYPEDEF PFifo* lpPFifo;

// TriEx.cpp exported functions.

CSTYLE_BEG
isproc FileCopy (int mode, text* pdup, text* pdir, text* psrc, CWaitS* pbar);
isproc FileComp (aint file1, aint file2);
isproc ShowHelp (int mode, text* phelp, text* ptopic);

isproc SerSpace (SerIn* pser, lint need);
isproc SerHead (int mode, SerIn* pser, text* phead);
isvoid SerTerm (int mode, SerIn* pser, text term);
isproc SerPath (int mode, SerIn* pser, text* pbuf, text* pabs);

isproc SerFile (int mode, SerIn* pser, aint file, int nfix);
isvoid SerRAWS (int mode, SerIn* pser, void* pval, int bytes);
isvoid SerRTXT (int mode, SerIn* pser, text* pval, int max);

isvoid SerINTV (int mode, SerIn* pser, int* pval);
isvoid SerLINT (int mode, SerIn* pser, lint* pval);
isvoid SerDWRD (int mode, SerIn* pser, dword* pval);
isvoid SerWORD (int mode, SerIn* pser, word* pval);
isvoid SerBYTE (int mode, SerIn* pser, byte* pval);
isvoid SerFLTV (int mode, SerIn* pser, flt* pval);
isvoid SerFLXV (int mode, SerIn* pser, flx* pval);
isvoid SerTEXT (int mode, SerIn* pser, text* pval, int bytes);
isvoid AltTEXT (int mode, SerIn* pser, text* pval, int bytes);
isvoid SerNAME (int mode, SerIn* pser, text* pval);
isvoid SerPTXT (int mode, SerIn* pser, lptext* ppval);
isvoid SerBLOK (int mode, SerIn* pser, void* pblok, int bytes);

isproc htmPost (text* phtm, text* ptag, text* pval, int nval);
isproc xmlFind (text* pxml, text* ptag, text* pval);
istext xmlInit (int mode, SerIn* pxml);
isvoid xmlCopy (SerIn* pxml, text* ptxt);
isvoid xmlLINT (SerIn* pxml, text* ptag, text* pval, lint* pset);
isvoid xmlDWRD (SerIn* pxml, text* ptag, text* pval, dword* pset);
isvoid xmlFLTV (SerIn* pxml, text* ptag, text* pval, flt* pset, int decs);
isvoid xmlTEXT (SerIn* pxml, text* ptag, text* pval, text* pset);
isvoid xmlPTXT (SerIn* pxml, text* ptag, text* pval, lptext* ppset);

isproc PFifoInit (int mode, dword nmsg, dword msiz, void* pbuf);
isptr  PFifoPut1 (void* pbuf);
isvoid PFifoPut2 (void* pbuf);
isptr  PFifoGet1 (void* pbuf);
isvoid PFifoGet2 (void* pbuf);
isptr  PFifoPut1X (void* pbuf, dword* pget);
isptr  PFifoGet1X (void* pbuf, dword* pget, lint* puse);
isvoid PFifoGet2X (void* pbuf, dword* pget);

isproc TargetFile (int mode, text* path, text* base, text* pdir, text* pext);
isvoid CurrentDir (int mode, text* ppath, text* pname);
isvoid UserOutput (int mode, text* pline, text* pname, lint value);
isproc UserInput (int mode, lint flag, text* prompt, int col, void* pdata);

isproc TextReg (int mode, dword key, text* preg, byte* pbin, int bits);
isproc TextMix (int key, text* ptxt);
isproc BinPack (int mode, dword* ppack, dword* pnums, int* pbits);
isproc Registry (lint mode, text* pkey, text* pname, text* pvalue);
isproc RegInfo (int mode, text* pfile, text* pkey, text* pname, text* pvalue);
dword  RegSum (int mode, text* ptxt);
dword  RegCPU (int mode);
dword  RegMAC (int mode);

istext Txt2Mac (int mode, byte* pmac, text* ptxt);
dword  Buf_CRC (void* pBuf, dword nLen, dword ulCRC);
dword  Get_CRC (void* pBuf, dword nLen);
dword  FileCheck (int mode, text* pname, dword siz, dword crc);
isproc FileXY (int mode, text* fname, double* pX, double* pY, int npts);

csproc ExTest (int mode, text* pdir);
CSTYLE_END

#endif                          //_TriEx

