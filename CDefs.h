//*************************************************************************
//  FILENAME:   CDefs.h
//  AUTHOR:     SCSimonoff 8-2-11
//
//  Common C Definitions and Types.
//
//  Overview: cint, sint, lint, qint are signed 8,16,32,64-bit integer types.
//            byte, word, dword, qword are unsigned 8,16,32,64-bit integers.
//            aint and aword are signed and unsigned pointer address offsets.
//            uint and ulong are unsigned integers that can vary in size.
//            int and long are signed integers that can vary in size.
//            text and wtxt are 8 and 16-bit text characters.
//            flt and flx are 32 and 64-bit floating values.
//            Also allows for i8, u8, i16, u16, i32, u32, ll64, ul64 types.
//            ADDOFF() and SUBOFF() add pointers always using byte offset sizes.
//            PTROFF() gets the byte offset between two pointers.
//
//  Common data types defined for conciseness and portability.
//  Note that contrary to common practice, typedefs may be
//  lower case symbols to aid in readability.
//  Other common definitions and macros are also included.
//
//  If WIN32|WIN64 defined assumes Windows VC, otherwise assumes Linux GCC.
//  If BIG_ENDIAN is defined then uses big endian non-Intel swaps.
//  If NOI86 is defined then cannot use Intel assembly code.
//  Currently sets USE32=1 for 32+ bit code (but has old 16-bit support).
//  Note: Undefines BIG_ENDIAN=4321 because GCC bug defines 4321 on Intel.
//  Note: Defines CBIG_ENDIAN if BIG_ENDIAN because sometimes redefined.
//  Note: Defines VISUALC6 if using old Visual C++ Version 6.
//  Note: Defines ISWIN if WIN32 or WIN64 and normally WIN64 has WIN32 too.
//  Note: Defines ISX64 if either WIN64 or _LP64.
//
//  These definitions were created 1996-2011 by SCSimonoff for Triscape Inc.
//  They may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _CDefs          //Only define once if include multiple times
#define _CDefs

// Always 32 bits these days.

#define USE32   1       //USE32=1 for 32-bit code

// Use to define our own namespace.

#define BEG_NAMESPACE(name) namespace name {
#define TRI_NAMESPACE       namespace tricom {
#define END_NAMESPACE       }

// Stuff formerly in h2inc.h which we included here
// and which was for MASM compatibility of .H sources.
// But now we use inline assembly instead.
// We no longer use the D? macros allowing MASM to use C structures.
// We include those macros for older code.

#define BEGSTRUCT(name) struct name {           //works in C++
#define ENDSTRUCT(name) };

#ifndef __cplusplus
#define DEFSTRUCT(name) typedef struct _##name {//works in C or C++
#define DEFENDING(name) } name;
#else
#define DEFSTRUCT(name) struct name {           //works in C++
#define DEFENDING(name) };
#endif

#define TYPEDEF 	typedef
#define STRUCT          struct
#define CLASS		class

#define DB(type,name)	  type name;	    //byte values
#define DW(type,name)	  type name;	    //word values
#define DD(type,name)	  type name;	    //dword values
#define DI(type,name)	  type name;	    //int values
#define AB(type,name,num) type name[num];   //byte arrays
#define DQ(type,name)	  type name;	    //qword values
#define DS(type,name)	  type name;	    //sub-structure values
#define AT(type,name,num) type name[num];   //text arrays
#define AW(type,name,num) type name[num];   //word arrays
#define AD(type,name,num) type name[num];   //dword arrays
#define AI(type,name,num) type name[num];   //int arrays
#define AS(type,name,num) type name[num];   //sub-structure arrays

// Allow for 32 bit x86 or 64 bit x64 platforms.
// Beware that GCC x64 uses int/long/pointer = 4/8/8
// But Microsoft VSx64 uses int/long/pointer = 4/4/8

#ifndef WIN32
#ifdef  _WIN32
#define WIN32   1
#endif
#endif

#ifndef WIN64
#ifdef  _WIN64
#define WIN64   1
#endif
#endif

#ifdef  WIN64
#define ISX64   1
#else    
#ifdef  _LP64
#define ISX64   1
#endif
#endif

#ifdef  ISX64
#ifndef _MSC_VER                                //64-bits Linux
#define __dint  int                             //used to define dword ints
#define __qint  long                            //used to define qword ints
#define __aint  long                            //used to hold address offsets
#define LL(val) val##L                          //used to define qword constants

#else                                           //64-bits Visual Studio
#define __dint  long                            //used to define dword ints
#define __qint  long long                       //used to define qword ints
#define __aint  long long                       //used to hold address offsets
#define LL(val) val##LL                         //used to define qword constants
#define ISWIN   1
#endif
#define SZAP2   3                               //address ptr size power of two
#else
#define SZAP2   2

#ifndef _MSC_VER                                //32-bits Linux
#define __dint  long                            //used to define dword ints
#define __qint  long long                       //used to define qword ints
#define __aint  long                            //used to hold address offsets
#define LL(val) val##LL                         //used to define qword constants
#else
#if     _MSC_VER > 1200                         //32-bits Visual Studio
#define __dint  long                            //used to define dword ints
#define __qint  long long                       //used to define qword ints
#define __aint  long                            //used to hold address offsets
#define LL(val) val##LL                         //used to define qword constants
#define ISWIN   1
#else                                           //32-bits Visual C++ 6.0
#define __dint  long
#define __qint  __int64
#define __aint  long
#define LL(val) val
#define VISUALC6 1
#define ISWIN   1
#endif
#endif
#endif

// Use sfar instead of far or __far to be more portable between 16 and 32 bits.

#if USE32
#define sfar
#define huge
#else
#define sfar            __far
#define huge            __huge
#endif

// Define the common data types.
// Some of these match the WINDOWS.H definitions (but are in lower case).
// The byte, word, dword, qword types remain constant when the int size changes.
// The cint, sint, lint, qint types are signed integers with the same sizes.
// The aword and aint types can hold pointer addresses, differences and offsets.
// The aword and aint types are 32 or 64 bits depending on environment.
// The int and long types have at least 16 and 32 bits repectively.
// If text is changed to short then must also change SZCHR to two bytes.
// If flt or flx sizes change, must also change FLTSZ, FLXSZ, FLTNULL, FLTMINUS.
// The lp for pointer prefixes allow lpV* to define a pointer to a pointer to V.
// The f for far prefixes applied to 16-bit environments and are obsolete.

typedef unsigned char   byte;           //Unsigned 8-bit value
typedef unsigned short  word;           //Unsigned 16-bit value
typedef unsigned __dint dword;          //Unsigned 32-bit value
typedef unsigned __qint qword;          //Unsigned 64-bit value
typedef unsigned __aint aword;          //Unsigned address offset or pointer

typedef signed char     cint;           //Signed 8-bit byte value
typedef short           sint;           //Signed 16-bit word value
typedef __dint          lint;           //Signed 32-bit dword value
typedef __qint          qint;           //Signed 64-bit qword value
typedef __aint          aint;           //Signed address offset

typedef unsigned char   u8;             //Unsigned 8-bit value      (alt convention)
typedef unsigned short  u16;            //Unsigned 16-bit value     (alt convention)
typedef unsigned __dint u32;            //Unsigned 32-bit value     (alt convention)
typedef unsigned __qint u64;            //Unsigned 64-bit value     (alt convention)
typedef unsigned __qint ul64;           //Unsigned 64-bit value     (alt convention)

typedef signed char     i8;             //Signed 8-bit byte value   (alt convention)
typedef short           i16;            //Signed 16-bit word value  (alt convention)
typedef __dint          i32;            //Signed 32-bit dword value (alt convention)
typedef __qint          i64;            //Signed 64-bit qword value (alt convention)
typedef __qint          ll64;           //Signed 64-bit qword value (alt convention)

typedef float           flt;            //IEEE 32-bit float (see FLTSZ below)
typedef double          flx;            //IEEE 64-bit float (see FLXSZ below)

typedef char            text;           //Text character (short if UNICODE)
typedef unsigned short  wtxt;           //UNICODE Wide character text (2 Bytes)

typedef unsigned int    uint;           //Note: int and uint have variable size
typedef unsigned long   ulong;          //Note: long and ulong are variable size
typedef void sfar*      hand;           //Windows handle values
typedef lint            errc;           //Error code value (same as HRESULT)
typedef lint            errv;           //Error code without std::errc conflict
typedef int             boolv;          //TRUE, FALSE, TRUENEG or TRUETWO value

typedef void  sfar      fvoid;          //Use fvoid* for long ptr to memory
typedef fvoid* sfar     lpvoid;         //Use lpvoid* for long ptr to void ptr
typedef text  sfar      ftext;          //Use ftext* for long ptr to text string
typedef ftext* sfar     lptext;         //Use lptext* for long ptr to text ptr
typedef wtxt  sfar      fwtxt;          //Use fwtxt* for long ptr to wtxt
typedef fwtxt* sfar     lpwtxt;         //Use lpwtxt* for long ptr to wtxt ptr
typedef byte  sfar      fbyte;          //Use fbyte* for long ptr to bytes
typedef fbyte* sfar     lpbyte;         //Use lpbyte* for long ptr to byte ptr
typedef word  sfar      fword;          //Use fword* for long ptr to words
typedef fword* sfar     lpword;         //Use lpword* for long ptr to word ptr
typedef dword sfar      fdword;         //Use fdword* for long ptr to dwords
typedef fdword* sfar    lpdword;        //Use lpdword* for long ptr to dword ptr
typedef qword sfar      fqword;         //Use fqword* for long ptr to qwords
typedef fqword* sfar    lpqword;        //Use lpaword* for long ptr to aword ptr
typedef aword sfar      faword;         //Use fqword* for long ptr to qwords
typedef faword* sfar    lpaword;        //Use lpaword* for long ptr to aword ptr
typedef cint sfar       fcint;          //Use fcint* for long ptr to cint
typedef fcint* sfar     lpcint;         //Use lpcint* for long ptr to cint ptr
typedef sint sfar       fsint;          //Use fsint* for long ptr to sint
typedef fsint* sfar     lpsint;         //Use lpsint* for long ptr to sint ptr
typedef lint sfar       flint;          //Use flint* for long ptr to lint
typedef flint* sfar     lplint;         //Use lplint* for long ptr to lint ptr
typedef qint sfar       fqint;          //Use fqint* for long ptr to qint
typedef fqint* sfar     lpqint;         //Use lpqint* for long ptr to qint ptr
typedef aint sfar       faint;          //Use faint* for long ptr to aint
typedef faint* sfar     lpaint;         //Use lpaint* for long ptr to aint ptr
typedef long  sfar      flong;          //Use flong* for long ptr to longs
typedef flong* sfar     lplong;         //Use lplong* for long ptr to long ptr
typedef short sfar      fshort;         //Use fshort* for long ptr to shorts
typedef fshort* sfar    lpshort;        //Use lpshort* for long ptr to short ptr
typedef int   sfar      fint;           //Use fint* for long ptr to int
typedef fint*  sfar     lpint;          //Use lpint* for long ptr to int ptr
typedef uint  sfar      fuint;          //Use fuint* for long ptr to uint
typedef fuint* sfar     lpuint;         //Use lpuint* for long ptr to uint ptr
typedef ulong  sfar     fulong;         //Use fulong* for long ptr to ulong
typedef fulong* sfar    lpulong;        //Use lpulong* for long ptr to ulong ptr
typedef flt sfar        fflt;           //Use fflt* for long ptr to flt
typedef fflt* sfar      lpflt;          //Use lpflt* for long ptr to flt ptr
typedef flx sfar        fflx;           //Use fflx* for long ptr to flx
typedef fflx* sfar      lpflx;          //Use lpflx* for long ptr to flx ptr

#define qdtime qword                    //Date and time as defined by OSDateTime

#if USE32
#define far                             //Use "sfar" for 16/32-bit portability
#define near
#undef  pascal
#undef  cdecls
#undef  stdcall
#ifdef ISWIN                            //---
#define pascal          __cdecl         //32-bit compiler does not have PASCAL
#define cdecls          __cdecl
#define stdcall         __stdcall
#else                                   //---
#ifdef __cdecl
#define pascal          __cdecl
#define cdecls          __cdecl
#define stdcall         __stdcall
#else
#define pascal                          //Older GCC may not allow this call spec
#define cdecls
#define stdcall
#endif
#endif
#define export
#else
#define far             __far
#define near            __near
#define pascal          __pascal
#define cdecls          __cdecl
#define stdcall         __stdcall
#define export          __export
#endif

#ifndef NULL
#define NULL            0               //code depends on this being 0
#endif

#define FALSE           0               //this must always be zero
#define TRUE            1               //some code depends on this being 1
#define TRUENEG        -1               //used for tri-state logic
#define TRUETWO        +2               //used for quad-state logic
#define TRUETRI        +3               //used for many-state logic
#define TRUETOP        +4               //used for many-state logic
#define TRUEMIN        -2               //used for many-state logic
#define TRUELOW        -3               //used for many-state logic

#define ESIGN           0x80000000      //Sign bit in 32 bit integer (max neg)
#define EMASK           0xFFFFFFFF      //Unsigned equivalent of -1
#define EMAXI           0x7FFFFFFF      //Maximum positive signed 32 bit integer
#define QMAXU          ((qword) -1)     //Maximum unsigned 64 bit integer
#define QMAXI          ((qword)QMAXU>>1)//Maximum signed 64 bit integer
#define QSIGN          ((qword)1 << 63) //Sign bit in 64 bit integer (max neg)
#define SZCHR           1               //Byte size of a single text character
#define SZCP2           0               //The SZCHR value as a power of two

#define ASM             __asm           //Used for assembly lines or blocks

// If a text file does not begin with 0xFF or 0xFE it is assumed to be ASCII.

#define UNI_LSB         0xFEFF          //Unicode TXT file beginning if LSB-1st
#define UNI_MSB         0xFFFE          //Unicode TXT file beginning if MSB-1st
#define UNI_NOT         0xFFFF          //Illegal Unicode char for private use

// Common ASCII control characters.
// Our convention is that SEP_BEL separates hidden ending of a name.
// Our convention is that SEP_VTB separates lines in a parameter value.
// Our convention is that SEP_TAB, SEP_REC, and SEP_PRM are record separators.

#define SEP_BEL     0x7                 //Alert Bell       (same as backslash a)
#define SEP_RUB     0x8                 //Rubout
#define SEP_TAB     0x9                 //Tab              (same as backslash t)
#define SEP_LF      0xA                 //Line Feed        (same as backslash n)
#define SEP_FF      0xC                 //Form Feed        (same as backslash f)
#define SEP_CR      0xD                 //Carriage Return  (same as backslash r)
#define SEP_ESC     0x1B                //Esc escape char
#define SEP_VTB     0x1F                //Unit separator   (parameter new line)
#define SEP_REC     0x1E                //Record separator
#define SEP_SPACE   0x20                //Space character
#define SEP_QT2     0x22                //Double quote character: "
#define SEP_QT1     0x27                //Single quote character: '
#define SEP_PRM     0x2C                //Comma character

// Define standard calling conventions.
// The xproc was originally intended to be PASCAL on all platforms.  However,
// Microsoft's 32-bit compiler does not allow PASCAL and mangles STDCALL names.
// Therefore, for 32-bit code aproc uses CDECL to allow linkage to and from
// assembly code (via CALLU32.INC).  When assembly linkage is not required,
// xproc using STDCALL conventions is smaller and faster since the called
// routine pops its own stack.  For 16-bit code we use the PASCAL convention.

#if USE32
#define xproc           stdcall             //Optimum C-only calling convention
#define aproc           cdecls              //Assembly or C calling convention
#define sproc           stdcall             //Windows API calling convention
#define cproc           cdecls              //Default C-only calling convention

#else
#define xproc           sfar pascal export
#define aproc           sfar pascal export
#define sproc           sfar stdcall export
#define cproc           sfar cdecls export
#endif

// *******
// Definitions allowing C code to be called from C++.

#ifdef __cplusplus
#define cplusplus       1
#define CSTYLE          extern "C"          //C linkage without name mangling
#define CSTYLE_BEG      extern "C" {        //Group of C-Style definitions
#define CSTYLE_END      }                   //Group of C-Style definitions
#define CTHIS                               //Object pointer argument if not C++
#define CPURE           = 0                 //Pure virtual method if C++
#else
#define cplusplus       0
#define CSTYLE
#define CSTYLE_BEG
#define CSTYLE_END
#define CTHIS           fvoid* This,
#define CPURE
#endif

#define csproc          CSTYLE errc xproc   //Standard C-style function errc rtn
#define cserrv          CSTYLE errv xproc   //Standard C-style function errv rtn
#define csvoid          CSTYLE void xproc   //C-style function void return
#define cslint          CSTYLE lint xproc   //C-style function lint return
#define csqint          CSTYLE qint xproc   //C-style function qint return
#define csint           CSTYLE int xproc    //C-style function int return
#define csflt           CSTYLE flt xproc    //C-style function flt return
#define csflx           CSTYLE flx xproc    //C-style function flx return
#define csptr           CSTYLE void* xproc  //C-style function void* return
#define cstext          CSTYLE text* xproc  //C-style function text* return
#define csword          CSTYLE word xproc   //C-style function word return
#define csdword         CSTYLE dword xproc  //C-style function dword return
#define csqword         CSTYLE qword xproc  //C-style function qword return
#define csboolv         CSTYLE boolv xproc  //C-style function boolv return
#define ctproc(type)    CSTYLE type xproc   //Other return type declaration

// We supply our own shorthand for COM interface declarations.
// This is for readability, to save space and because it is difficult to track
// down where all of the OLE definitions are amoung all Microsoft's .H files.
// Note that vsproc and isproc are the same type but that vsproc is used in
// the class definition and isproc for the method function implementation.
// Use isproc in the class definition for non-virtual (non-COM) local functions.
// Interface methods must return HRESULT (or errc) to be compatible with DCOM.
// To define a COM interface (as done by DECLARE_INTERFACE in BASETYPS.H), the
// methods must be pure virtual functions.  The IUnknown functions come first.
// Include WINDOWS.H first to get IUnknown and other OLE stuff.
// See also BASETYPS.H's DECLARE_INTERFACE & STDMETHOD and WTYPES.H.
// For example in the .H file used by C or C++ clients:
//      class IFace : public IUnknown {
//      public:
//          vsproc QueryInterface (CTHIS fvoid* riid, fvoid* ppvObj) CPURE;
//          vsused AddRef (CTHIS) CPURE;
//          vsused Release (CTHIS) CPURE;
//          vsproc Method (CTHIS long argument) CPURE;
//      };
//  And in the .CPP file implementing the interface methods:
//      itproc(dword) IFace::AddRef() {
//          return 0;
//      }
//      isproc IFace::Method (long argument) {
//          return 0;
//      }
// See also csproc (for CSTYLE errc xproc).

#define vsproc          virtual errc sproc  //HRESULT interface declaration
#define vserrv          virtual errv sproc  //HRESULT interface declaration
#define vsused          virtual dword sproc //ULONG UI4 interface declaration
#define vslint          virtual lint sproc  //LONG I4 interface declaration
#define vshand          virtual aint sproc  //Handle interface declaration
#define vsvoid          virtual void sproc  //Void type interface declaration
#define vsint           virtual int sproc   //Int type interface declaration
#define vsflt           virtual flt sproc   //FLT type interface declaration
#define vsflx           virtual flx sproc   //FLX type interface declaration
#define vsptr           virtual void* sproc //Void ptr interface declaration
#define vstext          virtual text* sproc //Text ptr interface declaration
#define vtproc(type)    virtual type sproc  //Other return type declaration

#define isproc          errc sproc          //HRESULT interface implementation
#define iserrv          errv sproc          //HRESULT interface implementation
#define ishuge          qword sproc         //QWORD interface implementation
#define isused          dword sproc         //ULONG UI4 interface implementation
#define islint          lint sproc          //LONG I4 interface implementation
#define ishand          aint sproc          //Handle interface implementation
#define isvoid          void sproc          //Void type interface implementation
#define isint           int sproc           //Int type interface implementation
#define isflt           flt sproc           //FLT type interface implementation
#define isflx           flx sproc           //FLX type interface implementation
#define isptr           void* sproc         //Void ptr interface implementation
#define istext          text* sproc         //Text ptr interface implementation
#define itproc(type)    type sproc          //Other return type implementation

// *******
// Define DLL entry points without .DEF file.
// Use DLLDEF when defining function (or global variable) in DLL source.
// Use DLLAPI in .H included in client code: void DLLAPI xproc Func(args);
// Also remember to use CSTYLE to stop C++ name mangling from .CPP sources.
// For example: CSTYLE errc DLLDEF xproc Function(int arg) { ... }

#ifdef __WINDLL
#define DLLDEF          __declspec(dllexport)
#define DLLAPI          __declspec(dllimport)
#else
#define DLLDEF
#define DLLAPI
#endif

// *******
// O32: Macro to convert address offset to 32-bit dword.
// Used where memory allocations guaranteed to be 4GB or less
// so that memory offsets and differences fit in 32 bits.
// May be implemented with debug version check.
// A32: Macro to get low dword portion of a pointer without compiler warning.
// Used mostly for debug information or to hash addresses.

#define O32(off) ((dword)(off))

#define A32(ptr) ((dword)((aword)ptr))

// *******
// CAPITAL: Macro to convert to upper case.
// LOWCASE: Macro to convert to lower case.
// ISCAPITAL: Macro to return TRUE if character is upper case.
// OFFSET: Macro to return the byte offset to an item m in a structure s.
// ABS: Macro to return the absolute value of a number.

#define CAPITAL(ch) ( ((ch>='a')&&(ch<='z')) ? (text)(ch-('a'-'A')):(text)(ch) )
#define LOWCASE(ch) ( ((ch>='A')&&(ch<='Z')) ? (text)(ch-('A'-'a')):(text)(ch) )
#define ISCAPITAL(chr)  ((chr <= 'Z')&&(chr >= 'A'))

#define OFFSET(s,m)     ((dword)( (byte *)&(((s *)0)->m) - (byte *)0 ))
#define ABS(val)        ((val >= 0) ? (val) : (-(val)))

// Macros to do byte offset arithmetic with pointers.
// ADDOFF: adds a byte offset to any pointer and returns any pointer type.
// SUBOFF: subtracts a byte offset from any pointer and returns any ptr type.
// PTROFF: returns the byte offset between any two pointers of any type.

#define ADDOFF(type,ptr,off) ((type) ((fbyte*)ptr + off))  //add offset to ptr
#define SUBOFF(type,ptr,off) ((type) ((fbyte*)ptr - off))  //sub offset from ptr
#define PTROFF(pbeg,pend)    (int)((fbyte*)pend - (fbyte*)pbeg)

// Debugging macro to force a break point.

#ifdef ISWIN
#ifdef ISX64
#define BREAK3          __debugbreak()
#else
#define BREAK3          _asm int 3
#endif
#else
#define BREAK3          asm("int $3")
#endif

// M_FENCE: Memory Barrier macro assures that loads and stores before
// are all fully committed before any loads or stores afterward.
// This requires at least Windows 2003 and a Pentium 4 or Athlon 64.
// The L_FENCE guarantees all following reads happen after those before it.
// The S_FENCE guarantees all preceeding writes happen before those after it.
// The M_FENCE combines the both load and store fence guarantees.
// Normally S_FENCE goes before store to final "data ready" shared location,
// but after the data is prepared and written to shared locations.
// Normally L_FENCE goes just after read from "data ready" shared location,
// but before the data is actually read from shared location.

#ifdef ISWIN
#define M_FENCE          MemoryBarrier()
#define S_FENCE          MemoryBarrier()
#define L_FENCE          MemoryBarrier()
#else
#define M_FENCE          __asm__ __volatile__ ("" ::: "memory")
#define S_FENCE          __asm__ __volatile__ ("" ::: "memory")
#define L_FENCE          __asm__ __volatile__ ("" ::: "memory")
#endif

// Definitions giving byte order in multibyte integers.
// For some non-Intel processors the most significant bytes come first.
// The WSWAP: DSWAP: QSWAP: macros always swap the bytes.
// The LS_WV: LS_DV: LS_QV: are for LSB first vals (swap only if big-endian).
// The MS_WV: MS_DV: MS_QV: are for MSB first vals (swap only if little-endian).
// The XLS_WV: Swaps a word value as a statement to be little-endian or is nop.
// The XLS_DV: Swaps a dword value as a statement to be little-endian or is nop.
// The XLS_QV: Swaps a qword value as a statement to be little-endian or is nop.
// The HIWSWAP and HIDSWAP macros swap bytes only if big-endian processor.
// The LOWSWAP and LODSWAP macros swap bytes only if little-endian (Intel).

// The QDW & QDI structures allow us to break qwords into two dword values.

#define WSWAP(w)        ( (((word)w & 0x00FF)<<8) + (((word)w & 0xFF00)>>8) )
#define DSWAP(d)        ( (((dword)d & 0x000000FF)<<24) + (((dword)d & 0x0000FF00)<<8) + (((dword)d & 0x00FF0000)>>8) + (((dword)d & 0xFF000000)>>24) )
#define QSWAP(q)        ( (((qword)q & LL(0x00000000000000FF))<<56) + (((qword)q & LL(0x000000000000FF00))<<40) + (((qword)q & LL(0x0000000000FF0000))<<24) + (((qword)q & LL(0x00000000FF000000))<<8) + (((qword)q & LL(0x000000FF00000000))>>8) + (((qword)q & LL(0x0000FF0000000000))>>24) + (((qword)q & LL(0x00FF000000000000))>>40) + (((qword)q & LL(0xFF00000000000000))>>56) )

#if (BIG_ENDIAN == 4321)
#undef BIG_ENDIAN       //GCC defines BIG_ENDIAN as 4321 on Intel little endian!
#endif
#ifndef BIG_ENDIAN      //Little Endian (Intel least significant byte first)

#define LS_WV(w)        ((word)w)
#define LS_DV(d)        ((dword)d)
#define LS_QV(q)        ((qword)q)

#define MS_WV(w)        WSWAP(w)
#define MS_DV(d)        DSWAP(d)
#define MS_QV(q)        QSWAP(q)

#define XLS_WV(w)       0 
#define XLS_DV(d)       0 
#define XLS_QV(q)       0 

#define HIWSWAP(w)      ((word)w)
#define HIDSWAP(d)      ((dword)d)
#define LOWSWAP(w)      WSWAP(w)
#define LODSWAP(d)      DSWAP(d)

typedef struct {        //QDW: Unsigned 64-bit integer broken into dwords
  union {
    qword qw;           //qword version qw
    struct {
    dword lo;           //dw.lo (low dword)
    dword hi;           //dw.hi (high dword)
    } dw;
    struct {
    dword lo;           //lo (low dword)
    dword hi;           //hi (high dword)
    };
    struct {
    dword LowPart;      //LowPart (compatible with Windows ULARGE_INTEGER)
    dword HighPart;     //HighPart
    };
    struct {
    dword LowPart;
    dword HighPart;
    } u;
    qword QuadPart;
  };
} QDW;

typedef struct {        //QDI: Signed 64-bit integer broken into dwords
  union {
    qint qw;            //qword version qw
    struct {
    dword lo;           //dw.lo (low dword)
    lint  hi;           //dw.hi (high dword)
    } dw;
    struct {
    dword lo;           //lo (low dword)
    lint  hi;           //hi (high dword)
    };
    struct {
    dword LowPart;      //LowPart (compatible with Windows LARGE_INTEGER)
    lint  HighPart;     //HighPart
    };
    struct {
    dword LowPart;
    lint  HighPart;
    } u;
    qint QuadPart;
  };
} QDI;

#else                   //Big Endian (Non-Intel) most significant byte first
#define CBIG_ENDIAN 1   //Use instead of BIG_ENDIAN in case redefined

#define LS_WV(w)        WSWAP(w)
#define LS_DV(d)        DSWAP(d)
#define LS_QV(q)        QSWAP(q)

#define MS_WV(w)        ((word)w)
#define MS_DV(d)        ((dword)d)
#define MS_QV(q)        ((qword)q)

#define XLS_WV(w)       w = WSWAP(w)
#define XLS_DV(d)       d = DSWAP(d)
#define XLS_QV(q)       q = QSWAP(q)

#define HIWSWAP(w)      WSWAP(w)
#define HIDSWAP(d)      DSWAP(d)
#define LOWSWAP(w)      ((word)w)
#define LODSWAP(d)      ((dword)d)

typedef struct {        //QDW:
  union {
    qword qw;           //qword version qw
    struct {
    dword hi;           //dw.hi (high dword)
    dword lo;           //dw.lo (low dword)
    } dw;
    struct {
    dword hi;           //hi (high dword)
    dword lo;           //lo (low dword)
    };
    struct {
    dword HighPart;     //HighPart (compatible with Windows ULARGE_INTEGER)
    dword LowPart;      //LowPart
    };
    struct {
    dword HighPart;
    dword LowPart;
    } u;
    qword QuadPart;
  };
} QDW;

typedef struct {
  union {
    qint qw;
    struct {
    lint  hi;
    dword lo;
    } dw;
    struct {
    lint  hi;
    dword lo;
    };
    struct {
    lint  HighPart;
    dword LowPart;
    };
    struct {
    lint  HighPart;
    dword LowPart;
    } u;
    qint QuadPart;
  };
} QDI;

#endif

// *******
// FLTABS: Macro forces the flt value to be positive absolute value.
// FLXABS: Macro forces the flx value to be positive absolute value.
// FLTNEG: Macro reverses the flt +/- sign and negates the value.
// FLXNEG: Macro reverses the flx +/- sign and negates the value.
// FLTNAN: Macro returns !0 if flt value is NaN (not a number) or infinity.
// FLXNAN: Macro returns !0 if flx value is NaN (not a number) or infinity.
// FLTZERO: Macro returns !0 if flt value is zero (quicker than C test).
// FLXZERO: Macro returns !0 if flx value is zero (quicker than C test).
// FLTNULL: Macro returns !0 if flt is "null", 0xFFFFFFFF in high dword.
// FLXNULL: Macro returns !0 if flx is "null", 0xFFFFFFFF in high dword.
// FLTMINUS: Macro to return non-zero if flt float value is negative.
// FLXMINUS: Macro to return non-zero if flx float value is negative.
// FLTFLOOR: Macro to return 32-bit lint integer <= to a flt or flx.
// FLTROUND: Macro to return rounded 32-bit lint integer from a flt,
// FLXROUND: Macro to return rounded 32-bit lint integer from a flx,
// Note that they must change if FLTSZ changes!

#define FLTABS(fltv)    ( (*( ((dword*)(&(fltv))) +0 )) &= (~ESIGN) )
#define FLXABS(flxv)    ( (*( ((dword*)(&(flxv))) +1 )) &= (~ESIGN) )

#define FLTNEG(fltv)    ( (*( ((dword*)(&(fltv))) +0 )) ^= ESIGN )
#define FLXNEG(flxv)    ( (*( ((dword*)(&(flxv))) +1 )) ^= ESIGN )

#define FLTNAN(fltv)    (( *( ((dword*)(&(fltv))) +0 ) & 0x7F000000) == 0x7F000000)
#define FLXNAN(flxv)    (( *( ((dword*)(&(flxv))) +1 ) & 0x7FE00000) == 0x7FE00000)

#define FLTZERO(fltv)   (!( *( ((dword*)(&(fltv))) +0 ) & 0x7F800000))
#define FLXZERO(flxv)   (!( *( ((dword*)(&(flxv))) +1 ) & 0x7FF00000))

#define FLTNULL(fltv)   ( *( ((dword*)(&(fltv))) +0 ) == 0xFFFFFFFF )
#define FLXNULL(flxv)   ( *( ((dword*)(&(flxv))) +1 ) == 0xFFFFFFFF )

#define FLTMINUS(fltv)  ( (*( ((dword*)(&(fltv))) +0 )) & ESIGN )
#define FLXMINUS(flxv)  ( (*( ((dword*)(&(flxv))) +1 )) & ESIGN )

#define FLTFLOOR(fltv)  ( (fltv < (lint)fltv) ? ((lint)fltv - 1) : ((lint)fltv) )

#define FLTROUND(fltv)  ( (lint) (fltv + ((FLTMINUS(fltv)) ? (-0.5f):(+0.5f))) )
#define FLXROUND(flxv)  ( (lint) (flxv + ((FLXMINUS(flxv)) ? (-0.5 ):(+0.5 ))) )

//#define FLTROUND(fltv)  (FLTFLOOR(fltv + 0.5f))
//#define FLXROUND(flxv)  (FLTFLOOR(flxv + 0.5))

// FLTINIT: Macro to make sure all 8087 exceptions are masked off.
// The t argument is a temporary word or int or lint for use by the macro.
// AMD might sometimes have exceptions unmasked?

#ifndef NOI86           //-------
#define FLTINIT(t)      __asm fnstcw word ptr t    \
                        __asm fwait                \
                        __asm mov al,byte ptr t    \
                        __asm or  al,CW_NOEX       \
                        __asm mov byte ptr t,al    \
                        __asm fldcw word ptr t
#endif                  //-------

// FLTTEMP: Used to define temp word used by FLT2INT and FLX2INT for no warn.
// FLT2INT: Macro used as statement to convert 32-bit flt float to rounded lint.
// FLX2INT: This is the same macro but for 64-bit flx values.
// This is faster than FLTROUND on Intel machines since it can use the 8087.
// It is also faster than C float to int conversion which uses a library call
// because C must chop toward 0 which Intel does not do by default.
// The i argument must be an lint variable which receives the rounded result.
// The f argument is the flt floating variable source.
// The t argument is a temporary word or int or lint for use by the macro.

#ifdef  NOI86
#define FLTTEMP(t)
#define FLT2INT(i,f,t)  i = FLTROUND(f);
#else
#ifdef  WIN64
#define FLTTEMP(t)
#define FLT2INT(i,f,t)  i = FLTROUND(f);
#else
#define FLTTEMP(t)      word t;
#ifdef  WIN32
#define FLT2INT(i,f,t)  __asm fld f                 \
                        __asm fnstcw word ptr t     \
                        __asm fwait                 \
                        __asm mov ah,byte ptr [t+1] \
                        __asm mov al,ah             \
                        __asm and ah,CW_MASK        \
                        __asm mov byte ptr [t+1],ah \
                        __asm fldcw word ptr t      \
                        __asm fistp i               \
                        __asm mov byte ptr [t+1],al \
                        __asm fldcw word ptr t;
#else                   //------- Intel assembler
#define FLT2INT(ii,ff,tt) asm ( "                                       \
                              fld %2                    ;               \
                              fnstcw %1                 ;               \
                              fwait                     ;               \
                              movw %3, %%ax             ;               \
                              mov %%eax, %%edx          ;               \
                              and $0xF3, %%ah           ;               \
                              movw %%ax, %1             ;               \
                              fldcw %3                  ;               \
                              fistpl %0                 ;               \
                              movw %%dx, %1             ;               \
                              fldcw %3                  ;               \
                              " : "=m"(ii), "=m"(tt)                    \
                                : "m"(ff), "m"(tt)                      \
                                : "%eax", "%edx"                        \
                              );
#endif                  //------- Linux assembler
#endif
#endif

#ifdef  NOI86
#define FLX2INT(i,f,t)  i = FLXROUND(f);
#else
#ifdef  WIN64
#define FLX2INT(i,f,t)  i = FLXROUND(f);
#else
#ifdef  WIN32
#define FLX2INT(i,f,t)  __asm fld f                 \
                        __asm fnstcw word ptr t     \
                        __asm fwait                 \
                        __asm mov ah,byte ptr [t+1] \
                        __asm mov al,ah             \
                        __asm and ah,CW_MASK        \
                        __asm mov byte ptr [t+1],ah \
                        __asm fldcw word ptr t      \
                        __asm fistp i               \
                        __asm mov byte ptr [t+1],al \
                        __asm fldcw word ptr t;
#else                   //------- Intel assembler
#define FLX2INT(ii,ff,tt) asm ( "                                       \
                              fld %2                    ;               \
                              fnstcw %1                 ;               \
                              fwait                     ;               \
                              movw %3, %%ax             ;               \
                              mov %%eax, %%edx          ;               \
                              and $0xF3, %%ah           ;               \
                              movw %%ax, %1             ;               \
                              fldcw %3                  ;               \
                              fistpl %0                 ;               \
                              movw %%dx, %1             ;               \
                              fldcw %3                  ;               \
                              " : "=m"(ii), "=m"(tt)                    \
                                : "m"(ff), "m"(tt)                      \
                                : "%eax", "%edx"                        \
                              );
#endif                  //------- Linux assembler
#endif
#endif

// Define sizes of flt and flx types.
// Include FLOAT.INC to get various definitions derived from FLTSZ and FLXSZ.

#define FLTSZ   4           //Byte size of flt values
#define FLXSZ   8           //Byte size of flx values

// Define sizes of lint and dword types.

#define SSZ     4           //Byte size of lint/dword values
#define SSZP2   2           //Byte size of lint/dword values as power-of-two shift

// Define 8087 CW Floating Point Control Word values.

#define CW_BASE 0x037F      //8087 Control Word Base Value - Round
#define CW_CHOP 0x0F7F      //8087 Control Word Chop Value - Chop toward 0
#define CW_DOWN 0x077F      //8087 Control Word Down Value - Round toward neg
#define CW_INTR 0x03FF      //8087 Control Word Interrupts Enabled
#define CW_MASK 0xF3        //8087 Control Word High Byte Mask for Round (Base)

#define CW_NOEX 0x3F        //8087 Control Word Low Byte bits for no exceptions

// *******
// Structure and macro to allow low level routines to call progress
// without linking in high level stuff.

typedef errc (* CallWait)(int mode, text* pinfo, lint have, lint done, void* pws);
typedef errv (* ErrvWait)(int mode, text* pinfo, lint have, lint done, void* pws);

DEFSTRUCT   (CWaitS)    //CWaitS:
DD( lint,   count     ) //Current progress number for client use
DD( lint,   total     ) //Maximum progress number for client use
DD( text*,  infos     ) //Progress dialog text to show for client use
DD( void*,  pwfun     ) //CallWait progress function pointer
DD( void*,  pwins     ) //Reserved for client use
DD( lint,   begin     ) //Begin percentage to show (0-100) or client use
DD( lint,   delta     ) //Delta percentage to show (0-100) or client use
DD( lint,   keeps     ) //Reserved for client use (TRUE = keep progress dialog)
DD( void*,  waits     ) //Used internally by client
DD( void*,  frees     ) //Used internally by client
DEFENDING   (CWaitS)
TYPEDEF CWaitS sfar fCWaitS;
TYPEDEF fCWaitS* sfar lpCWaitS;

// *******
// Function pointer that allows subroutine to poll caller for UI and abort requests.
// The parm can pass call environment and info can pass caller's information.
// The return may request the function abort.

typedef int (sfar* LPBACK) (void* parm, void* info);

DEFSTRUCT   (CBackS)    //CBackS: Easy way to pass callback parm to call with
DD( LPBACK, call      ) //Function pointer to be called
DD( void*,  parm      ) //The parm pointer to pass with calls
DEFENDING   (CBackS)
TYPEDEF CBackS sfar fCBackS;
TYPEDEF fCBackS* sfar lpCBackS;

// *******
// Macros to use Microsoft non-standard exception handling.

#define TRYBEG __try {          //Beginning of protected code block
#define TRYERR } __except(1) {  //End block and beginning of exception handler
#define TRYEND }                //End of exception handler block

// *******
// Define standard sizes which might vary from product to product.
// These must be multiples of 4.  BLKSIZE and SZNAME must be powers of 2.
// The SZPATH size is guaranteed to hold filename or internet url.
// The SZNOTE size is guaranteed to be at least double SZPATH.
// The SZHUGE size is guaranteed to be at least double SZNOTE.
// The SZITEM size is guaranteed to be at least double SZDISK.
// The SZFILE size is guaranteed to be between SZDISK and SZPATH.
// The SZDISK can be used for local disk file names with volume + 256 path.
// The SZPATH was formerly 264 for a path of 256 plus a disk name rounded up.
// However, Internet URL paths can be a maximum of 2048 characters plus
// 32 characters for the scheme plus 3 characters for "://" for 2084 total.

#define BLKSIZE 4096    //common granularity of OS memory allocations
#define BLKMASK (BLKSIZE-1)

#define SZHUGE  8336    //maximum size of huge text block
#define SZNOTE  4168    //maximum size of annotated file or resource name
#define SZPATH  2084    //maximum size of file names including Internet URL's
#define SZFILE  528     //maximum size of file path names in our own file system
#define SZITEM  528     //maximum size of text tips and other screen items
#define SZDISK  264     //maximum size of disk file names with disk and path
#define SZLINE  128     //common text size often used for readable lines
#define SZLABL  64      //common text size often used for long names
#define SZNAME  32      //common text size often used for names
#define SZTINY  16      //common text size often used for ids

#endif                  //_CDefs
