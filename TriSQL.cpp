//*************************************************************************
//  FILENAME:   TriSQL.cpp
//  AUTHOR:     SCSimonoff  6/20/2001
//  CPU:        Portable
//  O/S:        Portable
//  MRU:
//
//  Compiles SQLite3 3.36.0  amalgamation
//  and provides easy API for accessing it.
//  Must also include sqlite3.c and TriOS.cpp in the project.
//  Compile with /D NO_SQLMSG to stop sqlite3_prepare_v2 error info printf
//  
//  Uses TriOS.h portable non-gui system calls.
//  Uses CDefs.h custom types such as: cint,byte,sint,word,lint,dword,flt,flx
//
//  This software was created 2021 by SCSimonoff for Triscape Inc.
//  This may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#include <Windows.h>
#include "sqlite3.h"
#include "TriOS.h"
#include "TriSQL.h"

// Functions included in this file.

CSTYLE_BEG
int  Sqlite (int mode, DBase* data, char* psql, char* form, char* pout, int nout, SqlIn* pesc, int nesc);
int  SqlTest (int mode);
CSTYLE_END

//*************************************************************************
//  Sqlite:
//      Simple access to SQLite3 database.
//
//  Arguments:
//      int Sqlite (int mode, DBase* data, char* psql, char* form,  
//                  char* pout, int nout, SqlIn* pesc, int nesc)
//      mode   May be combination of these flags in given sequence:
//             +1 = Open or create database     SQ3_OPEN
//             +2 = Perform statement           SQ3_EXEC
//             +4 = Step statement              SQ3_STEP 
//             +8 = Finalize statement          SQ3_FINAL
//            +16 = Close database              SQ3_CLOSE
//           +256 = Do not allocate pesc memory SQ3_LIMIT
//             31 = All with open and close     SQ3_ONCE
//             14 = All without open or close   SQ3_ALL3
//             -1 = Free allocated pesc memory  SQ3_FREE
//      data    SQLite database file path name.
//              SQ3_OPEN creates new file if does not exist.
//              The db and res should be set to NULL initially.
//      psql    SQLite SQL statement text.
//      form    Format string for @TDR output columns (or ^TR to set binding).
//              @TR sets R for return column negative in BindC automatically
//              and you do not need to set up pesc escapes after clearing.
//              Has ^TR or @TDR escapes where 
//              ^T? is bind type (I=Integer, F=Float, T=Text, B=Blob)
//              ^?R is pesc structure to bind with dType and positive BindC
//              @T?? is select type (I=Integer, F=Float, T=Text, B=Blob)
//              @?D? is database column number  (1=1st,A=10th,Z=36th)
//              @??R is pesc structure return   (1=1st,A=10th,Z=36th)
//                   or 0 for formatted text output to pout string
//              Note that both D and R digits are 1=1st even though SQLite
//              uses column 0 for left most column and pesc[0] is 1st!
//              After 1-9 use A-Z for columns 10-36.
//              Even if pout is NULL we output only to pesc for @???.
//              The form may have extra chars for readability
//              like "@F11,@F22,F33" with unneeded comma separators
//              that returns three selected column values.
//              Bind values should use the first pesc structures
//              and return values then have higher R numbers after them
//              but lower D database column numbers like "@T12,@F23".
//              The form string may begin with "^TR,^TR...@TDR..." to set
//              the pesc dType and positive BindC values for binding before
//              later @TDR's set pesc dType and negative BindC for output.
//              The ^TR gives data type and pesc index (1=1st) to autoset
//              dType and +BindC, so must manually set rest with bind value,
//              but OSMemClear zeros so just set Point,Int64 or Value for each.
//              May be NULL if not needed (no output or set pesc manually).
//              Use NULL or blank for create or insert without output when
//              all pesc values are manually setup as needed before call.
//      pout    Text buffer to receive formatted text output
//      nout    Bytes available in pout (0=SZDISK)
//      pesc    Array of information used by form ^?? and @??? escapes.
//              Can often just be cleared to 0's if form sets outputs.
//              Set dType and +BindC for binding values to psql statement
//              The BindC is positive for bind values and negative for output,
//              bug form ^?? can set for binding type and col to just set value,
//              and form @??? can set for output without setting pesc manually,
//              May set Limit and Point to save select column to given memory.
//              But the Limit and Point may be zero to allocated for select.
//              See SqlIn comments.
//      nesc    Number of items in pesc array
//
//  Returns:
//      Returns 0 output normally
//      Returns postive sqlite error code if SQ3_STEP not row or done
//      Returns negative code for standard errors
//      ECOPEN    means the database file failed to open
//      ECNOINIT  means the statement failed to prepare
//      ECINVALID means SqlIn dType code is invalid
//      ECNOWRITE means statement value bind failed
//      ECFEWPTS  means no more select result rows
//      ECNULL    means pesc Point was NULL pointer
//      ECNOROOM  means pesc Limit was too small and cannot allocate
//      ECBADCODE means sqlite3_column function returned NULL pointer
//      ECCANNOT  means @B?? blob text output is not supported
//*************************************************************************

CSTYLE
int Sqlite (int mode, DBase* data, char* psql, char* form,  
            char* pout, int nout, SqlIn* pesc, int nesc) {
    sqlite3* db;
    sqlite3_stmt* res;
    sqlite3_int64 i64;
    SqlIn* pinfo;
    double fval;
    void* pfree;
    char* pfor;
    char* pend;
    char* psav;
    char esc[3];
    const void* pbl3;
    const unsigned char* ptx3;
    int noalloc, limit, bytes, have, step, icol, iout, type, rc, ii;

    if (pesc == NULL) nesc = 0;                 //just in case
    if (pout == NULL) nout = 0;
    else              *pout = 0;                //blank output in case of error

    // Free allocated pesc memory?
    if (mode == SQ3_FREE) {
        while (nesc-- > 0) {
            if ((pesc->Point)&&(pesc->Limit <= 0)) {
                pfree = OSFree(pesc->Point);    //free memory
                if (pfree) return(ECHANDLE);    //and make sure allocated
                pesc->Point = NULL;
                pesc->Limit = 0;
            }
            pesc += 1;
        }
        return(0);
    }

    // Get saved SQLite state.
    db  = (sqlite3*) data->db;
    res = (sqlite3_stmt*) data->res;
    noalloc = mode & SQ3_LIMIT;
    mode = mode & SQ3_MASKS;

    // Handle opening SQLite database file.
    if (mode & SQ3_OPEN) {
        if (db) sqlite3_close(db);
        db = NULL;
        rc = sqlite3_open(data->fname, &db);
        data->db = db;
        if (rc != SQLITE_OK) {
            data->err = rc;
            sqlite3_close(db);
            return(ECOPEN);
        }
    }

    // Handle executing SQL statement.
    if (mode & SQ3_EXEC) {                      //prepare SQL statement
        if (res) sqlite3_finalize(res);
        data->res = NULL;
        rc = sqlite3_prepare_v2(db, psql, -1, &res, 0);
        data->res = res;                        //remember statement handle
        if (rc != SQLITE_OK) {
            #ifndef NO_SQLMSG                   //---
            printf("!! SQLite Prepare %s: %s\n", sqlite3_errstr(sqlite3_extended_errcode(db)), sqlite3_errmsg(db));
            #endif                              //---
            data->err = rc;
            return(ECNOINIT);
        }

        // Use form ^? info to make binding easier.
        if ((form)&&(*form == '^')) {
            while (*form) {
                if        (*form == '^') {      //^?? bind escape value?
                    form += 1;
                    esc[0] = CAPITAL(*form); 
                    if (*form) form += 1;
                    if      (esc[0] == 'T') type = SQT_TXT;
                    else if (esc[0] == 'F') type = SQT_FLT;
                    else if (esc[0] == 'I') type = SQT_INT;
                    else if (esc[0] == 'B') type = SQT_BLB;
                    else                    type = 0;

                    esc[1] = CAPITAL(*form);
                    if (*form) form += 1;
                    icol = 0;
                    if        ((esc[1] >= '0')&&(esc[1] <= '9')) {
                        icol =  esc[1] - '0';
                    } else if ((esc[1] >= 'A')&&(esc[1] <= 'Z')) {
                        icol = (esc[1] - 'A') + 10;
                    }                       
                    if (icol > 0) {             //bind to pesc structure?
                        pinfo = pesc + (icol - 1);
                        pinfo->dType = type;    //set up structure to do below
                        pinfo->BindC = icol;    //positive column for binding
                    }
                } else if (*form == '@') {      //@??? output escape?
                    break;                      //leave output for later
                } else {
                    form += 1;                  //skip extra unused chars
                }
            }
        }

        // Handle binding values to SQL statement.
        ii = -1;
        while (++ii < nesc) {                   //see what values to bind
            icol = pesc[ii].BindC;              //statement's ? index (1=1st)
            if(icol > 0) {                      //binds to exec statement?
                type = pesc[ii].dType;          //type of data to bind
                if        (type == SQT_TXT) {
                    bytes = pesc[ii].Bytes;
                    if (bytes <= 0) bytes = OSTxtSize(pesc[ii].Point) - 1;
                    rc = sqlite3_bind_text(res, icol, pesc[ii].Point,
                                           bytes, SQLITE_STATIC);
                } else if (type == SQT_FLT) {
                    rc = sqlite3_bind_double(res, icol, 
                                           (double)pesc[ii].Value);
                } else if (type == SQT_INT) {
                    rc = sqlite3_bind_int64(res, icol,
                                           (sqlite3_uint64)pesc[ii].Int64);
                } else if (type == SQT_BLB) {
                    rc = sqlite3_bind_blob(res, icol, (void*)pesc[ii].Point, 
                                           pesc[ii].Bytes, SQLITE_STATIC);
                } else {
                    return(ECINVALID);
                }
                if (rc != SQLITE_OK) {
                    data->err = rc;
                    return(ECNOWRITE);
                }   
            }
        }
    }

    // Handle getting select row data.
    if (mode & SQ3_STEP) {           
        res = (sqlite3_stmt*) data->res;
        if (res == NULL) return(ECFEWPTS);      //no statement or statement done?
        step = sqlite3_step(res);               //execute statement step

        if (  (step != SQLITE_ROW)              //no more rows?
            &&((step != SQLITE_DONE)||(!(mode & SQ3_FINAL)))  ) {
            sqlite3_finalize(res);              //finalize statement
            data->res = NULL;                   //no more results
            return(ECFEWPTS);                   //fewer rows than expected
        }

        // Handle returning pout text data specified in form format text.
        if ((form)&&(*form)) {
            pfor = form;
            if (nout <= 0) nout = SQ3_OUTSZ;    //assume default buffer size?

            pend = NULL;                        //make stupid compiler happy
            if (pout) {                         //have text output?
                pend = pout + (nout - 1);       //space for ending zero                      
            }

            while (*pfor) {
                if (*pfor == '@') {             //@??? output escape value?
                    pfor += 1;
                    esc[0] = CAPITAL(*pfor); 
                    if (*pfor) pfor += 1;
                    if      (esc[0] == 'T') type = SQT_TXT;
                    else if (esc[0] == 'F') type = SQT_FLT;
                    else if (esc[0] == 'I') type = SQT_INT;
                    else if (esc[0] == 'B') type = SQT_BLB;
                    else                    type = 0;

                    esc[1] = CAPITAL(*pfor);
                    if (*pfor) pfor += 1;
                    icol = 0;
                    if        ((esc[1] >= '0')&&(esc[1] <= '9')) {
                        icol =  esc[1] - '0';
                    } else if ((esc[1] >= 'A')&&(esc[1] <= 'Z')) {
                        icol = (esc[1] - 'A') + 10;
                    }                       

                    esc[2] = CAPITAL(*pfor);
                    if (*pfor) pfor += 1;
                    iout = 0;
                    if        ((esc[2] >= '1')&&(esc[2] <= '9')) {
                        iout =  esc[2] - '0';
                    } else if ((esc[2] >= 'A')&&(esc[2] <= 'Z')) {
                        iout = (esc[2] - 'A') + 10;
                    }                       

                    if (iout > 0) {             //output to pesc structure?
                        pinfo = pesc + (iout - 1);
                        pinfo->dType = type;    //set up structure to do below
                        pinfo->BindC = -icol;   //negative column for output
                        continue;               //and skip for pout text
                    }
                    if (pout == NULL) {         //no pout text output?
                        continue;
                    }

                    icol -= 1;                  //sqlite uses 0 for 1st column
                    if        (type == SQT_TXT) {
                        ptx3 = (sq3txt*) sqlite3_column_text(res, icol);
                        if (ptx3 == NULL) return(ECBADCODE);
                        have = sqlite3_column_bytes(res, icol);

                        if ((pout+have) >= pend) return(ECNOROOM);
                        OSMemCopy(pout, (void*)ptx3, have);
                        pout += have;           //output text value
                        *pout = 0;

                    } else if (type == SQT_FLT) {
                        if ((pout+16) >= pend) return(ECNOROOM);
                        fval = sqlite3_column_double(res, icol);

                        pout = OSFloatPut(fval, pout, 0, 0);
                        *pout = 0;              //output float value

                    } else if (type == SQT_INT) {
                        if ((pout+16) >= pend) return(ECNOROOM);
                        i64 = sqlite3_column_int64(res, icol);

                        pout = OSIntPutQ((qint)i64, pout);
                        *pout = 0;              //output integer value

                    } else if (type == SQT_BLB) {
                        return(ECCANNOT);       //cannot handle blob text output
                    }
                    continue;
                }

                if (pout == NULL) {             //no text output?
                    pfor += 1;
                    continue;
                }
                if (pout >= pend) return(ECNOROOM);
                *pout++ = *pfor++;              //copy non-escape verbatum
            }
            if (pout) *pout = 0;                //zero terminate output
        }

        // Handle returning any column values specified in pesc structures.
        // Here we output to pesc structures rather than pout text.
        ii = -1;
        while (++ii < nesc) {                   //see what pesc cols to return
            icol = pesc[ii].BindC;              //column index (-1=1st)
            if (icol < 0) {                     //binds to column value?
                icol = -1 - icol;               //now 0=1st
                psav  = pesc[ii].Point;         //pointer to where we save it
                type  = pesc[ii].dType;         //type of data to bind
                limit = pesc[ii].Limit;         //bytes of space available
                limit = ABS(limit);             //negative if allocated memory

                if        (type == SQT_TXT) {
                    ptx3 = sqlite3_column_text(res, icol);
                    if (ptx3 == NULL) return(ECBADCODE);
                    have = sqlite3_column_bytes(res, icol);
                    if (  ((have+1) > limit)    //must allocate memory?
                        ||(psav == NULL)  ) {
                        if (noalloc) return(ECNOROOM);
                        limit = have+1;         //plus one char for ending 0
                        if (limit < SQ3_ALLSZ) limit = SQ3_ALLSZ;
                        psav = (char*) OSAlloc(limit);
                        if (psav == NULL) return(ECNOROOM);
                        if (pesc[ii].Point) OSFree(pesc[ii].Point);
                        pesc[ii].Point = psav;  //use allocated memory
                        pesc[ii].Limit = -limit;//shows we allocated
                    }

                    OSMemCopy(psav, (void*)ptx3, have);
                    psav[have] = 0;             //copy and zero terminate text
                    pesc[ii].Bytes = have;      //copy and return size

                } else if (type == SQT_FLT) {
                    fval = sqlite3_column_double(res, icol);
                    pesc[ii].Value = fval;      //return double value in pesc

                } else if (type == SQT_INT) {   //return big int as double
                    i64 = sqlite3_column_int64(res, icol);
                    pesc[ii].Int64 = (qint) i64;

                } else if (type == SQT_BLB) {
                    pbl3 = sqlite3_column_blob(res, icol);
                    if (pbl3 == NULL) return(ECBADCODE);
                    have = sqlite3_column_bytes(res, icol);
                    if (  (have > limit)
                        ||(psav == NULL)  ) {   //must allocate memory?
                        if (noalloc) return(ECNOROOM);
                        limit = have;
                        if (limit < SQ3_ALLSZ) limit = SQ3_ALLSZ;
                        psav = (char*) OSAlloc(limit);
                        if (psav == NULL) return(ECNOROOM);
                        if (pesc[ii].Point) OSFree(pesc[ii].Point);
                        pesc[ii].Point = psav;  //use allocated memory
                        pesc[ii].Limit = -limit;//shows we allocated
                    }

                    OSMemCopy(psav, (void*)pbl3, have);
                    pesc[ii].Bytes = have;      //copy and return size

                } else {
                    return(ECINVALID);
                }
            }
        }
    }

    // Handle finalizing statement.
    if (mode & SQ3_FINAL) {
        if (res) sqlite3_finalize(res);
        data->res = NULL;
    }

    // Handle closing database.
    if (mode & SQ3_CLOSE) {
        if (db) sqlite3_close(db);
        data->db = NULL;
    }

    // All done.
    return(0);
}

//*************************************************************************
//  SqlTest:
//      Self test for this files SQLite3 API.
//
//  Arguments:
//      int SqlTest (int mode)
//      mode    Reserved for future use and must be zero.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

CSTYLE
int SqlTest (int mode) {
    char outs[SQ3_OUTSZ];
    SqlIn info[32];
    DBase data;
    char* form;
    char* psql;
    int err, siz, row, ii, rv;

    err = 0;
    printf("** SqlTest running...\n");

    siz = 10;
    OSMemClear(info, sizeof(SqlIn)*siz);
    OSMemClear(&data, sizeof(DBase));
    data.fname = "C:\\ProgramData\\CernoPS\\fake.cpd";
    data.db = data.res = NULL;

    rv = Sqlite(SQ3_OPEN, &data, NULL, NULL, NULL,0, NULL,0);
    if (rv < 0) {
        printf("!! Sqlite SQ3_OPEN #1 error %i\n", rv);
        err = rv;
    }
    form = "@F11,@F22,@F33,@F44,@F55,@F66,@F77,@F88,@F99,@FAA";
    psql = "SELECT * FROM cps_meta";
    ii = SQ3_EXEC+SQ3_STEP+SQ3_FINAL;
    rv = Sqlite(ii, &data, psql, form, NULL,0, info,siz);
    if (rv < 0) {
        printf("!! Sqlite SQ3_EXEC #1 error %i\n", rv);
        err = rv;
    }
    if ((info[1].Value != 25001.0)||(info[8].Value != 1.0)||(info[9].Value != 0.5)) {
        printf("!! Sqlite SQ3_EXEC #1 bad returned values\n");
        err = -1;
    }
    printf("** fake.cpd meta = %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
           info[0].Value,info[1].Value,info[2].Value,info[3].Value,info[4].Value,
           info[5].Value,info[6].Value,info[7].Value,info[8].Value,info[9].Value);

    siz = 4;
    OSMemClear(info, sizeof(SqlIn)*siz);
    row = 25001;
    info[0].dType = SQT_INT;
    info[0].BindC = 1;
    info[0].Int64 = row;
    form = "** MW=@F10 [@T20] @T30 (Lib#%i)\n";
    psql = "SELECT mw,formula,name FROM cps_data WHERE rowid = ?";
    ii = SQ3_EXEC+SQ3_STEP+SQ3_FINAL;
    rv = Sqlite(ii, &data, psql, form, outs,0, info,siz);
    if (rv < 0) {
        printf("!! Sqlite SQ3_EXEC #2 error %i\n", rv);
        err = rv;
    }
    if (OSTxtCmp("** MW=234.5 [F4KE] Target",outs) & 1) {
        printf("!! Sqlite SQ3_EXEC #2 bad returned output text: %s\n",outs);
        err = -2;
    }
    if (*outs == 0) OSTxtCopy(outs,"!! BAD\n"); //show bad line if error
    printf("** #2 fake row=%i: %s",row,outs);   //show output formatted text
    printf(outs, row);                          //format was printf format

    siz = 4;
    OSMemClear(info, sizeof(SqlIn)*siz);
    row = 2;
    info[0].dType = SQT_INT;
    info[0].BindC = 1;
    info[0].Int64 = row;
    form = "** MW=@F10 [@T20] @T30 (Lib#%i)\n";
    psql = "SELECT mw,formula,name FROM cps_data WHERE rowid = ?";
    ii = SQ3_EXEC+SQ3_STEP+SQ3_FINAL;
    rv = Sqlite(ii, &data, psql, form, outs,0, info,siz);
    if (rv < 0) {
        printf("!! Sqlite SQ3_EXEC #3 error %i\n", rv);
        err = rv;
    }
    if (OSTxtCmp("** MW=123.4 [F4KE] Fake1",outs) & 1) {
        printf("!! Sqlite SQ3_EXEC #3 bad returned output text\n");
        err = -3;
    }
    if (*outs == 0) OSTxtCopy(outs,"!! BAD\n"); //show bad line if error
    printf("** #3 fake row=%i: %s",row,outs);   //show output formatted text
    printf(outs, row);                          //format was printf format

    siz = 4;
    OSMemClear(info, sizeof(SqlIn)*siz);
    row = 3;
    info[0].dType = SQT_INT;
    info[0].BindC = 1;
    info[0].Int64 = row;
    form = "MW=@F12,@T23,@T34\n";               //output to info not outs text
    psql = "SELECT mw,formula,name FROM cps_data WHERE rowid = ?";
    ii = SQ3_EXEC+SQ3_STEP+SQ3_FINAL;
    rv = Sqlite(ii, &data, psql, form, NULL,0, info,siz);
    if (rv < 0) {
        printf("!! Sqlite SQ3_EXEC #4 error %i\n", rv);
        err = rv;
    }
    if (((info[1].Value - 123.4) > 0.01)||(OSTxtCmpS("Fake2",info[3].Point))) {
        printf("!! Sqlite SQL_EXEC $4 bad returned values\n");
        err = -4;
    }
    printf("** #4 fake row = %.2f, %s, %s\n",
           info[0].Value,info[2].Point,info[3].Point);

    rv = Sqlite(SQ3_FREE, &data, NULL, NULL, NULL,0, info,siz);
    if (rv < 0) {
        printf("!! Sqlite SQ3_FREE #1 error %i\n", rv);
        err = rv;
    }

    rv = Sqlite(SQ3_CLOSE, &data, NULL, NULL, NULL,0, NULL,0);
    if (rv < 0) {
        printf("!! Sqlite SQ3_CLOSE #1 error %i\n", rv);
        err = rv;
    }
    return(err);
}