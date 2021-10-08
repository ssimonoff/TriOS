//*************************************************************************
//  FILENAME:   TriEx.cpp
//  AUTHOR: SCSimonoff 12-4-13
//  CPU:    Portable
//  O/S:    Portable except Registry,RegCPU,RegMAC
//  MRU:
//
//  Extra utility functions not part of TriOS.cpp
//  Compile with /D USEHELP to enable ShowHelp() function.
//  Compile with /D EUINPUT to enable UserInput(), UserOutput functions.
//  Compile with /D EXFIFOS to enable QFifo, BFifo functions.
//  Compile with /D EXTESTS to enable Test function.
//  Compile with /D QF_LOCK to add mutex lock to QFifo. Always Used Below!
//  Compile with /D DEB_REG \Temp\FakeRID.txt cpu+,mac+ text fake computer id.
//  Compile with /D ISWIN to make Windows dependent.
//  Windows requries linking with htmlhelp.lib
//
//  Uses TriOS.h portable non-gui system calls.
//  Uses CDefs.h custom types such as: cint,byte,sint,word,lint,dword,flt,flx
//
//  This software was created 2012-2018 by SCSimonoff.
//  It may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#include "TriOS.h"
#include "TriEx.h"

// Assume multi-core for fifos.

#define QF_LOCK

// Functions in this file.

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

//QFifo::QFifo ();
//QFifo::~QFifo ();
//isproc QFifo::Init (int mode, dword nmsg, dword msiz);
//isproc QFifo::IniV (lint* pmem, dword nmsg);
//isptr  QFifo::Put1 ();
//isvoid QFifo::Put2 ();
//isproc QFifo::PutV (lint val);
//isptr  QFifo::Get1 ();
//isvoid QFifo::Get2 ();
//isproc QFifo::GetV ();
//isptr  QFifo::Get1X (lint* puse);

//BFifo::BFifo ();
//BFifo::~BFifo ();
//isproc BFifo::Set (int mode, dword bytes);
//isproc BFifo::Put (void* pblock, dword bytes);
//isproc BFifo::Get (void* pblock, dword* pbytes);

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
dword  CheckBoth (int mode, text* pname, dword siz, dword crc, text* pext);
isproc FileXY (int mode, text* fname, double* pX, double* pY, int npts);

csproc ExTest (int mode, text* pdir);
CSTYLE_END

// Global data used to pass command line arguments to UserInput().
// Set to command line arguments to use them or NULL to use console input.

lptext* cmd_argv = NULL;

// Static data used to hold command line arguments in UTF8 format.

#ifdef EUINPUT              //---
#define MCARGS 32           //32 arguments max
#define MCBUFS 4096         //128 characters per argument max
text* cmd_pargs[MCARGS];    //Pointer to next command line argument
text  cmd_argsb[MCBUFS];    //Buffer for command line arguments
text  cmd_flags[MCBUFS];    //Buffer for -flag-value command line arguments

lint  cmd_quiet = FALSE;    //TRUE if UserOutput quiet mode
#endif                      //___

//*************************************************************************
//  FileCopy:
//      Copies a file to a target folder.
//      Allows progress bar.
//
//  Arguments:
//      errc FileCopy (int mode, text* pdup, text* pdir, text* psrc, 
//                     CWaitS* pbar)
//      mode    0 = Just copy full psrc path name to full pdir path
//      mode   +1 = Add number to base name to make unique           (CF_UNIQUE)
//                  Otherwise returns error rather than overwriting 
//             +2 = Overwrite if target file exits                   (CF_OVERWT)
//                  The +2 mode cannot be used with +1 
//             +4 = Copy to pdir path and file name not just folder  (CF_TONAME)
//                  The +4 mode may still change name to make unique 
//                  The +4 mode may still change pdir name extension
//             +8 = Copy to pdir path and file name with CF_EXT      (CF_STNAME)
//                  Changes pdir extension to CF_EXT or .dat.
//      pdup    Returns new copied path and file name.
//              Returns non-blank name even if copy failed (for error message).
//              May be NULL if not needed.
//      pdir    Target path and file name or target folder or folder and base.
//      psrc    Source path and file name.
//      pbar    Used to present progress bar.
//              Caller must supply progress bar handling function.
//              This function add bytes and calls pbar->pwfun.
//              May be NULL if no progress bar.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECOPEN if psrc cannot be opened.
//      Returns ECNOCREATE if target cannot be created.
//      Returns ECWRITE if write error (disk may be full).
//      Returns ECEXISTS if not CF_UNIQUE and exists in pdir.
//      Returns ECABORTED if pwin and pressed cancel.
//*************************************************************************

#define COPY_BUF (8*1024)                       //byte size of copy buffer

CSTYLE
isproc FileCopy (int mode, text* pdup, text* pdir, text* psrc, 
                 CWaitS* pbar) {
    byte bufr[COPY_BUF];
    text name[SZPATH];
    text* ptxt;
    text* pend;
    text* puse;
    aint sfile, dfile;
    lint have, done, cntr, add;
    errc e;

    if (pdup) *pdup = 0;                        //in case of error

    if (mode) {                                 //special modes?
        ptxt = name + OSFileDir(name, pdir);    //get target filename
        puse = (mode & CF_TONAME) ? pdir : psrc;
        pend = ptxt + OSFileBase(ptxt, puse, OS_NOEXT);
        ptxt = pend;
        cntr = 1024;
        while (TRUE) {
            if (mode & CF_STNAME) {
                OSTxtCopy(ptxt, CF_EXT);        //use .dat extension?
            } else {
                OSFileExt(ptxt, psrc);          //use source extension?
            }
            if (  (!(mode & CF_OVERWT))         //don't overwrite?
                &&(OSExists(0, name) == 0)  ) { //already exists on target?

                if (!(mode & CF_UNIQUE)) {
                    return(ECEXISTS);           //fail if exists?
                }

                cntr -= 1;
                if (cntr == 0) return(ECNOROOM);//cannot find free file name?

                ptxt = pend;
                *ptxt++ = '_';                  //name__HHHH.ext
                *ptxt++ = '_';
                add = OSTickCount() & 0xFFFF;
                ptxt = OSHexPut(add, ptxt);     //add four random hex digits
                continue;                       //and try again
            }
            break;
        }
    } else {                                    //mode 0 copy psrc->pdir
        OSTxtCopy(name, pdir);
    }
    if (pdup) OSTxtCopy(pdup, name);            //return new file name

    sfile = OSOpen(OS_READ, psrc);
    if (sfile == -1) return(ECOPEN);

    dfile = OSOpen(OS_CREATE, name);
    if (dfile == -1) return(ECNOCREATE);

    while (TRUE) {                              //copy buffers until done
        have = OSRead(sfile, bufr, COPY_BUF);
        if (have <= 0) break;

        done = OSWrite(dfile, bufr, have);
        if (done < have) goto wrter;
        
        if (pbar) {                             //show progress bar?
            pbar->count += (done >> PROG_KB);   //update bytes copied
            e = ((CallWait)pbar->pwfun)(0, pbar->infos, pbar->count, pbar->total, NULL);
            if (e) goto abter;
        }
        if (have < COPY_BUF) break;
    }

    OSClose(sfile);                             //all done
    OSClose(dfile);
    return(0);

abter:
    OSClose(sfile);
    OSClose(dfile);
    return(ECABORTED);

wrter:
    OSClose(sfile);
    OSClose(dfile);
    return(ECWRITE);
}

//*************************************************************************
//  FileComp:
//      Compares two open files.
//
//  Arguments:
//      errc FileComp (aint file1, aint file2)
//      file1   Open file handle for first file.
//      file2   Open file handle for second file.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECBADSIZE if file sizes do not match.
//      Returns ECNOMATCH if not all bytes match but sizes do.
//*************************************************************************

CSTYLE
isproc FileComp (aint file1, aint file2) {
    byte* pmem1;
    byte* pmem2;
    lint bytes, size2, did;
    
    bytes = OSSeek(file1, 0, OS_END);
    OSSeek(file1, 0, OS_SET);

    size2 = OSSeek(file2, 0, OS_END);
    OSSeek(file2, 0, OS_SET);

    if (size2 != bytes) {
        return(ECBADSIZE);                      //file sizes must match
    }

    pmem1 = (byte*) OSAlloc(bytes);
    if (pmem1 == NULL) {
        return(ECMEMORY);
    }
    did = OSRead(file1, pmem1, bytes);
    OSSeek(file1, 0, OS_SET);
    if (did != bytes) {
        OSFree(pmem1);
        return(ECREAD);
    }

    pmem2 = (byte*) OSAlloc(size2);
    if (pmem2 == NULL) {
        OSFree(pmem1);
        return(ECMEMORY);
    }
    did = OSRead(file2, pmem2, size2);
    OSSeek(file2, 0, OS_SET);
    if (did != size2) {
        OSFree(pmem1);
        OSFree(pmem2);
        return(ECREAD);
    }

    did = OSMemMatch(pmem1, pmem2, bytes);
    OSFree(pmem1);
    OSFree(pmem2);
    if (did != bytes) {
        return(ECNOMATCH);
    }
    return(0);
}

//*************************************************************************
//  ShowHelp:
//      Shows a help topic.
//      Must define /D USEHELP to enable this function.
//      That requires linking to Windows htmlhelp.lib.
//
//  Arguments:
//      errc ShowHelp (int mode, text* phelp, text* ptopic)
//      mode    0 = Show default help viewer, currently HELP_LOC.
//              1 = Show local help             (HELP_LOC)
//              2 = Show web help over internet (HELP_WEB)
//      phelp   Path name for help file to show.
//              May be "...\<file>.chm::/<subfile>.htm"
//              If HELP_WEB mode the must be web url.
//      ptopic  Topic name to view. Must be a HLP_? string defined in TText.h.
//              May be NULL to show from beginning.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

#ifdef ISWIN                                    //---
#include <HtmlHelp.h>
#endif                                          //---

CSTYLE
isproc ShowHelp (int mode, text* phelp, text* ptopic) {
    #ifndef USEHELP                             //===
    return(ECCANNOT);
    #else                                       //===
    text line[SZPATH];
    text exec[SZPATH];
    text* ptxt;
    errc e;

    // Use this to view help over the web.

    if (mode & HELP_WEB) {                      //mode 2?
        ptxt = OSTxtCopy(line, phelp);
        if (ptopic) {
            OSTxtCopy(ptxt, ptopic);            //add #topic name to url
        }

    // Under Windows use a compiled STS_Help.chm which encapsulates STS_Help.htm.

    } else {                                    //modes 0 or 1? (default)
        OSLocation(exec, OS_LOCEXE);            //get our exe path
        ptxt = line + OSFileDir(line, exec);    //get installation directory

        #ifdef ISWIN                            //--- Use Windows HtmlHelp
        ptxt = OSTxtCopy(ptxt, phelp);          //file.chm::/subfile.htm
        if (ptopic) {
            OSTxtCopy(ptxt, ptopic);            //...chm::/STS_Help.htm#topic
        }
        void* pw = HtmlHelpA(GetDesktopWindow(), line, HH_DISPLAY_TOPIC, NULL);
        return((pw) ? 0 : ECSYS);
        
    // Or use this to view help as local HTML file.
    // But unfortunately the browser may glitch the # anchor character!
    // So we currently use Help_?.htm redirect files locally.

        #else                                   //--- Use Default Browser Help
        if (ptopic) {
            ptxt = OSTxtCopy(ptxt, LOC_BASE);   //kludge redirect files because
            ptxt = OSTxtCopy(ptxt, ptopic+1);   //# does not work locally
            OSTxtCopy(ptxt, ".htm");            //STS_Help_<topic>.htm
        } else {                                //skipping ptopic's beginning #
            ptxt = OSTxtCopy(ptxt, LOC_HELP);   //or just use STS_Help.htm file
        }                                       //if no topic
        #endif                                  //---
    }

    e = OSBrowser(0, line);                     //view with default browser
    return(e);
    #endif                                      //===
}

//*************************************************************************
//  SerSpace:
//      Makes sure there is enough serializaton memory space.
//
//  Arguments:
//      errc SerSpace (SerIn* pser, lint need)
//      pser    Serialization information.
//      need    Maximum free bytes needed in serialization memory.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE
isproc SerSpace (SerIn* pser, lint need) {
    text* pmem;

    if ((pser->siz - pser->end) >= need) {
        return(0);                              //enough space already
    }
    need = pser->siz + ((need < SER_BLK) ? SER_BLK : (need+SER_BLK));
    pmem = (text*) OSReAlloc(pser->mem, need);  //allocate more space
    if (pmem == NULL) return(ECMEMORY);
    pser->mem = pmem;
    pser->siz = need;
    return(0);
}

//*************************************************************************
//  SerHead:
//      Writes or reads serialization file header to validate file.
//      This is not mandatory and is often not used.
//
//  Arguments:
//      errc SerHead (int mode, SerIn* pser, text* phead)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      phead   Serialization check text to validate file.
//              If NULL uses "Serialize".
//
//  Returns:
//      SER_GET returns ECDATABAD if does not begin with "STS Project" tag.
//*************************************************************************

CSTYLE
isproc SerHead (int mode, SerIn* pser, text* phead) {
    text line[SZNAME];
    text* ptxt;
    text* pval;
    int cntr;

    if (phead == NULL) phead = SER_HEAD;

    if (mode & SER_GET) {                       //Read value?
        pval = line;
        ptxt = pser->mem + pser->end;
        cntr = SZNAME-1;
        while ((*ptxt != SER_SEP)&&(*ptxt)) {
            *pval++ = *ptxt++;                  //read to tab ending
            if ((--cntr) == 0) break;           //just in case
        }
        *pval = 0;                              //zero terminate caller's value
        if (*ptxt == SER_SEP) ptxt += 1;
        if (OSTxtCmpS(line, phead)) {           //must be phead string
            pser->end = (lint)(ptxt - pser->mem);
            return(ECDATABAD);
        }
        
    } else {                                    //Write value?
        if (SerSpace(pser, OSTxtSize(phead))) return(ECMEMORY);
        ptxt = pser->mem + pser->end;           //write "STS Project" tag
        ptxt = OSTxtStop(ptxt, phead, SZPATH);
        *ptxt++ = SER_SEP;                      //end with tab not zero
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return(0);
}

//*************************************************************************
//  void SerTerm:
//      Serializes a record or section terminator.
//      This may only be used with text-based serialization.
//      This must NOT be used with SerRaws and SerRTXT!
//
//  Arguments:
//      void SerTerm (int mode, SerIn* pser, text term)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      term    Termination character: SER_REC or SER_SEC.
//              Allows \n and \t text: ALT_REC or ALT_TXT.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerTerm (int mode, SerIn* pser, text term) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        while (*ptxt) {
            if (*ptxt == term) {
                ptxt += 1;
                break;
            }
            ptxt += 1;                          //skip any extra before term
        }                                       //(probably from new version)
        
    } else {                                    //Write value?
        if (SerSpace(pser, 1)) return;
        ptxt = pser->mem + pser->end;
        *ptxt++ = term;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerPath:
//      Gets an absolute or relative file path name.
//      This is not mandatory and is often not used.
//
//  Arguments:
//      errc SerPath (int mode, SerIn* pser, text* pbuf, text* pabs)
//      mode    0 = Get absolute path                                  (SER_ABS)
//              4 = Get path relative to serialization file if can     (SER_REL)
//              8 = Get either absolute or relative name if exists     (SER_ANY)
//      pser    Serialization information with was and now for relative names.
//      pbuf    Buffer to receive the file path name.
//              May be NULL if not needed except for SER_ANY mode.
//              Can be same as pabs source for SER_ANY mode.
//      pabs    Absolute path name for file.
//              This is copied to pbuf if absolute.
//              Copied with possibly different directory if relative.
//
//  Returns:
//      Returns ECNODATA for SER_REL if no relative name exists.
//      Otherwise always returns zero.
//*************************************************************************

CSTYLE
isproc SerPath (int mode, SerIn* pser, text* pbuf, text* pabs) {
    text abs[SZPATH];
    errc e;

    if (mode & SER_ANY) {                       //find file anywhere we can?
        if (pbuf == NULL) return(ECARGUMENT);
        OSTxtStop(abs, pabs, SZPATH);           //make copy so pbuf can be same
        pabs = abs;
        OSTxtStop(pbuf, pabs, SZPATH);
        if (OSExists(0, pbuf) == 0) {
            return(0);                          //absolute path name exists?
        }
        e = SerPath(SER_REL, pser, pbuf, pabs); //if not try relative name
        if ((e == 0)&&(OSExists(0, pbuf) == 0)) {
            return(0);
        }
        *pbuf = 0;
        return(ECNODATA);
    }

    if (!(mode & SER_REL)) {                    //get absolute name?
        if (pbuf == NULL) return(0);
        OSTxtStop(pbuf, pabs, SZPATH);          //absolute name is just a copy
        return(0);
    }                                           //otherwise try relative name
    if ((pser->was[0] == 0)||(pser->now[0] == 0)) {
        return(ECNODATA);                       //just in case
    }
    if (OSTxtCmp(pser->was, pabs) != -2) {      //not in same dir or sub-dir?
        return(ECNODATA);
    }
    if (pbuf == NULL) return(0);
    pbuf = OSTxtCopy(pbuf, pser->now);          //place in new dir or sub-dir?
    pabs = pabs + (OSTxtSize(pser->was) - 1);   //part of name after orig dir
    OSTxtCopy(pbuf, pabs);
    return(0);
}

//*************************************************************************
//  SerFile:
//      Reads or writes ser.mem serialization memory block from/to open file.
//
//      Mostly used with SerRAWS and SerRTXT which keep pmem->use block size.
//      Text based serialization normally just reads and writes pmem->size
//      and is used for parameters so only keeps one serialization per file.
//
//      Raw serialization is meant for data files which need to be compact
//      and allows multiple sections that are separately serialized,
//      each preceeded by a dword block size.
//
//  Arguments:
//      errc SerFile (int mode, SerIn* pser, aint file, int nfix)
//      mode    1 = Read from serialization     (SER_GET)
//              2 = Write to serialization      (SER_PUT)
//              0x8002 = Repeat write beginning (SER_REP+SER_PUT)
//      pser    Serialization information.
//      file    Open file handle.
//      nfix    Fixed size block if non-zero byte count.
//              Use non-zero to avoid prefix size dword in file, which
//              can be convenent for fixed headers with just dword values.
//              Use 0 for variable-sized blocks with string or arrays.
//              Use 0 for unless using SerRAWS and SerRTXT raw format.
//              Text based serialization using SerDWRD and other functions
//              cannot be fixed size because it is value dependent.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE
isproc SerFile (int mode, SerIn* pser, aint file, int nfix) {
    lint need;
    lint rv;

    if (mode & SER_GET) {                       //Read?
        pser->end = 0;
        pser->use = 0;
        
        if (nfix <= 0) {
            rv = OSRead(file, &nfix, 4);        //block size saved first
            if (rv != 4) return(ECREAD);
            nfix -= 4;                          //less block size dword
        }
        if (nfix == 0) return(0);
        if (nfix < 0) return(ECBADFILE);
        if (pser->siz < nfix) {
            if (pser->mem) OSFree(pser->mem);
            pser->siz = 0;
            pser->use = 0;
            need = nfix;
            if (need < SER_BLK) need = SER_BLK;
            pser->mem = (text*) OSAlloc(need);  //allocate block memory
            if (pser->mem == NULL) return(ECMEMORY);
            pser->siz = need;
        }
        rv = OSRead(file, pser->mem, nfix);     //read file block
        if (rv != nfix) return(ECREAD);
        pser->use = nfix;
        return(0);

    } else if (mode & SER_REP) {                //Rewrite?
        if (pser->use == 0) pser->use = pser->siz;
        if ((pser->mem == NULL)||(pser->use <= 0)) return(0);
        OSSeek(file, (nfix <= 0) ? 4:0, OS_SET);//seek past old header size
        rv = OSWrite(file, pser->mem, pser->use);
        if (rv != pser->use) return(ECWRITE);   //write file block
        pser->end = 0;
        pser->use = 0;
        return(0);

    } else {                                    //Write?
        if (pser->use == 0) pser->use = pser->siz;
        if ((pser->mem == NULL)||(pser->use <= 0)) return(0);
        if (nfix <= 0) {
            nfix = pser->use + 4;
            rv = OSWrite(file, &nfix, 4);       //start block with byte size
            if (rv != 4) return(ECWRITE);
        }
        rv = OSWrite(file, pser->mem, pser->use);
        if (rv != pser->use) return(ECWRITE);   //write file block
        pser->end = 0;
        pser->use = 0;
        return(0);
    }
}

//*************************************************************************
//  SerRAWS:
//      Serializes raw bytes rather than text for more compact files.
//      This must not be mixed with the other Ser? functions above
//      This writes and reads raw bytes rather than text.
//      Read cannot stop on SerTerm termination characters.
//      Read does stop at end of file when extra parameters
//      have been added and new values at end are set to zeros.
//
//      Use these following macros from TriEx.h to call SerRAWS:
//        RAW_BODD(mode,pser,pval)      Serializes byte when doing 4 in row
//        RAW_BYTE(mode,pser,pval)      Serializes byte or cint value
//        RAW_BYTS(mode,pser,pval,cnt)  Serializes byte or cint array
//
//        RAW_WODD(mode,pser,pval)      Serializes word when doing 2 in row
//        RAW_WORD(mode,pser,pval)      Serializes word or sint value
//        RAW_WRDS(mode,pser,pval,cnt)  Serializes word or sint array
//
//        RAW_DWRD(mode,pser,pval)      Serializes dword or lint value
//        RAW_DWDS(mode,pser,pval,cnt)  Serializes dword or lint array
//
//        RAW_QWRD(mode,pser,pval)      Serializes qword or qint value
//        RAW_QWDS(mode,pser,pval,cnt)  Serializes qword or qint array
//
//        RAW_TEXT(mode,pser,pstr,max)  Serializes zero-term char* string
//
//        RAW_BALL(mode,pser,pval,siz)  Allocates qword array before RAW_BYTS
//        RAW_WALL(mode,pser,pval,siz)  Allocates qword array before RAW_WRDS
//        RAW_DALL(mode,pser,pval,siz)  Allocates qword array before RAW_DWDS
//        RAW_QALL(mode,pser,pval,siz)  Allocates qword array before RAW_QWDS
//        RAW_TALL(mode,pser,pstr,siz)  Allocates text memory before RAW_TEXT
//      These macros only need to be given SER_GET or SER_PUT for mode.
//      These macros automatically add SER_RAW and RAW_WRD,DRD,QWD to mode.
//      These macros are given value pointers and pass pval to SerRAWS.
//      Array macros are given array count and call SerRAWS with cnt*size.
//      The RAW_?ODD macros are used to serialize multiples of four bytes
//      and/or two words in a row and use the RAW_ODD flag to avoid
//      using four bytes of the file for a single value.
//      This keeps normal values dword aligned for OSMemCopy speed.
//      The RAW-BALL-RAW_QALL and RAW_TALL macros allocate a memory array
//      if SER_GET for the following RAW_QWDS and RAW_TEXT macros and
//      caller must define a memer: label which they goto for OSAlloc errors.
//
//  Arguments:
//      void SerRAWS (int mode, SerIn* pser, void* pval, int bytes)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//          +0x40 = Raw bytes is always required    (SER_RAW)
//         +0x100 = Do NOT align next on dword      (RAW_ODD)
//                  Use this for byte or word
//                  values which do not fill dword
//                  for fast dword OSMemCopy after
//                  Use if will serialize multiple
//                  of 4 bytes or 2 words
//         +0x200 = Low-endian word values          (RAW_WRD)
//         +0x400 = Low-endian dword values         (RAW_DRD)
//         +0x800 = Low-endian qword values         (RAW_QRD)
//              May use only one RAW_? flag and bytes must be
//              a multiple of 2, 4, or 8 if a RAW_? flag is used.
//              In CBIG_ENDIAN big-endian computers RAW_? forces
//              byte swap so files always have low-endan values,
//              which allows files to be moved betwen big and low.
//      pser    Serialization information.
//      pval    Returns or supplies data block to read or write.
//              This is usually a single value such as a float or lint.
//              This can be for multiple word,dword,qword signed or unsigned.
//      bytes   Bytes of data in pval.
//              This can be for multiple word,dword,qword values
//              either signed or unsigned but must be the byte count.
//              For example an array of 10 float values would be 40.
//              If zero, this function does nothing.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerRAWS (int mode, SerIn* pser, void* pval, int bytes) {
    char* pmem;
    int left, need;

    if (bytes <= 0) {                           //no bytes to do?
        return;
    }
    if (!(mode & SER_RAW)) {                    //make sure caller wants raw
        OSBeep(2);                              //error beep
        return;
    }
    if (mode & SER_GET) {                       //Read value?
        pmem = pser->mem + pser->end;
        left = pser->use - pser->end;           //bytes left
        if (bytes > left) {                     //new values not in old file?
            OSMemClear(pval, bytes);            //zero new values at end
            pser->end = pser->use;
            return;
        }

        #ifdef CBIG_ENDIAN                      //=== big-endian system
        if        (mode & RAW_DWD) {            //swap dword values?
            dword* pdw = (dword*) pmem;         //so file is always low-endian
            int cnt = bytes >> 2;
            while (cnt-- > 0) {
                *pdw++ = DSWAP(*pdw);           //swap dword bytes
            }
        } else if (mode & RAW_WRD) {            //swap word values?
            word* pwd = (word*) pmem;
            int cnt = bytes >> 1;
            while (cnt-- > 0) {
                *pwd++ = WSWAP(*pwd);           //swap word bytes
            }
        } else if (mode & RAW_QWD) {            //swap qword values?
            qword* pqw = (qword*) pmem;
            int cnt = bytes >> 3;
            while (cnt-- > 0) {
                *pqw++ = QSWAP(*pqw);           //swap qword bytes
            }
        }
        #endif                                  //===

        OSMemCopy(pval, pmem, bytes);           //copy raw bytes
        need = bytes;
        if ((bytes & 3)&&(!(mode & RAW_ODD))) {
            need += 4 - (bytes & 3);            //round up to even dwords
        }
      
    } else {                                    //Write value?
        need = bytes;
        if ((bytes & 3)&&(!(mode & RAW_ODD))) {
            need += 4 - (bytes & 3);            //round up to even dwords
        }
        if (SerSpace(pser, need)) return;       //make space for whole value
        pmem = pser->mem + pser->end;
        OSMemCopy(pmem, pval, bytes);           //copy raw bytes
        pser->use += (lint) need;

        #ifdef CBIG_ENDIAN                      //=== big-endian system
        if        (mode & RAW_DWD) {            //swap dword values?
            dword* pdw = (dword*) pmem;         //so file is always low-endian
            int cnt = bytes >> 2;
            while (cnt-- > 0) {
                *pdw++ = DSWAP(*pdw);           //swap dword bytes
            }
        } else if (mode & RAW_WRD) {            //swap word values?
            word* pwd = (word*) pmem;
            int cnt = bytes >> 1;
            while (cnt-- > 0) {
                *pwd++ = WSWAP(*pwd);           //swap word bytes
            }
        } else if (mode & RAW_QWD) {            //swap qword values?
            qword* pqw = (qword*) pmem;
            int cnt = bytes >> 3;
            while (cnt-- > 0) {
                *pqw++ = QSWAP(*pqw);           //swap qword bytes
            }
        }
        #endif                                  //===
    }
    if ((bytes & 3)&&(!(mode & RAW_ODD))&&(!(mode & SER_GET))) {
        pmem += bytes;
        left = 4 - (bytes & 3);                 //bytes left in last dword
        do {
            *pmem++ = 0;                        //zero end so files compare
        } while (--left);                       //even though bytes unused
    }
    pser->end += (lint) need;
    return;
}

//*************************************************************************
//  SerRTXT:
//      Serializes a text string value as raw bytes.
//
//      Use this macro from TriEx.h to call SerRTXT:
//        RAW_TALL(mode,pser,pstr,siz)  Allocates text memory before RAW_TEXT
//        RAW_TEXT(mode,pser,pstr,max)  Serializes zero-term char* string
//      This macro only needs to be given SER_GET or SER_PUT for mode.
//      The RAW_TALL macro allocates a memory array if SER_GET
//      for the following RAW_TEXT macros and caller must
//      define a memer: label which they goto for OSAlloc errors.
//
//  Arguments:
//      void SerRTXT (int mode, SerIn* pser, text* pval, int max)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//          +0x40 = Raw bytes is always required    (SER_RAW)
//         +0x100 = Do NOT align next on dword      (RAW_ODD)
//                  Use this for strings to
//                  compact file as much as possible
//                  This is NOT recommended!
//      pser    Serialization information.
//      pval    Returns or supplies zero terminated string.
//              If SER_GET *pval buffer must be large enough.
//              Handles saving and restoring NULL pointer or blank string.
//      bytes   Maximum byte size of SER_GET buffer.
//              Uses SZPATH if zero.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerRTXT (int mode, SerIn* pser, text* pval, int max) {
    char* pmem;
    lint used, need, left;

    if (max <= 0) max = SZPATH;
    if (!(mode & SER_RAW)) {                    //make sure caller wants raw
        OSBeep(2);                              //error beep
        return;
    }

    if (mode & SER_GET) {                       //Read value?
        pmem = pser->mem + pser->end;
        left = pser->use - pser->end;           //bytes left
        if (left < 4) {                         //new values not in old file?
            return;                             //return NULL strings if new
        }
        used = *((lint*)pmem);
        pmem += 4;
        if (used > (left - 4)) {                
            if (used) *pval = 0;                //return blank strings if new
            return;
        }
        if (used) {
            OSMemCopy(pval, pmem, used);        //copy string with zero term
        }
        need = used + 4;                        //total bytes used by string
        if ((used & 3)&&(!(mode & RAW_ODD))) {
            need += 4 - (used & 3);             //round up to even dwords
        }

    } else {                                    //Write value?
        used = (pval) ? OSTxtSize(pval) : 0;
        if (used > max) used = max;             //limit size
        need = used + 4;                        //four bytes for size dword
        if ((used & 3)&&(!(mode & RAW_ODD))) {
            need += 4 - (used & 3);             //round up to even dwords
        }
        if (SerSpace(pser, need)) return;
        pmem = pser->mem + pser->end;
        *((lint*)pmem) = used;                  //start with size dword
        pmem += 4;
        if (pval) {
            OSMemCopy(pmem, pval, used);        //followed by string with zero
        }
        pser->use += need;
    }

    if ((used & 3)&&(!(mode & RAW_ODD))&&(!(mode & SER_GET))) {
        pmem += used;
        used = 4 - (used & 3);                  //bytes left in last dword
        do {
            *pmem++ = 0;                        //zero end so files compare
        } while (--used);                       //even though bytes unused
    }
    pser->end += need;
    return;
}

//*************************************************************************
//  SerINTV:
//      Serializes an int value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerINTV (int mode, SerIn* pser, int* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerINTV (int mode, SerIn* pser, int* pval) {
    text* ptxt;
    dword num;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = (int) OSHexGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_INT)) return;
        ptxt = pser->mem + pser->end;
        num = (dword) *pval;                    //in case int is not dword
        ptxt = OSHexPut(num, ptxt);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerLINT:
//      Serializes a lint value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerLINT (int mode, SerIn* pser, lint* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerLINT (int mode, SerIn* pser, lint* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = OSHexGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_INT)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSHexPut(*pval, ptxt);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerDWRD:
//      Serializes a dword value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerDWRD (int mode, SerIn* pser, dword* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerDWRD (int mode, SerIn* pser, dword* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = OSHexGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_INT)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSHexPut(*pval, ptxt);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerWORD:
//      Serializes a word value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerLINT (int mode, SerIn* pser, word* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerWORD (int mode, SerIn* pser, word* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = (word) OSHexGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_INT)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSHexPut(*pval, ptxt);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerBYTE:
//      Serializes a byte value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerBYTE (int mode, SerIn* pser, byte* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerBYTE (int mode, SerIn* pser, byte* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = (byte) OSHexGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_INT)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSHexPut(*pval, ptxt);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerFLTV:
//      Serializes a flt 32-bit floating value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerFLTV (int mode, SerIn* pser, flt* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerFLTV (int mode, SerIn* pser, flt* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = (flt) OSFloatGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_FLT)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSFloatPut(*pval, ptxt, 0, 0);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerFLXV:
//      Serializes a flx 64-bit floating value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerFLTV (int mode, SerIn* pser, flt* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies value.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerFLXV (int mode, SerIn* pser, flx* pval) {
    text* ptxt;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            *pval = (flx) OSFloatGet(ptxt, &ptxt);
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, SER_FLX)) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSFloatPut(*pval, ptxt, OS_FLXWIDE,OS_FLXSIGS);
        *ptxt++ = SER_SEP;
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerTEXT:
//  AltTEXT:
//      Serializes a text string value.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//      AltTEXT version allows text to use \t, \n, \f chars.
//
//  Arguments:
//      void SerTEXT (int mode, SerIn* pser, text* pval, int bytes)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies zero terminated string.
//              If SER_GET *pval buffer must be large enough.
//              But text values are limited to SZPATH.
//      bytes   Maximum byte size of SER_GET buffer.
//              Uses SZPATH if zero.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerTEXT (int mode, SerIn* pser, text* pval, int bytes) {
    text* ptxt;
    int cntr;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            cntr = SZPATH-1;
            while ((*ptxt != SER_SEP)&&(*ptxt)) {
                *pval++ = *ptxt++;              //read to tab ending
                if ((--cntr) == 0) break;       //just in case
            }
            *pval = 0;                          //zero terminate caller's value
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, OSTxtSize(pval))) return;
        if (bytes <= 0) bytes = SZPATH;
        ptxt = pser->mem + pser->end;
        ptxt = OSTxtStop(ptxt, pval, (bytes-1));
        *ptxt++ = SER_SEP;                      //end with tab not zero
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

CSTYLE
isvoid AltTEXT (int mode, SerIn* pser, text* pval, int bytes) {
    text* ptxt;
    int cntr;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            cntr = SZPATH-1;
            while ((*ptxt != ALT_SEP)&&(*ptxt)) {
                *pval++ = *ptxt++;              //read to tab ending
                if ((--cntr) == 0) break;       //just in case
            }
            *pval = 0;                          //zero terminate caller's value
        }
        if (*ptxt == ALT_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, OSTxtSize(pval))) return;
        if (bytes <= 0) bytes = SZPATH;
        ptxt = pser->mem + pser->end;
        ptxt = OSTxtStop(ptxt, pval, (bytes-1));
        *ptxt++ = ALT_SEP;                      //end with tab not zero
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerNAME:
//      Serializes a text string value.
//      Same as SerTEXT but limits to SZNAME characters.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerNAME (int mode, SerIn* pser, text* pval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies zero terminated string.
//              If SER_GET *pval buffer must be large enough.
//              But text values are limited to SZPATH.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerNAME (int mode, SerIn* pser, text* pval) {
    text* ptxt;
    int cntr;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            cntr = SZNAME-1;
            while ((*ptxt != SER_SEP)&&(*ptxt)) {
                *pval++ = *ptxt++;              //read to tab ending
                if ((--cntr) == 0) break;       //just in case
            }
            *pval = 0;                          //zero terminate caller's value
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        if (SerSpace(pser, OSTxtSize(pval))) return;
        ptxt = pser->mem + pser->end;
        ptxt = OSTxtStop(ptxt, pval, SZNAME);
        *ptxt++ = SER_SEP;                      //end with tab not zero
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerPTXT:
//      Serializes a text string pointer value.
//      Writes a NULL string pointer as a blank string.
//      The SER_GET does not change value if nothing stored,
//      so caller must preset to a reasonable default.
//
//  Arguments:
//      void SerPTXT (int mode, SerIn* pser, lptext* ppval)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pval    Returns or supplies zero terminated string.
//              If SER_GET *ppval pointer is reallocated.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerPTXT (int mode, SerIn* pser, lptext* ppval) {
    text* pval;
    text* ptxt;
    int cntr;

    if (mode & SER_GET) {                       //Read value?
        ptxt = pser->mem + pser->end;
        if ((unsigned)*ptxt > ' ') {
            pval = ptxt;
            while ((*ptxt != SER_SEP)&&(*ptxt)) {
                ptxt += 1;                      //determine size of string
            }
            if (*ppval) OSFree(*ppval);
            cntr = PTROFF(pval, ptxt);          //allocate new memory for string
            *ppval = (text*) OSAlloc(cntr+SZCHR);
            if (*ppval) {
                OSMemCopy(*ppval, pval, cntr);  //and copy it in
                (*ppval)[ptxt-pval] = 0;        //zero terminate
            }
        }
        if (*ptxt == SER_SEP) ptxt += 1;
        
    } else {                                    //Write value?
        pval = *ppval;
        cntr = (pval) ? OSTxtSize(pval) : 1;
        if (SerSpace(pser, cntr)) return;
        ptxt = pser->mem + pser->end;
        if (pval) {
            ptxt = OSTxtCopy(ptxt, pval);
        }
        *ptxt++ = SER_SEP;                      //end with tab not zero
        *ptxt = 0;
    }
    pser->end = (lint)(ptxt - pser->mem);
    return;
}

//*************************************************************************
//  SerBLOK:
//      Serializes a fixed size block of data.
//
//  Arguments:
//      void SerBLOK (int mode, SerIn* pser, void* pblok, int bytes)
//      mode    1 = Read value from serialization   (SER_GET)
//              2 = Write value to serialization    (SER_PUT)
//      pser    Serialization information.
//      pblok   Returns or supplies data block.
//      bytes   Bytes of data in pblock
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid SerBLOK (int mode, SerIn* pser, void* pblok, int bytes) {
    int count;
    
    count = bytes >> 2;
    bytes = bytes - (count << 2);
    
    while (count--) {
        SerDWRD(mode, pser, (dword*)pblok);
        pblok = ADDOFF(void*, pblok, 4);
    }
    while (bytes--) {
        SerBYTE(mode, pser, (byte*)pblok);
        pblok = ADDOFF(void*, pblok, 1);
    }
    return;
}

//*************************************************************************
//  htmPost:
//      Finds a value in a HTML GET/POST style string,
//        tag1=value1&tag2=value2...
//      Converts from URL encoding to normal text.
//        tag=One%2C+Two&...=>"One, Two"
//
//  Arguments:
//      errc htmPost (text* phtm, text* ptag, text* pval, int nval)
//      phtm    Pointer to memory copy of posted text.
//      ptag    Name for pval setting (appears as "<tag>=<value>&...")
//              Upper and lower case count!
//      pval    Returns zero terminated value.
//      nval    Max characters in pval buffer (including ending zero)
//              If zero uses 256.
//
//  Returns:
//      Returns size of pval returned string or negative code if error.
//      Returns ECNOMATCH if pval not found in phtm.
//      Returns ECNOROOM if nval is not large enough.
//*************************************************************************

CSTYLE
isproc htmPost (text* phtm, text* ptag, text* pval, int nval) {
    text* pold;
    text cc, cv;
    int nc;

    if (nval <= 0) nval = 256;
    nc = OSTxtSize(ptag) - 1;
    while (*phtm) {
        cc = *phtm;
        cc = CAPITAL(cc);
        if (  (cc == *ptag)
            &&(*(phtm+nc) == '=')
            &&(!(OSTxtCmpU(ptag, phtm) & 1))  ) {
            phtm += nc + 1;                     //found ptag=
            while (*phtm == ' ') phtm += 1;
            pold = pval;
            while ((*phtm)&&(*phtm != '&')) {
                cc = *phtm++;                   //return value to caller
                if (cc == '+') {                //decode URL encoding
                    cc = ' ';                   //+ is blank char
                } else if (cc == '%') {         //%XX is hex ascii code
                    cc = 0;
                    if (*phtm) {
                        cv = *phtm++;
                        cv = CAPITAL(cv);
                        cv = (cv >= 'A') ? (0xA+(cv - 'A')) : (cv - '0');
                        cc = (cv & 0xF);
                        if (*phtm) {
                            cv = *phtm++;
                            cv = CAPITAL(cv);
                            cv = (cv >= 'A') ? (0xA+(cv - 'A')) : (cv - '0');
                            cc = (cc << 4) + (cv & 0xF);
                        }
                    }
                }
                *pval++ = cc;                   //return value to caller
                if ((--nval) <= 0) {            //not enough output room?
                    *pold = 0;                  //return blank value
                    return(ECNOROOM);
                }
            }
            *pval = 0;                          //ending of value?
            return(pval - pold);                //return size of returned value
        }
        while ((*phtm)&&(*phtm != '&')) {       //wrong tag so find end
            phtm += 1;
        }
        if (*phtm == '&') phtm += 1;            //skip to next value
    }
    *pval = 0;                                  //ptag= not found?
    return(ECNOMATCH);
}

//*************************************************************************
//  xmlFind:
//      Finds a named value in a simple XML file which consists of line:
//        <tag1>value1</tag1>\r\n
//        <tag2>value2</tag2>\r\n
//        ...
//      Or can find the value in a HTML GET/POST style string:
//        tag1=value1&tag2=value2...
//
//  Arguments:
//      errc xmlFind (text* pxml, text* ptag, text* pval)
//      pxml    Pointer to memory copy of XML file.
//      ptag    XML name for pval setting without the angle brackets.
//              Upper and lower are ignored!
//      pval    Returns zero terminated value.
//              Caller must supply large enough buffer, normally BLKSIZE.
//
//  Returns:
//      Non-zero code if error (and blank pval)
//      Returns ECNOMATCH if there is no <ptag> in pxml.
//      Returns ECSYNTAX if there is no </ptag> after value.
//*************************************************************************

CSTYLE
isproc xmlFind (text* pxml, text* ptag, text* pval) {
    text* pold;
    int nc, left;

    nc = OSTxtSize(ptag) - 1;
    while (*pxml) {
        if (*pxml == '<') {
            pxml += 1;
            if ((*pxml == *ptag)&&(!(OSTxtCmp(ptag, pxml) & 1))) {
                pxml += nc;
                if (*pxml == '>') {             //found <ptag>?
                    pxml += 1;
                    while (*pxml == ' ') pxml += 1;
                    pold = pval;
                    left = BLKSIZE;
                    while (*pxml) {
                        if (  (*pxml == '<')    //ending </ptag>?
                            &&(*(pxml+1) == '/')
                            &&(!(OSTxtCmp(ptag, pxml+2) & 1))
                            &&(*(pxml+nc+2) == '>')  ) {
                            *pval = 0;
                            return(0);
                        }
                        *pval++ = *pxml++;      //return value to caller
                        if ((--left) == 0) break;
                    }
                    *pold = 0;                  //no </ptag> ending?
                    return(ECSYNTAX);
                }
            }
        }
        pxml += 1;
    }
    *pval = 0;                                  //<ptag> not found?
    return(ECNOMATCH);
}

//*************************************************************************
//  xmlInit:
//      Handles initalizing for xmlLINT etc.
//
//  Arguments:
//      text* xmlInit (int mode, SerIn* pxml)
//      mode    0 = Free any existing XML file memory.
//              1 = Initialize before building an XML file.
//              2 = Return XML file text.
//
//  Returns:
//      Pointer to XML file text or NULL for modes 0 and 1.
//*************************************************************************

CSTYLE
istext xmlInit (int mode, SerIn* pxml) {
    if (mode == 0) {
        if (pxml->mem) OSFree(pxml->mem);
        OSMemClear(pxml, sizeof(SerIn));
        return(NULL);
    }
    if (mode == 1) {
        OSMemClear(pxml, sizeof(SerIn));
        return(NULL);
    }
    return(pxml->mem);
}

//*************************************************************************
//  xmlCopy:
//      Copys text to SerIn memory.
//
//  Arguments:
//      void xmlCopy (SerIn* pxml, text* ptxt)
//      pxml    Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptxt    Zero terminated string to add to end of pxml memory.
//
//  Returns:
//      Nothing.
//*************************************************************************

CSTYLE
isvoid xmlCopy (SerIn* pxml, text* ptxt) {
    int bytes = (OSTxtSize(ptxt) - 1)*SZCHR;    //grow memory if needed
    if (SerSpace(pxml, SZDISK)) return;
    OSMemCopy((pxml->mem + pxml->end), ptxt, bytes);
    return;                                     //and copy text to end
}

//*************************************************************************
//  xmlLINT:
//      Sets and/or gets a dword value.
//      Used to set or read Ajax XML settings.
//
//  Arguments:
//      void xmlLINT (SerIn* pxml, text* ptag, text* pval, lint* pset)
//      pxml    Used to build XML "<name>value</name>\r\n" settings in pser->mem.
//              Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptag    XML name for pval setting without the angle brackets.
//      pval    New text value for ptag.
//              Ignored if NULL or blank.
//      pset    Value to be modified and/or returned.
//
//  Returns:
//      Nothing other than pxml and pset.
//*************************************************************************

CSTYLE
isvoid xmlLINT (SerIn* pxml, text* ptag, text* pval, lint* pset) {
    text* ptxt;

    if (pval) {
        while (*pval == ' ') pval += 1;
        if (*pval) {                            //change value?
            *pset = OSIntGet(pval, NULL);
        }
    }
    if (pxml) {                                 //return new or existing value?
        if (SerSpace(pxml, SZDISK)) return;

        ptxt = pxml->mem + pxml->end;
        *ptxt++ = '<';                          //<name>value</name>
        ptxt = OSTxtCopy(ptxt, ptag);           //added to end of XML
        *ptxt++ = '>';
        ptxt = OSIntPut(*pset, ptxt);
        *ptxt++ = '<';
        *ptxt++ = '/';
        ptxt = OSTxtCopy(ptxt, ptag);
        *ptxt++ = '>';
        *ptxt++ = SEP_LF;        
        *ptxt = 0;
        pxml->end = (lint)(ptxt - pxml->mem);
    }
    return;
}

//*************************************************************************
//  xmlDWRD:
//      Sets and/or gets a dword value.
//      Used to set or read Ajax XML settings.
//
//  Arguments:
//      void xmlDWRD (SerIn* pxml, text* ptag, text* pval, dword* pset)
//      pxml    Used to build XML "<name>value</name>\r\n" settings in pser->mem.
//              Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptag    XML name for pval setting without the angle brackets.
//      pval    New text value for ptag.
//              Ignored if NULL or blank and existing pset is not modified.
//      pset    Value to be modified and/or returned.
//
//  Returns:
//      Nothing other than pxml and pset.
//*************************************************************************

CSTYLE
isvoid xmlDWRD (SerIn* pxml, text* ptag, text* pval, dword* pset) {
    text* ptxt;

    if (pval) {
        while (*pval == ' ') pval += 1;
        if (*pval) {                            //change value?
            *pset = OSIntGet(pval, NULL);
        }
    }
    if (pxml) {                                 //return new or existing value?
        if (SerSpace(pxml, SZDISK)) return;

        ptxt = pxml->mem + pxml->end;
        *ptxt++ = '<';                          //<name>value</name>
        ptxt = OSTxtCopy(ptxt, ptag);           //added to end of XML
        *ptxt++ = '>';
        ptxt = OSIntPut(*pset, ptxt);
        *ptxt++ = '<';
        *ptxt++ = '/';
        ptxt = OSTxtCopy(ptxt, ptag);
        *ptxt++ = '>';
        *ptxt++ = SEP_LF;        
        *ptxt = 0;
        pxml->end = (lint)(ptxt - pxml->mem);
    }
    return;
}

//*************************************************************************
//  xmlFLTV:
//      Sets and/or gets a dword value.
//      Used to set or read Ajax XML settings.
//
//  Arguments:
//      void xmlFLTV (SerIn* pxml, text* ptag, text* pval, flt* pset, int decs)
//      pxml    Used to build XML "<name>value</name>\r\n" settings in pser->mem.
//              Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptag    XML name for pval setting without the angle brackets.
//      pval    New text value for ptag.
//              Ignored if NULL or blank and existing pset is not modified.
//      pset    Value to be modified and/or returned.
//      decs    Number of digits after the decimal point.
//
//  Returns:
//      Nothing other than pxml and pset.
//*************************************************************************

CSTYLE
isvoid xmlFLTV (SerIn* pxml, text* ptag, text* pval, flt* pset, int decs) {
    text* ptxt;

    if (pval) {
        while (*pval == ' ') pval += 1;
        if (*pval) {                            //change value?
            *pset = (flt) OSFloatGet(pval, NULL);
        }
    }
    if (pxml) {                                 //return new or existing value?
        if (SerSpace(pxml, SZDISK)) return;

        ptxt = pxml->mem + pxml->end;
        *ptxt++ = '<';                          //<name>value</name>
        ptxt = OSTxtCopy(ptxt, ptag);           //added to end of XML
        *ptxt++ = '>';
        if ((decs == 0)&&(*pset >= 1.0)) {
            ptxt = OSIntPut((lint)*pset, ptxt);
        } else {
            ptxt = OSFloatPut(*pset, ptxt, -2, -decs);
        }
        *ptxt++ = '<';
        *ptxt++ = '/';
        ptxt = OSTxtCopy(ptxt, ptag);
        *ptxt++ = '>';
        *ptxt++ = SEP_LF;        
        *ptxt = 0;
        pxml->end = (lint)(ptxt - pxml->mem);
    }
    return;
}

//*************************************************************************
//  xmlTEXT:
//      Sets and/or gets a text string value.
//      Used to set or read Ajax XML settings.
//
//  Arguments:
//      void xmlTEXT (SerIn* pxml, text* ptag, text* pval, text* pset)
//      pxml    Used to build XML "<name>value</name>\r\n" settings in pser->mem.
//              Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptag    XML name for pval setting without the angle brackets.
//      pval    New text value for ptag.
//              Ignored if NULL or blank and existing pset is not modified.
//      pset    Value to be modified and/or returned.
//
//  Returns:
//      Nothing other than pxml and pset.
//*************************************************************************

CSTYLE
isvoid xmlTEXT (SerIn* pxml, text* ptag, text* pval, text* pset) {
    text* ptxt;
    int need;

    if (pval) {
        while (*pval == ' ') pval += 1;
        if (*pval) {                            //change value?
            OSTxtCopy(pset, pval);
        }
    }
    if (pxml) {                                 //return new or existing value?
        need = SZDISK;
        if (pset) {
            need += OSTxtSize(pset);
        }
        if (SerSpace(pxml, need)) return;

        ptxt = pxml->mem + pxml->end;
        *ptxt++ = '<';                          //<name>value</name>
        ptxt = OSTxtCopy(ptxt, ptag);           //added to end of XML
        *ptxt++ = '>';
        if (pset) {
            ptxt = OSTxtCopy(ptxt, pset);
        }
        *ptxt++ = '<';
        *ptxt++ = '/';
        ptxt = OSTxtCopy(ptxt, ptag);
        *ptxt++ = '>';
        *ptxt++ = SEP_LF;        
        *ptxt = 0;
        pxml->end = (lint)(ptxt - pxml->mem);
    }
    return;
}

//*************************************************************************
//  xmlPTXT:
//      Sets and/or gets an OSSafe allocated text string value.
//      Used to set or read Ajax XML settings.
//
//  Arguments:
//      void xmlPTXT (SerIn* pxml, text* ptag, text* pval, lptext* ppset)
//      pxml    Used to build XML "<name>value</name>\r\n" settings in pser->mem.
//              Automtically grows memory as needed.
//              Caller should clear the SerIn structure before first call.
//              May be NULL to skip setting XML value and just get the value.
//      ptag    XML name for pval setting without the angle brackets.
//      pval    New text value for ptag.
//              Ignored if NULL or blank and existing pset is not modified.
//      ppset   Value to be modified and/or returned.
//
//  Returns:
//      Nothing other than pxml and pset.
//*************************************************************************

CSTYLE
isvoid xmlPTXT (SerIn* pxml, text* ptag, text* pval, lptext* ppset) {
    text* ptxt;
    int need;

    if (pval) {
        while (*pval == ' ') pval += 1;
        if (*pval) {                            //change value?
            OSTxtSafe(ppset, pval, 0);
        }
    }
    if (pxml) {                                 //return new or existing value?
        need = SZDISK;
        if ((ppset)&&(*ppset)) {
            need += OSTxtSize(*ppset);
        }
        if (SerSpace(pxml, need)) return;

        ptxt = pxml->mem + pxml->end;
        *ptxt++ = '<';                          //<name>value</name>
        ptxt = OSTxtCopy(ptxt, ptag);           //added to end of XML
        *ptxt++ = '>';
        if ((ppset)&&(*ppset)) {
            ptxt = OSTxtCopy(ptxt, *ppset);
        }
        *ptxt++ = '<';
        *ptxt++ = '/';
        ptxt = OSTxtCopy(ptxt, ptag);
        *ptxt++ = '>';
        *ptxt++ = SEP_LF;        
        *ptxt = 0;
        pxml->end = (lint)(ptxt - pxml->mem);
    }
    return;
}

#ifdef EXFIFOS                                  //---
//*************************************************************************
//  QFifo::QFifo:
//  QFifo::~QFifo:
//      FIFO object constructor and destructor.
//      QFifo can pass fixed-size messages safely without mutex.
//*************************************************************************

QFifo::QFifo () {
    m_fifo = NULL;
    m_free = NULL;
    m_nmsg = 0;
    m_msiz = 0;
    m_iput = 0;
    m_iget = 0;
    m_errct = 0;
    m_extra = 0;
    m_lock = NULL;
    #ifdef QF_LOCK                              //---
    OSLock(OS_LOCKNEW, &m_lock);
    #endif                                      //---
    return;
}

QFifo::~QFifo () {
    if (m_free) OSFree(m_free);
    m_fifo = NULL;
    m_free = NULL;
    m_nmsg = 0;
    m_msiz = 0;
    m_iput = 0;
    m_iget = 0;
    m_errct = 0;
    m_extra = 0;
    if (m_lock) {
        #ifdef QF_LOCK                          //---
        OSLock(OS_LOCKOFF, &m_lock);
        #endif                                  //===
        m_lock = NULL;
    }
    return;
}

//*************************************************************************
//  QFifo::Init:
//      Allocates a different size and flushes any old data.
//      Must be called initially to allocate fifo.
//
//      Note: QFifo is safe without using a mutex as long as there
//      is only one send task and one receive task and also if run
//      on a single processor non-mutlicore system or with an
//      effective M_FENCE memory barrier on the target system.
//      Otherwise, compile with /D "QF_LOCK" to add mutex locking!
//
//  Arguments:
//      errc Init (int mode, dword nmsg, dword nsiz)
//      mode    0 = Normal
//              1 = Leave prexisting caller supplied m_fifo buffer pointer.
//                  1 can be used for fixed location fifos.
//      nmsg    Maximum FIFO messages or 0 for 256 default.
//              Note that put must leave one unused message so
//              get knows the difference between empty and full,
//              and so space is allocated for nmsg+1 messages.
//      msiz    Byte size per message or 0 for 512x4 default.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc QFifo::Init (int mode, dword nmsg, dword msiz) {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---
    if (msiz == 0) msiz = QF_SIZE;
    if (nmsg == 0) nmsg = QF_DEEP-1;
    nmsg += 1;                                  //need one unused slot
    if (mode & 1) {
        m_nmsg = nmsg;
        m_msiz = msiz;
    }
    if ((m_fifo == NULL)||(m_nmsg != nmsg)||(m_msiz != msiz)) {
        if (m_fifo) OSFree(m_fifo);
        m_nmsg = 0;
        m_msiz = 0;
        m_fifo = (byte*) OSAlloc(nmsg * msiz);  //allocate memory for fifo
        m_free = m_fifo;                        //must free because we own
        if (m_fifo == NULL) {
            #ifdef QF_LOCK
            OSLock_DONE(m_lock);
            #endif
            return(ECMEMORY);
        }
    }
    m_nmsg = nmsg;                              //init empty state
    m_msiz = msiz;
    m_iput = 0;
    m_oput = 0;
    m_iget = 0;
    m_oget = 0;
    m_errct = 0;
    m_extra = 0;
    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return(0);
}

//*************************************************************************
//  QFifo::IniV:
//      Initializes a fifo with 32-bit message values.
//      May be used with PutV and GetV to pass simple
//      messages or states between tasks.
//      Caller can supply memory.
//
//  Arguments:
//      errc IniV (lint* pmem, dword nmsg)
//      pmem    Caller supplied message buffer.
//              Must be nmsg*4 bytes.
//              May be NULL to allocate memory.
//      nmsg    Maximum FIFO messages or 0 for 16 default.
//              Note that put must leave one unused message so
//              get knows the difference between empty and full,
//              and so space is allocated for nmsg+1 messages.
//              If pmem, must have nmsg dwords but nmsg-1 are used.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc QFifo::IniV (lint* pmem, dword nmsg) {
    int mode = 0;
    if (nmsg == 0) nmsg = QF_VN;
    if (pmem) {
        mode = 1;
        m_fifo = (byte*) pmem;
        nmsg -= 1;
    }
    return( Init(mode, nmsg, sizeof(lint)) );
}

//*************************************************************************
//  QFifo::Put1:
//      First part of two calls needed to put a new item on the FIFO.
//      Returns pointer to where caller should write message.
//      Call Put2 when message has been written on FIFO to send it.
//
//  Arguments:
//      void* Put1 ()
//
//  Returns:
//      Pointer to where next message can be copied into the FIFO.
//      Returns NULL if the FIFO is full.
//*************************************************************************

isptr QFifo::Put1 () {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---

    dword index = m_iput + 1;                   //preview index after this put
    if (index >= m_nmsg) {                      //only to see if we have room
        index = 0;
    }
    if (index == m_iget) {
        #ifdef QF_LOCK
        OSLock_DONE(m_lock)
        #endif
        return(NULL);                           //fifo is full?
    }
    byte* pret = m_fifo + m_oput;               //return next head slot ptr

    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return(pret);
}

//*************************************************************************
//  QFifo::Put2:
//      Second part of two calls needed to put a new item on the FIFO.
//      Should have called Put1 first and copied into message slot.
//      Must not call if Put1 returned NULL!
//
//      Using two calls is convenient and also provides sequence fence,
//      WARNING: Do NOT use inline which may not be sequence-fence safe.
//
//  Arguments:
//      void Put2 ()
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid QFifo::Put2 () {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---

    m_oput += m_msiz;
    dword index = m_iput + 1;                   //advance index and offset
    if (index >= m_nmsg) {
        index = 0;
        m_oput = 0;
    }
    #ifndef QF_LOCK                             //if no mutex,
    M_FENCE;                                    //memory barrier is needed
    #endif
    m_iput = index;                             //commit change here

    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return;
}

//*************************************************************************
//  QFifo::PutV:
//      Puts a single 32-bit integer to the fifo.
//      Should have intialiazed with four-byte messages.
//      Put task may later check our m_extra member if Get task acknowedges.
//
//  Arguments:
//      errc PutV (lint val)
//      val     Value to send.
//
//  Returns:
//      Negative ECNOROOM if error.
//      Otherwise returns zero.
//*************************************************************************

isproc QFifo::PutV (lint val) {
    lint* pv = (lint*) Put1();
    if (pv == NULL) return(ECNOROOM);
    *pv = val;
    Put2();
    return(0);
}

//*************************************************************************
//  QFifo::Get1:
//      First part of two calls needed to get a new item from the FIFO.
//      Returns pointer to where caller should read message.
//      Call Get2 when message has copied out from FIFO to free its slot.
//
//  Arguments:
//      void* Get1 ()
//
//  Returns:
//      Pointer to where next message can be copied from the FIFO.
//      Returns NULL if the FIFO is empty.
//*************************************************************************

isptr QFifo::Get1 () {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---

    if (m_iget == m_iput) {
        #ifdef QF_LOCK
        OSLock_DONE(m_lock)
        #endif
        return(NULL);                           //fifo is empty?
    }
    byte* pret = m_fifo + m_oget;               //return next tail slot ptr

    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return(pret);                               //return next tail slot ptr
}

//*************************************************************************
//  QFifo::Get1X:
//      Works like Get1 but also returns number of used entries.
//      First part of two calls needed to get a new item from the FIFO.
//      Returns pointer to where caller should read message.
//      Call Get2 when message has copied out from FIFO to free its slot.
//
//  Arguments:
//      void* Get1X (lint* puse)
//      puse    Returns number of used entries before returned item.
//              Returns 0 if fifo was empty or 1 if fifo is now empty.
//
//  Returns:
//      Pointer to where next message can be copied from the FIFO.
//      Returns NULL if the FIFO is empty.
//*************************************************************************

isptr QFifo::Get1X (lint* puse) {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---

    if (m_iget == m_iput) {
        #ifdef QF_LOCK
        OSLock_DONE(m_lock)
        #endif
        *puse = 0;
        return(NULL);                           //fifo is empty?
    }
    *puse = (lint)m_iput - (lint)m_iget;
    if (*puse < 0) *puse += (lint)m_nmsg;       //return items used count
    byte* pret = m_fifo + m_oget;               //return next tail slot ptr

    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return(pret);                               //return next tail slot ptr
}

//*************************************************************************
//  QFifo::Get2:
//      Second part of two calls needed to put a new item on the FIFO.
//      Should have called Get1 first and copied into message slot.
//      Must not call if Get1 returned NULL.
//
//      Using two calls is convenient and also provides sequence fence,
//      WARNING: Do NOT use inline which may not be sequence-fence safe.
//
//  Arguments:
//      void Get2 ()
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid QFifo::Get2 () {
    #ifdef QF_LOCK                              //---
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    #endif                                      //---

    m_oget += m_msiz;
    dword index = m_iget + 1;                   //advance index and offset
    if (index >= m_nmsg) {
        index = 0;
        m_oget = 0;
    }
    #ifndef QF_LOCK                             //if no mutex,
    M_FENCE;                                    //memory barrier is needed
    #endif
    m_iget = index;                             //commit change here

    #ifdef QF_LOCK                              //---
    OSLock_DONE(m_lock);                        //=== Release optional mutex
    #endif                                      //---
    return;
}

//*************************************************************************
//  QFifo::GetV:
//      Gets a single 32-bit integer from the fifo.
//      Should have intialiazed with four-byte messages.
//      Caller may later set our m_extra member to acknowledge new value.
//      Does not signal empty-fifo error but instead returns m_extra.
//
//  Arguments:
//      errc GetV ()
//
//  Returns:
//      Returns next integer value from the fifo.
//      Returns current m_extra value if error.
//*************************************************************************

isproc QFifo::GetV () {
    lint* pv = (lint*) Get1();
    if (pv == NULL) return(m_extra);
    lint val = *pv;
    Get2();
    return(val);
}

//*************************************************************************
//  BFifo::BFifo:
//  BFifo::~BFifo:
//      Constructor and destructor.
//      A BFifo object can pass blocks from sender to receiver through a
//      FIFO (First In First Out) queue which is task safe.
//*************************************************************************

BFifo::BFifo () {
    m_fifo = NULL;
    m_bytes = 0;
    m_input = 0;
    m_output = 0;
    m_error = 0;
    m_lock = NULL;
    //Currently do not automatically allocate our fifo.
    //m_fifo = (dword*) OSAlloc(BF_SIZE);
    //m_bytes = BF_SIZE;
    //m_input = 0;
    //m_output = 0;
    //OSLock(OS_LOCKNEW, &m_lock);
    return;
}

BFifo::~BFifo () {
    if (m_fifo) OSFree(m_fifo);
    m_fifo = NULL;
    m_bytes = 0;
    if (m_lock) {
        OSLock(OS_LOCKOFF, &m_lock);
        m_lock = NULL;
    }
    return;
}

//*************************************************************************
//  BFifo::Set:
//      Allocates a different size and flushes any old data.
//      Must be called initially to allocate fifo.
//      Avoids reallocating same size if recalled.
//
//  Arguments:
//      errc Set (int mode, dword bytes)
//      mode    0 = Normal
//             +1 = Leave prexisting caller supplied m_fifo buffer pointer.
//                  +1 can be used for fixed location fifos.
//             +2 = Do not use m_lock mutex locking.
//                  +2 can be used for task safe one-way sending.
//             +4 = Use m_lock mutex if already exists. 
//                  +4 makes it safer to reallocate an in-use fifo.
//      bytes   New byte size which should be a multiple of 4.
//              Use BF_SIZE for default 16KB of space.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc BFifo::Set (int mode, dword bytes) {
    OSLock_GRAB(m_lock);                        //=== Mutex if reseting
    if ((!(mode & 1))||(m_fifo == NULL)) {
        if (m_fifo) {                           //existing fifo memory?
            if (m_bytes == bytes) {             //existing size is okay?
                m_error = 0;                    //avoid reallocating memory
                m_input = 0;
                m_output = 0;
                OSLock_DONE(m_lock);
                return(0);                
            }
            OSFree(m_fifo);
        }
        m_bytes = 0;
        m_fifo = (dword*) OSAlloc(bytes);
        if (m_fifo == NULL) {
            OSLock_DONE(m_lock);
            return(ECMEMORY);
        }
    }
    m_bytes = bytes;
    m_error = 0;
    m_input = 0;
    m_output = 0;
    OSLock_DONE(m_lock);                        //=== Release
    if ((!(mode & 2))&&(m_lock == NULL)) {      //allocate mutex if first time
        OSLock(OS_LOCKNEW, &m_lock);
    }
    return(0);
}

//*************************************************************************
//  BFifo::Put:
//      Puts a new block on the FIFO.
//      Can also empty the fifo.
//
//  Arguments:
//      errc Put (void* pblock, dword bytes)
//      pblock  Block of data to put on fifo
//      bytes   Byte size of pblock data.
//              Use pblock=NULL and bytes=-1 to empty fifo.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc BFifo::Put (void* pblock, dword bytes) {
    dword need, left, pads, ends;
    dword* pdw;

    if (m_fifo == NULL) return(ECNULL);         //no fifo memory (Set not done)?
    OSLock_GRAB(m_lock);                        //grab mutex

    if ((pblock == NULL)&&((signed)bytes == -1)) {
        m_error = 0;                            //empty fifo?
        m_input = 0;
        m_output = 0;
        OSLock_DONE(m_lock);
        return(0);
    }

    ends = m_output;
    left = m_input - ends;                      //sent bytes in fifo
    if ((signed)left < 0) left = (m_bytes - ends) + m_input;
    left = m_bytes - left;                      //free bytes left

    pads = bytes & 3;
    if (pads) pads = 4 - pads;                  //padding bytes to dword align
    need = bytes + pads + 4;                    //bytes we will need on fifo
    if (need > left) {
        OSLock_DONE(m_lock);
        return(ECNOROOM);
    }
    
    pdw = ADDOFF(dword*, m_fifo, m_input);
    ends = m_bytes - m_input;
    if ((signed)ends < 4) {                     //must wrap even for size dword?
        ends = m_bytes;
        pdw = m_fifo;
    }
    *pdw++ = bytes;
    ends = ends - 4;
    if ((signed)ends < (signed)bytes) {         //divide block to wrap around?
        OSMemCopy(pdw, pblock, ends);
        pblock = ADDOFF(void*, pblock, ends);
        bytes = bytes - ends;
        pdw = m_fifo;
    }
    OSMemCopy(pdw, pblock, bytes);              //copy data into fifo
    pdw = ADDOFF(dword*, pdw, (bytes+pads));
    m_input = PTROFF(m_fifo, pdw);              //update input to next dword

    OSLock_DONE(m_lock);                        //release mutex
    return(0);
}

//*************************************************************************
//  BFifo::Get:
//      Gets the next block of the FIFO.
//
//  Arguments:
//      errc Get (void* pblock, dword* pbytes)
//      pblock  Caller supplied buffer where block is copied.
//      pbytes  Returns byte size of returned block.
//
//  Returns:
//      Non-zero code if error and *pbytes set to 0 if error.
//*************************************************************************

isproc BFifo::Get (void* pblock, dword* pbytes) {
    dword* pdw;
    dword bytes, over, left, ends, pads;

    if (m_fifo == NULL) return(ECNULL);         //no fifo memory (Set not done)?
    OSLock_GRAB(m_lock);                        //grab mutex

    ends = m_input;
    left = ends - m_output;                     //sent bytes in fifo
    if ((signed)left < 0) left = (m_bytes - m_output) + ends;

    if (left <= 0) {                            //nothing left on fifo?
        *pbytes = 0;
        OSLock_DONE(m_lock);
        return(ECNOPTS);
    }
    pdw = ADDOFF(dword*, m_fifo, m_output);
    ends = m_bytes - m_output;
    if ((signed)ends < 4) {                     //must wrap even for size dword?
        ends = m_bytes;
        pdw = m_fifo;
    }
    bytes = *pdw++;
    ends = ends - 4;

    over = bytes;
    if ((signed)ends < (signed)bytes) {         //divided block wraps around?
        OSMemCopy(pblock, pdw, ends);
        pblock = ADDOFF(void*, pblock, ends);
        over = over - ends;
        pdw = m_fifo;
    }
    OSMemCopy(pblock, pdw, over);               //return block
    *pbytes = bytes;                            //return block size

    pads = bytes & 3;
    if (pads) pads = 4 - pads;                  //padding bytes to dword align
    pdw = ADDOFF(dword*, pdw, (over+pads));
    m_output = PTROFF(m_fifo, pdw);             //update output to next block

    OSLock_DONE(m_lock);                        //release mutex
    return(0);
}
#endif                                          //--- EXFIFOS

//*************************************************************************
//  PFifoInit:
//      Version of QFifo allows caller supplied buffer and assembly sender.
//      Intended to be implemented in PRU assemble to send to user space.
//      The PRU assembly must duplicate PFifoPut1 and PFifoPut2.
//      Must be called initially to allocate fifo.
//
//  Arguments:
//      errc PFifoInit (int mode, dword nmsg, dword nsiz, void* pbuf)
//      mode    Reserved for future use and must be zero.
//      nmsg    Maximum FIFO messages or 0 for 256 default.
//              Note that put must leave one unused message so
//              get one fewer messages is the maximum at a time.
//              This is unlike the PFifo class which allocates one more
//              because here caller has provided nmsg*msiz bytes in pbuf.
//      msiz    Byte size per message or 0 for 512x4 default.
//      pbuf    Caller supplied memory buffer.
//              Must be big enough for PFifo structure plus nmsg*msiz buf.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

isproc PFifoInit (int mode, dword nmsg, dword msiz, void* pbuf) {
    PFifo* pf = (PFifo*) pbuf;
    OSMemClear(pf, sizeof(PFifo));
    if (msiz == 0) msiz = QF_SIZE;
    if (nmsg == 0) nmsg = QF_DEEP-1;
    pf->m_nmsg = nmsg;                          //init empty state
    pf->m_msiz = msiz;
    pf->m_iput = 0;
    pf->m_iget = 0;
    return(0);
}

//*************************************************************************
//  PFifoPut1:
//      First part of two calls needed to put a new item on the FIFO.
//      Returns pointer to where caller should write message.
//      Call Put2 when message has been written on FIFO to send it.
//
//  Arguments:
//      void* PFifoPut1 (void* pbuf)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//
//  Returns:
//      Pointer to where next message can be copied into the FIFO.
//      Returns NULL if the FIFO is full.
//*************************************************************************

isptr PFifoPut1 (void* pbuf) {
    PFifo* pf = (PFifo*) pbuf;
    dword index = pf->m_iput + 1;               //preview index after this put
    if (index >= pf->m_nmsg) {                  //only to see if we have room
        index = 0;
    }
    if (index == pf->m_iget) {
        return(NULL);                           //fifo is full?
    }
    return(&pf->m_data[pf->m_iput * pf->m_msiz]);
}                                               //return next head slot ptr

//*************************************************************************
//  PFifoPut2:
//      Second part of two calls needed to put a new item on the FIFO.
//      Should have called Put1 first and copied into message slot.
//      Must not call if Put1 returned NULL!
//
//      Using two calls is convenient and also provides sequence fence,
//      WARNING: Do NOT use inline which may not be sequence-fence safe.
//
//  Arguments:
//      void PFifoPut2 (void* pbuf)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid PFifoPut2 (void* pbuf) {
    PFifo* pf = (PFifo*) pbuf;
    dword index = pf->m_iput + 1;               //advance index and offset
    if (index >= pf->m_nmsg) {
        index = 0;
    }
    M_FENCE;                                    //memory barrier
    pf->m_iput = index;                         //commit change here
    return;
}

//*************************************************************************
//  PFifoGet1:
//      First part of two calls needed to get a new item from the FIFO.
//      Returns pointer to where caller should read message.
//      Call Get2 when message has copied out from FIFO to free its slot.
//
//  Arguments:
//      void* PFifoGet1 (void* pbuf)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//
//  Returns:
//      Pointer to where next message can be copied from the FIFO.
//      Returns NULL if the FIFO is empty.
//*************************************************************************

isptr PFifoGet1 (void* pbuf) {
    PFifo* pf = (PFifo*) pbuf;
    if (pf->m_iget == pf->m_iput) {
        return(NULL);                           //fifo is empty?
    }
    return(&pf->m_data[pf->m_iget * pf->m_msiz]);
}                                               //return next tail slot ptr

//*************************************************************************
//  PFifoGet2:
//      Second part of two calls needed to put a new item on the FIFO.
//      Should have called Get1 first and copied into message slot.
//      Must not call if Get1 returned NULL.
//
//      Using two calls is convenient and also provides sequence fence,
//      WARNING: Do NOT use inline which may not be sequence-fence safe.
//
//  Arguments:
//      void PFifoGet2 (void* pbuf)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid PFifoGet2 (void* pbuf) {
    PFifo* pf = (PFifo*) pbuf;
    dword index = pf->m_iget + 1;               //advance index and offset
    if (index >= pf->m_nmsg) {
        index = 0;
    }
    M_FENCE;                                    //memory barrier
    pf->m_iget = index;                         //commit change here
    return;
}

//*************************************************************************
//  PFifoPut1X:
//      This works like PFifoPut2 but uses a separate m_iget value
//      which can be placed in a separate cache line to communicate
//      between different processors using cached memory. All put values
//      are in the pbuf strucuture written by the sending processor and
//      the only get values changed by the receiving processor is separate.
//      This is used receiving data from the TI PRU coprocessor.
//      This is not needed if caching is disabled on the DDR memory
//      which it normally is but we stil use this for safety.
//
//      NOTE: There is no PFifoPut2X and you must call PFifoPut2!
//
//      First part of two calls needed to put a new item on the FIFO.
//      Returns pointer to where caller should write message.
//      Call Put2 when message has been written on FIFO to send it.
//
//  Arguments:
//      void* PFifoPut1X (void* pbuf, dword* pget)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//
//  Returns:
//      Pointer to where next message can be copied into the FIFO.
//      Returns NULL if the FIFO is full.
//*************************************************************************

isptr PFifoPut1X (void* pbuf, dword* pget) {
    PFifo* pf = (PFifo*) pbuf;
    dword index = pf->m_iput + 1;               //preview index after this put
    if (index >= pf->m_nmsg) {                  //only to see if we have room
        index = 0;
    }
    if (index == (*pget)) {
        return(NULL);                           //fifo is full?
    }
    return(&pf->m_data[pf->m_iput * pf->m_msiz]);
}                                               //return next head slot ptr

//*************************************************************************
//  PFifoGet1X:
//      This works like PFifoGet1 but uses a separate m_iget value
//      which can be placed in a separate cache line to communicate
//      between different processors using cached memory. All put values
//      are in the pbuf strucuture written by the sending processor and
//      the only get values changed by the receiving processor is separate.
//      This is used receiving data from the TI PRU coprocessor.
//      This is not needed if caching is disabled on the DDR memory
//      which it normally is but we stil use this for safety.
//      Also unlike PFifoGet1 returns number of used entries.
//
//      First part of two calls needed to get a new item from the FIFO.
//      Returns pointer to where caller should read message.
//      Call Get2 when message has copied out from FIFO to free its slot.
//
//  Arguments:
//      void* PFifoGet1X (void* pbuf, dword* pget, lint* puse)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//      pget    Value used instead of pbuf->m_iget.
//              Must be zeroed when PFifoInit is called.
//      puse    Returns number of used entries before returned item.
//              Returns 0 if fifo was empty or 1 if fifo is now empty.
//
//  Returns:
//      Pointer to where next message can be copied from the FIFO.
//      Returns NULL if the FIFO is empty.
//*************************************************************************

isptr PFifoGet1X (void* pbuf, dword* pget, lint* puse) {
    PFifo* pf = (PFifo*) pbuf;
    if ((*pget) == pf->m_iput) {
        *puse = 0;
        return(NULL);                           //fifo is empty?
    }
    *puse = (lint)pf->m_iput - (lint)(*pget);
    if (*puse < 0) *puse += (lint)pf->m_nmsg;   //return items used count
    return(&pf->m_data[(*pget) * pf->m_msiz]);
}                                               //return next tail slot ptr

//*************************************************************************
//  PFifoGet2X:
//      This works like PFifoGet2 but uses a separate m_iget value
//      which can be placed in a separate cache line to communicate
//      between different processors using cached memory. All put values
//      are in the pbuf strucuture written by the sending processor and
//      the only get values changed by the receiving processor is separate.
//      This is used receiving data from the TI PRU coprocessor.
//      This is not needed if caching is disabled on the DDR memory
//      which it normally is but we stil use this for safety.
//
//      Second part of two calls needed to put a new item on the FIFO.
//      Should have called Get1 first and copied into message slot.
//      Must not call if Get1 returned NULL.
//
//      Using two calls is convenient and also provides sequence fence,
//      WARNING: Do NOT use inline which may not be sequence-fence safe.
//
//  Arguments:
//      void PFifoGet2 (void* pbuf, dword* pget)
//      pbuf    Caller supplied buffer used previously with PFifoInit.
//      pget    Value used instead of pbuf->m_iget.
//              Must be zeroed when PFifoInit is called.
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid PFifoGet2X (void* pbuf, dword* pget) {
    PFifo* pf = (PFifo*) pbuf;
    dword index = (*pget) + 1;                  //advance index and offset
    if (index >= pf->m_nmsg) {
        index = 0;
    }
    M_FENCE;                                    //memory barrier
    *pget = index;                              //commit change here
    return;
}

//*************************************************************************
//  TargetFile:
//      Allows a base name input to be converted to and from a full path.
//      Handles adding unique number to end as needed.
//      Handles placing files in Documents\Data or LinuxExe/Data
//
//  Arguments:
//      errc TargetFile (int mode, text* path, text* base, text* pdir, text* pext)
//      mode    0 = Convert base name +pdir to path     (TF_PATH)
//              1 = Convert path to base name + pdir    (TF_BASE)
//             +2 = Allow read without incrementing     (TF_READ)
//             +4 = Do not insist on unique name & num  (TF_OVER)
//                  Allow file to overwrite existing
//                  Otherwises adds incrementing number
//             +8 = Insist on \Data folder              (TF_DATA) 
//            +16 = Find last existing filename before  (TF_LAST)
//                  given base name for TF_PATH mode
//                  If not, finds 1st free at or after
//            +32 = Add to mode 0 to also update base   (TF_REDO)
//                  Returns both new path and new base
//      path    Pointer to full path name.
//              For mode 0 must be SZPATH buffer.
//              For mode 0 may be same as base source buffer.
//      base    Pointer to base name.
//              Can optionally be pathname source for mode 0.
//              For mode 1 or 32 must be SZDISK buffer.
//      pdir    Pointer to directory name.
//              For mode 1 must be SZPATH buffer.
//              If blank uses Documents/EXE Data subdir.
//              If NULL for mode 0 uses base's directory.
//      pext    Filename extension with beginning period.
//              If blank then TF_PATH fills in.
//              If NULL uses ".dat" default.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECEXISTS if cannot find free name.
//      But still returns valid name for overwriting.
//*************************************************************************

#define TF_TRY 10000                            //maximum files to try

isproc TargetFile (int mode, text* path, text* base, text* pdir, text* pext) {
    text name[SZPATH];
    text dir[SZPATH];
    text* ptxt;
    text* pend;
    lint ival;
    lint cntr;
    lint step;
    int retv;
    errc e;

    if (pext == NULL) pext = CF_EXT;            //use .dat default extension

    // Handle TF_BASE mode.
    if (mode & TF_BASE) {
        OSFileBase(base, path, OS_NOEXT);       //return base name w/o extension
        if (pdir) {
            OSFileDir(pdir, path);              //also directory path
            if ((mode & TF_DATA)||(*pdir == 0)) {
                ptxt = OSLocation(pdir, OS_LOCDOC);
                ptxt = OSTxtCopy(ptxt, TF_SUB); //force Data subdir?
                *ptxt++ = OS_SLASH;
                *ptxt = 0;
                OSDirNew(0, pdir);              //make sure Data dir exists
            }
        }
        if (*pext == 0) {
            OSFileExt(pext, path);              //ext if caller does not know
        }
        if (!(mode & TF_OVER)) {                //make sure not overwriting old file
            mode = TF_PATH + (mode & (~TF_BASE));
            e = TargetFile(mode, name, base, pdir, pext);
            if (e) return(e);                   //call ourself to update base name
            OSFileBase(base, name, OS_NOEXT);   //and return that
        }
        return(0);
    }

    // Handle TF_PATH mode.
    step = (mode & TF_LAST) ? (-1) : (+1);      //ending num increment or decrement
    retv = (mode & TF_LAST) ?  (0) : (-1);      //desired OSExists return

    if (pdir == NULL) {
        OSFileDir(dir, base);
        pdir = dir;
    }
    if ((mode & TF_DATA)||(*pdir == 0)) {
        ptxt = OSLocation(dir, OS_LOCDOC);      //My Documents or Linux exe dir
        ptxt = OSTxtCopy(ptxt, TF_SUB);         //Data subfilder
        *ptxt++ = OS_SLASH;
        *ptxt = 0;
        OSDirNew(0, dir);                       //make sure Data dir exists
        pdir = dir;
    }

    OSFileBase(name, base, OS_NOEXT);           //get rid of any directory or ext

    ptxt = name + (OSTxtSize(name) - 1);
    ival = 1;
    if (!(mode & TF_OVER)) {
        while ((ptxt > name)&&(*(ptxt-1) <= '9')&&(*(ptxt-1) >= '0')) {
            ptxt -= 1;                          //back up to ending number
            ival = FALSE;
        }
        if (ival == FALSE) {
            ival = OSIntGet(ptxt, NULL);        //get existing ending number
            if (ival <= 0) ival = 1;
        }
    }
    pend = ptxt;

    cntr = TF_TRY;
    while (TRUE) {                              //keep trying until find unused name
        ptxt = pend;                            //inc number rather than overwrite?
        if (!(mode & TF_OVER)) {
            if (ival < 100) *ptxt++ = '0';
            if (ival < 10)  *ptxt++ = '0';
            ptxt = OSIntPut(ival, ptxt);        //add 3+ digit number to end
        }
        OSTxtCopy(ptxt, pext);                  //add extension

        ptxt = OSTxtCopy(path, pdir);           //start with directory
        OSTxtCopy(ptxt, name);                  //add filename
        if ((--cntr) == 0) {
            e = ECEXISTS;
            break;                              //punt and overwrite after 10,000
        }
        if (  (mode & (TF_OVER+TF_READ))
            ||(OSExists(0, path) == retv)  ) {
            e = 0;
            break;                              //no file by that name already?
        }
        ival += step;                           //try next sequential end number
    }
    if (mode & TF_REDO) {                       //also return new base name?
        OSFileBase(base, name, OS_NOEXT);
    }
    return(e);
}

//*************************************************************************
//  CurrentDir:
//      Makes full path name if needed for relative names in current directory.
//      This is useful for console applications where a file name argument
//      is usually assumed to be in the current working directory.
//
//  Arguments:
//      void CurrentDir (int mode, text* ppath, text* pname)
//      mode    Reserved for future use and must be zero.
//      ppath   Buffer to receive output path name. Should be SZPATH.
//              can optionally be the same as the pname input.
//      pname   Input file name or full path name.
//              If relative non-path name then assumes in current directory.
//
//  Returns:
//      Nothing.
//*************************************************************************

isvoid CurrentDir (int mode, text* ppath, text* pname) {
    text name[SZPATH];

    OSTxtStop(name, pname, SZPATH);
    if ((name[0] == '/')||(name[0] == '\\')||(name[1] == ':')) {
        OSTxtCopy(ppath, name);                 //already absolute pathname?
        return;
    }
    pname = OSLocation(ppath, OS_CURDIR);       //get current working directory
    OSTxtCopy(pname, name);                     //and add relative name
    return;
}

//*************************************************************************
//  UserOutput:
//      Shows a text line to the user.
//      Automatically adds final linefeed.
//      Can optionally also write to log file.
//
//  Arguments:
//      void UserOutput (int mode, text* pline, text* pname, lint value)
//      mode    0 = Normal.
//              1 = Raw output without added line feed (USER_RAW)
//             +2 = Do not show if quiet mode          (USER_OPT)
//             +4 = Also write to log file             (USER_LOG)
//             +8 = Show value as decimal w/o ()       (USER_ALL) 
//                  Shows even if zero (not hex)
//             18 = Set quiet mode to value TRUE/FALSE (USER_SET)
//            +32 = Just return text in *pline buffer  (USER_RET)  
//              
//      pline   Text to show normally without ending \n. SZITEM 512 max.
//      pname   Optional name to show after line or NULL.
//      value   Optional [value] to show after line and name or 0.
//
//  Returns:
//      Nothing.
//*************************************************************************

#ifdef EUINPUT                                  //---
CSTYLE
isvoid UserOutput (int mode, text* pline, text* pname, lint value) {
    text line[SZITEM+SZDISK+SZNAME];
    text temp[SZITEM+SZDISK+SZNAME];
    text* pouts;
    text* pend;
    text* ptxt;

    if (mode == USER_SET) {
        cmd_quiet = (int) value;
        return;
    }
    if ((mode & USER_RET)&&(pline)) {
        OSTxtCopy(temp, pline);
        pouts = pline;
        pline = temp;
    }
    ptxt = OSTxtStop(line, pline, SZITEM);
    if (pname) {
        *ptxt++ = ' ';
        ptxt = OSTxtStop(ptxt, pname, SZDISK);
    }
    if ((value)||(mode & USER_ALL)) {
        *ptxt++ = ' ';
        pend = ptxt + SHOW_INT;
        if (mode & USER_ALL) {
            ptxt = OSIntPut(value, ptxt);
        } else {
            *ptxt++ = '(';
            ptxt = OSHexPut(value, ptxt);
            *ptxt++ = ')';
        }
        while (ptxt < pend) *ptxt++ = ' ';      //pad with spaces at end
    }                                           //so covers last even if smaller
    if (mode & USER_LOG) {
        *ptxt = 0;
        OSAddLog((0+8+256), NULL, line, 0, 0, NULL);
    }
    if (mode & USER_RET) {
        *ptxt = 0;
        OSTxtCopy(pouts, line);
        return;
    }
    if ((mode & USER_OPT)&&(cmd_quiet)) {
        return;
    }
    if (!(mode & USER_RAW)) *ptxt++ = '\n';
    *ptxt = 0;
    OSConLine(line);
    return;
}

//*************************************************************************
//  UserInput:
//      Allows a number or string to be entered from the console.
//      User may press Enter to keep old value.
//      Optionally allows values or -flag=values from prog's command line.
//      Optionally allows unformated arguments from prog's command line.
//
//      Note that the cmd_argv global may be set to comand line parameters
//      to use them instead of console input. Not used if cmd_argv is NULL.
//
//  Arguments:
//      errc UserInput (int mode, lint flag, text* prompt, int col, void* pdata)
//      mode    0 = Input a lint signed integer value.  (USER_INT)
//              1 = Input a flt floating value.         (USER_FLT)
//              2 = Input a text* text string.          (USER_TXT)
//              3 = Input a byte unsigned small value   (USER_BYT)
//              4 = Input a dword hexidecminal value    (USER_HEX)
//            +16 = Do not show current default value   (USER_NEW)
//            +32 = Ask for value if no -flag           (USER_ASK)
//            +64 = Allow flag to match -flag beginning (USER_BEG)
//             -1 = Init cmd_argv: argc=col argv=pdata  (USER_AV1)
//             -2 = Init cmd_argv when pdata is wtxt    (USER_AV2)
//                  The -1 and -2 modes automatically
//                  copy any command line strings that
//                  begin with a - flag character to
//                  cmd_flags for use with flag arg later.
//             
//      flag    Optional -FLAG letter or letters for named command line arg.
//              This must be the OSNameINT representation of 1 to 4 capital
//              option flag letters, lsb first, or 0 to skip -flag search.
//              Used to search for -f=value or -f:value commmand line arguments.
//              Returns matching value or returns 1 if just -flag w/o =value.
//              If no matching value is found for non-zero flag argument
//              then returns leaving caller's *pdata value unchanged
//              unless +32 USER_ASK is added to mode.
//      prompt  Prompt text to show.
//              Use NULL to never ask if command line does not have value.
//      col     Screen column number for entry.
//              Pads prompt with "....: " to align a series of prompts.
//              May be zero for no padding.
//              May be -1 to -N if just return unformated command line arg
//              where -1 is 1st arg, -2 is 2nd, etc.
//      pdata   Supplies old value and returns new.
//              Must be proper lint*, flt*, text* data type.
//              For USER_TXT must be at least SZDISK buffer.
//              May be NULL if prompt is NULL to just return value.
//
//  Returns:
//      Returns interger version of the value or 0 if error.
//      The returned value is the first character for a text string.
//      USER_AV? returns TRUE if -flags or no command-line args.
//      USER_AV? returns FALSE if unformatted command-line args.
//*************************************************************************

CSTYLE
isproc UserInput (int mode, lint flag, text* prompt, int col, void* pdata) {
    text bufr[SZDISK];
    text line[SZDISK];
    wtxt** pwarg;
    text** pcarg;
    text* pflags;
    text* ptxt;
    text* pend;
    text* pbeg;
    lint argf;
    lint mask;
    int smode;
    int isflg;
    int stop;
    int iout;
    
    // Handle remembering command line arguments for use later.

    if (mode < 0) {                             //USER_AV1 or USER_AV2?
        if (col >= MCARGS) col = MCARGS-1;      //only allow 31 args + NULL term
        pwarg = (wtxt**) pdata;
        pcarg = (text**) pdata;
        pflags = cmd_flags;
        ptxt = cmd_argsb;
        stop = 1;                               //exe is first argument
        iout = 0;
        isflg = (col > 1) ? FALSE : TRUE;
        while (stop < col) {
            pbeg = ptxt;
            cmd_pargs[iout++] = ptxt;           //must convert from wide chars
            if (mode == USER_AV1) {             //char text?
                ptxt = OSTxtCopy(ptxt, pcarg[stop]);
                ptxt += 1;                      //skip string ending zero
            } else {                            //wide text?
                ptxt = ptxt + OSWideUTF8(OS_2UTF8, pwarg[stop], ptxt,
                                         -1, (BLKSIZE/SZNAME));
            }
            if (*pbeg == '-') {                 //-flag arg?
                isflg = TRUE;
                pflags = OSTxtCopy(pflags, pbeg);
                pflags += 1;                    //move to cmd_flags buffer
                ptxt = pbeg;                    //and keep out of cmd_argsb
                iout -= 1;
            } else {
                pend = pbeg;                    //if unformatted mode
                while ((*pend)&&(*pend != '=')) pend += 1;
                if ((*pend == '=')&&(iout == 1)) {
                    *pend++ = 0;                //allow name=value as two args
                    cmd_pargs[iout++] = pend;   //first is just name
                }                               //second is just value
            }
            stop += 1;
        }
        *pflags++ = 0;                          //end with extra zeroes
        
        cmd_pargs[iout] = NULL;                 //need ending NULL
        cmd_argv = &cmd_pargs[0];                        
        return(isflg);
    }

    // Handle unformated command-line arguments.

    if (col < 0) {                              //just return command line arg?
        ptxt = cmd_pargs[(-col) - 1];           //-1 for first argument
        if (ptxt == NULL) return(0);            //past last argument?
        stop = 0;
        smode = mode & USER_TYP;                //get value type
        if        (smode == USER_TXT) {         //text data?
            OSTxtStop((text*)pdata, ptxt, SZDISK);
            stop = (int) *((text*)pdata);
        } else if (smode == USER_INT) {         //integer data?
            *((lint*)pdata) = OSIntGet(ptxt, NULL);
            stop = (int) *((lint*)pdata);
        } else if (smode == USER_HEX) {         //hex data?
            *((dword*)pdata) = OSHexGet(ptxt, NULL);
            stop = (int) *((dword*)pdata);
        } else if (smode == USER_BYT) {         //byte data?
            *((byte*)pdata) = (byte) OSIntGet(ptxt, NULL);
            stop = (int) *((byte*)pdata);
        } else if (smode == USER_FLT) {         //floating data?
            *((flt*)pdata)  = (flt) OSFloatGet(ptxt, NULL);
            stop = (int) *((flt*)pdata);
        }
        return(stop);
    }

    // Handle prompts for entering data 
    // which can be overridden by command-line -flag=value.

    isflg = FALSE;
    if (flag) {                                 //search for matching -flag?
        mask = 0xFFFFFFFF;
        if (mode & USER_BEG) {                  //allow -flag beginning match?
            mask = 0xFF;
            if (flag & (~mask)) mask = 0xFFFF;  //flag is 1, 2, or 3 letters?
            if (flag & (~mask)) mask = 0xFFFFFF;
            if (flag & (~mask)) mask = 0xFFFFFFFF;
        }
        pflags = cmd_flags;
        while (*pflags) {                       //search through -flag options
            while (*pflags == ' ') pflags += 1;
            if (*pflags == '-') pflags += 1;
            argf = OSNameInt(pflags, &pflags);  //get -flag value
            if ((argf & mask) == flag) {        //matches what caller wants?
                if (*pflags) pflags += 1;       //skip over = or : separator
                while (*pflags == ' ') pflags += 1;
                OSTxtStop(line, pflags, SZDISK);//copy value
                if (line[0] == 0) {             //no value?
                    line[0] = '1';              //return 1 for -flag
                    line[1] = 0;                //without value
                }
                isflg = TRUE;                   //skip input below
                break;
            }
            while (*pflags) pflags += 1;        //skip rest of value
            pflags += 1;                        //skip value's ending zero
        }                                       //two zeros ends all -flags
        if ((isflg == FALSE)&&(!(mode & USER_ASK))) {
            return(0);                          //no matching -flag found?
        }                                       //leave caller's existing value
    }

    // Now use NULL prompt to stop from asking if not in command line.
    // if (prompt == NULL) prompt = "Enter value";

    smode = mode & USER_TYP;                    //get value type
    if (prompt) {
        ptxt = OSTxtCopy(bufr, prompt);

        if (!(mode & USER_NEW)) {               //show old default value?
            ptxt = OSTxtCopy(ptxt, " [");
            if        (smode == USER_TXT) {     //text data?
                stop = (col) ? (col - PTROFF(bufr, ptxt)) : EMAXI;
                ptxt = OSTxtStop(ptxt, (text*)pdata, stop);
            } else if (smode == USER_INT) {     //integer data?
                ptxt = OSIntPut(*((lint*)pdata), ptxt);
            } else if (smode == USER_HEX) {     //hex data?
                ptxt = OSHexPut(*((dword*)pdata), ptxt);
            } else if (smode == USER_BYT) {     //byte data?
                ptxt = OSIntPut(*((byte*)pdata), ptxt);
            } else if (smode == USER_FLT) {     //floating data?
                ptxt = OSFloatPut(*((flt*)pdata), ptxt, 0, 0);
            }
            ptxt = OSTxtCopy(ptxt, "] ");
        }
        prompt = bufr;

        pend = (col) ? ADDOFF(text*, bufr, col) : NULL;
        if (pend) {
            pend = pend - 2;
            while (ptxt < pend) *ptxt++ = '.';  //padd with dots
            ptxt = pend;
        }
        ptxt = OSTxtCopy(ptxt, ": ");
    }

    if (isflg == FALSE) {                       //don't ask if have -flag=value
        line[0] = 0;                            //just in case
        OSInputArg(prompt, line, &cmd_argv);    //present prompt and get line
    }
    ptxt = line;
    while (*ptxt == ' ') ptxt += 1;             //ignore blanks before
    if (*ptxt == 0) {
        return(0);                              //no input keeps old value
    }

    if (pdata == NULL) pdata = bufr;            //just return value?

    stop = 0;
    if        (smode == USER_TXT) {             //text data?
        OSTxtStop((text*)pdata, ptxt, SZDISK);
        stop = (int) *((text*)pdata);
    } else if (smode == USER_INT) {             //integer data?
        *((lint*)pdata) = OSIntGet(ptxt, NULL);
        stop = (int) *((lint*)pdata);
    } else if (smode == USER_HEX) {             //hex data?
        *((dword*)pdata) = OSHexGet(ptxt, NULL);
        stop = (int) *((dword*)pdata);
    } else if (smode == USER_BYT) {             //byte data?
        *((byte*)pdata) = (byte) OSIntGet(ptxt, NULL);
        stop = (int) *((byte*)pdata);
    } else if (smode == USER_FLT) {             //floating data?
        *((flt*)pdata)  = (flt) OSFloatGet(ptxt, NULL);
        stop = (int) *((flt*)pdata);
    }
    return(stop);
}
#endif                                          //---


//*************************************************************************
//  TextReg:
//      Encrypts and Decrypts binary values to and from registration text.
//      Can also encode and docode text from binary without encryption.
//
//  Arguments:
//      errc TextReg (int mode, dword key, text* preg, byte* pbin, int* bits)
//      mode    1 = Decrypt text to binary bytes        (TR_2BIN)
//              2 = Encrypt text from binary bytes      (TR_2TXT)
//            +16 = Make first text section bigger      (TR_ADD1)
//            +32 = Put dash every three chars          (TR_SEP3)
//            +64 = Put dash every four chars           (TR_SEP4)
//            +96 = Put dash every five chars           (TR_SEP5)
//              TR_ADD1-TR_SEP5 ignored for TR_2BIN
//      key     Key for encryption and decryption.
//              May be zero to encode or decode without encryption.
//      preg    Supplies or returns registration zero terminated text.
//              For mode 1 may not exceed TR_MAXS=511 chars with term.
//              For mode 2 must supply large enough buffer.
//      pbin    Supplies or returns binary values.
//              Last byte is padded with high zeroes of not even bytes.
//      bits    Number of bits in pbin values for mode 2.
//              Max pbin bits output for mode 1 (may not exceed 256*8).
//
//  Returns:
//      Mode 1 returns number of bytes output.
//      Mode 2 returns number of text characters of output.
//      Returns negative code if error.
//      Returns ECARGUMENT if nbin is wrong.
//      Returns ECNOROOM if bits is not big enough.
//*************************************************************************

#define TR_MAXB (64*4)                          //max bytes we can handle
#define TR_MAXS 512                             //max characters to decrypt

#define TR_BRKD 3                               //value above omitted D
#define TR_BRKI 7                               //value above omitted I
#define TR_BRKO 12                              //value above omitted O
#define TR_BRKS 15                              //value above omitted S
#define TR_BRKZ 22                              //value above Z (0 is next)
#define TR_BRK9 32                              //value above 9

CSTYLE
isproc TextReg (int mode, dword key, text* preg, byte* pbin, int bits) {
    text regs[TR_MAXS];
    byte data[TR_MAXB];
    byte* pdat;
    text* pout;
    int code, ival, nbin, have, left, need, used, todo, shft, mask;
    int nsec, dash, done;

    nbin = (bits + 7) >> 3;                     //bytes

    if (mode & TR_2TXT) {                       //TR_2TXT encrypt to text?
        if ((nbin <= 0)||(nbin > TR_MAXB)) return(ECARGUMENT);
        OSMemCopy(data, pbin, nbin);            //get copy of binary data
        if (key) {                              //encrypt to text?
            OSCipher(OS_ENBYTES+32, &key, data, nbin);
        }
        nsec = (mode & TR_SEP5) >> TR_SFT3;
        if (nsec) nsec += 2;                    //chars per dash & 1st dash cnt
        dash = (mode & TR_ADD1) ? (nsec+1) : nsec;

        pout = preg;
        pdat = data;
        need = bits;
        left = 0;                               //bits left in ival byte
        have = 0;                               //bits have in code
        shft = 0;                               //left shift to place in output
        ival = 0;                               //input bytes
        code = 0;                               //output five bit codes

        while ((bits > 0)||(left >= 5)) {       //encode bytes to text
            if (left == 0) {                    //get next byte?
                left = 8;                       //eight bits available in ival
                ival = *pdat++;
                bits -= 8;
            }            
            if (have < 5) {                     //five bits per char code
                used = 5 - have;                //bits to be used
                if (used > left) {              //not enough left in code?
                    used = left;                //just use what we have
                }

                mask = 0;
                todo = used;
                do {
                    mask = (mask << 1) | 1;     //build mask for used bits
                } while (--todo);

                code |= (ival & mask) << shft;  //fill in more bits
                ival = ival >> used;            //less bits available from byte
                left = left - used;
                have = have + used;
                shft += used;
            }
            if (have == 5) {                    //enough bits for character?
                if        (code < TR_BRKD) {    //encode five bits to character
                    *pout++ = 'A' + code;       //A-Z encodes 0-22 w/o D,I,O,S
                } else if (code < TR_BRKI) {
                    *pout++ = 'E' + (code - TR_BRKD);
                } else if (code < TR_BRKO) {
                    *pout++ = 'J' + (code - TR_BRKI);
                } else if (code < TR_BRKS) {
                    *pout++ = 'P' + (code - TR_BRKO);
                } else if (code < TR_BRKZ) {
                    *pout++ = 'T' + (code - TR_BRKS);
                } else {                        //0-9 encodes 22-31
                    *pout++ = '0' + (code - TR_BRKZ);
                }
                need -= 5;
                if (need <= 0) {
                    break;                      //finished all desired bits?
                }
                have = 0;                       //used up the code
                shft = 0;
                code = 0;

                if ((dash > 0)&&(bits > 0)) {   //output section dashes?
                    dash -= 1;
                    if (dash == 0) {            //time for next dash?
                        *pout++ = '-';
                        dash = nsec;
                    }
                }
            }
        }
        *pout = 0;                              //zero terminate
        return (pout - preg);                   //return chars of output

    } else {                                    //TR_2BIN decrypt to binary?
        pout = regs;
        while (*preg) {                         //remove whitespace chars
            if ((*preg != '-')&&(((unsigned)*preg) > ' ')) {
                *pout++ = *preg;
                if ((pout - regs) >= (TR_MAXS-1)) {
                    break;                      //don't overflow buffer
                }
            }
            preg += 1;
        }
        *pout = 0;                              //zero term cleaned up string
        preg = regs;                            //use cleaned up input

        if (nbin > TR_MAXB) nbin = TR_MAXB;
        pdat = data;
        left = 0;                               //bits left in code char
        have = 0;                               //bits have in ival bytes
        shft = 0;                               //left shift to place in output
        code = 0;                               //input five-bit character
        ival = 0;                               //output byte
        done = 0;                               //bytes done

        while ((*preg)||(left)) {
            if (left == 0) {
                code = (int) *preg++;           //get next char
                code = CAPITAL(code);           //convert char to binary

                if      (code=='D') code = '0'; //easily confused letters
                else if (code=='O') code = '0'; //treated as numeral
                else if (code=='I') code = '1'; //they look like
                else if (code=='S') code = '5';

                if      ((code>='A')&&(code< 'D')) code = (code-'A');
                else if ((code>='E')&&(code< 'I')) code = (code-'E')+TR_BRKD;
                else if ((code>='J')&&(code< 'O')) code = (code-'J')+TR_BRKI;
                else if ((code>='P')&&(code< 'S')) code = (code-'P')+TR_BRKO;
                else if ((code>='T')&&(code<='Z')) code = (code-'T')+TR_BRKS;
                else if ((code>='0')&&(code<='9')) code = (code-'0')+TR_BRKZ;
                else continue;                  //skip non-coded characters

                left = 5;                       //each char encodes five bits
            }
            if (have < 8) {                     //need bits to get to byte?
                used = 8 - have;                //bits to be used
                if (used > left) {              //not enough bits in code?
                    used = left;                //just use what we have
                }

                mask = 0;
                todo = used;
                do {
                    mask = (mask << 1) | 1;     //build mask for used bits
                } while (--todo);

                ival |= (code & mask) << shft;  //fill in more bits
                code = code >> used;            //less bits available from byte
                left = left - used;
                have = have + used;
                shft += used;
            }
            if ((have == 8)||(*preg == 0)) {    //enough bits for byte?
                done += 1;
                if (done > nbin) return(ECNOROOM);
                *pdat++ = (byte) ival;
                have = 0;
                shft = 0;
                ival = 0;
            }
        }
        if (key) {                              //decrypt from text?
            OSCipher(OS_ENCRYPT+32+256, &key, data, done);
        }
        left = (nbin*8) - bits;
        if (left > 0) {                         //extra bits in last byte?
            todo = 8 - left;                    //used bits in last byte
            mask = 0;
            do {
                mask = (mask << 1) | 1;         //build mask for used bits
            } while (--todo);
            data[nbin-1] &= mask;               //zero unused bits at end
        }
        OSMemCopy(pbin, data, done);
        return(done);
    }
}

//*************************************************************************
//  TextMix:
//      Obfuscates a text string or hex number so cannot be searched.
//      Call a second time with same key to unscramble.
//      Unscrambled text is normally all upper case but otherwise the same.
//      Can preserve lower case through scrambling by using -1 as the key.
//      However lower case is less obfuscated than upper case.
//      Scrambles hex digits (0-1,A-F) differently than other letters.
//      One weakness is that it keeps 0-F, G-V, W-Z separated so
//      they stay in the same category when scrambled.
//
//  Arguments:
//      errc TextMix (int key, text* ptxt)
//      key     Obfuscation key 0x1-0xA or 0 for 0xA default.
//              Negative preserves lower case and uses -key.
//              Use -10 to preserve lower case and use 0xA.
//      ptxt    Text is scrambled or restored in place.
//
//  Returns:
//      Size of text processed text string.
//      Currently never returns negative error code.
//*************************************************************************

CSTYLE
isproc TextMix (int key, text* ptxt) {
    text* pold;
    text hex, cc;
    int upper;

    upper = TRUE;
    if (key < 0) {
        key = -key;
        upper = FALSE;
    }
    if (key == 0) key = 0xA;
    pold = ptxt;
    while (*ptxt) {
        cc = *ptxt;
        if (((unsigned)cc) < ' ') {
            cc = ' ';                           //switch CR,LF etc to blank
        }
        if (upper) {
            cc = CAPITAL(cc);                   //normally force upper case

        } else if ((cc >= 'a')&&(cc <= 'z')) {  //handling lower case?
            if (cc <= 'p') {                    //scramble first 16
                cc = cc - 'a';
                cc ^= (text) (key & 0xF);
                key += 5;
                cc = cc + 'a';
            } else if (cc <= 'x') {             //scramble next 8
                cc = cc - 'q';
                cc ^= (text) (key & 0x3);
                key += 5;
                cc = cc + 'q';
            } else {
                cc = cc - 'y';                  //scramble last 2
                cc ^= (text) (key & 0x1);
                key += 5;
                cc = cc + 'y';
            }
            *ptxt++ = cc;
            continue;        
        }

        if        ((cc >= 'A')&&(cc <= 'F')) {  //handle upper case and digits
            hex = 0xA + (cc - 'A');             //hex digit?
            hex ^= (text) (key & 0xF);          //scramble hex digit with key
            key += 5;                           //change key through string
            hex += (hex < 0xA) ? '0' : ('A'-10);
            cc = hex;
        } else if ((cc >= '0')&&(cc <= '9')) {
            hex = cc - '0';
            hex ^= (text) (key & 0xF);          //scramble same as A-F
            key += 5;
            hex += (hex < 0xA) ? '0' : ('A'-10);
            cc = hex;
        } else if ((cc >= 'G')&&(cc <= 'V')) {  //handle upper case afer A-F
            cc = cc - 'G';
            cc ^= (text) (key & 0xF);           //scramble 16 letters G-V
            key += 5;
            cc = cc + 'G';
        } else if ((cc >= 'W')&&(cc <= 'Z')) {
            cc = cc - 'W';
            cc ^= (text) (key & 0x3);           //scramble 4 letters W-Z
            key += 5;
            cc = cc + 'W';
        } else if (cc == ' ') {
            cc = '#';                           //scramble common punctuation
        } else if (cc == '#') {                 //especially so space and dash
            cc = ' ';                           //and period don't show easily
        } else if (cc == '-') {
            cc = '$';
        } else if (cc == '$') {
            cc = '-';
        } else if (cc == '.') {
            cc = '^';
        } else if (cc == '^') {
            cc = '.';
        }
        *ptxt++ = cc;
    }
    return(ptxt-pold);                          //return length
}

//*************************************************************************
//  BinPack:
//      Packs 1 to 32 bit integers into consecutive bytes.
//
//  Arguments:
//      errc BinPack (int mode, dword* ppack, dword* pnums, int* pbits)
//      mode    Reserved for future use and must be zero.
//      ppack   Output buffer (must be large enough).
//      pnums   Input integers to be packed lsb first and low index first.
//      pbits   Array of bit field sizes in output, 1-32, with 0 terminator.
//
//  Returns:
//      Number of dwords in ppack output.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
isproc BinPack (int mode, dword* ppack, dword* pnums, int* pbits) {
    int left, have, used, shft, bits, mask, todo, done;
    dword ival, iout;

    left = 0;                                   //bits left in ival dword
    have = 0;                                   //bits have in iout dword
    shft = 0;                                   //left shift to place in output
    ival = 0;                                   //input values
    iout = 0;                                   //output dwords
    done = 0;

    while (TRUE) {
        if (left == 0) {                        //get next value?
            ival = *pnums++;
            bits = *pbits++;
            if (bits  <= 0) {                   //end of list?
                break;
            }
            left = bits;                        //bits available in ival
        }            
        if (have < 32) {                        //need 32 bits to output
            used = 32 - have;                   //bits to be used
            if (used > left) {                  //not enough left in code?
                used = left;                    //just use what we have
            }

            if (used < 32) {
                mask = 0;
                todo = used;
                do {
                    mask = (mask << 1) | 1;     //build mask for used bits
                } while (--todo);

                iout |= (ival & mask) << shft;  //fill in more bits
                ival = ival >> used;            //less bits available from ival
                left = left - used;
                have = have + used;
                shft += used;
            } else {
                iout = ival;
                ival = 0;
                left = 0;
                have = 32;
            }
        }
        if (have == 32) {                       //enough bits for output dword?
            *ppack++ = iout;
            done += 1;
            have = 0;
            shft = 0;
            iout = 0;
        }
    }
    if (have > 0) {                             //write rest of last dword?
        *ppack++ = iout;
        done += 1;
    }
    return(done);
}

//*************************************************************************
//  Registry:
//      Sets or deletes or reads a registry key text value.
//      This ONLY works with Windows.
//
//  Arguments:
//      errc Registry (lint mode, text* pkey, text* pname, text* pvalue)
//      mode    REG_CLASS = Use HKEY_CLASSES_ROOT
//              REG_USER  = Use HKEY_CURRENT_USER
//              REG_LOCAL = Use HKEY_LOCAL_MACHINE
//              We currently do not handle the others because there is no need.
//             +REG_GET = Add in to mode to get rather than set the value.
//                        The text value is returned in *pvalue which
//                        must be a buffer at least SZPATH characters
//                        (or SZDISK characters id REG_SZDISK added to mode).
//                        A blank string (zero only) is returned if error.
//             +REG_DEL = Add in to mode to delete the key's value.
//                        To delete whole key with all values use pname=NULL.
//                        Otherwise deletes only the given pname value for key.
//                        Ignores the pvalue argument which should be NULL.
//             +REG_PUT = Set the value (default zero mode).
//              If neither +REG_GET or +REG_DEL is added then we put the value.
//              If desired +REG_PUT can be added in for this but is always zero.
//             +REG_SZDISK = Add to REG_GET if pvalue buffer only SZDISK chars.
//             +REG_SZITEM = Add to REG_GET if pvalue buffer only SZITEM chars.
//             +REG_SZSIZE = Add to REG_GET to just return the required size. 
//             +REG_SZHUGE = Add to REG_GET if buffer guaranteed big enough. 
//             +REG_GOREAL = Add in to force using real register not FxFoto.ini. 
//              Note that if both REG_SZITEM and REG_SZDISK then uses SZITEM.
//      pkey    Name of subkey under root (eg "Software\Triscape\MapX").
//      pname   Name of value within the key to be set.
//              May be NULL to specify default value for key.
//              May be NULL for mode REG_DEL delete to remove whole key.
//      pvalue  Text value to be assocated with key and name.
//              Ignored for mode REG_DEL delete (and should be NULL).
//
//  Returns:
//      Non-zero code if error.
//      For mode REG_GET+REG_SZSIZE returns required by size (with zero term).
//*************************************************************************

CSTYLE
isproc Registry (lint mode, text* pkey, text* pname, text* pvalue) {
    #ifndef ISWIN                               //--- Linux
    return(ECCANNOT);
    #else                                       //--- Windows
    HKEY root;
    HKEY key;
    lint rmode;
    dword cnt;
    long fail;

    rmode = mode & REG_ROOT;
    if      (rmode == REG_CLASS) root = HKEY_CLASSES_ROOT;
    else if (rmode == REG_USER)  root = HKEY_CURRENT_USER;
    else if (rmode == REG_LOCAL) root = HKEY_LOCAL_MACHINE;
    else return(ECARGUMENT);

    if (mode & REG_DEL) {                       //delete key?
        if (pname) {                            //delete one value?
            fail = RegOpenKeyA(root, pkey, &key);
            if (fail) return(ECSYS);
            fail = RegDeleteValueA(key, pname);
            RegCloseKey(key);
            if (fail) return(ECSYS);

        } else {                                //delete whole key?
            fail = RegDeleteKeyA(root, pkey);
            if (fail) return(ECSYS);
        }

    } else if (mode & REG_GET) {                //get existing value for key?
        fail = RegOpenKeyA(root, pkey, &key);
        if (fail) return(ECSYS);

        cnt = SZPATH;
        if      (mode & REG_SZITEM) cnt = SZITEM;
        else if (mode & REG_SZDISK) cnt = SZDISK;
        if (mode & (REG_SZSIZE+REG_SZHUGE)) {
            cnt = 0;
            fail = RegQueryValueExA(key, pname, NULL, NULL, NULL, &cnt);
            if (fail) return(ECSYS);            //get size of value first
            if (mode & REG_SZSIZE) return((cnt+1)*SZCHR);                
        }

        *pvalue = 0;
        fail = RegQueryValueExA(key, pname, NULL, NULL, (fbyte*)pvalue, &cnt);
        RegCloseKey(key);
        if (fail) return(ECSYS);

    } else {                                    //set new value for key?
        fail = RegCreateKeyA(root, pkey, &key);
        if (fail) return(ECSYS);

        cnt = OSTxtSize(pvalue) - 1;
        fail = RegSetValueExA(key, pname, NULL, REG_SZ, (fbyte*)pvalue, cnt);
        RegCloseKey(key);
        if (fail) return(ECSYS);
    }
    return(0);                                  //all done
    #endif                                      //---
}

//*************************************************************************
//  RegInfo:
//      Uses Windows Registry to read and write app information.
//      Also backs up information to a disk file and reads if needed.
//      Automatically obfuscates text with TextMix unless +REG_NOMIX mode.
//
//  Arguments:
//      int RegInfo (int mode, text* file, text* key, text* name, text* value)
//      mode    Same as Registry function mode argument.
//             0        = Use REG_BOTH to save both REG_LOCAL and REG_USER.
//             1,2,3    = Use REG_CLASS, REG_USER, REG_LOCAL in registry.
//             +REG_GET = Add in to mode to get rather than set the value.
//                        The text value is returned in *pvalue.
//                        A blank string (zero only) is returned if error.
//             +REG_PUT = Set the value (default zero mode).
//                        The text value should not exceed SZDISK chars.
//             +REG_NOMIX Store and retrieve clear text without TextMix. 
//                        Otherwise autoamtically obfuscates storage.
//      file    Path and filename of backup file (ends with .ini by convention).
//              For example, "C:\ProgramData\Triscape\Reg.ini"
//      key     Name of subkey under root (eg "Software\Triscape").
//              Uses only the last part in INI file (e.g. "Triscape").
//      name    Name of value within the key to be set.
//      value   Text value to be assocated with key and name to put.
//              SZDISK buffer to receive text value if REG_GET.
//                  
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE
isproc RegInfo (int mode, text* pfile, text* pkey, text* pname, text* pvalue) {
    text* ptxt;
    IReg ini;
    int rv, user, where;

    where = mode & REG_ROOT;                    //where in registry root
    user = 0;
    if (where == REG_BOTH) {                    //both => try local first
        user = mode | REG_USER;                 //and try user second
        mode |= REG_LOCAL;
    }
    if (mode & REG_GET) {
        pvalue[0] = 0;                          //try reading from registry
        mode |= REG_SZDISK;                     //must be this size for ini
        rv = Registry(mode, pkey, pname, pvalue);
        if ((pvalue[0] == 0)&&(user)) {
            user |= REG_SZDISK;
            rv = Registry(user, pkey, pname, pvalue);
        }                                       //try user second if both
        if ((pvalue[0] == 0)&&(pfile)) {        //still missing in registry?
            OSMemClear(&ini, sizeof(ini));      //try reading from file
            rv = OSRegIni(CREG_LOAD, &ini, NULL, NULL, pfile);
            if (rv == 0) {
                ptxt = pkey;                    //use only end of registry path
                while (*ptxt) {
                    if (*ptxt == '\\') break;
                    ptxt += 1;
                }
                ptxt = (*ptxt == '\\') ? (ptxt+1) : pkey;
                rv = OSRegIni(CREG_GET, &ini, ptxt, pname, pvalue);
                OSRegIni(CREG_FREE, &ini, NULL, NULL, NULL);
            }
        }
        if (!(mode & REG_NOMIX)) {
            TextMix(-10, pvalue);               //unscramble
        }
        return(rv);

    } else {                                    //REG_PUT
        if (!(mode & REG_NOMIX)) {
            TextMix(-10, pvalue);               //obfuscate
        }
        mode |= REG_SZDISK;                     //must be this size for ini
        if (pfile) {                            //backup write to file
            OSMemClear(&ini, sizeof(ini));
            ptxt = pkey;                        //no error if no ini file yet
            while (*ptxt) {                     //use only end of registry path
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
            }
            OSRegIni(CREG_FREE, &ini, NULL, NULL, NULL);
        }
        rv = Registry(mode, pkey, pname, pvalue);
        if (user) {                             //also save to registry
            rv = Registry(user, pkey, pname, pvalue);
        }                                       //save to both local and user?
        return(rv);
    }
}

//*************************************************************************
//  RegSum:
//      Performs a text checksum ignoring capitals, spaces and punctuation.
//
//  Arguments:
//      dword RegSum (int mode, text* ptxt)
//      mode    Reserved for future use and must be zero.
//      ptxt    Text string to be check summed without case etc.
//              Should be 256 or fewer characters (ignores rest).
//
//  Returns:
//      Checksum of the ptxt name.
//*************************************************************************

CSTYLE
dword RegSum (int mode, text* ptxt) {
    byte name[260];
    byte* puse;
    byte cc;
    int cnt;

    cnt = 256;
    puse = name;
    while (*ptxt) {
        cc = *ptxt++;
        cc = CAPITAL(cc);                       //capitals only
        if (  ((cc >= 'A')&&(cc <= 'Z'))
            ||((cc >= '0')&&(cc <= '9'))        //and numerals
            ||(cc >= 0x80)  ) {                 //and unicode stuff
            *puse++ = cc;                       //keep only letters and numerals
            if (--cnt <= 0) break;
        }
    }
    *puse = 0;                                  //just in case
    cnt = puse - name;
    return( Get_CRC(name, (puse - name)) );     //and checksum
}

//*************************************************************************
//  RegCPU:
//      Returns the CPUID type, family, model, stepping id information.
//      Can combine CPU ID with memory size and volume lable for computer ID.
//
//  Arguments:
//      dword RegCPU (int mode)
//      mode    0 = Just get CPUID number.
//              3 = Get ComputerID DiskVolumeSerialNum+CPUID+MemorySize.
//
//  Returns:
//      CPUID Version Info or 1,2,3,4 for Non-Intel,386,486,586 w/o CPUID.
//*************************************************************************

//#define DEB_REG
#ifdef DEB_REG                                  //=== SPECIAL DEBUG FAKE CPU
int deb_cpu = -1;                               //allows chaning CPU/MAC ID
int deb_mac = -1;
#endif                                          //===

CSTYLE
dword RegCPU (int mode) {
    if (mode == 3) {                            //get computer id?
        #ifndef ISWIN                           //--- Linux
        return(0);
        #else                                   //--- Windows
        text disk[SZDISK];
        text bufr[SZDISK];
        dword len, index, sern;
        QDW siz;

        sern = 0;
        GetWindowsDirectoryA(bufr, SZDISK);     //get windows directory
        OSFileRoot(disk, bufr);                 //get windows disk name
        GetVolumeInformationA(disk,bufr,SZDISK,&sern,&len,&index,NULL,0);
        siz.qw = 0;                             //just in case
        siz.qw = OSMemory(0);                   //total bytes of physical memory
        sern ^= siz.dw.lo;
        sern ^= siz.dw.hi;
        sern ^= RegCPU(0);                      //and CPUID type,family,model
        if ((unsigned)sern < 2) sern = 2;       //do not allow reserved values
        #ifdef DEB_REG                          //=== SPECIAL DEBUG FAKE CPU
        if (deb_cpu == -1) {                    //first time after loaded?
            deb_cpu = deb_mac = 0;              //do no harm if fails
            aint file = OSOpen(OS_READ, "C:\\Temp\\FakeRID.txt");
            char buff[SZNAME];
            buff[0] = 0;
            OSRead(file, buff, SZNAME);
            char* preg = buff;
            deb_cpu = OSNumGet(preg, &preg);    //read cpu and mac increments
            if (*preg == ',') preg += 1;        //from FakeRID.txt file
            deb_mac = OSNumGet(preg, &preg);    //to be added to computer's IDs
            OSClose(file);
            printf("@@ RegCPU Temp\\FakeRID.txt CPU+ %i and MAC+ %i Debug ID\n", deb_cpu, deb_mac);
        }
        sern += deb_cpu;                        //change to fake CPU ID
        #endif                                  //===
        return(sern);
        #endif                                  //---
    }

    #ifndef ISWIN                               //---
    return(1);
    #else                                       //---
    #ifdef NO86 //-------
    return(1);
    #else       //-------
    dword rval;

    ASM {                               ;optimizing compiler bug causes EBX to
        push    ebx                     ;not be preserved probably due to CPUID
        pushfd
        pop     eax                     ;get original EFLAGS in eax
        mov     edx,eax                 ;save original EFLAGS
        xor     eax,240000h             ;flip ID and AC bits in EFLAGS
        push    eax
        popfd                           ;copy to EFLAGS
        pushfd
        pop     eax                     ;get new EFLAGS value
        push    edx
        popfd                           ;restore original EFLAGS
        xor     edx,eax
        mov     ecx,2
        test    edx,040000h
        jz      havcpu                  ;if AC bit cannot be changed, CPU=386
        inc     ecx
        test    edx,200000h
        jz      havcpu                  ;if ID bit cannot be changed, CPU=486
        inc     ecx

        mov     eax,1                   ;request feature flags
        CPUID                           ;0Fh, 0A2h is CPUID instr
        mov     ecx,eax                 ;use Version Information
havcpu: mov     rval,ecx
        pop     ebx
    }
    return(rval);
    #endif      //-------
    #endif                                      //---
}

//*************************************************************************
//  RegMAC:
//      Returns a single mac address for computer compressed to 32 bits.
//      Chooses just one eithernet or wifi (or other device) MAC address,
//      prefering the eithernet and then wifi types and prefering the
//      lowest index if there are multiple for a type, and then
//      compresses six byte MAC to four byte dword. This scheme can fail
//      if the chosen adapter is taken off line in the future, in which
//      case caller should keep a second RegCPU identifier to fall back on.
//
//  Arguments:
//      dword RegMAC (int mode)
//      mode    Reserved for future use and must be zero.
//
//  Returns:
//      Consolidated MAC for computer identification purposes.
//      Returns 0 if error.
//*************************************************************************

#ifdef ISWIN                                    //---
#include <IPHLPAPI.h>
#define CID_MAX 32                              //maximum number of NICs
#endif                                          //---

dword RegMAC (int mode) {
    #ifndef ISWIN                               //---
    return(0);
    #else                                       //---
    IP_ADAPTER_INFO info[CID_MAX];
    IP_ADAPTER_INFO* pin;
    byte* padd;
    dword len, sern, ethn, wifi, ring, atmn, pppn, index;
    dword iethn, iwifi, iring, iatmn, ipppn;
    int shift;
    uint adds;
    errc e;

    ethn = wifi = ring = atmn = pppn =0;
    iethn = iwifi = iring = iatmn = ipppn = 0xFFFFFFFF;

    pppn = 0;
    sern = 0;
    len = sizeof(info);
    e = GetAdaptersInfo(info, &len);
    if (e == 0) {
        index = 0xFFFFFFFF;
        padd = NULL;
        pin = info;
        do {                                    //go through listed adapters
            padd = pin->Address;
            adds = pin->AddressLength;

            sern = 0;
            shift = 0;
            while (adds--) {                    //consolidate into 32 bits
                sern ^= ((lint)(*padd)) << shift;
                padd += 1;
                shift += 8;
                if (shift == 32) shift = 0;
            }
            if ((unsigned)sern < 2) sern = 2;   //do not allow reserved values

            if (pin->Type == MIB_IF_TYPE_ETHERNET) {
                if (pin->Index < iethn) {       //choose the lowest index
                    iethn = pin->Index;
                    ethn = sern;
                }
            }
            if (pin->Type == IF_TYPE_IEEE80211) {
                if (pin->Index < iwifi) {       //but remember if just one
                    iwifi = pin->Index;         //for various types
                    wifi = sern;
                }
            }                                   
            if (pin->Type == IF_TYPE_ISO88025_TOKENRING) {
                if (pin->Index < iwifi) {
                    iring = pin->Index;
                    ring = sern;
                }
            }
            if (pin->Type == MIB_IF_TYPE_SLIP) {
                if (pin->Index < iwifi) {
                    iatmn = pin->Index;
                    atmn = sern;
                }
            }
            if (pin->Type == MIB_IF_TYPE_PPP) {
                if (pin->Index < iwifi) {
                    ipppn = pin->Index;
                    pppn = sern;
                }
            }
            pin = pin->Next;                    //continue through list
        } while (pin);
        #ifdef DEB_REG                          //=== SPECIAL DEBUG FAKE CPU
        if (deb_cpu == -1) {                    //first time after loaded?
            deb_cpu = deb_mac = 0;              //do no harm if fails
            aint file = OSOpen(OS_READ, "C:\\Temp\\FakeRID.txt");
            char buff[SZNAME];
            buff[0] = 0;
            OSRead(file, buff, SZNAME);
            char* preg = buff;
            deb_cpu = OSNumGet(preg, &preg);    //read cpu and mac increments
            if (*preg == ',') preg += 1;        //from FakeRID.txt file
            deb_mac = OSNumGet(preg, &preg);    //to be added to computer's IDs
            OSClose(file);
            printf("@@ RegMAC Temp\\FakeRID.txt CPU+ %i and MAC+ %i Debug ID\n", deb_cpu, deb_mac);
        }
        ethn += deb_mac;                        //change to fake MAC ID
        wifi += deb_mac;                        //just in case
        #endif                                  //===
        if (ethn > 2) return(ethn);             //return most common
        if (wifi > 2) return(wifi);             //so repeatable call to call
        if (ring > 2) return(ring);
        if (atmn > 2) return(atmn);
        if (pppn > 2) return(pppn);
    }
    return(0);                                  //error return is zero
    #endif                                      //---
}

//*************************************************************************
//  Txt2Mac:
//      Converts the "HH:HH:HH:HH:HH:HH" text representation of a MAC address
//      to or from the six byte representation.
//
//  Arguments:
//      text* Txt2Mac (int mode, byte* pmac, text* ptxt)
//      mode    1 = Convert pmac to ptxt zero-term text representation
//              2 = Convert ptxt HH:HH:HH:HH:HH:HH text to pmac bytes.
//      pmac    Points to six bytes with binary mac address.
//      ptxt    Points to "HH:HH:HH:HH:HH:HH" text representation.
//
//  Returns:
//      Pointer to end of ptxt HH:HH:HH:HH:HH:HH string.
//*************************************************************************

#ifndef MACSZ
#define MACSZ        6  //Number of bytes in a MAC address
#endif

istext Txt2Mac (int mode, byte* pmac, text* ptxt) {
    int cnt;

    cnt = MACSZ;
    if (mode & 2) {
        do {
            *pmac++ = (byte) OSHexGet(ptxt, &ptxt);
            if (*ptxt) ptxt += 1;
        } while (--cnt);
    } else {
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
//  Get_CRC:
//  Buf_CRC:
//      Computes ethernet-style crc check value.
//      Uses precomputed crc32-adler table by Mark Adler,
//      x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
//
//      Get_CRC computes crc for single buffer.
//      Buf_CRC can calculate the CRC in multiple buffer calls.
//      The initial ulCLC value must be CRC_INIT (0xFFFFFFFF).
//
//      Beware: Must xor return value with CRC_INIT (0xFFFFFFFF)!
//        crc = Buf_CRC(buf1, bytes, CRC_INIT);
//        crc = Buf_CRC(buf2, bytes, crc);
//        crc = Buf_CRC(buf3, bytes, crc);
//        crc = crc ^ CRC_INIT;
//
//  Arguments:
//      dword Get_CRC (void* pBuf, dword, nLen)
//      dword Buf_CRC (void* pBuf, dword, nLen, dword ulCRC)
//      pBuf    Data buffer that needs a CRC checksum.
//      nLen    Number of bytes in pBuf
//      ulCRC   Beginning CRC value. Must be CRC_INIT (0xFFFFFFFF) initially.
//
//  Returns:
//      Updated CRC for end of pBuf block.
//      Important: After final Buf_CRC xor with 0xFFFFFFFF for correct CRC!
//      Example:   crc = Buf_CRC(buf, butes, CRC_INIT) ^ CRC_INIT;
//*************************************************************************

dword crc32_table[] = {                         //crc32-adler with 256 dword values
        0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
        0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
        0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
        0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
        0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
        0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
        0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
        0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
        0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
        0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
        0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
        0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
        0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
        0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
        0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
        0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
        0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
        0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
        0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
        0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
        0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
        0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
        0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
        0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
        0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
        0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
        0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
        0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
        0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
        0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
        0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
        0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
        };

#ifndef CRC_INIT
#define CRC_INIT 0xFFFFFFFF                     // Initial CRC before adding
#endif

dword Buf_CRC (void* pBuf, dword nLen, dword ulCRC) {
    byte* buffer;

    buffer = (byte*) pBuf;
    while(nLen--) {
        ulCRC = (ulCRC >> 8) ^ crc32_table[(byte)((ulCRC & 0xFF) ^ *buffer)];
        buffer++;
    }
    return(ulCRC);
}

dword Get_CRC (void* pBuf, dword nLen) {
    return( Buf_CRC(pBuf, nLen, CRC_INIT) ^ CRC_INIT );
}

//*************************************************************************
//  FileCheck:
//      Checks whether a file has a given byte size and/or checksum.
//
//  Arguments:
//      dword FileCheck (int mode, text* pname, dword siz, dword crc)
//      mode    0 = Check the pname file.
//              1 = Return pname file byte size.
//              2 = Return pname file checksum.
//             +4 = Add to mode 0 to skip siz check. 
//             +8 = Add to mode 0 to skip crc check. 
//      pname   Path and file name to check.
//      siz     Expected file byte size for mode 0.
//              Ignored for modes 1 and 2.
//      crc     Expected file CRC checksum for mode 0.
//              Ignored for modes 1 and 2.
//
//  Returns:
//      Mode 0 returns TRUE if file matches crc and siz.
//      Mode 0 returns FALSE if file does not match siz.
//      Mode 0 returns -1 TRUENEG if file does not match crc.
//      Mode 0 returns -2 TRUEMIN if cannot open file.
//      Modes 1 and 2 return unsigned byte size or checksum.
//      Modes 1 and 2 return -1 if missing file.
//*************************************************************************

CSTYLE
dword FileCheck (int mode, text* pname, dword siz, dword crc) {
    byte buffer[BLKSIZE];
    text name[SZPATH];
    FFHand find;
    aint file;
    dword sum;
    int rv, have, done, bytes;

    if (!(mode & (4+2))) {                      //need to get file size?
        OSTxtCopy(name, pname);
        rv = OSFindFile(FF_FIRST, name, &find);
        if (rv < 0) return((mode & (1+2)) ? -1:-2);
        bytes = (int) find.ffBytes;             //get file size
        OSFindFile(FF_DONE, NULL, &find);
        if (mode & 1) {
            return(bytes);                      //just return file size?
        }
        if (bytes != siz) {
            return(FALSE);                      //file size does not match?
        }
    }
    if (!(mode & (8+1))) {                      //need to get crc?
        file = OSOpen(OS_READ, pname);
        if (file < 0) return((mode & (1+2)) ? -1:-2);
        done = 0;
        sum = CRC_INIT;
        while (TRUE) {                          //read through file and get crc
            have = OSRead(file, buffer, BLKSIZE);
            if (have <= 0) break;
            sum = Buf_CRC(buffer, have, sum);
            done += have;                       //also recheck file size
        }
        sum = sum ^ CRC_INIT;
        if (sum == -1) sum = -2;                //reserve -1 for missing
        OSClose(file);
        if (mode & 2) {
            return(sum);                        //just return checksum?
        }
        if (done != siz) {                      //second way to check file size
            if (!(mode & 4)) return(FALSE);
        }
        if (sum != crc) {
            return(-1);                         //checksum does not match?
        }
    }
    return(TRUE);                               //file checks okay
}

//*************************************************************************
//  CheckBoth:
//      Like FileCheck but checks a second file with a different extension
//      and adds the size or checksum to the first file's.
//      Unlike FileCheck, second file adds 0 if missing.
//
//  Arguments:
//      dword CheckBoth (int mode, text* pname, dword siz, dword crc,
//                       text* pext) 
//      mode    0 = Check the pname file.
//              1 = Return pname file byte size.
//              2 = Return pname file checksum.
//             +4 = Add to mode 0 to skip siz check. 
//             +8 = Add to mode 0 to skip crc check. 
//      pname   Path and file name to check.
//      siz     Expected file byte size for mode 0.
//              Ignored for modes 1 and 2.
//      crc     Expected file CRC checksum for mode 0.
//              Previous file's CRC for modes 1 and 2.
//      pext    Extsion (like ".ext") for second filename.
//              If NULL just handles single pname file.
//              Otherwise uses sum of siz or crc,
//              where second file adds 0 if missing.
//
//  Returns:
//      Mode 0 returns TRUE if files match crc and siz.
//      Mode 0 returns FALSE if files do not match siz.
//      Mode 0 returns -1 TRUENEG if files do not match crc.
//      Mode 0 returns -2 TRUEMIN if cannot open first file.
//      Modes 1 and 2 add unsigned byte size or checksum to crc.
//      Modes 1 and 2 add 0 if missing second file.
//*************************************************************************

CSTYLE
dword CheckBoth (int mode, text* pname, dword siz, dword crc, text* pext) {
    text other[SZPATH];
    dword sum, one, two;

    if (pext == NULL) {                         //no second file?
        sum = FileCheck(mode, pname, siz, crc); //handle first file
        return(sum);
    }
    OSFileType(other, pname, pext);             //path with other extension
    if (mode & (1+2)) {                         //return siz or crc?
        sum = FileCheck(mode, pname, 0, 0);     //handle first file
        two = FileCheck(mode, other, 0, 0);     //get other file's siz or crc
        if (two == -1) two = 0;                 //missing adds zero
        return(sum + two);                      //add to previous siz or crc
    }
    sum = 0;                                    //chcke size and/or crc?
    if (!(mode & 4)) {                          //check size?
        one = FileCheck(1, pname, 0, 0);        //handle first file
        if (one == -1) return(-2);              //first does not exist?
        sum += one;
        two = FileCheck(1, other, 0, 0);        //get other file's siz
        if (two == -1) two = 0;                 //missing adds zero
        sum += two;                             //sum of two sizes
        if (sum != siz) {
            return(FALSE);                      //sizes do not match
        }
    }
    sum = 0;
    if (!(mode & 8)) {                          //check
        one = FileCheck(2, pname, 0, 0);        //handle first file
        if (one == -1) return(-2);              //first does not exist?
        sum += one;
        two = FileCheck(2, other, 0, 0);        //get other file's crc
        if (two == -1) two = 0;                 //missing adds zero
        sum += two;                             //sum of two checksums
        if (sum != crc) {
            return(TRUENEG);                    //checksums do not match
        }
    }
    return(TRUE);                               //everything matches?
}

//*************************************************************************
//  FileXY:
//      Reads X,Y data from text file.
//      X,Y values may be separated by comma, space, tab, newline.
//
//  Arguments:
//      errc FileXY (int mode, text* fname, double* pX,double* pY,int npts)
//      mode    1 to read pX,pY from fname file.
//              2 to write pX,py to fname file.
//             +4 to just return number of X,Y in fname file.
//             +8 to write comma separated X,Y lines instead of space sep.
//            +16 to write tab separated X,Y liens instead of space sep.
//      fname   Path and file name to read or create.
//      pX      Array of X values.
//      pY      Array of Y values.
//      npts    Number of pX and pY values.
//              This is the buffer size for mode 1 read.
//              This is the number to write for mode 2.
//
//  Returns:
//      Number of X,Y values read.
//*************************************************************************

CSTYLE
isproc FileXY (int mode, text* fname, double* pX, double* pY, int npts) {
    text line[SZDISK];
    text* pbuf;
    text* ptxt;
    aint file;
    int need, used, cntr;
    text sep;
    errc ee;

    file = -1;
    pbuf = NULL;
    if (mode & (1+4)) {                         //read X,Y?
        ee = ECREAD;
        file = OSOpen(OS_READ, fname);
        if (file < 0) goto error;
        need = OSSeek(file, 0, OS_END);
        if (need <= 0) goto error;
        pbuf = (text*) OSAlloc(need+1);
        if (pbuf == NULL) goto error;
        OSSeek(file, 0, OS_SET);
        used = OSRead(file, pbuf, need);        //read text file into memory
        if (used != need) goto error;
        pbuf[need] = 0;                         //zero terminate
        OSClose(file);
        file = -1;

        ptxt = pbuf;
        while ((*ptxt == ' ')||(*ptxt == 0x9)) ptxt += 1;
        while (*ptxt >= 'A') {                  //skip any non-number lines
            while ((*ptxt)&&(*ptxt != 0xD)&&(*ptxt != 0xA)) ptxt += 1;
            if ((*ptxt == 0xD)||(*ptxt == 0xA)) ptxt += 1;
            if ((*ptxt == 0xD)||(*ptxt == 0xA)) ptxt += 1;
            while ((*ptxt == ' ')||(*ptxt == 0x9)) ptxt += 1;
        }

        used = 0;
        if (mode & 4) {                         //just get number of points?
            while (*ptxt) {                     //go through X,Y pairs
                OSFloatGet(ptxt, &ptxt);
                if (*ptxt == 0) break;
                if (*ptxt == ',') ptxt += 1;
                while ((unsigned)*ptxt <= ' ') ptxt += 1;
                OSFloatGet(ptxt, &ptxt);
                used +=1;                       //and cound points
                if (*ptxt == 0) break;
                if (*ptxt == ',') ptxt += 1;
                while ((unsigned)*ptxt <= ' ') ptxt += 1;
            }
            return(used);
        }

        while (*ptxt) {                         //go through X,Y pairs
            *pX++ = OSFloatGet(ptxt, &ptxt);    //and read values
            if (*ptxt == 0) break;
            if (*ptxt == ',') ptxt += 1;
            while ((unsigned)*ptxt <= ' ') ptxt += 1;
            *pY++ = OSFloatGet(ptxt, &ptxt);
            used +=1;
            if (used >= npts) break;            //no more space
            if (*ptxt == 0) break;
            if (*ptxt == ',') ptxt += 1;
            while ((unsigned)*ptxt <= ' ') ptxt += 1;
        }
        OSFree(pbuf);
        return(used);
    }

    if (mode & 2) {                             //write X,Y?
        sep = (mode & 8) ? ',' : ' ';
        if (mode & 16) sep =0x9;

        ee = ECWRITE;
        file = OSOpen(OS_CREATE, fname);
        if (file < 0) goto error;

        cntr = npts;
        do {
            ptxt = line;
            ptxt = OSFloatPut(*pX++, ptxt, 0, 0);
            *ptxt++ = sep;
            ptxt = OSFloatPut(*pY++, ptxt, 0, 0);
            *ptxt++ = 0xD;
            *ptxt++ = 0xA;
            need = ptxt - line;
            used = OSWrite(file, line, need);
            if (need != used) goto error;
        } while (--cntr);

        OSClose(file);
        return(npts);
    }
    return(ECARGUMENT);

error:
    if (pbuf) OSFree(pbuf);
    if (file != -1) OSClose(file);
    return(ee);
}

//*************************************************************************
//  ExTest:
//      Test for TriEx.cpp functions.
//      This is a general prototype for function-driven module tests.
//      Test functions should use the same parameters.
//      Compile with /D EXTESTS to include this function.
//
//  Arguments:
//      errc ExTest (int mode, text* pdir)
//      mode    0 = Run tests
//             +1 = Create test data if needed before running test. (T_NEW)
//                  When tests match with saved result files,
//                  Useful for test working legacy code when ported.
//             +2 = Verbose operation about each test beforehand.   (T_SEE)
//                  Otherwise only shows failures not hangs.
//             +4 = Continue tests after a failure.                 (T_ALL)
//                  For multiple tests does not stop on failure.
//             +8 = Add test output to log file.                    (T_LOG) 
//            +32 = Do longer QFifo test.                           (T_DIF)  
//      pdir    Folder path name ending in slash where test data exists.
//              May be NULL to use current working directory.
//
//  Returns:
//      Non-zero code if failure.
//*************************************************************************

#ifdef EXTESTS                                  //---
CSTYLE OSTASK_RET FifoTestTask (void* parm) {   //FifoTestTask:
    lint ii;
    lint* pdw;
    lpvoid* pparms = (lpvoid*) parm;            //parm must point to an array
    QFifo* fifo = (QFifo*) *pparms++;           //of three pointers:
    lint* pival = (lint*) *pparms++;            //pparms[0] = fifo object
    lint cnt = *pival;
    do {
        ii = 1000000;
        while (TRUE) {
            pdw = (lint*) fifo->Put1();
            if (pdw) break;
            OSSleep(0);
            if (--ii == 0) {
                cnt = -cnt;
                break;
            }
        }
        if (cnt < 0) break;
        *pdw = cnt;
        fifo->Put2();
    } while(--cnt);
    *pival = cnt;                               //return non-zero if error
    return(0);
}

CSTYLE csproc ExTest (int mode, text* pdir) {
    text name[SZPATH];
    text dest[SZPATH];
    text line[SZPATH];
    text* pend;
    text* ptxt;
    aint file1, file2;
    lint data[1024];
    lint ii, cnt, ival;
    void* parms[3];
    BFifo fifo;
    QFifo que;
    SerIn ser;
    lint* pdw;
    word wv;
    byte bv;
    errc ee;
    flx fx;
    flt fv;

    OSPrint((mode|T_ASIS), "ExTest: TriEx.cpp Testing", 0);
    ee = 0;
    if (pdir) {
        pend = OSTxtCopy(name, pdir);
    } else {
        pend = OSLocation(name, OS_CURDIR);     //get current working directory
    }

    // Test FileCopy function.
    *pend = 0;
    ptxt = OSTxtCopy(dest, name);
    while (ee == 0) {
        OSPrint(mode, "Testing FileCopy...", 0);
        ee = ECERROR;

        OSTxtCopy(pend, "Data.bin");
        file1 = OSOpen(OS_CREATE, name);
        if (file1 == -1) {
            OSPrint(mode+T_FAIL, "Create Data.bin error", file1);
            ee = ECNODATA;
            break;
        }
        for (ii=0; ii<512; ++ii) data[ii] = ii;
        OSWrite(file1, data, SZPATH);           //create Data.bin source
        OSClose(file1);
        file1 = OSOpen(OS_READ, name);
        if (file1 == -1) {
            OSPrint(mode+T_FAIL, "Open Data.bin error", file1);
            ee = ECNODATA;
            break;
        }

        OSTxtCopy(ptxt, "Test.bin");            //copy Data.bin to Test.bin
        ee = FileCopy(CF_OVERWT+CF_TONAME, line, dest, name, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "FileCopy error", ee);
            break;
        }
        file2 = OSOpen(OS_READ, line);          //open Test.bin
        if (file2 == -1) {
            OSPrint(mode+T_FAIL, "No Test.bin file", 0);
            ee = ECREAD;
            break;
        }
        ee = FileComp(file1, file2);
        if (ee) {
            OSPrint(mode+T_FAIL, "FileComp error:", ee);
            break;
        }
        if (file1 != -1) OSClose(file1);
        if (file2 != -1) OSClose(file2);
        ee = 0;
        break;
    }

    // Test Serialization functions.
    while (ee == 0) {                           //fake loop for easy error breaks
        OSPrint(mode, "Testing SerLINT and other serialization functions...", 0);
        ee = ECERROR;

        OSMemClear(&ser, sizeof(SerIn));
        ii = -1234;
        SerLINT(SER_PUT, &ser, &ii);
        ii = 0x12345678;
        SerDWRD(SER_PUT, &ser, (dword*)&ii);
        wv = 0x1234;
        SerWORD(SER_PUT, &ser, &wv);
        bv = 0x12;
        SerBYTE(SER_PUT, &ser, &bv);
        fv = -123.4f;
        SerFLTV(SER_PUT, &ser, &fv);
        fx = -1234.5;
        SerFLXV(SER_PUT, &ser, &fx);
        ptxt = "This is a test.";
        SerTEXT(SER_PUT, &ser, ptxt, 0);
        OSTxtCopy(line, "More testing...");
        ptxt = line;
        SerPTXT(SER_PUT, &ser, &ptxt);
        ptxt = NULL;
        SerPTXT(SER_PUT, &ser, &ptxt);
        ptxt = "The name test.";
        SerNAME(SER_PUT, &ser, ptxt);
        OSTxtCopy(line, "Block test...");
        SerBLOK(SER_PUT, &ser, line, sizeof(line));
        
        ser.end = 0;                            //back to beginning to read
        SerLINT(SER_GET, &ser, &ii);
        if (ii != -1234) {
            OSPrint(mode+T_FAIL, "SerLINT error:", ii);
            break;
        }
        SerDWRD(SER_GET, &ser, (dword*)&ii);
        if (ii != 0x12345678) {
            OSPrint(mode+T_FAIL, "SerDWORDerror:", ii);
            break;
        }
        SerWORD(SER_GET, &ser, (word*)&wv);
        if (wv != 0x1234) {
            OSPrint(mode+T_FAIL, "SerWORD error:", wv);
            break;
        }
        SerBYTE(SER_GET, &ser, (byte*)&bv);
        if (bv != 0x12) {
            OSPrint(mode+T_FAIL, "SerBYTE error:", bv);
            break;
        }
        SerFLTV(SER_GET, &ser, &fv);
        if (fv != -123.4f) {
            OSPrint(mode+T_FAIL, "SerFLTV error:", (lint)fv);
            break;
        }
        SerFLXV(SER_GET, &ser, &fx);
        if (fx != -1234.5) {
            OSPrint(mode+T_FAIL, "SerFLXV error:", (lint)fx);
            break;
        }
        SerTEXT(SER_GET, &ser, line, SZNAME);
        if (OSTxtCmp(line, "This is a test.")) {
            OSPrint(mode+T_FAIL, "SerTEXT error:", OSTxtSize(line));
            break;
        }
        ptxt = NULL;
        SerPTXT(SER_GET, &ser, &ptxt);
        if (OSTxtCmp(ptxt, "More testing...")) {
            OSPrint(mode+T_FAIL, "SerPTXT error:", OSTxtSize(ptxt));
            break;
        }
        OSFree(ptxt);
        ptxt = NULL;
        SerPTXT(SER_GET, &ser, &ptxt);
        if (ptxt != NULL) {
            OSPrint(mode+T_FAIL, "SerPTXT NULL error:", OSTxtSize(ptxt));
            OSFree(ptxt);
            break;
        }
        SerNAME(SER_GET, &ser, line);
        if (OSTxtCmp(line, "The name test.")) {
            OSPrint(mode+T_FAIL, "SerNAME error:", OSTxtSize(line));
            break;
        }
        OSTxtCopy(line, "Block test...");
        SerBLOK(SER_GET, &ser, line, sizeof(line));
        if (OSTxtCmp(line, "Block test...")) {
            OSPrint(mode+T_FAIL, "SerBLOK error:", OSTxtSize(line));
            break;
        }
        OSFree(ser.mem);
        ee = 0;
        break;                                  //not a real loop!
    }

    // Test XML functions.
    while (ee == 0) {                           //fake loop for easy error breaks
        OSPrint(mode, "Testing xmlInit, xmlDWRD other XML functions...", 0);
        ee = ECERROR;

        xmlInit(1, &ser);                       //setup to return XML
        fv = 123.4f;
        ii = 1234;
        ptxt = NULL;
        OSTxtSafe(&ptxt, "Good", 0);
        xmlFLTV(&ser, "float", NULL, &fv, 1);
        xmlDWRD(&ser, "dword", NULL, (dword*)&ii);
        xmlLINT(&ser, "int",   NULL, &ii);
        xmlTEXT(&ser, "text",  NULL, "okay");
        xmlPTXT(&ser, "ptext", NULL, &ptxt);
        OSFree(ptxt);
    
        ii = xmlFind(ser.mem, "ptext", line);
        if ((ii)||(OSTxtCmpS(line, "Good"))) {
            OSPrint(mode+T_FAIL, "xmlPTxt error:", ii);
            break;
        }
        ii = xmlFind(ser.mem, "text", line);
        if ((ii)||(OSTxtCmpS(line, "okay"))) {
            OSPrint(mode+T_FAIL, "xmlPTxt error:", ii);
            break;
        }
        ii = xmlFind(ser.mem, "int", line);
        if ((ii)||(OSTxtCmpS(line, "1234"))) {
            OSPrint(mode+T_FAIL, "xmlLINT error:", ii);
            break;
        }
        ii = xmlFind(ser.mem, "dword", line);
        if ((ii)||(OSTxtCmpS(line, "1234"))) {
            OSPrint(mode+T_FAIL, "xmlDWRD error:", ii);
            break;
        }
        ii = xmlFind(ser.mem, "float", line);
        if ((ii)||(OSTxtCmpS(line, "123.4"))) {
            OSPrint(mode+T_FAIL, "xmlFLTV error:", ii);
            break;
        }
        OSFree(ser.mem);
        ee = 0;
        break;                                  //not a real loop!
    } 

    // Test QFifo class functions.
    while (ee == 0) {                           //fake loop for easy error breaks
        OSPrint(mode, "Testing QFifo class...", 0);

        ee = que.Init(0, 4, 4);                 //fits 4x1 dwords
        if (ee) {
            OSPrint(mode+T_FAIL, "QFifo::Init error:", ee);
            break;
        }
        pdw = (lint*) que.Get1();
        if (pdw) {
            OSPrint(mode+T_FAIL, "QFifo::Get1 first false positive error:", ee);
            ee = ECERROR;
            break;
        }
        ee = 0;
        for (ii=0; ii<4; ++ii) {
            pdw = (lint*) que.Put1();
            if (pdw == NULL) {
                OSPrintI(mode+T_FAIL, "QFifo::Put1 loop error:", ii);
                ee = ECERROR;
                break;
            }
            *pdw = ii+1;
            que.Put2();
        }
        if (ee) break;
        pdw = (lint*) que.Put1();
        if (pdw) {
            OSPrint(mode+T_FAIL, "QFifo::Put1 false positive error:", ee);
            break;
        }
        for (ii=0; ii<4; ++ii) {
            pdw = (lint*) que.Get1();
            if (pdw == NULL) {
                OSPrintI(mode+T_FAIL, "QFifo::Get1 loop null error:", ii);
                ee = ECERROR;
                break;
            }
            if (*pdw != ii+1) {
                OSPrintI(mode+T_FAIL, "QFifo::Get1 loop match error:", *pdw);
                ee = ECERROR;
                break;
            }
            que.Get2();
        }
        if (ee) break;
        pdw = (lint*) que.Get1();
        if (pdw) {
            OSPrint(mode+T_FAIL, "QFifo::Get1 last false positive error:", ee);
            ee = ECERROR;
            break;
        }
        pdw = (lint*) que.Put1();
        if (pdw == NULL) {
            OSPrint(mode+T_FAIL, "QFifo::Put1 last false negative error:", ee);
            ee = ECERROR;
            break;
        }
        parms[0] = &que;                        //pass QFifo object ptr to task
        ival = 10000;                           //messages to test
        if (mode & T_DIF) ival = 50000000;      //longer 50 million (~10 seconds)
        cnt = ival;
        parms[1] = (void*) &ival;
        ii = OSTask(OS_TASKNEW, &parms[2], FifoTestTask, parms);
        if (ii != 0) {
            OSPrint(mode+T_FAIL, "OSTask error", ii);
            ee = ECERROR;
            break;
        }
        OSSleep(100);
        do {
            ii = 1000000;
            while (TRUE) {
                pdw = (lint*) que.Get1();
                if (pdw) break;
                OSSleep(0);
                if (--ii == 0) {
                    cnt = -cnt;                 //hung?
                    break;
                }
            }
            if (cnt < 0) break;
            if (*pdw != cnt) {
                break;                          //value mismatch?
            }
            que.Get2();
        } while(--cnt);
        if ((cnt != 0)&&(ival >= 0)) {          //our get error?
            OSPrint(mode+T_FAIL, "QFifo task get error", cnt);
            break;
        }
        if (ival != 0) {
            OSPrint(mode+T_FAIL, "QFifo task put error", ival);
            break;
        }
        ee = 0;
        break;
    }

    // Test PFifo functions.
    while (ee == 0) {                           //fake loop for easy error breaks
        OSPrint(mode, "Testing PFifo functions...", 0);

        ee = PFifoInit(0, 5, 4, line);          //fits 4x1 dwords
        if (ee) {
            OSPrint(mode+T_FAIL, "PFifoInit error:", ee);
            break;
        }
        pdw = (lint*) PFifoGet1(line);
        if (pdw) {
            OSPrint(mode+T_FAIL, "PFifoGet1 first false positive error:", ee);
            ee = ECERROR;
            break;
        }
        for (ii=0; ii<4; ++ii) {
            pdw = (lint*) PFifoPut1(line);
            if (pdw == NULL) {
                OSPrintI(mode+T_FAIL, "PFifoPut1 loop error:", ii);
                ee = ECERROR;
                break;
            }
            *pdw = ii+1;
            PFifoPut2(line);
        }
        if (ee) break;
        pdw = (lint*) PFifoPut1(line);
        if (pdw) {
            OSPrint(mode+T_FAIL, "PFifoPut1 false positive error:", ee);
            break;
        }
        for (ii=0; ii<4; ++ii) {
            pdw = (lint*) PFifoGet1(line);
            if (pdw == NULL) {
                OSPrintI(mode+T_FAIL, "PFifoGet1 loop null error:", ii);
                ee = ECERROR;
                break;
            }
            if (*pdw != ii+1) {
                OSPrintI(mode+T_FAIL, "PFifoGet1 loop match error:", *pdw);
                ee = ECERROR;
                break;
            }
            PFifoGet2(line);
        }
        if (ee) break;
        pdw = (lint*) PFifoGet1(line);
        if (pdw) {
            OSPrint(mode+T_FAIL, "PFifoGet1 last false positive error:", ee);
            break;
        }
        pdw = (lint*) PFifoPut1(line);
        if (pdw == NULL) {
            OSPrint(mode+T_FAIL, "PFifoPut1 last false negative error:", ee);
            break;
        }
        ee = 0;
        break;
    }

    // Test BFifo class functions.
    while (ee == 0) {                           //fake loop for easy error breaks
        OSPrint(mode, "Testing BFifo class...", 0);
        ee = ECERROR;

        ee = fifo.Set(0, 560);                  //fits 5x100+4 with 40 bytes wrap
        if (ee) {
            OSPrint(mode+T_FAIL, "BFifo::Set error:", ee);
            break;
        }
        ee = fifo.Get(line, (dword*)&ival);
        if (ee == 0) {
            OSPrint(mode+T_FAIL, "BFifo::Get first false positive error:", ee);
            break;
        }
        ee = ECERROR;
        for (ii=0; ii<(25*6); ++ii) data[ii] = 1000 + ii;
        for (ii=0; ii<5; ++ii) {                //25 dwords in a block
            ee = fifo.Put(&data[ii*25], 100);
            if (ee) {
                OSPrintI(mode+T_FAIL, "BFifo::Put loop error:", ii);
                break;
            }
        }
        if (ee) break;
        ee = fifo.Put(&data[5*25], 100);
        if (ee == 0) {
            OSPrint(mode+T_FAIL, "BFifo::Set false positive error:", ee);
            ee = ECERROR;
            break;
        }
        ee = fifo.Get(line, (dword*)&ival);
        if ((ee)||(ival != 100)||(OSMemMatch(line, data, 100) != 100)) {
            OSPrint(mode+T_FAIL, "BFifo::Get error:", ee);
            if (ee == 0) ee = ECERROR;
            break;
        }
        ee = fifo.Put(&data[5*25], 100);
        if (ee) {
            OSPrint(mode+T_FAIL, "BFifo::Set false negative error:", ee);
            break;
        }
        for (ii=1; ii<6; ++ii) {                //25 dwords in a block
            ee = fifo.Get(line, (dword*)&ival);
            if ((ee)||(ival != 100)||(OSMemMatch(line, &data[ii*25], 100) != 100)) {
                OSPrintI(mode+T_FAIL, "BFifo::Get loop error:", ii);
                if (ee == 0) ee = ECERROR;
                break;
            }
        }
        if (ee) break;
        ee = fifo.Get(line, (dword*)&ival);
        if (ee == 0) {
            OSPrint(mode+T_FAIL, "BFifo::Get ending false positive error:", ee);
            ee = ECERROR;
            break;
        }
        ee = 0;
        break;                                  //not a real loop!
    } 

    return(ee);                                 //success
}
#endif                                          //--- EXTESTS