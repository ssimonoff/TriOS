//*************************************************************************
//  FILENAME:   TriRPC.cpp
//  AUTHOR:     SCSimonoff  7/05/21
//  CPU:        Portable
//  O/S:        Portable
//  MRU:
//
//  Remote Procedure Call API.  
//  Compile with /D DEB_SOCK (or define below) to print accept socket info.
//  Compile with /D DEB_BEST (or define below) to print vital information.
//  Compile with /D DEB_MORE (or define below) to print extra information.
//  Compile with /D NOP_WAIT (or define below) to skip RP_PROC processing msg.
//  
//  Our scheme for communicating abitrary arguments and return values is to
//  serialize them into RBlok blocks of memory that are sent via pipes.
//  The blocks consist of a header and followed by data "slots" for each
//  scalar or array value. The entire block and each slot's data begins
//  with a dword byte size for the entire block or slot. The slots are
//  padded to maintain eight-byte alignment for faster double and int64 access.
//  The RBlok blocks do not contain meta data describing their data.
//  Instead both the sender and receive must agree on the slots types and order.
//  Both sides keep an array of RSlot structures describing this meta data.
//  Both sides must have the same RSlot array describing the RBlok data slots
//  in the same order. The RBlok header includes an iproc integer set by the
//  sender to inform the receiver which slots to use to decode the data block.
//
//  Uses TriOS.h portable non-gui system calls.
//  Uses CDefs.h custom types such as: cint,byte,sint,word,lint,dword,flt,flx
//  This software was created 2021 by SCSimonoff for Triscape Inc.
//  This may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#include "CDefs.h"
#include "TriOS.h"
#include "TriRPC.h"

// Uncomment to debug.
#define     DEB_SOCK                            //socket debug print output
#define     DEB_BEST                            //vital debug print output
//#define   DEB_MORE                            //extra debug print output

// Functions in this file.

CSTYLE_BEG
int    RpcTxtIni (int mode, RSlot* pslot, int nslot, char* pform);
int    RpcEnText (int mode, RSlot* pslot, int nslot, text* ptxt, text csep);
int    RpcEnDone (int mode, RSlot* pslot, int nslot);
int    RpcEncode (int mode, RBlok* pblok, int nblok, RSlot* pslot, int nslot);
int    RpcDecode (int mode, RSlot* pslot, int nslot, RBlok* pblok);

int    RpcRingSet (int mode, ARing* pfifo);
int    RpcRingPut (int mode, ARing* pfifo, aint handle);
aint   RpcRingGet (int mode, ARing* pfifo);

int    RpcFifoSet (int mode, BRing* pfifo);
int    RpcFifoPut (int mode, BRing* pfifo, RBlok* pblok);
RBlok* RpcFifoGet (int mode, BRing* pfifo);

int    RpcSendQ (dword dval, qint qval, aint sock, int timeout);
int    RpcSend (int mode, aint sock, RBlok* pblok, int timeout);
RBlok* RpcRecv (int mode, aint sock, int timeout);

int    RpcCall (int mode, CChan* pchan, int iproc, RBlok* pargs, RBlok** ppret);
int    TxtCall (int mode, CChan* pchan, int iproc, char* pname, char* plibs, int smode, int tasks, int nkeep, int nramb, float blend, RBlok** ppret);
int    TxtFrom (int mode, CCVal* pvals, RBlok* pblok);
int    VecCall (int mode, CChan* pchan, int iproc, double* pspc, int nspc, double begx, double endx, text* plibs, int smode, int tasks, int nkeep, int nramb, float blend, RBlok** ppret);
int    VecFrom (int mode, CSVal* pvals, RBlok* pblok);
int    SetCall (int mode, CChan* pchan, int iproc, int prm, int get, double val, double* psetv, RBlok** ppret);
int    SetFrom (int mode, CVVal* pvals, RBlok* pblok);
int    RpcFree (int mode, RBlok* pblok);

aint   RpcConnect (int mode, aint hpipe, text* pname, int port, ConEx* pex);
int    RpcConSend (aint hpipe, void* pv, dword cb);
int    RpcConRecv (aint hpipe, void* pv, dword cb);

void   RpcAddress (int mode, char* pname, int* pport, int* ptype);
int    RpcServer (int mode, SChan* pin, RSlot* slots, int nslot, char* pform, RBlok** ppblok);
int    RpcStart (int mode, char* sexec, char* pipen);
int    RpcClose (int mode, CChan* pchan);
char*  RpcPath (int mode, char* plibs);
int    RpcTest (int mode);
CSTYLE_END

OSTASK_RET RpcServTask (BRing* pfifo);

//*************************************************************************
//  RpcTxtIni:
//      Initializes slot value definitions from text string.
//
//  Arguments:
//      int RpcTxtIni (int mode, RSlot* pslot, int nslot, char* pform)
//      mode    Reserved for future use and must be zero
//      pslot   Array of slot information structures to be initialized
//      nslot   Number of slot information structures to be initialized
//      pform   Format string like "@T,@I,@F8,@D" (text,int,8 floats, double)
//              The @ and , are optional ("TIF8D") but easier to read
//              @T for text string
//              @B for blob array of bytes
//              @I for int32 integer value
//              @L for int64 integer value
//              @F for float32 floating point value
//              @D for float64 double floating point value
//              @R for int32 row index and nvec number of rows
//
//  Returns:
//      Returns number of slots initialized (normally nslot).
//      Otherwise returns negative code if error.
//*************************************************************************

CSTYLE
int RpcTxtIni (int mode, RSlot* pslot, int nslot, char* pform) {
    char* ptxt;
    char letter;
    int slots;

    OSMemClear(pslot, nslot * sizeof(RSlot));   //zero slots memory
    slots = 0;
    while (*pform) {                            //go through format string
        while (*pform == ' ') pform += 1;
        if (*pform == '@') pform += 1;
        letter = CAPITAL(*pform);               //get lette code
        if        (letter == 'T') {             //text string?
            pslot->code = RPC_TXT;
        } else if (letter == 'I') {
            pslot->code = RPC_INT;
        } else if (letter == 'F') {
            pslot->code = RPC_FLT;
        } else if (letter == 'D') {
            pslot->code = RPC_DBL;
        } else if (letter == 'L') {
            pslot->code = RPC_I64;
        } else if (letter == 'B') {
            pslot->code = RPC_BLB;
        } else if (letter == 'R') {
            pslot->code = RPC_ROW;
        } else {
            return(ECSYNTAX);
        }
        pform += 1;
        pslot->nvec = OSNumGet(pform, &pform);  //array size or 0
        if (*pform == ',') pform += 1;          //skip comma separator
        slots += 1;
        pslot += 1;
        if (slots >= nslot) break;              //stop at given slot
    }
    return(slots);
}

//*************************************************************************
//  RpcEnText:
//      Encodes text field data to RPC slot information.
//      The ptxt string gives values for each slot in order
//      and csep character separates these values.
//      Cannot fill integer and floating arrays.
//      The RPC_ROW code optionally allows irow/nrow value
//      and the slash char must preceed nvec number of rows
//      but may just be irow with nvec=0 for unknown total rows.
//      Saves text and blobs in allocated memory.
//      Caller must use RpcEnDone to free memory.
//
//  Arguments:
//      int RpcEnText (int mode, RSlot* pslot, int nslot, text* ptxt, text csep)
//      mode    Reserved for future use and must be zero.
//      pslot   RPC slot information to receive ptxt data.
//              Previous pall pointers are ignored.
//              Caller must use RpcEnDone afterward.
//      nslot   Available slots but may use less.
//      ptxt    Source text string with csep separator characters.
//      csep    Source text field separator character.
//              If zero uses comma character.
//
//  Returns:
//      Number of slots filled.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int RpcEnText (int mode, RSlot* pslot, int nslot, text* ptxt, text csep) {
    text* pend;
    text* pall;
    int slots, bytes;

    if (csep == 0) csep = ',';

    slots = 0;
    while (*ptxt) {                             //go through source text
        pend = ptxt;
        while ((*pend)&&(*pend != csep)) {      //find end of substring
            pend += 1;                          //to next separator char
        }
        bytes = pend - ptxt;
        pslot->pvec = NULL;
        pslot->nvec = 0;
        if        (pslot->code == RPC_TXT) {    //handle various data types
            pall = (text*) OSAlloc(bytes+1);
            if (pall==  NULL) return(ECMEMORY);
            OSMemCopy(pall, ptxt, bytes);
            pall[bytes] = 0;
            pslot->pvec = pall;
            pslot->nvec = bytes+1;
            ptxt = pend;

        } else if (pslot->code == RPC_INT) {
            pslot->inum = OSNumGet(ptxt, &ptxt);

        } else if (pslot->code == RPC_FLT) {
            pslot->fnum = (float) OSFloatGet(ptxt, &ptxt);

        } else if (pslot->code == RPC_DBL) {
            pslot->fbig = OSFloatGet(ptxt, &ptxt);

        } else if (pslot->code == RPC_I64) {
            while (*ptxt == ' ') ptxt += 1;
            if ((*ptxt == '0')&&((*(ptxt+1) == 'x')||(*(ptxt+1) == 'X'))) {
                ptxt += 2;
                pslot->ibig = OSHexGetQ(ptxt, &ptxt);
            } else {
                pslot->ibig = OSIntGetQ(ptxt, &ptxt);
            }
        } else if (pslot->code == RPC_BLB) {
            if (bytes == 0) return(ECFEWPTS);
            pall = (text*) OSAlloc(bytes);
            if (pall==  NULL) return(ECMEMORY);
            OSMemCopy(pall, ptxt, bytes);
            pslot->pvec = pall;
            pslot->nvec = bytes;
            ptxt = pend;

        } else if (pslot->code == RPC_ROW) {
            pslot->inum = OSNumGet(ptxt, &ptxt);
            while (*ptxt == ' ') ptxt += 1;
            if (*ptxt == '/') {                 //allow irow or irow/nrow
                ptxt += 1;
                pslot->nvec = OSNumGet(ptxt, &ptxt);
            }
        } else {
            return(ECSYNTAX);
        }
        slots += 1;
        pslot += 1;
        if (slots >= nslot) break;              //stop at given slot
        if (*ptxt == csep) ptxt += 1;           //skip separator
    }
    return(slots);                              //return slots filled
}

//*************************************************************************
//  RpcEnDone:
//      Frees slot pvec memory which can be allocated by RpcEnText.
//
//  Arguments:
//      int RpcEnDone (int mode, RSlot* pslot, int nslot);
//      mode    Reserved for future use and must be zero.
//      pslot   Source data information slots, one value or array per slot.
//      nslot   Number of source data slots.
//
//  Returns:
//      Currently always returns zero for no errors.
//*************************************************************************

CSTYLE
int RpcEnDone (int mode, RSlot* pslot, int nslot) {
    int slots;
    slots = nslot;
    while (slots > 0) {
        if (pslot->pvec) OSFree(pslot->pvec);
        pslot->pvec = NULL;
        pslot += 1;
        slots -= 1;
    }
    return(0);
}

//*************************************************************************
//  RpcEncode:
//      Encodes RPC slot information to an RBlok memory block.
//
//  Arguments:
//      int RpcEncode (int mode, RBlok* pblok,int nblok, RSlot* pslot,int nslot)
//      mode    Low RP_MASK byte gives proceedure call index number.
//      pblok   Target block of allocated memory to return encoded data.
//              May be NULL to just return needed bytes
//      nblok   Number of bytes avaliable in pblok.
//              May be zero to just return needed bytes.
//      pslot   Source data information slots, one value or array per slot.
//      nslot   Number of source data slots.
//
//  Returns:
//      Number of bytes in returned pblok data block.
//      and if pblok is given then pblok->bytes gives required size.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int RpcEncode (int mode, RBlok* pblok, int nblok, RSlot* pslot, int nslot) {
    dword* pdata;
    dword bytes, total, nleft;
    int slots, need, vals, rv;

    if (nblok <= 0) pblok = NULL;               //just getting needed size?
    pdata = (dword*) pblok;                     //pointer into block
    if (pdata) {
        pdata += sizeof(RBlok) >> 2;            //advance over RBlok header
    }
    total = sizeof(RBlok);                      //total bytes used
    nleft = nblok - sizeof(RBlok);              //buffer bytes left

    rv = 0;
    if (nslot < 0) nslot = 0;                   //just in case
    slots = nslot;
    while (slots > 0) {                         //go through data slots
        vals = ((pslot->pvec)&&(pslot->nvec)) ? pslot->nvec : 1;
        if (vals < 0) vals = 0;
        if        (pslot->code == RPC_TXT) {    //byte size for data types
            need = OSTxtSize((text*)pslot->pvec);
        } else if (pslot->code == RPC_INT) {
            need = vals * sizeof(lint);
        } else if (pslot->code == RPC_FLT) {
            need = vals * sizeof(float);
        } else if (pslot->code == RPC_DBL) {
            need = vals * sizeof(double);
        } else if (pslot->code == RPC_I64) {
            need = vals * sizeof(qint);
        } else if (pslot->code == RPC_BLB) {
            need = vals * sizeof(byte);
        } else if (pslot->code == RPC_ROW) {
            need = sizeof(lint);                //row is always row dword
            vals = pslot->nvec;                 //and number of rows in nvec
        } else {
            return(ECSYNTAX);
        }
        bytes = (need + 7) & (~7);              //padd to eight byte boundry
        if (pdata) {
            if (bytes <= nleft) {               //bytes of data for slot
                *pdata++ = bytes + (2*sizeof(dword));
                *pdata++ = vals;                //number of array values
                OSMemCopy(pdata, (pslot->pvec) ? pslot->pvec : &pslot->inum, need);
                pdata += (bytes >> 2);          //copy value(s)
            } else {
                pdata = NULL;                   //stop saving block if too small
            }
        }
        bytes += 2 * sizeof(dword);             //room for preceeding dword sizes
        total += bytes;                         //total bytes used so far
        nleft -= bytes;                         //total bytes now left

        pslot += 1;
        slots -= 1;
    }
    if (pblok) {
        pblok->bytes = total;                   //fill in header
        pblok->check = RPC_CHECK;
        pblok->iproc = mode & RP_MASK;
        pblok->slots = nslot;
        pblok->hpipe = 0;
    }
    return((rv) ? rv : total);
}

//*************************************************************************
//  RpcDecode:
//      Decodes RBlok memory block saved in RPC slot information.
//
//  Arguments:
//      int RpcDecode (int mode, RSlot* pslot,int nslot, RBlok* pblok)
//      mode    Reserved for future use and must be zero.
//      pslot   Target data information slots, one value or array per slot.
//              This function ignores ay existing pvec memory!
//              Note nvec for RPC_TXT includes ending zero.
//              Returned pvec pointers are into provided pblok
//              and are invalid if pblok is freed!
//      nslot   Number of target data slots.
//      pblok   Source block of data.
//
//  Returns:
//      Returns number of slots initialized (normally nslot).
//      Returns negative code if error.
//      Returns ECBADPEAK if pblok check value is wrong.
//      Returns ECBADSIZE if pblok decode size is wrong.
//*************************************************************************

CSTYLE
int RpcDecode (int mode, RSlot* pslot, int nslot, RBlok* pblok) {
    dword* pdata;
    dword bytes, total, limit;
    int slots, ndone, vals;

    if (pblok->check != RPC_CHECK) return(ECBADPEAK);
    slots = pblok->slots;
    if (slots > nslot) return(ECARGUMENT);      //not enough slots for block?
    if (slots < 0) slots = nslot;               //just in case
    limit = pblok->bytes;                       //total block bytes after header

    pdata = (dword*) pblok;                     //pointer into block
    pdata += sizeof(RBlok) >> 2;                //advance over RBlok header

    ndone = 0;                                  //count slots done
    total = sizeof(RBlok);
    while (slots > 0) {                         //go through data slots
        total += (*pdata);                      //total blocks used after slot
        if (total > limit) {                    //will be past end of block?
            return(ECBADSIZE);                  //caller may have nslots wrong?
        }
        bytes = (*pdata++) - (2*sizeof(dword)); //bytes of data after slot header
        vals = *pdata++;                        //array size for slot
        pslot->nvec = 0;                        //assume no array
        pslot->pvec = NULL;

        if        (pslot->code == RPC_TXT) {    //handle data type
            pslot->pvec = (void*)pdata;
            pslot->nvec = OSTxtSize((text*)pdata);

        } else if (pslot->code == RPC_INT) {
            if (vals <= 1) {
                pslot->inum = *((lint*)pdata);
            } else {
                pslot->pvec = (void*)pdata;
                pslot->nvec = (vals > 0) ? vals : (bytes >> 2);
            }

        } else if (pslot->code == RPC_FLT) {
            if (vals <= 1) {
                pslot->fnum = *((float*)pdata);
            } else {
                pslot->pvec = (void*)pdata;
                pslot->nvec = (vals > 0) ? vals : (bytes >> 2);
            }

        } else if (pslot->code == RPC_DBL) {
            if (vals <= 1) {
                pslot->fbig = *((double*)pdata);
            } else {
                pslot->pvec = (void*)pdata;
                pslot->nvec = (vals > 0) ? vals : (bytes >> 3);
            }

        } else if (pslot->code == RPC_I64) {
            if (vals <= 1) {
                pslot->ibig = *((qint*)pdata);
            } else {
                pslot->pvec = (void*)pdata;
                pslot->nvec = (vals > 0) ? vals : (bytes >> 3);
            }

        } else if (pslot->code == RPC_BLB) {
            pslot->pvec = (void*)pdata;
            pslot->nvec = (vals > 0) ? vals : bytes;

        } else if (pslot->code == RPC_ROW) {
            pslot->inum = *((lint*)pdata);      //row index
            pslot->nvec = vals;                 //optional number of rows

        } else {
            return(ECSYNTAX);
        }

        pdata += (bytes >> 2);                  //advance over slot in block
        ndone += 1;
        pslot += 1;
        slots -= 1;
    }

    return(ndone);
}

//*************************************************************************
//  RpcRingSet:
//      Initialize or release ARing FIFO for aint handle messages.
//
//  Arguments:
//      int RpcRingSet (int mode, ARing* pfifo);
//      mode    0 = Intialize pfifo
//              1 = Free pfifo
//      pfifo   Caller allocated ring buffer object to use
//  
//  Returns:
//      Normally returns zero.
//*************************************************************************

CSTYLE
int RpcRingSet (int mode, ARing* pfifo) {
    int index;

    if (mode & 1) {
        OSLock(OS_LOCKOFF, &pfifo->lock);       //free mutex if any
        pfifo->lock = NULL;
        index = 0;
        do {
            pfifo->fifo[index] = -1;
        } while (++index < RBA_MAX);
        OSMemClear(pfifo->fifo, RBA_MAX * sizeof(aint));
        pfifo->iput = 0;
        pfifo->iget = 0;
        pfifo->task = NULL;
        pfifo->chan = NULL;
        pfifo->pipe = SOCK_ERR;
        return(0);
    }
    OSLock(OS_LOCKNEW, &pfifo->lock);           //create mutex
    index = 0;
    do {
        pfifo->fifo[index] = -1;                //intialize to -1 non-handle
    } while (++index < RBA_MAX);
    pfifo->iput = 0;
    pfifo->iget = 0;
    pfifo->task = NULL;
    pfifo->chan = NULL;
    pfifo->pipe = SOCK_ERR;
    return(0);
}

//*************************************************************************
//  RpcRingPut:
//      Puts an aint handle message from FIFO.
//      Uses allocated memory the caller gives to FIFO.
//
//  Arguments:
//      void RpcRingPut (int mode, ARing* pfifo, aint handle) {
//      mode    Reserved for future use and must be zero
//      pfifo   Ring buffer object initialized with RpcRingSet
//      handle  Handle aint to add to FIFO.
//
//  Returns:
//      Returns zero if worked.
//      Returns ECNOROOM if FIFO is full.
//*************************************************************************

CSTYLE
int RpcRingPut (int mode, ARing* pfifo, aint handle) {
    dword index;

    OSLock_GRAB(pfifo->lock);                   //=== Grab mutex
    index = pfifo->iput + 1;                    //preview index after this put
    if (index >= RBA_MAX) {                     //to see if we have room
        index = 0;
    }
    if (index == pfifo->iget) {
        OSLock_DONE(pfifo->lock)
        return(ECNOROOM);                       //fifo is full?
    }
    pfifo->fifo[ pfifo->iput ] = handle;        //install block
    pfifo->iput = index;                        //advance next slot ptr
    OSLock_DONE(pfifo->lock);                   //=== Release mutex
    return(0);
}

//*************************************************************************
//  RpcRingGet:
//      Gets an aint handle message from FIFO.
//      Gets allocated memory the caller must free.
//
//  Arguments:
//      aint RpcRingGet (int mode, ARing* pfifo) {
//      mode    Reserved for future use and must be zero
//      pfifo   Ring buffer object initialized with RpcRingSet
//
//  Returns:
//      Next aint handle retrieved from the FIFO.
//      Returns -1 if there is nothing on the FIFO.
//*************************************************************************

CSTYLE
aint RpcRingGet (int mode, ARing* pfifo) {
    aint handle;
    dword index;

    OSLock_GRAB(pfifo->lock);                   //=== Grab mutex
    index = pfifo->iget;
    if (index == pfifo->iput) {
        OSLock_DONE(pfifo->lock)
        return(-1);                             //fifo is empty?
    }
    handle = pfifo->fifo [ index ];             //return next tail slot ptr
    pfifo->fifo[ index ] = NULL;                //buffer no longer owns pointer
    index += 1;
    if (index >= RBA_MAX) {
        index = 0;                              //ring wraps around
    }
    pfifo->iget = index;                        //advance next get pointer
    OSLock_DONE(pfifo->lock);                   //=== Release mutex
    return(handle);                             //return next tail slot ptr
}

//*************************************************************************
//  RpcFifoSet:
//      Initialize or release BRing FIFO for RBlok messages.
//
//  Arguments:
//      int RpcFifoSet (int mode, BRing* pfifo);
//      mode    0 = Intialize pfifo
//              1 = Free pfifo
//              3 = Free pfifo and all message memory 
//      pfifo   Caller allocated ring buffer object to use
//  
//  Returns:
//      Normally returns zero.
//*************************************************************************

CSTYLE
int RpcFifoSet (int mode, BRing* pfifo) {
    int index;

    if (mode & 1) {
        OSLock(OS_LOCKOFF, &pfifo->lock);       //free mutex if any
        pfifo->lock = NULL;
        if (mode & 2) {                         //free message memory?
            index = 0;
            do {
                if (pfifo->fifo[index]) {
                    OSFree(pfifo->fifo[index]);
                }
            } while (++index < RBR_MAX);
        }
        OSMemClear(pfifo->fifo, RBR_MAX * sizeof(RBlok*));
        pfifo->iput = 0;
        pfifo->iget = 0;
        pfifo->task = NULL;
        pfifo->chan = NULL;
        pfifo->pipe = SOCK_ERR;
        return(0);
    }
    OSLock(OS_LOCKNEW, &pfifo->lock);           //create mutex
    OSMemClear(pfifo->fifo, RBR_MAX * sizeof(RBlok*));
    pfifo->iput = 0;
    pfifo->iget = 0;
    pfifo->task = NULL;
    pfifo->chan = NULL;
    pfifo->pipe = SOCK_ERR;
    return(0);
}

//*************************************************************************
//  RpcFifoPut:
//      Puts a RBlok message from FIFO.
//      Uses allocated memory the caller gives to FIFO.
//
//  Arguments:
//      void RpcFifoPut (int mode, BRing* pfifo, RBlok* pblok) {
//      mode    Reserved for future use and must be zero
//      pfifo   Ring buffer object initialized with RpcFifoSet
//      pblok   Allocated block to add to FIFO.
//
//  Returns:
//      Returns zero if worked.
//      Returns ECNOROOM if FIFO is full.
//*************************************************************************

CSTYLE
int RpcFifoPut (int mode, BRing* pfifo, RBlok* pblok) {
    dword index;

    OSLock_GRAB(pfifo->lock);                   //=== Grab mutex
    index = pfifo->iput + 1;                    //preview index after this put
    if (index >= RBR_MAX) {                     //to see if we have room
        index = 0;
    }
    if (index == pfifo->iget) {
        OSLock_DONE(pfifo->lock)
        return(ECNOROOM);                       //fifo is full?
    }
    pfifo->fifo[ pfifo->iput ] = pblok;         //install block
    pfifo->iput = index;                        //advance next slot ptr
    OSLock_DONE(pfifo->lock);                   //=== Release mutex
    return(0);
}

//*************************************************************************
//  RpcFifoGet:
//      Gets a RBlok message from FIFO.
//      Gets allocated memory the caller must free.
//
//  Arguments:
//      RBlok* RpcFifoGet (int mode, BRing* pfifo) {
//      mode    Reserved for future use and must be zero
//      pfifo   Ring buffer object initialized with RpcFifoSet
//
//  Returns:
//      Pointer to next block retrieved from the FIFO.
//      Returns NULL if there is nothing on the FIFO.
//*************************************************************************

CSTYLE
RBlok* RpcFifoGet (int mode, BRing* pfifo) {
    RBlok* pblok;
    dword index;

    OSLock_GRAB(pfifo->lock);                   //=== Grab mutex
    index = pfifo->iget;
    if (index == pfifo->iput) {
        OSLock_DONE(pfifo->lock)
        return(NULL);                           //fifo is empty?
    }
    pblok = pfifo->fifo [ index ];              //return next tail slot ptr
    pfifo->fifo[ index ] = NULL;                //buffer no longer owns pointer
    index += 1;
    if (index >= RBR_MAX) {
        index = 0;                              //ring wraps around
    }
    pfifo->iget = index;                        //advance next get pointer
    OSLock_DONE(pfifo->lock);                   //=== Release mutex
    return(pblok);                              //return next tail slot ptr
}

//*************************************************************************
//  RpcSendQ:
//      Sends an minimal RPC block with both dword message and qint value.
//      Sends without slot data.
//
//  Arguments:
//      int RpcSendQ (dword dval, qint qval, aint sock, int timeout)
//      dval    32-bit value sent as iproc message
//      qval    64-bit value sent in hpipe part of minimal message
//              This is usually an error value returned to client
//              This value is lost if sent to server where ipipe is used
//              RP_TEST messages sent to server return this from accept task
//              nuless it is zero in case fifo sends to main task for handling
//      mode    The iproc message to send if pblok is NULL
//      sock    RpcConnect handle for pipe message to receive
//      timeout Maximum milliseconds to wait or 0 for default
//
//  Returns:
//      Normally returns byte size of block sent.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int RpcSendQ (dword dval, qint qval, aint sock, int timeout) {
    RBlok quick;

    quick.bytes = sizeof(RBlok);
    quick.check = RPC_CHECK;
    quick.slots = 0;                            //no slots follow
    quick.iproc = dval;                         //message to send
    quick.hpipe = qval;                         //extra value sent to client
    return( RpcSend(0, sock, &quick, timeout) );
}

//*************************************************************************
//  RpcSend:
//      Sends an entire RPC block.
//      Can also and send a quick iproc message without slot data.
//
//  Arguments:
//      int RpcSend (int mode, aint sock, RBlok* pblok, int timeout)
//      mode    The iproc message to send if pblok is NULL
//      sock    RpcConnect handle for pipe message to receive
//      pblok   RPC block to be sent
//              Should free on return if allocated and no longer need
//              May be NULL to send quick RBlok mode message without data
//      timeout Maximum milliseconds to wait or 0 for default
//
//  Returns:
//      Normally returns byte size of block sent.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int RpcSend (int mode, aint sock, RBlok* pblok, int timeout) {
    RBlok quick;
    byte* pbyte;
    dword left, done, time;
    int rv;

    if (pblok == NULL) {                        //allocate quick message?
        pblok = &quick;
        pblok->bytes = sizeof(RBlok);
        pblok->check = RPC_CHECK;
        pblok->iproc = mode & RP_MASK;          //message to send
        pblok->slots = 0;
    }

    if (timeout <= 0) timeout = RP_TIME;
    time = OSTickCount();
    left = pblok->bytes;
    done = 0;
    pbyte = (byte*) pblok;
    do {
        rv = RpcConSend(sock, pbyte, left);     //try to write rest
        if (rv < 0) return(ECCOMM);
        if (rv > left) return(ECBADCODE);       //should never happen
        done += rv;
        left -= rv;
        if (left <= 0) {                        //finished sending block?
            break;
        }
        pbyte += rv;
        if ((OSTickCount() - time) > timeout) {
            return(ECTIMEOUT);
        }
        OSSleep(0);
    } while (left > 0);
    return(done);
}

//*************************************************************************
//  RpcRecv:
//      Receives and returns an entire RPC block into allocated memory.
//
//  Arguments:
//      RBlok* RpcRecv (int mode, aint sock, int timeout)
//      mode    Reserved for future use and must be zero
//      sock    RpcConnect handle for pipe message to receive
//      timeout Maximum milliseconds to wait or 0 for default
//              Suggest default 1000 (1 sec) to receive processing message.
//              Suggest non-default 10000 (10 sec) to receive call result.
//
//  Returns:
//      Returns RPC block memory which caller owns and must free.
//      Returns NULL if error or timeout.
//*************************************************************************

RBlok* RpcRecv (int mode, aint sock, int timeout) {
    RBlok* pblok;
    byte* pbyte;
    dword start[2];
    dword left, time;
    int rv;

    if (timeout <= 0) timeout = RP_TIME;
    time = OSTickCount();
    left = 2 *sizeof(dword);                    //read block size and check val
    pbyte = (byte*) start;
    do {                                        //wait for first four byte size
        rv = RpcConRecv(sock, NULL, 0);         //peek available bytes
        if (rv == 0) {                          //nothing available to read?
            if ((OSTickCount() - time) > timeout) {
                return(NULL);                   //waited too long (timeout)?
            }
            OSSleep(0);
            continue;
        }
        rv = RpcConRecv(sock, pbyte, left);     //try to read rest
        if (rv > left) return(NULL);            //should never happen
        if (rv < 0) break;
        left -= rv;
        pbyte += rv;
    } while (left > 0);
    if ((start[0] == 0)||(start[1] != RPC_CHECK)) {
        return(NULL);                           //not valid RBlok?
    }

    pblok = (RBlok*) OSAlloc(start[0]);         //allocate RPC block
    if (pblok == NULL) return(NULL);

    pblok->bytes = start[0];                    //fill in byte size
    pblok->check = start[1];                    //and check value
    pbyte = ((byte*) pblok) + (2 * sizeof(dword));
    left = start[0] - (2 * sizeof(dword));      //remaining block bytes to read
    do {                                        //read rest of block
        rv = RpcConRecv(sock, NULL, 0);         //peek available bytes
        if (rv == 0) {                          //nothing available to read?
            if ((OSTickCount() - time) > timeout) {
                break;                          //waited too long (timeout)?
            }
            OSSleep(0);
            continue;
        }
        rv = RpcConRecv(sock, pbyte, left);     //try to read rest
        if (rv > left) break;                   //should not happen
        if (rv < 0) rv = 0;
        left -= rv;
        if (left <= 0) {                        //finished receiving block?
            break;
        }
        pbyte += rv;
        if ((OSTickCount() - time) > timeout) {
            break;
        }
        OSSleep(0);
    } while (left > 0);

    if (left != 0) {                            //error receiving?
        OSFree(pblok);
        return(NULL);
    }
    return(pblok);                              //return received block
}

//*************************************************************************
//  RpcServTask:
//      Task function used by server to receive call requests quickly and
//      send them via a BRing FIFO to the main server process for handling.
//      Using this as a separate task allows quick handling of call messages
//      through the pipe so RPC caller can use a short timeout before
//      starting the server program if needed. Normally only a single
//      server task (in the server's main process) handles CPS search
//      because each search uses multi tasking to be as quick as possible
//      with available resources and more server tasks would not help CPS.
//      Normally this is called via OSTask and runs in a separate task.
//
//      Clients can send special RP_DONE=254 message to close their socket.
//      ServTask passes to main task so it can stop using the socket and closes.
//      Clients can also send RP_STOP=255 message to shut down task on exit.
//      ServTask passes to main task so it can exit application and stops tasks.
//      These special messages are just the RBlok header without a data block.
//
//  Arguments:
//      int RpcServTask (BRing* pfifo)
//      pfifo   Used to pass received call messages back to main server task.
//              Must set pfifo->chan to SChan server information.
//              Must set pfifo->pipe to SOCK_SERVER.
//
//  Returns:
//      Negative code if error during task.
//*************************************************************************

OSTASK_RET RpcAccept (ARing* pfifo) {           //RpcAccept: task of ServTask
    char svname[CC_SIZ];
    SChan* pin;
    aint hpipe, sock;
    int rv, svport, svtype;

    if (pfifo == NULL) return(ECBADCODE);
    hpipe = pfifo->pipe;

    svname[0] = 0;
    pin = (SChan*) pfifo->chan;
    if (pin) {
        OSTxtCopy(svname, pin->server);
        svport = pin->svport;
        svtype = pin->svtype;
    }
    if (svname[0] == 0) {                       //did not receive server info?
        RpcAddress(0, svname, &svport, &svtype);//get CPS.set server info again
    }

    #ifdef DEB_SOCK                             //===
    printf("** RpcAccept task starting...\n");
    #endif                                      //===

    while (TRUE) {                              //loop forever
        sock = RpcConnect(SOCK_ACCEPT+svtype, hpipe, svname, svport, NULL);
        if (sock == SOCK_ERR) {
            #ifdef DEB_SOCK                     //===
            printf("!! RpcAccept socket accept error\n");
            #endif                              //===
            continue;
        }
        #ifdef DEB_MORE                         //===
        printf("** RpcAccept accepted new client\n");
        #endif                                  //===

        rv = RpcRingPut(0, pfifo, sock);        //send accept socket to ServTask
        #ifdef DEB_SOCK                         //===
        if (rv < 0) printf("!! RpcAccept accept fifo full %i\n", rv);
        #endif                                  //===
    }
    return(0);                                  //must in case
}

OSTASK_RET RpcServTask (BRing* pfifo) {         //new task starts here
    aint sockets[RP_SOCKS];                     //accepted sockets (or -1)
    ARing accept;                               //accept fifo
    RBlok* pblok;
    void* haccept;
    aint hpipe, sock;
    int index, msg, rv;

    if (pfifo == NULL) return(ECBADCODE);
    hpipe = pfifo->pipe;

    #ifdef DEB_SOCK                             //===
    printf("** RpcServTask starting...\n");
    #endif                                      //===

    index = 0;
    do {
        sockets[index] = SOCK_ERR;              //set all sock handles to -1
    } while (++index < RP_SOCKS);

    RpcRingSet(0, &accept);                     //setup fifo to receive accepts
    accept.chan = pfifo->chan;     
    accept.pipe = hpipe; 
    rv = OSTask(OS_TASKNEW, &haccept, (LPFTASK)RpcAccept, &accept);
    if (rv < 0) {
        #ifdef DEB_SOCK                         //===
        printf("!! RpcServTask accept task fatal start error %i\n", rv);
        #endif                                  //===
        return(ECNULL);
    }
    accept.task = haccept;                      //remember accept task handle
    pfifo->task = haccept;                      //and give to main task too

    index = -1;
    while (TRUE) {                              //loop forever
        index += 1;                             //try another socket
        if (index >= RP_SOCKS) index = 0;       //wrap around if needed

        sock = RpcRingGet(0, &accept);          //new accept socket?
        if (sock != SOCK_ERR) {
            index = 0;
            do {                                //find free socket index
                if (sockets[index] == SOCK_ERR) break;
            } while (++index < RP_SOCKS);
            if (index >= RP_SOCKS) {
                RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
                #ifdef DEB_SOCK                 //===
                printf("!! RpcServTask too many accepted sockets\n");
                #endif                          //===
                continue;
            }
            sockets[index] = sock;              //remember new accepted socket
            #ifdef DEB_BEST                     //===
            printf("** RpcServer new accepted socket=%i index=%i\n",(int)sock,index);
            #endif                              //===
        }                                       //and switch to it

        rv = RP_SOCKS;
        while (sockets[index] == SOCK_ERR) {    //find accepted socket handle
            if (--rv < 0) break;                //only once around if nothing
            index += 1;                         //try another index
            if (index >= RP_SOCKS) index = 0;   //wrap around sockets buffer
        }
        if (sockets[index] == SOCK_ERR) {       //no active accept sockets?
            OSSleep(0);                         //allow other tasks to execute
            continue;                           //and try again
        }

        sock = sockets[index];                  //check current socket
        rv = RpcConRecv(sock, NULL, 0);         //peek available bytes
        if (rv != 0) {                          //something to read (or error)?
            pblok = RpcRecv(0, sock, 0);        //wait for call message
            if (pblok == NULL) {                //timeout?
                #ifdef DEB_SOCK                 //===
                printf("!! RpcServTask socket %i read timeout\n",index);
                #endif                          //===
                RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
                sockets[index] = SOCK_ERR;
                continue;
            }
            #ifdef DEB_MORE                     //===
            printf("** RpcServTask socket %i received call 0x%X\n",index,pblok->iproc);
            #endif                              //===

            if (pblok->iproc == RP_DONE) {      //socket done message?
                #ifdef DEB_MORE                 //===
                printf("** RpcServer socket index %i closing (RP_DONE)\n",index);
                #endif                          //===
                RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
                sockets[index] = SOCK_ERR;
                continue;
            }

            if ((pblok->iproc == RP_TEST)&&(pblok->hpipe)) {
                rv = RpcSendQ(pblok->iproc, pblok->hpipe, sock, 0);
                #ifdef DEB_MORE                 //===
                printf("** RpcServTask RP_TEST sent: 0x%X\n",(dword)pblok->hpipe);
                #endif                          //===
                continue;
            }

            msg = (pblok->iproc == RP_STOP) ? RP_STOP : RP_PROC;
            if (pfifo->flag & SCF_PROC) {       //send extra processing message?
                rv = RpcSend(msg,sock,NULL,0);  //send waiting to process reply
                if (rv < 0) {
                    #ifdef DEB_SOCK             //===
                    printf("!! RpcServTask socket %i RP_PROC reply send error\n",index);
                    #endif                      /===
                    RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
                    sockets[index] = SOCK_ERR;
                    continue;
                }
            }

            pblok->hpipe = sock;                //pass socket handle to main task
            rv = RpcFifoPut(0, pfifo, pblok);   //send block to main server task
            if (rv < 0) {
                #ifdef DEB_SOCK                 //===
                printf("!! RpcServTask socket %i FIFO full\n",index);
                #endif                          //===
                OSFree(pblok);
                RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
                sockets[index] = SOCK_ERR;
                continue;
            }

            // Not a good idea - Main task should handle the RP_STOP from fifo.
            // Causes the main task to never get RP_STOP and never exit app.
            // if (pblok->iproc == RP_STOP) {   //stop tasks message? 
            //     #ifdef DEB_SOCK              //===
            //     printf("** RpcServTask accept task is stopping (RP_STOP)\n");
            //     #endif                       //===
            //     OSTask(OS_TASKEND, &haccept, NULL, NULL);
            //     return(0);                   //kill RpcAccept task and exit
            // }

            pblok = NULL;                       //no longer own received block
            sock = SOCK_ERR;                    //no longer own accept socket
        }
    }
    return(-1);                                 //just in case
}

//*************************************************************************
//  RpcCall:
//      Client remote procedure call.
//      Starts server if needed to connect.
//      Sents RBlok arguments to server.
//      Receives RBlok return values.
//
//  Arguments:
//      RBlok* RpcCall (int mode, CChan* pchan, int iproc,
//                      RBlok* pargs, RBlok** ppret)
//      mode   +1 = Initialize pchan            (CC_INIT)
//                  Sets pchan->socket=SOCK_ERR (-1)
//                  Sets pchan->server CPS.set  (CPS_S)
//             +2 = Call server                 (CC_CALL)
//             +4 = Peek receive w/o waiting    (CC_PEEK)
//             +8 = Free pargs block after send (CC_FREE)
//            +16 = Close connection after call (CC_CLOSE)
//                  Does not free if error
//              Note that CC_CALLX includes all but CC_PEEK
//              Note that CC_CALLS includes CC_CALL+CC_FREE
//      pchan   Channel information for client
//              May have sstart server executable to start if needed
//              May have timout max wait for server return or 0 for default
//              May be NULL for temporary info and CC_INIT+CC_FREE+CC_CLOSE
//      iproc   Call message code to send
//              May be zero to leave pargs->iproc code
//      pargs   Argument data block to send to server
//              For peek mode must be NULL on subequent calls after sending
//              If not peek may be NULL to just send iproc message
//      ppret   Returns data block of result values
//              Caller must free using RpcFree
//              Returns NULL if error
//              May be NULL for just CC_INIT
//
//  Returns:
//      Normally returns zero.
//      Returns negative code if error.
//      Returns ECSYS if cannot start server executable.
//      Returns ECBUSY if server did not connect soon enough.
//      Returns ECCOMM if server reply not received soon enough.
//      Returns ECARGUMENT if pchan sstart server needed but not given.
//      Returns ECPENDING if CC_PEAK mode and no reply.
//*************************************************************************

CSTYLE
int RpcCall (int mode, CChan* pchan, int iproc, RBlok* pargs, RBlok** ppret) {
    RBlok temps;
    RBlok* pret;
    CChan chans;
    char* pname;
    aint socket;
    dword time;
    int rv, timeout;

    // Use temporary channel info if not given.
    if (pchan == NULL) {
        pchan = &chans;
        mode |= CC_INIT+CC_FREE+CC_CLOSE;
    }

    // Initialize structure?
    // We cannot fill in sstart without
    // allocated memory so leave for RpcStart.
    if (mode & CC_INIT) {
        pchan->socket = SOCK_ERR;               //this is important!
        OSTxtCopy(pchan->server, CS_NAME);      //standard server pipe name
        OSTxtCopy(pchan->sstart, RP_PIPE);      //standard server path name
        pchan->sportn = CS_PORT;                //standard server port number
        pchan->stypen = CS_TYPE;                //standard server is named pipe
        pchan->timout = 0;                      //standard server timeout ms
    }

    // Call server if needed
    if (ppret) *ppret = NULL;                   //in case of error
    pret = NULL;                                //in case just closing
    if (mode & CC_CALL) {                       //call?
        socket = pchan->socket;

        // First connect with server if needed (and start if cannot)
        if (pchan->socket == SOCK_ERR) {
            if (pchan->server[0] == 0) {
                RpcAddress(0, pchan->server, &pchan->sportn, &pchan->stypen);
            }
            socket = RpcConnect(SOCK_CLIENT+pchan->stypen, SOCK_ERR, 
                                pchan->server, pchan->sportn, NULL);
            if (socket == SOCK_ERR) {           //cannot connect?

                // Start named pipe server executable if cannot connect
                OSTxtCopy(pchan->server, RP_NAME);
                pchan->stypen = CS_PIPE;        //force named pipe server
                pchan->sportn = 0;

                rv = RpcStart(RS_ALWAYS, pchan->sstart, pchan->server);
                if (rv < 0) return(ECSYS);

                OSSleep(100);                   //extra safety margin
                time = OSTickCount();           //(can be race if too fast!)
                while (TRUE) {                  //wait for server to start
                    OSSleep(1);                 //0 is too fast!
                    socket = RpcConnect(SOCK_CLIENT+pchan->stypen, SOCK_ERR, 
                                        pchan->server, pchan->sportn, NULL);
                    if (socket != SOCK_ERR) {   //try to connect to new server
                        break;                  //connected?
                    }
                    if ((OSTickCount() - time) >= RP_EXEC) {
                        return(ECBUSY);         //cannot start
                    }
                }
            }                                      
            pchan->socket = socket;             //remember connection socket
        }

        // Send arguments to server and "call" desired proc
        // Handle peek mode differently
        if (mode & CC_PEEK) {                   //peek mode?
            // Send arguments to server and "call" desired proc
            // But skip on subsequent peek calls when pargs not supplied
            if (pargs) {
                if (iproc) pargs->iproc = iproc;//add iproc msg if needed
                rv = RpcSend(0,socket,pargs,0); //send call msg to server
                if (mode & CC_FREE) {
                    OSFree(pargs);              //free args block after send
                }
                if (rv < 0) return(rv);         //send error?
            }

            // Then use peek to check if reply is back yet.
            // Beware that this is not portable to Linux!
            rv = RpcConRecv(socket, NULL, 0);   //peek available bytes
            if (rv < sizeof(RBlok)) {           //not enough bytes yet?
                return(ECPENDING);              //still waiting?
            }
            
        // Without peek allows no data block
        } else {                                //normal call without peek?
            if (pargs == NULL) {
                OSMemClear(&temps, sizeof(RBlok));
                temps.bytes = sizeof(RBlok);    //use just header if needed
                temps.check = RPC_CHECK;
                pargs = &temps;
                mode &= (~CC_FREE);             //must not free below!
            }
            if (iproc) pargs->iproc = iproc;    //add iproc msg if needed
            rv = RpcSend(0, socket, pargs, 0);  //send call msg to server
            if (mode & CC_FREE) {
                OSFree(pargs);                  //free args block after send
            }
            if (rv < 0) return(rv);             //send error?
            if (iproc == RP_STOP) return(ECSTOPPED);
        }

        // Receive the returned message.
        // Even with peek, once we have a minimum message wait here for rest.
        pret = NULL;
        do {
            if (pret) OSFree(pret);             //free last waiting message
            timeout = (pchan->timout) ? pchan->timout : RP_SLOW;
            pret = RpcRecv(0, socket,timeout);  //wait for reply
            if (pret == NULL) {                 //failed?
                RpcClose(0, pchan);             //close connection
                pchan->socket = SOCK_ERR;
                return(ECCOMM);                 //and return error
            }
        } while (pret->iproc == RP_PROC);       //try again if waiting message
    }

    // Close connection if desired
    if (mode & CC_CLOSE) {
        RpcClose(0, pchan);
    }
    
    // Return the results.
    if (ppret) *ppret = pret;
    if ((pret)&&(pret->iproc == RP_ERRS)) {     //received error message?
        rv = (int) pret->hpipe;                 //server's error code is here
        if (rv >= 0) rv = ECERROR;              //or unknown error
        return(rv);
    }
    return(0);
}

//*************************************************************************
//  TxtCall:
//      Client remote procedure call for common CPS arguments.
//      Starts server if needed to connect.
//      Sents RBlok arguments to server.
//      Receives RBlok return values.
//      Creates CP_CALL iproc call.
//
//  Arguments:
//      int TxtCall (int mode, CChan* pchan, int iproc,
//                   char* pname, char* plibs, int smode, 
//                   int tasks, int nkeep, float blend, RBlok** ppret)
//      mode   27 = Normal 1+2+8+26             (CC_CALLX)
//             +1 = Initialize pchan            (CC_INIT)
//             +2 = Call server                 (CC_CALL)
//                  Sends iproc and ppret data
//             +4 = Peek receive w/o waiting    (CC_PEEK)
//                  Use RpcCall after send
//             +8 = Free pargs block after call (CC_FREE)
//                  Does not free if error
//            +16 = Close connection            (CC_CLOSE)
//                  Ignored unless +1 CC_CALL
//      pchan   Channel information for client
//              May have sstart server executable to start if needed
//              May have timout max wait for server return or 0 for default
//              May be NULL for mode 0 CC_PREP
//              May be NULL for temporary info and CC_INIT+CC_FREE+CC_CLOSE
//      iproc   Call message code to send
//              May be zero to use standard CP_CALL code for this args type
//      pname   Call argument for CPS unknown file path name
//      plibs   Call argument for CPS library list to search
//      smode   Call argument for CPS server mode and flags
//      tasks   Call argument for CPS number of tasks (0 for current setting)
//      nkeep   Call argument for CPS number of hits to keep (0 for current)
//      nramb   Call argument for CPS maximum RAM in MB (0 for current setting)
//      blend   Call argument for CPS reverse blend percent fraction (0 for cur)
//      ppret   Returns data block of result values
//              Call TxtFrom to decode this to returned values
//              For mode 0 CC_PREP this is arguments data
//              For mode 1 CC_CALL this is call return
//              Caller must free using RpcFree
//              Returns NULL if error
//
//  Returns:
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int TxtCall (int mode, CChan* pchan, int iproc, 
             text* pname, text* plibs, int smode,
             int tasks, int nkeep, int nramb, float blend, RBlok** ppret) {
    RSlot slots[CC_USED];
    RBlok* pblok;
    RBlok* prets;
    int rv, need;

    *ppret = NULL;                              //in case of error
    if (iproc == 0) iproc = CP_CALL;            //standard call number?

    rv = RpcTxtIni(0, slots,CC_USED, CC_TEXT);  //set up slot typtes TTIIIF
    if (rv != CC_USED) return(ECBADCODE);       //create data slots

    slots[CC_NAME].pvec = pname;                //and supply arguments
    slots[CC_LIBS].pvec = plibs;
    slots[CC_MODE].inum = smode;
    slots[CC_TASK].inum = tasks;
    slots[CC_KEEP].inum = nkeep;
    slots[CC_RAMB].inum = nramb;
    slots[CC_FRAC].fnum = blend;

    need = RpcEncode(iproc, NULL,0, slots,CC_USED);
    if (need < 0) return(need);                 //get needed block bytes
    pblok = (RBlok*) OSAlloc(need);
    if (pblok == NULL) return(ECMEMORY);
    rv = RpcEncode(iproc, pblok,need, slots,CC_USED);
    if (rv != need) {                           //create data block
        OSFree(pblok);
        return((rv < 0) ? rv : ECBADDIMS);
    }

    if (mode & CC_CALL) {
        mode |= CC_FREE;                        //must free pblok
        rv = RpcCall(mode, pchan, iproc, pblok, &prets);
        if (rv < 0) {                           //call server
            if (prets) OSFree(prets);
            return(rv);
        }
        pblok = prets;                          //server returned values
    }

    *ppret = pblok;                             //return data block
    return(0);
}

//*************************************************************************
//  TxtFrom:
//      Receives CPS argument values sent by TxtCall.
//      Returns them in a structure and also returns called proceedure number.
//      Can also free block and values afterward.
//
//  Arguments:
//      int TxtFrom (int mode, CCVal* pvals, RBlok* pblok)
//      mode   +1 = Get pvals from pblok                (CF_VALS)
//             +2 = Free pblok and any allocated pvals  (CF_FREE)
//                  Must free after receiving block
//                  and may not use pvals afterward
//              3 = Get pvals from pblok and free pblok (CF_NORM)
//      pvals   Structure to receive argument values
//              text* name,text* libs,int mode,int tasks,int nkeep,float blend
//              For -1 free mode should be values returned from 0 mode
//      pblok   Provides received TxtCall data block.
//              Must not be freed until finished reading pvals.
//  Returns:
//      Normally pomaximum RAMsitive proceedure code to call.
//      Free pblok normally returns zero.
//      Returns negative code if error.
//      Returns ECNOROOM if file or libs names don't fit in SZNAME buffer.
//*************************************************************************

CSTYLE
int TxtFrom (int mode, CCVal* pvals, RBlok* pblok) {
    RSlot slots[CC_USED];
    char* ptxt;
    int rv, need;

    rv = 0;
    if (mode & CF_VALS) {                       //set up slot typtes TTIIIF
        rv = RpcTxtIni(0, slots,CC_USED, CC_TEXT);
        if (rv != CC_USED) return(ECBADCODE);   //create data slots
        rv = RpcDecode(0,slots,CC_USED, pblok); //decode data block
        if (rv < 0) return(rv);

        ptxt = (char*) slots[CC_NAME].pvec;
        need = OSTxtSize(ptxt);
        if (need > SZFILE) rv = ECNOROOM;
        OSTxtStop(pvals->file, ptxt, SZFILE);

        ptxt = (char*) slots[CC_LIBS].pvec;
        if (need > SZFILE) rv = ECNOROOM;
        OSTxtStop(pvals->libs, ptxt, SZFILE);

        pvals->mode  = slots[CC_MODE].inum;
        pvals->tasks = slots[CC_TASK].inum;
        pvals->nkeep = slots[CC_KEEP].inum;
        pvals->nramb = slots[CC_RAMB].inum;
        pvals->blend = slots[CC_FRAC].fnum;
        if (rv >= 0) {
            rv = pblok->iproc;                  //return proceedure number
        }
    }
    if (mode & CF_FREE) {                       //also free the block
        OSFree(pblok);                          //no pvals
    }
    return(rv);                                 //return proceedure number
}

//*************************************************************************
//  VecCall:
//      Client remote procedure call for common CPS arguments.
//      Unlike TxtCall, this version passes the unknown profile
//      spectrum array instead of a .crn file name.
//      It uses double instead of float for convenience,
//      because most clients deal with double data.
//      Starts server if needed to connect.
//      Sents RBlok arguments to server.
//      Receives RBlok return values.
//      Creates CP_SPEC iproc call.
//
//  Arguments:
//      int VecCall (int mode, CChan* pchan, int iproc,
//                   double* pspc, int nspc, double begx, double endx,
//                   char* plibs, int smode, 
//                   int tasks, int nkeep, int nramb, float blend, RBlok** ppret)
//      mode   27 = Normal 1+2+8+26             (CC_CALLX)
//             +1 = Initialize pchan            (CC_INIT)
//             +2 = Call server                 (CC_CALL)
//                  Sends iproc and ppret data
//             +4 = Peek receive w/o waiting    (CC_PEEK)
//                  Use RpcCall after send
//             +8 = Free pargs block after call (CC_FREE)
//                  Does not free if error
//            +16 = Close connection            (CC_CLOSE)
//                  Ignored unless +1 CC_CALL
//      pchan   Channel information for client
//              May have sstart server executable to start if needed
//              May have timout max wait for server return or 0 for default
//              May be NULL for mode 0 CC_PREP
//              May be NULL for temporary info and CC_INIT+CC_FREE+CC_CLOSE
//      iproc   Call message code to send
//              May be zero to use standard CP_SPEC code for this args type
//      pspc    Float32 array of unknown profile spectrum
//              Need not be normalized or interpolate dot library spacing
//      nspc    Number of spectrum points in pspc
//      begx    Mass of first spectrum point
//      endx    Mass of last spectrum point
//      plibs   Call argument for CPS library list to search
//      smode   Call argument for CPS server mode and flags
//      tasks   Call argument for CPS number of tasks (0 for current setting)
//      nkeep   Call argument for CPS number of hits to keep (0 for current)
//      nramb   Call argument for CPS maximum RAM in MB (0 for current setting)
//      blend   Call argument for CPS reverse blend percent fraction (0 for cur)
//      ppret   Returns data block of result values
//              Call VecFrom to decode this to returned values
//              For mode 0 CC_PREP this is arguments data
//              For mode 1 CC_CALL this is call return
//              Caller must free using RpcFree
//              Returns NULL if error
//
//  Returns:
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int VecCall (int mode, CChan* pchan, int iproc,
             double* pspc, int nspc, double begx, double endx,
             text* plibs, int smode,
             int tasks, int nkeep, int nramb, float blend, RBlok** ppret) {
    RSlot slots[CS_USED];
    RBlok* pblok;
    RBlok* prets;
    int rv, need;

    *ppret = NULL;                              //in case of error
    if (iproc == 0) iproc = CP_SPEC;            //standard call number?

    rv = RpcTxtIni(0, slots,CS_USED, CS_TEXT);  //set up slot typtes DIDDTIIIF
    if (rv != CS_USED) return(ECBADCODE);       //create data slots

    slots[CS_PSPC].pvec = pspc;                 //and supply arguments
    slots[CS_PSPC].nvec = nspc;
    slots[CS_NSPC].inum = nspc;
    slots[CS_BEGX].fbig = begx;
    slots[CS_ENDX].fbig = endx;
    slots[CS_LIBS].pvec = plibs;
    slots[CS_MODE].inum = smode;
    slots[CS_TASK].inum = tasks;
    slots[CS_KEEP].inum = nkeep;
    slots[CS_RAMB].inum = nramb;
    slots[CS_FRAC].fnum = blend;

    need = RpcEncode(iproc, NULL,0, slots,CS_USED);
    if (need < 0) return(need);                 //get needed block bytes
    pblok = (RBlok*) OSAlloc(need);
    if (pblok == NULL) return(ECMEMORY);
    rv = RpcEncode(iproc, pblok,need, slots,CS_USED);
    if (rv != need) {                           //create data block
        OSFree(pblok);
        return((rv < 0) ? rv : ECBADDIMS);
    }

    if (mode & CC_CALL) {
        mode |= CC_FREE;                        //must free pblok
        rv = RpcCall(mode, pchan, iproc, pblok, &prets);
        if (rv < 0) {                           //call server
            if (prets) OSFree(prets);
            return(rv);
        }
        pblok = prets;                          //server returned values
    }

    *ppret = pblok;                             //return data block
    return(0);
}

//*************************************************************************
//  VecFrom:
//      Receives CPS argument values sent by VecCall.
//      Returns them in a structure and also returns called proceedure number.
//      Can also free block and values afterward.
//
//      WARNING:
//      Unlike TxtFrom, the spectrum array is too big to fit in CSVal!
//      You must not use CF_FREE until you have finished using pvals!
//      Using CF_NORM to free during CF_VALS call does not work here!
//
//  Arguments:
//      int VecFrom (int mode, CSVal* pvals, RBlok* pblok)
//      mode    1 = Get pvals from pblok                (CF_VALS)
//              2 = Free pblok and any allocated pvals  (CF_FREE)
//                  Must free after receiving block
//                  and may not use pvals afterward
//      pvals   Structure to receive argument values
//              double* pspc, int npts, double begx, double endx,
//              text* name,text* libs,int mode,int tasks,int nkeep,float blend
//              For -1 free mode should be values returned from 0 mode
//      pblok   Provides received VecCall data block.
//              Must not be freed until finished reading pvals.
//  Returns:
//      Normally positive proceedure code to call.
//      Free pblok normally returns zero.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int VecFrom (int mode, CSVal* pvals, RBlok* pblok) {
    RSlot slots[CS_USED];
    int rv, need;

    if (mode == CF_FREE) {                      //free the block
        OSFree(pblok);                          //no pvals
        return(0);
    }

    rv = RpcTxtIni(0, slots,CS_USED, CS_TEXT);
    if (rv != CS_USED) return(ECBADCODE);       //create data slots
    rv = RpcDecode(0,slots,CS_USED, pblok);     //decode data block
    if (rv < 0) return(rv);
    pvals->pspc  = (double*) slots[CS_PSPC].pvec;
    pvals->nspc  = slots[CS_NSPC].inum;
    pvals->begx  = slots[CS_BEGX].fbig;
    pvals->endx  = slots[CS_ENDX].fbig;
    pvals->libs  = (char*) slots[CS_LIBS].pvec;
    pvals->mode  = slots[CS_MODE].inum;
    pvals->tasks = slots[CS_TASK].inum;
    pvals->nkeep = slots[CS_KEEP].inum;
    pvals->nramb = slots[CS_RAMB].inum;
    pvals->blend = slots[CS_FRAC].fnum;
    return(pblok->iproc);                       //return proceedure number
}

//*************************************************************************
//  SetCall:
//      Client remote procedure call to set or get a parameter value.
//      Starts server if needed to connect.
//      Sents RBlok arguments to server.
//      Receives RBlok return values.
//
//  Arguments:
//      int SetCall (int mode, CChan* pchan, int iproc,
//                   int prm, int get, double val, 
//                   double* psetv, RBlok** ppret)
//      mode    0 = Just prepare ppret data     (CC_PREP)
//             +1 = Call server                 (CC_CALL)
//                  Sends iproc and ppret data
//             +2 = Peek receive w/o waiting    (CC_PEEK)
//                  Use RpcCall after send
//             +4 = Free pargs block after call (CC_FREE)
//                  Does not free if error
//             +8 = Close connection            (CC_CLOSE)
//                  Ignored unless +1 CC_CALL
//              Note that CC_CALLX includes all but CC_PEEK
//      pchan   Channel information for client
//              May have sstart server executable to start if needed
//              May have timout max wait for server return or 0 for default
//              May be NULL for mode 0 CC_PREP
//              May be NULL for temporary info and CC_INIT+CC_FREE+CC_CLOSE
//      iproc   Call message code to send
//              May be zero to use standard CP_SETV code for this args type
//      prm     Parameter number to set or get
//      get     1 = Get current parameter value
//              0 = Set parameter to val and return new value
//      val     Parameter value to set
//              This is ignored if get is 1
//              All values (integer or float) are passed as double
//      psetv   Returns parameter setting after any new val is set
//              May be NULL to just return ppret and use SetFrom separately
//      ppret   Returns data block of result values
//              Call SetFrom to decode this to returned value
//              For mode 0 CC_PREP this is arguments data
//              For mode 1 CC_CALL this is call return
//              Caller must free using RpcFree
//              Returns NULL if error
//              May be NULL if ppval returns the parameter value
//
//  Returns:
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int SetCall (int mode, CChan* pchan, int iproc, 
             int prm, int get, double val, double* psetv, RBlok** ppret) {
    RSlot slots[CV_USED];
    CVVal temps;
    RBlok* pblok;
    RBlok* prets;
    int rv, need;

    if (psetv) *psetv = -999999.999;            //in case of error
    if (ppret) *ppret = NULL;                   //in case of error
    if (iproc == 0) iproc = CP_SETV;            //standard call number?

    rv = RpcTxtIni(0, slots,CV_USED, CV_TEXT);  //set up slot typtes TTIIIF
    if (rv != CV_USED) return(ECBADCODE);       //create data slots

    slots[CV_PRM].inum = prm;                   //and supply arguments
    slots[CV_GET].inum = get;
    slots[CV_VAL].fbig = val;

    need = RpcEncode(iproc, NULL,0, slots,CV_USED);
    if (need < 0) return(need);                 //get needed block bytes
    pblok = (RBlok*) OSAlloc(need);
    if (pblok == NULL) return(ECMEMORY);
    rv = RpcEncode(iproc, pblok,need, slots,CV_USED);
    if (rv != need) {                           //create data block
        OSFree(pblok);
        return((rv < 0) ? rv : ECBADDIMS);
    }

    prets = NULL;
    if (mode & CC_CALL) {
        mode |= CC_FREE;                        //must free pblok
        rv = RpcCall(mode, pchan, iproc, pblok, &prets);
        if (rv < 0) {                           //call server
            if (prets) OSFree(prets);
            return(rv);
        }
        pblok = prets;                          //server returned values
    }

    if ((psetv)&&(prets)) {                     //return value?
        rv = SetFrom(CF_VALS, &temps, prets);   //decode block
        *psetv = temps.val;                     //and return parameter value
    }

    if (ppret) *ppret = pblok;                  //return data block
    else if (prets) OSFree(prets);              //or free if not returned
    return(0);
}

//*************************************************************************
//  SetFrom:
//      Receives CPS argument values sent by SetCall.
//      Returns them in a structure and also returns called proceedure number.
//      Can also free block and values afterward.
//
//  Arguments:
//      int SetFrom (int mode, CVVal* pvals, RBlok* pblok)
//      mode    1 = Get pvals from pblok                (CF_VALS)
//              2 = Free pblok and any allocated pvals  (CF_FREE)
//                  Must free after receiving block
//                  and may not use pvals afterward
//      pvals   Structure to receive argument values
//              int prm, int get, double val
//              For -1 free mode should be values returned from 0 mode
//      pblok   Provides received SetCall data block.
//              Must not be freed until finished reading pvals.
//  Returns:
//      Normally positive proceedure code to call.
//      Free pblok normally returns zero.
//      Returns negative code if error.
//*************************************************************************

CSTYLE
int SetFrom (int mode, CVVal* pvals, RBlok* pblok) {
    RSlot slots[CS_USED];
    int rv, need;

    if (mode == CF_FREE) {                      //free the block
        OSFree(pblok);                          //no pvals
        return(0);
    }

    rv = RpcTxtIni(0, slots,CV_USED, CV_TEXT);
    if (rv != CV_USED) return(ECBADCODE);       //create data slots
    rv = RpcDecode(0,slots,CV_USED, pblok);     //decode data block
    if (rv < 0) return(rv);
    pvals->prm = slots[CV_PRM].inum;
    pvals->get = slots[CV_GET].inum;
    pvals->val = slots[CV_VAL].fbig;
    return(pblok->iproc);                       //return proceedure number
}

//*************************************************************************
//  RpcFree:
//      Frees memory passed between different DLLs.
//      The CPS.dll may use a different memory allocation heap than caller,
//      which means the caller cannot free the returned RBlok memory from
//      CpsCall,SpcCall,HitCall with caller's OSFree. Instead, the caller
//      should free returned RBlok pointer hear in the CPS.dll here.
//
//  Arguments:
//      int RpcFree (int mode, void* pblok)
//      mode    Reserved for future used and must be zero.
//      pblok   Memory to free, normally a RBlok from CpsCall,SpcCall,HitCall.
//
//  Returns:
//      Normally returns zero.
//      Returns ECCANNOT if OSFree failed.
//*************************************************************************

CSTYLE
int RpcFree (int mode, RBlok* pblok) {
    void* ptr = OSFree(pblok);
    return (ptr) ? ECCANNOT : 0;
}

//*************************************************************************
//  RpcConnect:
//      Allocates or frees a socket handle number which
//      Converts a socket handle number into an object
//      which supports Named Pipes, UDP or TCP/IP.
//      See OSConnect and OSipcPipe functions in TriOS.cpp.
//
//  Arguments:
//      aint RpcConnect (int mode, aint hpipe, text* pname, 
//                       int port, ConEx* pex)
//      mode    0 = Close sock handle from modes 1, 2 or 3.        (SOCK_CLOSE)
//              1 = Prepare to accept server connections           (SOCK_SERVER)
//                  Returns handle for future mode 2 calls.
//              2 = Connect as pipe server and return new socket   (SOCK_ACCEPT)
//                  Must have previously called mode 1.
//              3 = Connect as pipe client.                        (SOCK_CLIENT)
//                  Unlike server, no previous call needed.
//             11 = Application startup (not needed for Linux)    (SOCK_APPINIT)
//             12 = Application exiting (not needed for Linux)    (SOCK_APPDONE)
//                  Modes 11 and 12 calls must be balanced.
//                  RpcServer and SOCK_CLIENT auto calls.
//                  Unlike Windows these are not balanced
//                  and 11 does nothing if already used
//                  and 12 does nothing if not needed.
//            +32 = Add for UDP instead of named pipe.                (CS_UDP)
//            +64 = Add for TCP instead of named pipe.                (CS_TCP)
//                  CS_UDP,CS_TCP required for modes 1 and 3.
//                  Uses hpipe information for mode 2.
//       +0x40000 = Add for advanced non-automatic mode               (CS_ADV)
//      hpipe   Handle from prior RpcConnect for modes 0 and 2.
//              Ignored for other modes and should be zero.
//      pname   Pipe name, IP or web address. Ignored for mode 2.
//              May be web domain name for mode 2 UDP/TCP client.
//              Unlike OSConnect pname must also be given for mode 2.
//              For UDP/TCP automatically set to NULL unless CS_ADV.
//              For UDP/TCP this is ignored for mode even if given in mode 1.
//              Also ignored for close, app init and app done 0,11,12 modes.
//      port    Port 1-65535. Normally above 1024, 49152-65535 non-assigned.
//              If zero then automatically assigned default if needed.
//              Required for mode 3, optional for mode 1, 0 for others.
//              Not used for Named Pipes. See OSConnect for more information.
//              For UDP/TCP this is ignored for mode 2 because given in mode 1.
//              Also ignored for close, app init and app done 0,11,12 modes.
//      pex     Extra information or use NULL for defaults.
//              If pex supplied then pex->error returns error.
//              Ignored for Named Pipes. See OSConnect for more information.
//  Returns:
//      Positive handle for future RpcConnect,RpcConSend,RpcConRecv calls.
//      Always returns zero for close, app init, app done 0,11,12 modes.
//      Returns -1 (SOCK_ERR) if error.
//*************************************************************************

#define RPC_HANDS 32                            //Available connect handles
CSock   s_conn[RPC_HANDS];                      //Connect Objects for handles
int     s_used[RPC_HANDS] = {0};                //Non-zero if in use
int     s_init = FALSE;                         //Non-zero if WSAStartup

CSTYLE
aint RpcConnect (int mode, aint hpipe, text* pname, int port, ConEx* pex) {
    CSock* po;
    CSock* pn;
    aint hnew, sock;
    int smode;

    smode = mode;
    mode &= CS_MASK;

    // Get object for old handle number.
    po = NULL;
    if (hpipe > 0) {
        hpipe -= 1;
        if (s_used[hpipe] != TRUE) return(ECHANDLE);
        po = &s_conn[hpipe];                    //get object info from handle
        hpipe += 1;
    }

    // Close socket.
    if (mode == SOCK_CLOSE) {
        if (po == NULL) return(ECNOINIT);
        if (po->socket == SOCK_NOT) {
            s_used[hpipe-1] = FALSE;            //unused accept fake socket?
            return(0);
        }
        if (po->sotype & CS_NET) {              //UDP or TCP?
            sock = OSConnect(SOCK_CLOSE, po->socket, NULL,0,NULL);
        } else {
            sock = OSipcPipe(SOCK_CLOSE, po->socket, NULL);
        }
        s_used[hpipe-1] = FALSE;                //free the handle
        return(sock);
    }

    // Initialize or free for Windows connect.
    if (mode >= SOCK_APPINIT) {                 //infrequent calls?
        if (mode == SOCK_APPINIT) {
            if (s_init == TRUE) return(SOCK_ERR);
            sock = OSConnect(SOCK_APPINIT,0,NULL,0,NULL);
            s_init = TRUE;
            return(sock);
        }
        if (mode == SOCK_APPDONE) {
            if (s_init == FALSE) return(SOCK_ERR);
            sock = OSConnect(SOCK_APPDONE,0,NULL,0,NULL);
            s_init = FALSE;
            return(sock);
        }
        return(ECARGUMENT);
    }

    // Allocate new socket handle.
    hnew = 0;
    while (hnew < RPC_HANDS) {
        if (s_used[hnew] == 0) break;           //find free handle
        hnew += 1;
    }
    if (hnew >= RPC_HANDS) return(ECBUSY);      //no free handles?
    pn = &s_conn[hnew];                         //corresponding object
    s_used[hnew] = TRUE;                        //mark in use
    hnew += 1;                                  //avoid zero handle number

    OSMemClear(pn, sizeof(CSock));              //mostly initial zeros suffice
    pn->socket = SOCK_ERR;
    pn->sotype = smode & CS_NET;
    pn->soport = port;
    if (pex) OSMemCopy(&pn->sextra, pex, sizeof(ConEx));

    // Create a new server.
    if (mode == SOCK_SERVER) {
        if (smode & CS_NET) {                   //UDP or TCP?
            if (!(smode & CS_ADV)) {
                pname = NULL;                   //address not for TCP/UDP here
            }
            if (s_init == FALSE) {              //init if needed
                RpcConnect(SOCK_APPINIT, SOCK_ERR, NULL,0,NULL);
            }
            sock = OSConnect(SOCK_SERVER, SOCK_ERR, pname, port, &pn->sextra);
            if (pex) pex->error = pn->sextra.error;
        } else {
            sock = OSipcPipe(SOCK_SERVER, SOCK_ERR, pname);
        }
        if (sock == SOCK_ERR) {                 //error? (SOCK_NOT -3 is OK!)
            s_used[hnew-1] = FALSE;             //free our new handle
            return(sock);
        }
        pn->socket = sock;                      //remember new low-level socket
        return(hnew);                           //but return our socket handle
    }

    // Accept a new server socket.
    if (mode == SOCK_ACCEPT) {
        if (po == NULL) return(ECNOINIT);
        pn->sotype = po->sotype;                //copy server info
        pn->soport = po->soport;
        if (po->sotype & CS_NET) {              //UDP or TCP?
            sock = OSConnect(SOCK_ACCEPT,po->socket,pname, po->soport,&pn->sextra);
            if (pex) pex->error = pn->sextra.error;
        } else {
            sock = OSipcPipe(SOCK_ACCEPT,po->socket,pname);
        }
        if (sock == SOCK_ERR) {                 //error? (SOCK_NOT -3 is OK!)
            s_used[hnew-1] = FALSE;             //free our new handle
            return(sock);
        }
        pn->socket = sock;                      //remember new low-level socket
        return(hnew);                           //but return our socket handle
    }

    // Connect a new client.
    if (mode == SOCK_CLIENT) {
        if (smode & CS_NET) {                   //UDP or TCP?
            if (s_init == FALSE) {              //init if needed
                RpcConnect(SOCK_APPINIT, SOCK_ERR, NULL,0,NULL);
            }
            sock = OSConnect(SOCK_CLIENT, SOCK_ERR, pname, port, &pn->sextra);
            if (pex) pex->error = pn->sextra.error;
        } else {
            sock = OSipcPipe(SOCK_CLIENT, SOCK_ERR, pname);
        }
        if (sock == SOCK_ERR) {                 //error? (SOCK_NOT -3 is OK!)
            s_used[hnew-1] = FALSE;             //free our new handle
            return(sock);
        }
        pn->socket = sock;                      //remember new low-level socket
        return(hnew);                           //but return our socket handle
    }

    return(ECARGUMENT);                         //invalide mode
}

//*************************************************************************
//  RpcConSend:
//      Sends RpcConnect socket data.
//
//  Arguments:
//      int RpcConSend (aint hpipe, void* pv, dword cb)
//      hpipe   Handle from prior RpcConnect.
//      pv      Pointer to memory buffer to write from.
//      cb      Number of bytes to write.
//      
//  Returns:
//      Number of bytes sent or negative code if error.
//*************************************************************************

CSTYLE
int RpcConSend (aint hpipe, void* pv, dword cb) {
    CSock* po;
    int rv;

    // Get object for the handle number.
    if (hpipe <= 0) return(ECNOINIT);
    hpipe -= 1;
    if (s_used[hpipe] != TRUE) return(ECHANDLE);
    po = &s_conn[hpipe];                        //get object info from handle

    // Send the data.
    if (po->sotype & CS_NET) {                  //UDP or TCP?
        rv = OSConSend(po->socket, pv, cb, &po->sextra);
    } else {
        rv = OSipcSend(po->socket, pv, cb);
    }
    return(rv);
}

//*************************************************************************
//  RpcConRecv:
//      Receives RpcConnect socket data.
//      Can also check available bytes to be read without reading (peek).
//      But this peek operation is available only in Windows!
//
//  Arguments:
//      int RpcConRecv (aint hpipe, void* pv, dword cb)
//      hpipe   Handle from prior RpcConnect.
//      pv      Pointer to memory buffer to read to.
//              Use NULL for Windows peek when cb is zero.
//      cb      Number of bytes to read.
//              May be 0 in Windows to peek available bytes without reading.
//      
//  Returns:
//      Number of bytes read (or available if cp is 0) or -1 if error.
//      The peak mode may report first 32 bytes available even if more,
//      because TCP/UDP peek must be given a read size and we use 32,
//      which is enough to read the RBlok size so can read the rest.
//*************************************************************************

#define CR_PEEK 32                              //max bytes to peek

CSTYLE
int RpcConRecv (aint hpipe, void* pv, dword cb) {
    CSock* po;
    byte peek[CR_PEEK];
    lint oldflag;
    int rv;

    // Get object for the handle number.
    if (hpipe <= 0) return(ECNOINIT);
    hpipe -= 1;
    if (s_used[hpipe] != TRUE) return(ECHANDLE);
    po = &s_conn[hpipe];                        //get object info from handle

    // Receive data.
    if (po->sotype & CS_NET) {                  //UDP or TCP?
        if ((cb == 0)&&(pv == NULL)) {          //peek?
            oldflag = po->sextra.conflag;
            po->sextra.conflag |= CI_PEEK;      //force peek mode
            rv = OSConRecv(po->socket, peek, CR_PEEK, &po->sextra);
            po->sextra.conflag = oldflag;       //preserve old flag
            return(rv);                         //return available bytes
        }
        rv = OSConRecv(po->socket, pv, cb, &po->sextra);
    } else {
        rv = OSipcRecv(po->socket, pv, cb);
    }
    return(rv);
}

//*************************************************************************
//  RpcAddress:
//      Looks up address information for connecting to a server.
//      Checks CernoPS\CPS.set text file for "S=<name>,<port>,<type>"
//      where <name> can be local pipe name or IP4 or URL
//            <port> can be 1-6535 port number (normally 49152-65535)
//            <type> can be 0 for local named pipe, 32=UDP, 64=TCP
//
//  Arguments:
//      void RpcAddress (int mode, char* pname, int* pport, int* ptype)
//      mode    Reserved for future use and must be zero.
//      pname   Returns server name in CC_SIZ buffer.
//      pport   Returns server port number.
//      ptype   Returns 0 for named pipe, 32 for UDP, 64 for TCP.
//
//  Returns:
//      Nothing because returns local named pipe server if error.
//*************************************************************************

text    s_name[CC_SIZ];                         //server address name
int     s_port = -1;                            //server port (-1 unknown) 
int     s_type = -1;                            //server type

CSTYLE
void RpcAddress (int mode, char* pname, int* pport, int* ptype) {
    char buff[SZDISK];
    char* ptxt;
    char* puse;
    aint file;
    char cc;
    int rv;

    if (s_port != -1) {                         //already read CPS.set?
        OSTxtCopy(pname, s_name);               //use static info
        *pport = s_port;                        //to avoid re-reading
        *ptype = s_type;
        return;
    }

    OSTxtCopy(RpcPath(RPP_DATA, buff), "CPS.set");
    file = OSOpen(OS_READ, buff);               //open CPS.set file
    buff[0] = 0;                                //assume no CPS.set file
    if (file >= 0) {
        rv = OSRead(file, buff, SZDISK);        //read whole file
        OSClose(file);
        if (rv >= 0) {
            buff[rv] = 0;                       //zero terminate text
        }
    }
    if ((buff[0] == 'S')&&(buff[1] == '=')) {   //valid file?
        ptxt = &buff[2];
        while (*ptxt == ' ') ptxt += 1;
        puse = pname;
        rv = CC_SIZ-1;
        while (*ptxt != ',') {                  //size limit
            if (--rv < 0) break;                //too big?
            *puse++ = *ptxt++;                  //copy name to caller's buffer
        }
        if (rv >= 0) {                          //name fits?
            *puse = 0;
            if (*ptxt == ',') ptxt += 1;        //skip over comma
            *pport = OSNumGet(ptxt, &ptxt);     //copy port number to caller
            while (*ptxt == ' ') ptxt += 1;     //skip trailing blanks
            if (*ptxt == ',') ptxt += 1;        //skip over comma
            *ptype = OSNumGet(ptxt, &ptxt) & CS_NET;
         
            OSTxtCopy(s_name, pname);           //also copy to our static copy
            s_port = *pport;
            s_type = *ptype;

            #ifdef DEB_SOCK                     //===
            printf("** RpcAddress %s port=%i, type=%i\n", s_name,s_port,s_type);
            #endif                              //===
            return;                             //copy CS_UDP/CS_TCP type number
        }
    }
    OSTxtCopy(pname, RP_NAME);                  //if error, use local named pipe
    *pport = 0;
    *ptype = 0;

    OSTxtCopy(s_name, RP_NAME);                 //also copy to our static copy
    s_port = 0;
    s_type = 0;
    return;
}

//*************************************************************************
//  RpcServer:
//      Server helper calls for various functions.
//      Sets up accept task with fifo to send call data to main task.
//      Call from main task to get and decode call data.
//      Call from main task to send call return data.
//      Automatically handles close requests.
//
//  Arguments:
//      int RpcServer (int mode, SChan* pin, 
//                     RSlot* slots, int nslot, char* pform, RBlok** ppblok)
//      mode    0 = Create server socket and start accept task      CS_INIT
//              1 = Receive next call to return *ppblok and iproc   CS_NEXT
//              2 = Decode *ppblok call data with slots,nslot,form  CS_ARGS
//              3 = Send *ppblok call return values from *ppblok    CS_RETV
//              4 = Free *ppblok call data                          CS_FREE
//                  Then call mode 1 again for another message
//            +32 = Add for UDP socket instead of named pipe        CS_UDP
//            +64 = Add for TCP socket instead of named pipe        CS_TCP
//             -1 = Exit server application (kill accept task)      CS_EXIT
//      pin     Server information
//              Clear and set server name and timout for CS_INIT
//              Must NOT set non-standard or non-pipe server
//              in pin->server,pin->svport,pin->svtype
//              because accept task must read from CPS.set!
//      slots   Caller provided slot structures to receive data
//              For mode 4 provides slots data to free or NULL
//              May be NULL if not needed for modes 2 or 4
//      nslot   Maximum number of slots available
//              Should be 0 if no slots (not mode 4)
//              May be iproc sent by CS_RETV if NULL *ppblok
//      pform   Format text describing the expected data
//              May be NULL if not needed for mode 4
//      ppblok  For mode 1 returns call arguments block data
//              For mode 2 supplies call arguments block data
//              For mode 3 supplies call return block data
//              For mode 4 supplies block data to free
//  
//  Returns:
//      Normally returns zero for most modes.
//      Returns positive iproc call number after CS_NEXT mode.
//      Returns positive number of slots used for CS_ARGS mode.
//      Returns negative code if error.
//      Returns ECPENDING if no call yet for CS_NEXT mode.
//      Returns ECNOROOM if nslot is not enough for CS_ARGS mode.
//      Returns ECCOMM if communications error.
//      Returns ECSTOPPED if server should exit.
//*************************************************************************

CSTYLE
int RpcServer (int mode, SChan* pin, 
               RSlot* slots, int nslot, char* pform, RBlok** ppblok) {
    RBlok* pblok;
    BRing* pfifo;
    aint server, accept;
    int rv, used, iproc, smode;

    smode = mode;
    mode &= CS_MASK;

    if (mode == CS_NEXT) {                      //get next call
        pfifo = &pin->sfifo;
        while (TRUE) {                          //allows throw-away messages
            pblok = RpcFifoGet(0, pfifo);
            if (pblok == NULL) {                //fifo empty
                pin->accept = SOCK_ERR;         //nothing accepted
                *ppblok = NULL;                 //no call args block
                return(ECPENDING);              //wait more
            }
            if (pblok->iproc == RP_DONE) {      //accept socket was closed?
                OSFree(pblok);                  //ServTask handles and we ignore
                continue;                       //get another fifo message
            }
            if (pblok->iproc == RP_STOP) {      //shut down server application?
                OSFree(pblok);
                *ppblok = NULL;                 //no call args block
                return(ECSTOPPED);              //tell caller to exit
            }
            if (pblok->iproc == RP_TEST) {
                #ifdef DEB_MORE                 //---
                printf("** RpcServer returning RP_TEST\n");
                #endif                          //---
                rv = RpcSendQ(pblok->iproc, 0, pblok->hpipe, 0);
                OSFree(pblok);                  //test round trip message?
                continue;                       //send back test message
            }
            break;
        }
        accept = pblok->hpipe;                  //get accept socket
        pin->accept = accept;                   //return to caller
        *ppblok = pblok;                        //return block to caller
        return(0);
    }
    if (mode == CS_ARGS) {                      //get call arguments
        pblok = *ppblok;
        used = RpcTxtIni(0,slots,nslot,pform);  //set up slot types
        if (used < 0) return(ECBADCODE);        //create data slots
        rv = RpcDecode(0, slots,used, pblok);   //decode data block
        return(rv);
    }
    if (mode == CS_RETV) {                      //return call result data
        pblok = *ppblok;
        accept = pin->accept;                   //accept socket from CS_NEXT
        if (accept == SOCK_ERR) return(ECBADCODE);
        iproc = (pblok) ? pblok->iproc : nslot;
        rv = RpcSend(iproc, accept, pblok, pin->timout);
        pin->accept = SOCK_ERR;                 //no longer own
        return(rv);
    }
    if (mode == CS_FREE) {                      //free data?
        pblok = *ppblok;
        if (pblok) OSFree(pblok);
        *ppblok = NULL;
    }
    if (mode == CS_INIT) {                      //initialize accept task
        RpcConnect(SOCK_APPINIT, SOCK_ERR, NULL,0,NULL);
        if (pin->server[0] == 0) {
            RpcAddress(0, pin->server, &pin->svport, &pin->svtype);
        }
        server = RpcConnect(SOCK_SERVER+pin->svtype, SOCK_ERR,
                            pin->server, pin->svport, NULL);
        if (server == SOCK_ERR) {               //create our server socket
            return(ECCOMM);
        }

        pfifo = &pin->sfifo;
        rv = RpcFifoSet(0, pfifo);              //set up fifo ring
        if (rv < 0) return(rv);

        pfifo->chan = pin;                      //accept needs channel info
        pfifo->pipe = server;                   //accept parent server socket
        rv = OSTask(OS_TASKNEW, &pin->atask, (LPFTASK)RpcServTask, pfifo);
        return(rv);                             //start accept task
    }
    if (smode == CS_EXIT) {                     //exit application?
        pfifo = &pin->sfifo;
        while (TRUE) {                          //free all calls in fifo
            pblok = RpcFifoGet(0, pfifo);
            if (pblok == NULL) break;           //fifo is now empty?
            OSFree(pblok);                      //free args memory
        }
        rv = OSTask(OS_TASKEND, &pfifo->task, NULL, NULL);
        rv = OSTask(OS_TASKEND, &pin->atask, NULL, NULL);
        RpcConnect(SOCK_CLOSE, pfifo->pipe, NULL,0,NULL);
        RpcConnect(SOCK_APPDONE, SOCK_ERR, NULL,0,NULL);
        return(rv);
    }
    return(ECARGUMENT);
}

//*************************************************************************
//  RpcStart:
//      Called to start named pipe server if not already running.
//      Cannot start UDP/TCP internet server.
//
//  Arguments:
//      int RpcStart (int mode, char* sexec, char* pipen)
//      mode    0 = Called from client to check for server  (RS_CLIENT or 0)
//                  Starts pname serverif needed
//              1 = Called from server when first started   (RS_SERVER)
//                  Returns ECCANNOT to exit if redundant
//              2 = Always start server                     (RS_ALWAYS)
//                  Called from client after connect error
//           +256 = Quite mode without server window        (RS_QUIET)
//                  Not recommended (shares caller cli)!
//      sexec   Path name to start server executable, uses RP_SEXE if NULL
//      pipen   Remote pipe name for server. If NULL, uses RP_NAME if NULL
//              The pipen is not used for mode 2
//
//  Returns:
//      Returns zero for client and server is already running.
//      Returns zero for server and we are only server running.
//      Returns positive ECWAIT if started server and may need to wait for it.
//      Returns ECCANNOT for server and another server is already running.
//*************************************************************************

CSTYLE
int RpcStart (int mode, char* sexec, char* pipen) {
    char path[SZDISK];
    char line[SZFILE];
    char* ptxt;
    aint sock;
    int rv;

    if ((sexec == NULL)||(*sexec == 0)) {       //if not specified, 
        RpcPath(RPP_SERV, path);                //get server executable path
        sexec = path;
    }

    if (mode & RS_ALWAYS) {                     //always start?
        if (mode & RS_QUIET) {
            rv = OSExec(OS_NOWAIT, sexec, NULL);//start server executable
        } else {
            ptxt = OSTxtCopy(line, "start ");
            OSTxtCopy(ptxt, sexec);
            system(line);                       //start with command window
            rv = 0;
        }
        #ifdef DEB_MORE                             //---
        printf("** RpcStart %i starting %s (%i)\n",mode,sexec,rv);
        #endif                                      //---
        return(rv);
    }

    if (pipen == NULL) pipen = RP_NAME;         //try to connect to see if there
    sock = RpcConnect(SOCK_CLIENT, SOCK_ERR, pipen, 0, NULL);
    if (sock != SOCK_ERR) {                     //can connect as client?
        RpcSend(RP_ERRS, sock, NULL, 0);        //tell real server closing
        RpcConnect(SOCK_CLOSE, sock, pipen, 0, NULL);
        return (mode & RS_SERVER) ? ECCANNOT:0; //if server we should exit
    }
    if (mode & RS_SERVER) return(0);            //if server we are only one
    if (mode & RS_QUIET) {                      //if not,
        rv = OSExec(OS_NOWAIT, sexec, NULL);    //start server executable
    } else {
        ptxt = OSTxtCopy(line, "start ");
        OSTxtCopy(ptxt, sexec);
        system(line);                           //start with command window
        rv = 0;
    }
    #ifdef DEB_MORE                             //---
    printf("** RpcStart %i starting %s (%i)\n",mode,sexec,rv);
    #endif                                      //---
    if (rv <= 0) return(ECSYS);
    return(ECWAIT);
}

//*************************************************************************
//  RpcClose:
//      Closes a client socket connection to server.
//      Sends an RP_DONE message to server first allowing it to close too.
//
//  Arguments:
//      int RpcClose (int mode, CChan* pchan)
//      mode    Reserved for future use and must be zero
//      pchan   Channel information for client
//              Closes pchan->socket
//
//  Returns:
//      Normally returns zero.
//      Returns negative code if error.
//*************************************************************************

int RpcClose (int mode, CChan* pchan) {
    aint sock;
    int rv, rs;

    sock = pchan->socket;
    if (sock == SOCK_ERR) return(ECNOINIT);     //get connection socket
    rs = RpcSend(RP_DONE, sock, NULL, 0);       //tell server we are closing
    rv = RpcConnect(SOCK_CLOSE, sock, NULL, 0, NULL);
    rv = (rv == SOCK_ERR) ? ECCOMM : 0;
    #ifdef DEB_MORE                             //---
    printf("** RpcClose %i (%i)\n", rv,rs);
    #endif                                      //---
    pchan->socket = SOCK_ERR;
    return(rv);
}

//*************************************************************************
//  RpcPath:
//      Returns server folder or executable path names.
//
//  Arguments:
//      char* RpcPath (int mode, char* pout)
//      mode    1 = Return server folder path name      (RPP_DATA)
//                  Normally C:\ProgramData\CernoPS\
//              2 = Return server executable path name  (RPP_SERV)
//                  Normally ...\CernoPS\CPServer.exe
//      pout    Returns specified path name.
//              Buffer must have at least SZDISK chars!
//
//  Returns:
//      Returns pointer to terminating zero in pout path name.
//      Returns NULL if error.
//*************************************************************************

CSTYLE
char* RpcPath (int mode, char* plibs) {
    char path[SZPATH];
    char name[SZDISK];
    char* ptxt;

    // Get server folder path.
    if (mode == RPP_DATA) {
        ptxt = OSLocation(plibs, OS_LOCALL);
        ptxt = OSTxtCopy(ptxt, RP_SDIR);
        return(ptxt);
    }

    // Get server executable path.
    if (mode == RPP_SERV) {
        ptxt = OSLocation(plibs, OS_LOCALL);
        ptxt = OSTxtCopy(ptxt, RP_SEXE);
        return(ptxt);
    }
    return(NULL);
}

//*************************************************************************
//  RpcTest:
//      Self test of various RPC remote procedure call functions.
//
//  Arguments:
//      int RpcTest (int mode)
//      mode    Reserved for future use and must be zero.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE
int RpcTest (int mode) {
    RSlot slots[8];
    RBlok bloks[8];
    BRing bring;
    RBlok* pblok;
    RBlok* psend;
    SChan scinfo;
    CChan ccinfo;
    CCVal ccvals;
    CSVal csvals;
    void* htask;
    char* pserv;
    double pcrn[6];
    aint server, client;
    int bytes, cntr, err, rv;
    dword time;

    err = 0;
    rv = RpcTxtIni(0, slots,6, "@T,@I@L,@B,F4D");
    if (rv != 6) {
        printf("!! RpcTxtIni error %i\n", rv);
        err = rv;
    }
    if (  (slots[0].code != RPC_TXT)||(slots[4].nvec != 4)||(slots[5].code != RPC_DBL)) {
        printf("!! RpcTxtIni wrong codes: %i %i %i\n", slots[0].code,slots[4].nvec,slots[5].code);
        err = -20;
    }

    rv = RpcEnText(0, slots,3, "This is a test.\f123\f0x123456789", '\f');
    if (rv != 3) {
        printf("!! RpcEnText error %i\n", rv);
        err = rv;
    }
    if ((OSTxtCmpS("This is a test.", (text*)slots[0].pvec)) || (slots[2].ibig != 0x123456789)) {
        printf("!! RpcEnText wrong values: %s, %llX\n",(text*)slots[0].pvec,slots[2].ibig);
        err = -21;
    }

    bytes = RpcEncode(0, NULL,0, slots,3);
    if (bytes <= 0) {
        printf("!! RpcEncode get size error %i\n", bytes);
        err = bytes;
        bytes = SZPATH;
    }
    pblok = (RBlok*) OSAlloc(bytes);
    rv = RpcEncode(RP_VERB, pblok,bytes, slots,3);
    if (rv != bytes) {
        printf("!! RpcEncode create block error %i\n",rv);
        err = rv;
    }

    rv = RpcEnDone(0, slots,3);                 //free stuff from RpcEnText
    if (rv < 0) {
        printf("!! RpcEnDone error %i\n", rv);
        err = rv;
    }

    rv = RpcDecode(0, slots,3, pblok);
    if (rv != 3) {
        printf("!! RpcDecode error %i\n", rv);
        err = rv;
    }
    if ((OSTxtCmpS("This is a test.", (text*)slots[0].pvec)) || (slots[2].ibig != 0x123456789)) {
        printf("!! RpcDecode wrong values: %s, %llX\n",(text*)slots[0].pvec,slots[2].ibig);
        err = -22;
    }
    psend = pblok;                              //save fake call for latter


    // Test fifo.
    rv = RpcFifoSet(0, &bring);
    if (rv < 0) {
        printf("!! RpcFifoSet #1 error %i\n", rv);
        err = rv;
    }
    bloks[0].iproc = 1;
    bloks[1].iproc = 2;
    bloks[2].iproc = 3;
    rv = RpcFifoPut(0, &bring, &bloks[0]);
    if (rv < 0) {
        printf("!! RpcFifoPut #1 error %i\n", rv);
        err = rv;
    }
    pblok = RpcFifoGet(0, &bring);
    if (pblok == NULL) {
        printf("!! RpcFifoGet #1 error\n");
        err = -23;
    }
    if (pblok->iproc != 1) {
        printf("!! RpcFifoGet #1 bad value %i\n", pblok->iproc);
        err = -24;
    }
    pblok = RpcFifoGet(0, &bring);
    if (pblok != NULL) {
        printf("!! RpcFifoGet #2 should be NULL\n");
        err = -25;
    }
    rv = RpcFifoPut(0, &bring, &bloks[1]);
    if (rv < 0) {
        printf("!! RpcFifoPut #2 error %i\n", rv);
        err = rv;
    }
    cntr = RBR_MAX-2;
    do {                                        //fill fifo
        rv = RpcFifoPut(0, &bring, &bloks[2]);
        if (rv < 0) {
            printf("!! RpcFifoPut #3 error %i %i\n", rv, cntr);
            err = rv;
        }
    } while (--cntr);

    pblok = RpcFifoGet(0, &bring);
    if (pblok == NULL) {
        printf("!! RpcFifoGet #3 error\n");
        err = -26;
    }
    if (pblok->iproc != 2) {
        printf("!! RpcFifoGet #3 bad value %i\n", pblok->iproc);
        err = -27;
    }
    cntr = RBR_MAX-2;
    do {
        pblok = RpcFifoGet(0, &bring);
        if (pblok == NULL) {
            printf("!! RpcFifoGet #4 error %i\n",cntr);
            err = rv;
        }
        if (pblok->iproc != 3) {
            printf("!! RpcFifoGet #3 bad value %i %i\n", pblok->iproc, cntr);
            err = -28;
        }
    } while (--cntr);
    pblok = RpcFifoGet(0, &bring);
    if (pblok != NULL) {
        printf("!! RpcFifoGet #4 should be NULL\n");
        err = -29;
    }

    rv = RpcFifoSet(1, &bring);                 //release old ring
    if (rv < 0) {
        printf("!! RpcFifoSet release error %i\n", rv);
        err = rv;
    }


    // Test client/server basics.
    // We use a different server name to avoid conflict with second test below.
    OSMemClear(&scinfo, sizeof(SChan));
    RpcAddress(0, scinfo.server, &scinfo.svport, &scinfo.svtype);
    pserv = scinfo.server;
    if (scinfo.svtype & CS_NET) {               //UDP or TCP?
        scinfo.svport += 1;                     //non-standard port number
    } else {
        OSTxtCopy(pserv, "CPSSX");              //non-standard server name
    }                                           //to avoid conflict with above,
    rv = SOCK_SERVER+scinfo.svtype;             //use different server address
    server = RpcConnect(rv, SOCK_ERR, pserv, scinfo.svport, NULL);
    if (server == SOCK_ERR) {
        printf("!! RpcConnect server socket error\n");
        err = -30;
    }

    rv = RpcFifoSet(0, &bring);                 //set up ring again
    bring.flag = SCF_PROC;                      //need RP_PROC messages for test
    if (rv < 0) {
        printf("!! RpcFifoSet #2 error %i\n", rv);
        err = rv;
    }
    bring.chan = &scinfo; 
    bring.pipe = server;
    rv = OSTask(OS_TASKNEW, &htask, (LPFTASK)RpcServTask, &bring);
    if (rv < 0) {
        printf("!! OSTask error %i\n", rv);
        err = rv;
    }
    OSSleep(100);                               //wait some after server starts

    rv = SOCK_CLIENT+scinfo.svtype;             //use different server address
    client = RpcConnect(rv, SOCK_ERR, pserv, scinfo.svport, NULL);
    rv = RpcSend(0, client, psend, 0);          //send fake call from above
    if (rv < 0) {
        printf("!! RpcSend psend error %i\n", rv);
        err = rv;
    }
    OSFree(psend);                              //send copies message so free mem

    pblok = RpcRecv(0, client, 0);              //receive processing reply
    if (pblok == NULL) {
        printf("!! RpcConnect client error\n");
        err = -31;
    }
    if (pblok->iproc != RP_PROC) {
        printf("!! RpcConnect client iproc %i\n", pblok->iproc);
        err = -32;
    }
    OSFree(pblok);

    RpcConnect(SOCK_CLOSE, client, pserv, 0, NULL);

    OSSleep(100);                               //wait some before next client
    rv = SOCK_CLIENT+scinfo.svtype;             //use different server address
    client = RpcConnect(rv, SOCK_ERR, pserv, scinfo.svport, NULL);
    rv = RpcSend(RP_STOP, client, NULL, 0);     //send stop message to server
    pblok = RpcRecv(0, client, 0);              //receive stopping reply
    if ((pblok == NULL)||(pblok->iproc != RP_STOP)) {
        printf("!! RpcConnect server failed to stop\n");
        err = -33;
    }
    rv = RpcFifoSet(3, &bring);                 //release old ring and messages
    OSSleep(100);


    // Test CPS CRN filename argument block encode and decode.
    rv = TxtCall(CC_PREP, &ccinfo, 0, "MyName.crn", "Lib1.cps,Lib2.cps",
                 1234,6,10,0, 66.5f, &pblok);
    if (rv < 0) {
        printf("!! TxtCall error %i\n", rv);
        err = -40;
    }
    rv = TxtFrom(CF_NORM, &ccvals, pblok);
    if (rv < 0) {
        printf("!! TxtFrom error %i\n", rv);
        err = -41;
    }
    if (  OSTxtCmp(ccvals.file, "MyName.crn")
        ||OSTxtCmp(ccvals.libs, "Lib1.cps,Lib2.cps")
        ||(ccvals.mode != 1234)||(ccvals.tasks != 6)
        ||(ccvals.nkeep != 10) ||(ccvals.blend != 66.5f)  ) {
        printf("!! TxtFrom bad value: %s %s %i %i %i %.2f\n", ccvals.file,ccvals.libs,ccvals.mode,ccvals.tasks,ccvals.nkeep,ccvals.blend);
        err = -42;
    }

    // Test CPS Double Spectrum Array argument block encode and decode.
    pcrn[0]=1.00;pcrn[1]=2.34;pcrn[2]=3.45;pcrn[3]=4.56;pcrn[4]=5.67;pcrn[5]=6.00;
    rv = VecCall(CC_PREP, &ccinfo, 0, pcrn,6,123.45,234.56, "Lib1.cps,Lib2.cps",
                 1234,6,10,0, 66.5f, &pblok);
    if (rv < 0) {
        printf("!! VecCall error %i\n", rv);
        err = -44;
    }
    rv = VecFrom(CF_VALS, &csvals, pblok);
    if (rv < 0) {
        printf("!! VecFrom error %i\n", rv);
        err = -45;
    }
    if (  OSTxtCmp(csvals.libs, "Lib1.cps,Lib2.cps")
        ||(csvals.pspc[0] != 1.00)||(csvals.pspc[5] != 6.00)
        ||(csvals.nspc != 6)||(csvals.begx != 123.45)||(csvals.endx != 234.56)
        ||(csvals.mode != 1234)||(csvals.tasks != 6)
        ||(csvals.nkeep != 10) ||(csvals.blend != 66.5f)  ) {
        printf("!! VecFrom bad value: %.2f %.2f %i %.2f %.2f %s %i %i %i %.2f\n", 
                csvals.pspc[0],csvals.pspc[1],csvals.nspc,csvals.begx,csvals.endx,
                csvals.libs,csvals.mode,csvals.tasks,csvals.nkeep,csvals.blend);
        err = -46;
    }
    VecFrom(CF_FREE, &csvals, pblok);

    // The rest of these tests are too kludgy for slower TCP/UDP so stop here.
    if (scinfo.svtype & CS_NET) {
        return(err);                            //return early if UDP or TCP
    }

    // Test automated client/server methods.
    // This is a kludge that uses server tasks but no server main handler.
    // Instead we use the returned RP_PROC messages to complete fake calls.
    OSMemClear(&scinfo, sizeof(SChan));
    scinfo.sfifo.flag = SCF_PROC;               //PF_PROC messages needed for test
    rv = RpcServer(CS_INIT, &scinfo, NULL,0,NULL, NULL);
    if (rv < 0) {                               //start server tasks
        printf("!! RpcServer CS_INIT error %i\n",rv);
        err = -50;
    }
    OSSleep(100);                               //wait some before next client

    OSMemClear(&ccinfo, sizeof(CChan));
    rv = TxtCall(CC_INIT+CC_CALL+CC_PEEK+CC_FREE, &ccinfo, CP_CALL, 
                 "C:\\Temp\\Peak1.crn", "MainLib.cps", 2, 6,10,0,100, &pblok);
    if ((rv < 0)&&(rv != ECPENDING)) {          //send call arguments block
        printf("!! TxtCall CC_CALL error %i\n", rv);
        err = -51;
    }
    OSSleep(100);                               //wait some before next client

    time = OSTickCount();
    do {
        OSSleep(0);
        if ((OSTickCount() - time) > 1000) {    //waited too long for call args?
            printf("!! RpcServer CC_NEXT timeout\n");
            rv = ECTIMEOUT;
            break;
        }
        rv = RpcServer(CS_NEXT, &scinfo, NULL,0,NULL, &pblok);
    } while (rv == ECPENDING);                  //receive call arguments block
    if ((rv < 0)||(pblok == NULL)) {
        printf("!! RpcServer CS_NEXT error %i\nRpcServer.exe: BE SURE REAL SERVER IS NOT RUNNING!\n(Skipping the rest of this test)\n", rv);
        err = -52;
        return(err);
    }

    rv = RpcServer(CS_ARGS, &scinfo, slots,8,CC_TEXT, &pblok);
    if (rv < 0) {
        printf("!! RpcServer CS_ARGS error %i\n", rv);
        err = -54;
    }
    if (slots[CC_TASK].inum != 6) {
        printf("!! RpcServer CS_ARGS bad value %i\n", slots[CC_TASK].inum);
        err = -55;
    }
    bloks[0].bytes = sizeof(RBlok);
    bloks[0].check = RPC_CHECK;
    bloks[0].iproc = 12345;                     //create simple return message
    bloks[0].slots = 0;
    bloks[0].hpipe = 0;
    pblok = &bloks[0];
    rv = RpcServer(CS_RETV, &scinfo, NULL,0,NULL, &pblok);
    if (rv < 0) {
        printf("!! RpcServer CS_RETV error %i\n", rv);
        err = -54;
    }

    time = OSTickCount();
    do {
        OSSleep(0);
        if ((OSTickCount() - time) > 1000) {    //waited too long for return?
            printf("!! RpcCall CC_PEEK timeout\n");
            rv = ECTIMEOUT;
            err = -55;
            break;
        }
        rv = RpcCall(CC_CALL+CC_PEEK, &ccinfo, 0, NULL, &pblok);
    } while (rv == ECPENDING);                  //receive call arguments block
    if ((rv < 0)||(pblok == NULL)) {
        printf("!! RpcCall CC_PEEK error %i\n", rv);
        err = -56;
    }
    if (pblok->iproc != 12345) {
        printf("!! RpcCall return value error %i\n", pblok->iproc);
        err = -57;
    }
    if (pblok) OSFree(pblok);

    rv = RpcServer(CS_EXIT, &scinfo, NULL,0,NULL, NULL);
    if (rv < 0) {                               //tell server to clean up
        printf("!! RpcServer CS_EXIT error %i\n", rv);
        err = -54;
    }

    return(err);
}
