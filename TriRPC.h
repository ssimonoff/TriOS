//*************************************************************************
//  FILENAME:   TriRPC.h
//  AUTHOR:     SCSimonoff  7/05/21
//
//  Definitions for TriRPC.cpp Remote Procedure Call API.
//
//  Uses TriOS.h portable non-gui system calls.
//  Uses CDefs.h custom types such as: cint,byte,sint,word,lint,dword,flt,flx
//  This software was created 2021 by SCSimonoff for Triscape Inc.
//  This may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _TriRPC                 //Only define once if include multiple times
#define _TriRPC

// Helpful definitions.
// Note that RPC_ROW uses nvec as total number of rows or 0 if not known.

#define RPC_TXT 1       //Text    value (utf8 with zero term)       @T text name
#define RPC_BLB 2       //Blob    value (bytes array)               @B text name
#define RPC_INT 3       //Integer value (int32 value or array)      @I text name
#define RPC_I64 4       //Int64   value (int64 value or array)      @L text name
#define RPC_FLT 5       //Float   value (float32 value or array)    @F text name
#define RPC_DBL 6       //Double  value (float64 value or array)    @D text name
#define RPC_ROW 7       //Integer value row before associated slots @R text name

#define RPP_DATA 1      //RpcPath mode to get server program data path
#define RPP_SERV 2      //RpcPath mode to get server executable path

#define RS_CLIENT    0  //RpcStart if called from client
#define RS_SERVER    1  //RpcStart if called from server
#define RS_ALWAYS    2  //RpcStart to always start server
#define RS_QUIET   256  //RpcStart flag to start without window

// RBlok iproc message code numbers.
// By convention CP_? are from client and RP_? are replies from server.
// Other messages are allowed but these are built in to current code.
// The RP_MASK limits basic message to 12 bits but extra iproc bits
// are not currently used and this limitation is not really needed.
// RP_VERB uses CPS.cpp CpsText to decode RBlok return.
// RP_HITS uses CPS.cpp CpsHits to decode RBlok then CpsMeta for local info.
// RP_ALLS uses CPS.cpp CpsBlok to decode but is not currently used.

#define RP_PROC      0  //CPS server is processing call (not currently used)

#define CP_CALL      1  //CPS normal call iproc with crn unknown,libs,etc
#define CP_SPEC      2  //CPS spectrum call iproc with profile Y array (future)
#define CP_SETV      3  //CPS set or get parameter value (both call and return)
#define CP_HIT1      4  //CPS get hit single hit result

#define RP_VERB     10  //CPS verbose text iproc server return
#define RP_HITS     11  //CPS RxHit hit list information iproc return
#define RP_ALLS     12  //CPS all search information in binary format
#define RP_HITFORM  14  //CPS hit format result text return
#define RP_HITMETA  15  //CPS hit library MetaHit struture result

#define RP_STOP    252  //CPS iproc message to stop server
#define RP_TEST    253  //CPS iproc message to test server round trip
#define RP_DONE    254  //CPS iproc message from client to close socket
#define RP_ERRS    255  //CPS iproc message with error code in hpipe

#define RP_MASK  0xFFF  //Limit mask for basic RP_? iproc values

// Default connection names, timings and concurrent server sockets limit.

#define RP_SDIR "CernoPS\\"
#define RP_SEXE "CernoPS\\CPServer.exe"
#define RP_NAME "CPS_S" //OSipcPipe name or OSConnect IP for CPS server
#define RP_PIPE ""      //Server executable path name

#define RP_TIME   5000  //RpcSend,RpcRecv default timeout msec ( 5sec, was 1)
#define RP_SLOW  30000  //RpcSend,RpcRecv slow receive timeout (30sec, was 5)
#define RP_EXEC   5000  //Default time to execute server starup (5sec)
#define RP_SOCKS    32  //RpcServTask maximum number of sockets we can handle

// Single value slot for RPC call or return data.

DEFSTRUCT(RSlot)        //RSlot: RPC value information
lint    code;           //RPC_? type code
lint    nvec;           //Number of array chars or values (0 for single value)
union {
lint    inum;           //Single integer value       (@I)
qint    ibig;           //Single large integer value (@L)
float   fnum;           //Single float value         (@F)
double  fbig;           //Single double float value  (@D)
};
void*   pvec;           //Pointer to text, blob or array
DEFENDING(RSlot)        //RSlot: RPC value information

// RPC memory block is passed to or returned from remote procedure calls
// instead of normal function arguments and return values.
// It begins with a byte size, checkword, call type, and number of slots header.
// The caller normally leaves hpipe set to zero reserving it for server use.
// The block continues with data for each RPC slot specified by the iproc.
// Each slot's information begins with the number of bytes in a dword.
// Slot info has a second dword giving array values (1 for non-array int/flt).
// Slot info should be 8-byte aligned and beginning size must include padding.

DEFSTRUCT(RBlok)        //RBlok: RPC block header
dword   bytes;          //Number of bytes in block including this header
dword   check;          //Check value to detect pipe errors (RPC_CHECK)
dword   iproc;          //Proceedure index called to to handle block
dword   slots;          //Number of RSlot data value slots used in block
qint    hpipe;          //Pipe handle to return results (used by server) or error
DEFENDING(RBlok)        //MUST have even of qwords (bytes/8)!

#define RPC_CHECK 0xFB435052

// Simple ring buffer to pass aint handles

#define RBA_MAX 128     //Maximum ARing FIFO size (was 32)

DEFSTRUCT(ARing)        //ARing: Accept Socket (aint) FIFO Ring
aint    fifo[RBA_MAX];  //FIFO ring buffer of socket handles
int     iput;           //FIFO index to put next block
int     iget;           //FIFO index to get next block
void*   lock;           //Mutex lock for accessing FIFO
void*   task;           //Handle for the accept task
void*   chan;           //SChan server channel information
aint    pipe;           //RPC server pipe handle to use for accept
DEFENDING(ARing)

// Simple ring buffer to pass received RBlok messages from receive task.

#define RBR_MAX 256     //Maximum BRing FIFO size

DEFSTRUCT(BRing)        //BRing: RPC Block FIFO Ring
RBlok*  fifo[RBR_MAX];  //FIFO ring buffer of block pointers
int     iput;           //FIFO index to put next block
int     iget;           //FIFO index to get next block
void*   lock;           //Mutex lock for accessing FIFO
void*   task;           //Handle for the accept task
void*   chan;           //SChan server channel information
dword   flag;           //Server control flags (SCF_?)
aint    pipe;           //RPC server pipe handle to use for accept
DEFENDING(BRing)

#define SCF_PROC 1      //Send extra RP_PROC processing messages on receipt

// Client's channel object.

#define CC_SIZ   64     //Maximum server name size

DEFSTRUCT(CChan)        //CChan: Client Channel information
aint    socket;         //Socket to server
text    server[CC_SIZ]; //Remote pipe name ip address for server (RP_NAME)
text    sstart[SZDISK]; //Server executable to start if needed
int     sportn;         //TCP/UDP port number (1-65535)
int     stypen;         //Sockets format (Pipe=0, UDP=1, TCP=2)
dword   timout;         //Timeout ms for return or 0 for default
DEFENDING(CChan)

// RpcCall information.

#define CC_PREP    0    //RpcCall mode prepares data without calling (must be 0)
#define CC_INIT    1    //RpcCall mode to initialize CChan structure
#define CC_CALL    2    //RpcCall mode added to call
#define CC_PEEK    4    //RpcCall mode added for peek mode
#define CC_FREE    8    //RpcCall mode added to free data after send
#define CC_CLOSE  16    //RpcCall mode added to close socket after call
#define CC_LOCAL  -1    //RpcCall mode reserved for CPS non-server search
#define CC_CALLX  (CC_PREP+CC_INIT+CC_CALL+CC_FREE+CC_CLOSE)  
#define CC_CALLS  (CC_PREP+CC_CALL+CC_FREE)  

// TxtCall slot indicies for CP_CALL iproc number.

#define CC_TEXT "@T,@T,@I,@I,@I,@I,@F"

#define CC_NAME 0       //Unknown file name text
#define CC_LIBS 1       //Library file names text
#define CC_MODE 2       //Mode integer
#define CC_TASK 3       //Number of tasks integer
#define CC_KEEP 4       //Hits to keep integer
#define CC_RAMB 5       //Maximum RAM MB integer
#define CC_FRAC 6       //Blend fraction float
#define CC_USED 7       //Number of slots used

#define CF_VALS 1       //TxtFrom mode to get data values
#define CF_FREE 2       //TxtFrom mode to free
#define CF_NORM (1+2)   //Normal call does both

DEFSTRUCT(CCVal)        //CCVal: CPS Argument Values for TxtFrom
char    file[SZFILE];   //Unknown file name
char    libs[SZFILE];   //Library name or comma separate list
int     mode;           //Mode value and flags
int     tasks;          //Number of tasks
int     nkeep;          //Hits to keep
int     nramb;          //Maximum RAM MB
float   blend;          //Reverse blend fraction
DEFENDING(CCVal)

// VecCall slot indicies for CP_SPEC iproc number.

#define CS_TEXT "@D,@I,@D,@D,@T,@I,@I,@I,@I,@F"

#define CS_PSPC 0       //Unknown spectrum double Y values
#define CS_NSPC 1       //Unknown spectrum number of points
#define CS_BEGX 2       //Unknown spectrum mass of first point
#define CS_ENDX 3       //Unknown spectrum mass of last point
#define CS_LIBS 4       //Library file names text
#define CS_MODE 5       //Mode integer
#define CS_TASK 6       //Number of tasks integer
#define CS_KEEP 7       //Hits to keep integer
#define CS_RAMB 8       //Maximum RAM MB integer
#define CS_FRAC 9       //Blend fraction float
#define CS_USED 10      //Number of slots used

DEFSTRUCT(CSVal)        //CSVal: CPS Argument Values for VecFrom
double* pspc;           //Unknown spectrum Y array
int     nspc;           //Unknown spectrum number of points
double  begx;           //Unknown specrrum mass of first point
double  endx;           //Unknown specrrum mass of last point
char*   libs;           //Library name or comma separate list
int     mode;           //Mode value and flags
int     tasks;          //Number of tasks
int     nkeep;          //Hits to keep
int     nramb;          //Maximum RAM MB
float   blend;          //Reverse blend fraction
DEFENDING(CSVal)

// SetCall slot indicies for CP_SETV iproc number.

#define CV_TEXT "@I,@I,@D"

#define CV_PRM  0       //Parameter number integer
#define CV_GET  1       //Just get if 1 (otherwise sets even zero value)
#define CV_VAL  2       //Parameter value double (but can hold most integers too)
#define CV_USED 3       //Number of slots used

DEFSTRUCT(CVVal)        //CVVal: CPS Argument Values for SetFrom
int     prm;            //Parameter number
int     get;            //Just get if 1
double  val;            //Parameter value
DEFENDING(CVVal)

// RpcServer definitions.

#define CS_EXIT -1      //RpcServer mode to exit server application
#define CS_INIT 0       //RpcServer mode to initialize accept task
#define CS_NEXT 1       //RpcServer mode to check for next call
#define CS_ARGS 2       //RpcServer mode to decode slots from block
#define CS_RETV 3       //RpcServer mode to send call return value
#define CS_FREE 4       //RpcServer mode to free slots and block
#define CS_MASK 31      //RpcServer mask of basic modes

#define CS_PIPE  0      //RpcServer/RpcConnect mode added for Named Pipe
#define CS_UDP  32      //RpcServer/RpcConnect mode added for UDP socket
#define CS_TCP  64      //RpcServer/EpcConnect mode added for TCP socket
#define CS_NET (CS_UDP+CS_TCP)

#define CS_NAME ""      //Default server name (use RP_NAME or "" for CPS.set)
#define CS_TYPE CS_PIPE //Default type is 0 for named pipe
#define CS_PORT  0      //Default port number not used for named pipe

#define SV_NAME RP_NAME //Server ip, url, pipe name
#define SV_TYPE CS_PIPE //Server type (CS_PIPE,CS_UDP,CS_TCP)
#define SV_PORT  0      //Server port (0 if pipe, 54345 "5forCPS" for tcp/udp)

#define CS_ADV  0x40000 //RpcConnect flag for advanced non-auto arguments

DEFSTRUCT(SChan)        //SChan: Server Channel information and state
aint    socket;         //Server socket (accept sockets in ring messages)
aint    accept;         //Current accept socket for pending call
text    server[CC_SIZ]; //Remote pipe name ip address for server (RP_NAME)
int     svport;         //Server TCP/UDP port number (1-65535)
int     svtype;         //Server sockets format (Pipe=0, UDP=1, TCP=2)
dword   timout;         //Timeout ms for return or 0 for default
void*   atask;          //Server accept task handle
BRing   sfifo;          //Server call block data fifo
DEFENDING(SChan)

// RpcConnect, RpcConSend, RpcConRecv socket information.
// Allows a common socket handle number to do Named Pipe, UDP/IP, TCP/IP.

DEFSTRUCT(CSock)        //CShan: Channel socket state
aint    socket;         //Lower level socket handle (OSConnect, OSipcPipe)
int     sotype;         //Socket format: CS_PIPE,CS_UDP,CS_TCP
int     soport;         //Socket port number
ConEx   sextra;         //Extra connection information for UDP/TCP
DEFENDING(CSock)

// API functions defined in TriRPC.cpp.

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

#endif                          //_TriRPC
