//*************************************************************************
//  FILENAME:   TriSQL.h
//  AUTHOR:     SCSimonoff  6/20/2021
//
//  Definitions for TriSQL.cpp SQLite3 API.
//
//  This software was created 2021 by SCSimonoff for Triscape Inc.
//  This may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _TriSQL                 //Only define once if include multiple times
#define _TriSQL

#include "TriOS.h"
#include "CDefs.h"

// Helpful definitions.

#define SQ3_OPEN    1   //Sqlite mode: Open database file
#define SQ3_EXEC    2   //Sqlite mode: Execute SQL statement
#define SQ3_STEP    4   //Sqlite mode: Step to next selected row
#define SQ3_COMMIT  8   //Sqlite mode: Commit transaction
#define SQ3_FINAL   16  //Sqlite mode: Finalize statement
#define SQ3_CLOSE   32  //Sqlite mode: Close database
#define SQ3_MASKS  255  //Sqlite mode: Mask for basic operations
#define SQ3_LIMIT  256  //Sqlite mode: Return ECNOROOM rather than alloc pesc
#define SQ3_FREE    -1  //Sqlite mode: Free allocated pesc memory
#define SQ3_ONCE   (SQ3_OPEN+SQ3_EXEC+SQ3_STEP+SQ3_FINAL+SQ3_CLOSE)
#define SQ3_ALL3   (SQ3_EXEC+SQ3_STEP+SQ3_FINAL)

#define SQ3_OUTSZ 2048  //Default pout buffer size
#define SQ3_ALLSZ  256  //Mimumum allocated text/blob buffer size
#define sq3txt    const unsigned char

#define SQT_INT 1       //Integer SQLite value
#define SQT_FLT 2       //Float   SQLite value
#define SQT_TXT 3       //Text    SQLite value
#define SQT_BLB 4       //Blob    SQLite value

// Structure used by Sqlite() to hold bind input and select output values.
// The dType, BindC can be zero for text pout values specified in @??? escapes.
// Bytes may be zero for binding zero terminated text.
// Limit may be zero for to automatically allocate memory.
// Limit for allocated memory is set negative so SQ3_FREE automatically frees.
// Limit should be zero for float and integer output.
// Bytes is not returned for float and integer output.
// The form format text using 1+ for the @??R return blob
// only uses the SqlIn Limit and Point and only returns Bytes and Value,
// so the other SqlIn information is unused and can be zero.

DEFSTRUCT(SqlIn)        //SqlIn: Information for binds and form escapes
int     dType;          //SQT_? data type for sqlite3 column value
int     BindC;          //Bind statement ? (1=1st) or neg select col (-1=1st)
int     Limit;          //Text or blob byte size limit for output (0=allocate)
int     Bytes;          //Text or blob byte size actual (less ending zero)
char*   Point;          //Text or blob value pointer
qint    Int64;          //Integer database value
double  Value;          //Floating database value
DEFENDING(SqlIn)        //(If point is NULL then allocs and caller must free)

// Structure used by Sqlite() to pass database filename and keep state.
// Note to avoid including sqlite3.h outside of TriSQL.cpp,
// define the db and res as NULL pointers here and cast inside TriSQL.cpp code.

DEFSTRUCT(DBase)        //DBase: SQLite database information
char*   fname;          //Database file path name
void*   db;             //SQLite3 open database handle (sqlite3*) or NULL
void*   res;            //SQLite3 statement handle (sqlite3_stmt*) or NULL
int     err;            //Returns SQLite error code only on error
DEFENDING(DBase)

// API functions defined in TriSQL.cpp

CSTYLE_BEG
int  Sqlite (int mode, DBase* data, char* psql, char* form, char* pout, int nout, SqlIn* pesc, int nesc);
int SqlTest (int mode);
CSTYLE_END

#endif                          //_TriSQL


