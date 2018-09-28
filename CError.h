//*************************************************************************
//  FILENAME:   CError.h
//  AUTHOR:     SCSimonoff 10-18-96
//  MRU:        3-30-03
//
//  Common Error Codes.
//  This verson uses negative decimal numbers instead of hex
//  for python script readability and is not intended for C/C++ apps
//  Compile with /D CERROR_HEX for old hex error codes.
//
//  These definitions were created 1996-2011 by SCSimonoff for Triscape Inc.
//  They may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _CError                     //Only define once if include multiple times
#define _CError

#ifndef CERROR_HEX                  //****
// Negative decimal numbers instead of hex
// Versions of standard Windows errors with different numbers (-1000 to -1199).

#define ECERROR       (       -1000)//Unknown error
#define ECNOFILE      (       -1001)//File not found
#define ECNOCREATE    (       -1002)//Cannot create new file (or path not found)
#define ECNOOPEN      (       -1003)//Cannot open file (or too many open files)
#define ECDENIED      (       -1004)//Access denied     (RESERVED FOR STMERRORS)
#define ECHANDLE      (       -1005)//Invalid handle
#define ECMEMORY      (       -1006)//Insufficient memory
#define ECDATABAD     (       -1007)//Data is not formatted as expected
#define ECNOMORE      (       -1008)//No more files
#define ECPROTECT     (       -1009)//Write protected   (RESERVED FOR STMERRORS)
#define ECEOF         (       -1010)//End of file
#define ECDISKFULL    (       -1011)//Disk is full      (RESERVED FOR STMERRORS)
#define ECSHARING     (       -1012)//File is in use    (RESERVED FOR STMERRORS)
#define ECEXISTS      (       -1014)//Already exists
#define ECBADNAME     (       -1015)//Bad filename      (RESERVED FOR STMERRORS)
#define ECPENDING     (       -1016)//Background operation is still in progress
#define ECCORRUPT     (       -1017)//Bad file encrypt

// Non-standard Triscape error codes (-1020 to -1199).

#define ECOPEN        (       -1020)//File open or create error
#define ECREAD        (       -1021)//File read error
#define ECWRITE       (       -1022)//File write error
#define ECSEEK        (       -1023)//File seek error
#define ECSYS         (       -1024)//OS error return
#define ECSHARE       (       -1025)//Needs share to run

#define ECLICENSE     (       -1026)//Product not licensed
#define ECPLUG        (       -1027)//Bad or missing protection dongle
#define ECDEMO        (       -1028)//Demo version (function not supported)
                                 
#define ECCANNOT      (       -1030)//Operation not supported in this version
#define ECARGUMENT    (       -1031)//Invalid argument
#define ECNOTLEGAL    (       -1032)//Invalid operation request
#define ECSYNTAX      (       -1033)//Syntax error
#define ECBADFILE     (       -1034)//File not correctly formatted
#define ECCONVERT     (       -1035)//File conversion error

#define ECNOPTS       (       -1040)//Result has no data points
#define ECFEWPTS      (       -1041)//Too few data points
#define ECBADNPTS     (       -1042)//Wrong number of points
#define ECBADDIMS     (       -1043)//Wrong matrix dimensions
#define ECBADPEAK     (       -1044)//Bad peak location
#define ECNEEDALL     (       -1045)//Operation requires whole file to be loaded
#define ECNEEDXY      (       -1046)//XY data is required
#define ECNACK        (       -1047)//NACK not acknowledged

#define ECNOINIT      (       -1048)//Not properly initialized
#define ECNULL        (       -1049)//NULL pointer or No source points
#define ECCOMM        (       -1050)//Server or communications error
#define ECBUSY        (       -1051)//Routine or resource already in use
#define ECRANGE       (       -1052)//Value is out of allowed range
#define ECNOROOM      (       -1053)//Insufficient free data space or structures
#define ECNOMATCH     (       -1054)//No matching data or Not found

#define ECDELETED     (       -1055)//Object deleted itself
#define ECRECURSE     (       -1056)//Cannot be called recursively
#define ECINVALID     (       -1057)//Operation not valid at this time
#define ECBADCODE     (       -1058)//Unanticipated error that should not happen
#define ECSTOPPED     (       -1059)//Operation should be stopped now
#define ECNOWRITE     (       -1060)//Read-only or data cannot be written
#define ECEXCEPTN     (       -1061)//Handled exception error
#define ECNORIGHT     (       -1062)//Do not have data rights
#define ECPASSWRD     (       -1063)//Password error
#define ECBADSIZE     (       -1064)//File size overflow
#define ECBEHIND      (       -1065)//Behind or too slow
#define ECINVERT      (       -1066)//Cannot invert/svd/pca data
#define ECMISSING     (       -1067)//Missing file
#define ECTIMEOUT     (       -1068)//Timeout
#define ECFILEIO      (       -1069)//File I/O open/read/write/seek error

// These are used for special globally unique Cerno Rx error codes and
// allow certain RxSearch error codes to indicate specific problems.
// ECERROR1 - ECERROR3 NIST SetAux errors.
// ECERROR4 - ECERROR7 NIST Create invalid libs argument.
// ECERROR9 CG Calibrate missing .ric file widths.
// ECERRORA Calibrate std does not fit X range.
// ECERRORB Calibrate did not find expected cgm peaks.
// ECERRORC WorksClips spectrum spc buffer npts insufficient.
// ECERRORD WorksClips modify list syntax error.
// ECERRORE WorksClips element list syntax error.
// ECERRORF WorksClips formula argument syntax error.
// ECERRORH WorksIsotopes formula error.
// ECERRORI Matrix inversion error.
// ECERRORJ WorksIsotopes distribution error.

#define ECERROR0      (       -1070)//Not yet used
#define ECERROR1      (       -1071)//NIST SetAux szaux invalid
#define ECERROR2      (       -1072)//NIST SetAux szcontrols invalid
#define ECERROR3      (       -1073)//NIST SetAux szconstr invalid
#define ECERROR4      (       -1074)//NIST Create libs text too many characters
#define ECERROR5      (       -1075)//NIST Create not a valid library
#define ECERROR6      (       -1076)//NIST Create cannot find library folder
#define ECERROR7      (       -1077)//NIST Create library does not exist
#define ECERROR8      (       -1078)//Not yet used
#define ECERROR9      (       -1079)//CG Calibrate missing .ric file widths

#define ECERRORA      (       -1080)//Calibrate std does not fit X range
#define ECERRORB      (       -1081)//Calibrate did not find expected cgm peaks
#define ECERRORC      (       -1082)//WorksClips spc buffer npts insufficient
#define ECERRORD      (       -1083)//WorksClips modify argument syntax error
#define ECERRORE      (       -1084)//WorksClips elemList argument syntax error
#define ECERRORF      (       -1085)//WorksClips formula argument syntax error
#define ECERRORG      (       -1086)//Gaussian/Boxcar shape generation error
#define ECERRORH      (       -1087)//Chemical Formula error
#define ECERRORI      (       -1088)//Matrix Inversion error
#define ECERRORJ      (       -1089)//Isotope Distribution error

// These are non-severe "warning" messages (-1200 to -1249).

#define ECNODATA      (       -1101)//Requested data does not exist (warning)
#define ECNOTDONE     (       -1102)//Operation not fully completed (warning)
#define ECREDRAW      (       -1103)//Caller should redraw the client window
#define ECRESTATUS    (       -1104)//Caller should redraw the status bar
#define ECREBOTH      (       -1105)//Caller should redraw both client & status
#define ECNOFREE      (       -1106)//Caller should not free memory (warning)
#define ECNOTSAVED    (       -1107)//Data changed and not saved (warning)
#define ECABORTED     (       -1108)//Operation was aborted by user (warning)

// These are positive "information" messages (+1250 to +1299).

#define ECSTOP        (       +1251)//False return value or operation stopped
#define ECTRUE        (       +1252)//True return value         or solid green
#define ECSOME        (       +1253)//Only partially sucessful  or flash green
#define ECSKIP        (       +1254)//Nothing needed to be done or flash yellow
#define ECWAIT        (       +1255)//Waiting to complete       or solid yellow
#define ECIDLE        (       +1256)//Routine or resource has nothing to do
#define ECDRAW        (       +1257)//Drawing or save needed
#define ECINIT        (       +1258)//Initialized
#define ECSAVE        (       +1259)//Save data

// These are Cerno specific warnings (100-199) and errors (-200-299)

#define ECNOCAL                 101 //Processed without mwc calibration warning
#define ECNORXC                 102 //Processed without rxc calibration warning
#define ECNORIC                 103 //Processed without ric calibration warning
#define ECMISSED                104 //Processed with missing peaks due to errors
#define ECMTDER                -200 //Python Method Process Error

#else                               //****
// Old-style hex error code numbers.
// Error values can be returned as errc or HRESULT (which are equivalent).
// All COM (and especially DCOM) functions must return HRESULT (or errc).
// This is because the RPC mechanism needs to flag transmission errors.
// It is OK to return postive long values as valid HRESULT (or errc) codes.
// It is guaranteed that ECNONE is zero and ECINFO values are positive,
// while warnings and severe error values are always negative.
// Note that test codes conform to the WINERROR.H definitions and
// have the Customer code bit set and use the FACILITY_NULL facility code.
// The ECBASE adds the "severe error" and "customer" bits and the NULL facility.

#define ECNONE          0           //no error (this must always be zero)
#define ECINFO          0x060000000 //added to informational value in low bits
#define ECWARN          0x0A0000000 //added to warning values
#define ECBASE          0x0E0000000 //added to error values

#define IS_SUCCESS(e)   (((e & 0xC0000000) == 0x00000000)?TRUE:FALSE) //succeed?
#define IS_INFO(e)      (((e & 0xC0000000) == 0x40000000)?TRUE:FALSE) //info?
#define IS_WARN(e)      (((e & 0xC0000000) == 0x80000000)?TRUE:FALSE) //warning?
#define IS_SEVERE(e)    (((e & 0xC0000000) == 0xC0000000)?TRUE:FALSE) //severe?

// These match the Windows error codes from WINERROR.H in the low 16 bits.

#define ECERROR       (ECBASE+0x001)//Unknown error
#define ECNOFILE      (ECBASE+0x002)//File not found
#define ECNOCREATE    (ECBASE+0x003)//Cannot create new file (or path not found)
#define ECNOOPEN      (ECBASE+0x004)//Cannot open file (or too many open files)
#define ECDENIED      (ECBASE+0x005)//Access denied     (RESERVED FOR STMERRORS)
#define ECHANDLE      (ECBASE+0x006)//Invalid handle
#define ECMEMORY      (ECBASE+0x008)//Insufficient memory
#define ECDATABAD     (ECBASE+0x00D)//Data is not formatted as expected
#define ECNOMORE      (ECBASE+0x012)//No more files
#define ECPROTECT     (ECBASE+0x013)//Write protected   (RESERVED FOR STMERRORS)
#define ECEOF         (ECBASE+0x026)//End of file
#define ECDISKFULL    (ECBASE+0x027)//Disk is full      (RESERVED FOR STMERRORS)
#define ECSHARING     (ECBASE+0x032)//File is in use    (RESERVED FOR STMERRORS)
#define ECEXISTS      (ECBASE+0x0B7)//Already exists
#define ECBADNAME     (ECBASE+0x07B)//Bad filename      (RESERVED FOR STMERRORS)
#define ECPENDING     (ECBASE+0x3E5)//Background operation is still in progress
#define ECCORRUPT     (ECBASE+0x3E6)//Bad file encrypt

// These in the 300-400 hex region are not standard Windows error codes.

#define ECOPEN        (ECBASE+0x300)//File open or create error
#define ECREAD        (ECBASE+0x301)//File read error
#define ECWRITE       (ECBASE+0x302)//File write error
#define ECSEEK        (ECBASE+0x303)//File seek error
#define ECSYS         (ECBASE+0x304)//OS error return
#define ECSHARE       (ECBASE+0x305)//Needs share to run

#define ECLICENSE     (ECBASE+0x306)//Product not licensed
#define ECPLUG        (ECBASE+0x307)//Bad or missing protection dongle
#define ECDEMO        (ECBASE+0x308)//Demo version (function not supported)

#define ECCANNOT      (ECBASE+0x309)//Operation not supported in this version
#define ECARGUMENT    (ECBASE+0x30A)//Invalid argument
#define ECNOTLEGAL    (ECBASE+0x30B)//Invalid operation request
#define ECSYNTAX      (ECBASE+0x30C)//Syntax error
#define ECBADFILE     (ECBASE+0x30D)//File not correctly formatted
#define ECCONVERT     (ECBASE+0x30E)//File conversion error

#define ECNOPTS       (ECBASE+0x30F)//Result has no data points
#define ECFEWPTS      (ECBASE+0x310)//Too few data points
#define ECBADNPTS     (ECBASE+0x311)//Wrong number of points
#define ECBADDIMS     (ECBASE+0x312)//Wrong matrix dimensions
#define ECBADPEAK     (ECBASE+0x313)//Bad peak location
#define ECNEEDALL     (ECBASE+0x314)//Operation requires whole file to be loaded
#define ECNEEDXY      (ECBASE+0x315)//XY data is required
#define ECNACK        (ECBASE+0x316)//NACK not acknowledged

#define ECNOINIT      (ECBASE+0x317)//Not properly initialized
#define ECNULL        (ECBASE+0x318)//NULL pointer or No source points
#define ECCOMM        (ECBASE+0x319)//Server or communications error
#define ECBUSY        (ECBASE+0x31A)//Routine or resource already in use
#define ECRANGE       (ECBASE+0x31B)//Value is out of allowed range
#define ECNOROOM      (ECBASE+0x31C)//Insufficient free data space or structures
#define ECNOMATCH     (ECBASE+0x31D)//No matching data or Not found

#define ECDELETED     (ECBASE+0x31E)//Object deleted itself
#define ECRECURSE     (ECBASE+0x31F)//Cannot be called recursively
#define ECINVALID     (ECBASE+0x320)//Operation not valid at this time
#define ECBADCODE     (ECBASE+0x321)//Unanticipated error that should not happen
#define ECSTOPPED     (ECBASE+0x322)//Operation should be stopped now
#define ECNOWRITE     (ECBASE+0x323)//Read-only or data cannot be written
#define ECEXCEPTN     (ECBASE+0x324)//Handled exception error
#define ECNORIGHT     (ECBASE+0x325)//Do not have data rights
#define ECPASSWRD     (ECBASE+0x326)//Password error
#define ECBADSIZE     (ECBASE+0x327)//File size overflow
#define ECBEHIND      (ECBASE+0x328)//Behind or too slow
#define ECINVERT      (ECBASE+0x329)//Cannot invert
#define ECMISSING     (ECBASE+0x32A)//Missing file
#define ECTIMEOUT     (ECBASE+0x32B)//Timeout
#define ECFILEIO      (ECBASE+0x32C)//File I/O open/read/write/seek error

#define ECERROR0      (ECBASE+0x330)//Unexpected error #0 (meaning varies)
#define ECERROR1      (ECBASE+0x331)//Unexpected error #1 (meaning varies)
#define ECERROR2      (ECBASE+0x332)//Unexpected error #2 (meaning varies)
#define ECERROR3      (ECBASE+0x333)//Unexpected error #3 (meaning varies)
#define ECERROR4      (ECBASE+0x334)//Unexpected error #4 (meaning varies)
#define ECERROR5      (ECBASE+0x335)//Unexpected error #5 (meaning varies)
#define ECERROR6      (ECBASE+0x336)//Unexpected error #6 (meaning varies)
#define ECERROR7      (ECBASE+0x337)//Unexpected error #7 (meaning varies)
#define ECERROR8      (ECBASE+0x338)//Unexpected error #8 (meaning varies)
#define ECERROR9      (ECBASE+0x339)//Unexpected error #9 (meaning varies)

// These are non-severe "warning" messages.

#define ECNODATA      (ECWARN+0x341)//Requested data does not exist (warning)
#define ECNOTDONE     (ECWARN+0x342)//Operation not fully completed (warning)
#define ECREDRAW      (ECWARN+0x343)//Caller should redraw the client window
#define ECRESTATUS    (ECWARN+0x344)//Caller should redraw the status bar
#define ECREBOTH      (ECWARN+0x345)//Caller should redraw both client & status
#define ECNOFREE      (ECWARN+0x346)//Caller should not free memory (warning)
#define ECNOTSAVED    (ECWARN+0x347)//Data changed and not saved (warning)
#define ECABORTED     (ECWARN+0x348)//Operation was aborted by user (warning)

// These are positive "information" messages.

#define ECSTOP        (ECINFO+0x360)//False return value or operation stopped
#define ECTRUE        (ECINFO+0x361)//True return value         or solid green
#define ECSOME        (ECINFO+0x362)//Only partially sucessful  or flash green
#define ECSKIP        (ECINFO+0x363)//Nothing needed to be done or flash yellow
#define ECWAIT        (ECINFO+0x364)//Waiting to complete       or solid yellow
#define ECIDLE        (ECINFO+0x365)//Routine or resource has nothing to do
#define ECDRAW        (ECINFO+0x366)//Drawing or save needed
#define ECINIT        (ECINFO+0x367)//Initialized
#define ECSAVE        (ECINFO+0x368)//Save data

#define ECNOT0        (ECINFO+0x370)//Alternate return value
#define ECONE1        (ECINFO+0x371)//Alternate return value
#define ECTWO2        (ECINFO+0x372)//Alternate return value
#define ECTRI3        (ECINFO+0x373)//Alternate return value
#define ECFOR4        (ECINFO+0x374)//Alternate return value
#define ECPEN5        (ECINFO+0x375)//Alternate return value
#define ECSIX6        (ECINFO+0x376)//Alternate return value
#define ECSEV7        (ECINFO+0x377)//Alternate return value
#define ECOCT8        (ECINFO+0x378)//Alternate return value
#define ECNIN9        (ECINFO+0x379)//Alternate return value

// More general error codes.

#define ECERRORA      (ECBASE+0x390)//Unexpected error #0 (meaning varies)
#define ECERRORB      (ECBASE+0x391)//Unexpected error #1 (meaning varies)
#define ECERRORC      (ECBASE+0x392)//Unexpected error #2 (meaning varies)
#define ECERRORD      (ECBASE+0x393)//Unexpected error #3 (meaning varies)
#define ECERRORE      (ECBASE+0x394)//Unexpected error #4 (meaning varies)
#define ECERRORF      (ECBASE+0x395)//Unexpected error #5 (meaning varies)
#define ECERRORG      (ECBASE+0x396)//Unexpected error #6 (meaning varies)
#define ECERRORH      (ECBASE+0x397)//Unexpected error #7 (meaning varies)
#define ECERRORI      (ECBASE+0x398)//Unexpected error #8 (meaning varies)
#define ECERRORJ      (ECBASE+0x399)//Unexpected error #9 (meaning varies)

// These are exact dulplicates of OLE error codes (from WINERROR.H).
// They are redefined here so that we need not include all of the OLE stuff.

#define ECIUNKNOWN      0x80020001  //Interface is unknown

#endif                              //**** Hex
#endif                              //_CError
