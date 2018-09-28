//*************************************************************************
//  FILENAME:   TriGL.cpp
//  AUTHOR:     SCSimonoff  4-3-14
//  CPU:        Portable but contitionally compiles for x86 optimizations.
//  O/S:        Portable
//  MRU:
//
//  This is a simple 2D Graphics Library
//  wutg various low-level image processing functions using BMAP bitmaps.
//  It uses x86 MMX if available but not graphics co-processors.
//  So it is fairly portable yet also fairly fast.
//  It includes the ability to read/write JPEG and PNG files.
//  JPEG requires linking with a specially modified jpeg.lib
//  that has file I/O calls linked to StmRead/StmWrite in TriOS.cpp.
//  PNG requires linking with a standard libpng.lib.
//  
//  Compile with /D "NOI86" for C-only version that does not require x86.
//  Compile with /D "NOMMX" for version that does not use MMX instructions.
//  Compile with /D "XDEBUG" for special memory overwrite testing.
//  Compile with /D "NOWRITE" to disable JPEG and PNG file writing.
//  Compile with /D "PLOTAVG" to use DrawPlot average instead of max/min.
//  Compile with /D "NOANGLE" to disable BitsAngle, BitsTurn90, BitsSwivel.
//  Compile with /D "NOJPEG" to disable JPEG and not require jpeg.lib.
//  Compile with /D "NOPNG" to disable PNG and not require libpng.lib.
//  Normally the x86 version uses MMX if available or floating if not.
//
//  Uses TriOS.h portable non-gui system calls.
//  Uses CDefs.h custom types such as: cint,byte,sint,word,lint,dword,flt,flx
//
//  This software was created 1997-2012 by SCSimonoff.
//  It may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#include "TriOS.h"
#include "TriGL.h"

#ifndef NOJPEG                  //---
#undef   FAR                    //Solve some definition conflicts with JPEGLIB
#undef   FILE
#define  FILE void*
#include <jpeglibtri.h>         //for our special libjpegtri.a version
#endif                          //---
#ifndef NOPNG                   //---
#include <png.h>
#endif                          //---

// Functions in this file.

CSTYLE_BEG                      //Keep it simple and do not require C++ caller
csproc      BitsNew (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal);
csproc      BitsMake (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal);
csproc      BitsFree (BMAP* pmap);

lint        BitsColor (BMAP* pmap, lint color);
csproc      BitsBlank (BMAP* pmap, IBox* pbox, lint color);
csproc      BitsBlankInv (int mode, BMAP* pmap, IBox* pbox, lint color);

csproc      BitsCopy (int mode, BMAP* pdes, IPt* pto, BMAP* psrc, IBox* pfrom);
csproc      BitsColumn (int mode, BMAP* pdes, IPt* pto, lint* psrc, lint npt);
csproc      BitsStretch (int mode, BMAP* pdes, IBox* pto, BMAP* psrc, IBox* pfrom);

csproc      BitsAngle (int mode, BMAP* pdes, IPt* pto, flt angle, BMAP* psrc, IBox* pfrom);
csproc      BitsTurn90 (int mode, BMAP* pdes, BMAP* psrc);
csproc      BitsSwivel (int mode, BMAP* pdes, BMAP* psrc, flt angle, IPt* pctr, IBox* pto);

csproc      BitsLine (int mode, BMAP* pmap, lint x1, lint y1, lint x2, lint y2, lint width, lint color);
csproc      BitsLines (int mode, BMAP* pmap, IPt* ppts, lint npts, lint width, lint color, lint* pcolors);
csvoid      BitsLineHue (int mode, BMAP* pmap, lint* pcolors, lint width, lint color);
csproc      BitsPolygon (int mode, BMAP* pmap, IPt* ppts, lint npts, lint color);

csproc      BitsPen (int mode, BMAP* pmap, IPt* ppts, lint npts, BMAP* ppen, IBox* pfrom);
csvoid      BitsPenPoint (BMAP* pmap, IPt* pto, BMAP* ppen, IBox* pfrom);
csvoid      BitsPenErase (BMAP* pmap, IPt* pto, BMAP* ppen, IBox* pfrom, int mode);
csproc      BitsPenCreate (int mode, BMAP* ppen, lint width, lint* pcolors);
csproc      BitsPattern (int mode, BMAP* pmap, IPt* ppts, lint npts, lint pattern, lint fore, lint back);

csvoid      BitsAlpha (BMAP* pmap, lint alpha);
csproc      BitsAlphaFill (int mode, BMAP* pmap, IBox* pbox, lint alpha, lint color);
csproc      BitsAlphaKeep (int mode, BMAP* pmap, IBox* pbox, BMAP* ptmp);
csproc      BitsAlphaDup (int mode,BMAP* pdes,IPt* pto,BMAP* psrc,IBox* pfrom);
csproc      BitsAlphaSet (int mode, BMAP* pmap, lint color, lint alpha);

csproc      BitsBytes (lint pix, lint nx, lint ny, lint* prow, lint* psize);
csproc      BitsCPU ();
csproc      BitsCPU2 ();

csvoid      BitsHorz (BMAP* pmap, lint ox, lint oy, lint nx, lint ny, lint skipbeg, lint skipend, lint value, lint omit);
csvoid      BitsVert (BMAP* pmap, lint ox, lint oy, lint nx, lint ny, lint skipbeg, lint skipend, lint value, lint omit);

csvoid      BitsBlendInit (lint cpu, lint iratio, lint* mxshift, word* mxratio, word* mxone);
csvoid      BitsBlend (BGR* pd, lint dnum, lint dinc, BGR* ps, lint snum, lint sinc, lint iratio, flt fratio, int cpu, lint mxshift, word* mxratio, word* mxone);
csvoid      BitsBlendC (BGR* pd, lint dnum, lint dinc, BGR* ps, lint snum, lint sinc, lint iratio, flt fratio, int cpu, lint mxshift, word* mxratio, word* mxone);
csvoid      BitsBlend8 (BGR* pd, lint dnum, lint dinc, byte* ps, lint snum, lint sinc, BGR* spal, lint iratio, flt fratio, int cpu, lint mxshift, word* mxratio, word* mxone);
csvoid      BitsBlend1 (BGR* pd, lint dnum, lint dinc, byte* ps, lint snum, lint bitm, BGR* spal, lint iratio, flt fratio, int cpu, lint mxshift, word* mxratio, word* mxone);

csproc      BitsText (int mode, BFont* pfont, BMAP* pmap, lint x, lint y, lint color, text* ptxt, lint ctxt, IPt* psiz);

cslint      DrawStd (lint color);
cslint      DrawRGB (lint color);
csvoid      DrawBGR (BGR* pbgr, lint color);

cslint      DrawHLS (flx hue, flx lum, flx sat);
csvoid      DrawHLS2BGR (BGR* pbgr, flt hue, flt lum, flt sat);
csvoid      DrawBGR2HLS (flt* phue, flt* plum, flt* psat, lint bgr);

csvoid      DrawAngles (lint mode, flt angle, IPt* pctr, IPt* ppts, lint npts);
csproc      DrawCircle (lint mode, ISegs* ppts, IPt* pctr, IPt* pbeg, IPt* pend);
csproc      DrawOval (lint mode, ISegs* ppts, IPt* pctr, IPt* poff);

csproc      DataHues (int mode, lint* pdat, lint npts, LHUE* phue, IPt* plim);
csproc      DataPlot (int mode, lint* pout, lint nout, lint* pdat, lint npts, flt yfac);

csproc      BitsGraph (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts, lint ydelt, lint yzero, lint* pcolor);
csproc      Bits2DAdd (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts, LHUE* phue, IPt* plim);

#ifndef     NOJPEG                              //---
csproc      JPEGWrite (lint mode, fStm* pstream, BMAP* pmap, IBox* pbox, byte* pexifs);
csproc      JPEGRead (lint mode, fStm* pstream, BMAP* pmap);
METHODDEF(void) JPEGErrorExit (j_common_ptr cinfo);
#endif                                          //---

#ifndef     NOPNG                               //---
void        my_read_data(png_structp png_ptr, png_bytep data, png_size_t length);
void        my_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
void        my_flush_data(png_structp png_ptr);
void        png_my_error (png_structp png_ptr, png_const_charp message);
void        png_my_warning(png_structp png_ptr, png_const_charp message);

csproc      PNGWrite (lint mode, fStm* pstream, BMAP* pmap);
csproc      PNGRead (lint mode, fStm* pstream, BMAP* pmap);

csproc      GLTest (int mode, text* pdir);
#endif                          //---

// Standard Colors.

const lint StdColors[] = {
    0x04000000,     //CNULL     No Color

    0x00000000,     //CBLACK    Windows Black
    0x00000080,     //CMAROON   Windows Dark Red
    0x00008000,     //CGREEN    Windows Dark Green
    0x00008080,     //COLIVE    Windows Dark Yellow
    0x00800000,     //CNAVY     Windows Dark Blue
    0x00800080,     //CPURPLE   Windows Dark Magenta
    0x00808000,     //CTEAL     Windows Dark Cyan
    0x00808080,     //CGREY     Windows Dark Grey

    0x00C0DCC0,     //CSAGE     Windows Money Green (Not VGA)
    0x00F0CAA6,     //CSKY      Windows Sky Blue    (Not VGA)

    0x00404080,     //CBROWN    Non-Windows Dark Dull Red
    0x00AAAAC5,     //CDUSK     Non-Windows Dull Red
    0x00AAD5AA,     //CFOREST   Non-Windows Dull Green
    0x00DFF7FF,     //CSALMON   Non-Windows Light Dull Red
    0x000080FF,     //CORANGE   Non-Windows Yellow-Red
    0x00B0FFF0,     //CKHAKI    Non-Windows Yellow-Green
    0x0080FF00,     //CSEA      Non-Windows Cyan-Green
    0x00FF8000,     //CCYAN     Non-Windows Cyan-Blue
    0x00FF0080,     //CINDIGO   Non-Windows Magenta-Blue
    0x008000FF,     //CROSE     Non-Windows Magenta-Red
    0x00C080FF,     //CPINK     Non-Windows Bright Magenta

    0x00F0FBFF,     //CCREAM    Windows Cream       (Not VGA)
    0x00A4A0A0,     //CSTEEL    Windows Medium Grey (Not VGA)

    0x00C0C0C0,     //CSILVER   Windows Light Grey
    0x000000FF,     //CRED      Windows Red
    0x0000FF00,     //CLIME     Windows Green
    0x0000FFFF,     //CYELLOW   Windows Yellow
    0x00FF0000,     //CBLUE     Windows Blue
    0x00FF00FF,     //CFUCHSIA  Windows Magenta
    0x00FFFF00,     //CAQUA     Windows Cyan
    0x00FFFFFF,     //CWHITE    Windows White

    0x04030201,     //CCLEAR    Transparent (CINV)
};

//*************************************************************************
//  DrawDivTab:
//      This is a table of 2^32 divided by 0 through DIVMAX (511).
//      It is used to replace integer divisions with faster multiplication.
//      See the IFRACTION: and IDIVIDE: macros in CDraw.h.
//*************************************************************************

// This table must have DIVMAX (as defined in CDraw.h) entries.

const lint DrawDivTab[] = {
    0xFFFFFFFF,             //CANNOT USE FIRST ENTRY (DIVIDE BY 0)!
    0xFFFFFFFF,             //(2^32) /   1 (4294967295)
    0x7FFFFFFF,             //(2^32) /   2 (2147483648-1 to keep pos)
    1431655765,             //(2^32) /   3
    1073741824,             //(2^32) /   4
    858993459 ,             //(2^32) /   5
    715827883 ,             //(2^32) /   6
    613566757 ,             //(2^32) /   7
    536870912 ,             //(2^32) /   8
    477218588 ,             //(2^32) /   9
    429496730 ,             //(2^32) /  10
    390451572 ,             //(2^32) /  11
    357913941 ,             //(2^32) /  12
    330382100 ,             //(2^32) /  13
    306783378 ,             //(2^32) /  14
    286331153 ,             //(2^32) /  15
    268435456 ,             //(2^32) /  16
    252645135 ,             //(2^32) /  17
    238609294 ,             //(2^32) /  18
    226050910 ,             //(2^32) /  19
    214748365 ,             //(2^32) /  20
    204522252 ,             //(2^32) /  21
    195225786 ,             //(2^32) /  22
    186737709 ,             //(2^32) /  23
    178956971 ,             //(2^32) /  24
    171798692 ,             //(2^32) /  25
    165191050 ,             //(2^32) /  26
    159072863 ,             //(2^32) /  27
    153391689 ,             //(2^32) /  28
    148102321 ,             //(2^32) /  29
    143165577 ,             //(2^32) /  30
    138547332 ,             //(2^32) /  31
    134217728 ,             //(2^32) /  32
    130150524 ,             //(2^32) /  33
    126322568 ,             //(2^32) /  34
    122713351 ,             //(2^32) /  35
    119304647 ,             //(2^32) /  36
    116080197 ,             //(2^32) /  37
    113025455 ,             //(2^32) /  38
    110127367 ,             //(2^32) /  39
    107374182 ,             //(2^32) /  40
    104755300 ,             //(2^32) /  41
    102261126 ,             //(2^32) /  42
    99882960  ,             //(2^32) /  43
    97612893  ,             //(2^32) /  44
    95443718  ,             //(2^32) /  45
    93368854  ,             //(2^32) /  46
    91382283  ,             //(2^32) /  47
    89478485  ,             //(2^32) /  48
    87652394  ,             //(2^32) /  49
    85899346  ,             //(2^32) /  50
    84215045  ,             //(2^32) /  51
    82595525  ,             //(2^32) /  52
    81037119  ,             //(2^32) /  53
    79536431  ,             //(2^32) /  54
    78090314  ,             //(2^32) /  55
    76695845  ,             //(2^32) /  56
    75350303  ,             //(2^32) /  57
    74051160  ,             //(2^32) /  58
    72796056  ,             //(2^32) /  59
    71582788  ,             //(2^32) /  60
    70409300  ,             //(2^32) /  61
    69273666  ,             //(2^32) /  62
    68174084  ,             //(2^32) /  63
    67108864  ,             //(2^32) /  64
    66076420  ,             //(2^32) /  65
    65075262  ,             //(2^32) /  66
    64103989  ,             //(2^32) /  67
    63161284  ,             //(2^32) /  68
    62245903  ,             //(2^32) /  69
    61356676  ,             //(2^32) /  70
    60492497  ,             //(2^32) /  71
    59652324  ,             //(2^32) /  72
    58835168  ,             //(2^32) /  73
    58040099  ,             //(2^32) /  74
    57266231  ,             //(2^32) /  75
    56512728  ,             //(2^32) /  76
    55778796  ,             //(2^32) /  77
    55063683  ,             //(2^32) /  78
    54366675  ,             //(2^32) /  79
    53687091  ,             //(2^32) /  80
    53024288  ,             //(2^32) /  81
    52377650  ,             //(2^32) /  82
    51746594  ,             //(2^32) /  83
    51130563  ,             //(2^32) /  84
    50529027  ,             //(2^32) /  85
    49941480  ,             //(2^32) /  86
    49367440  ,             //(2^32) /  87
    48806447  ,             //(2^32) /  88
    48258060  ,             //(2^32) /  89
    47721859  ,             //(2^32) /  90
    47197443  ,             //(2^32) /  91
    46684427  ,             //(2^32) /  92
    46182444  ,             //(2^32) /  93
    45691141  ,             //(2^32) /  94
    45210182  ,             //(2^32) /  95
    44739243  ,             //(2^32) /  96
    44278013  ,             //(2^32) /  97
    43826197  ,             //(2^32) /  98
    43383508  ,             //(2^32) /  99
    42949673  ,             //(2^32) / 100
    42524429  ,             //(2^32) / 101
    42107523  ,             //(2^32) / 102
    41698712  ,             //(2^32) / 103
    41297762  ,             //(2^32) / 104
    40904450  ,             //(2^32) / 105
    40518559  ,             //(2^32) / 106
    40139881  ,             //(2^32) / 107
    39768216  ,             //(2^32) / 108
    39403370  ,             //(2^32) / 109
    39045157  ,             //(2^32) / 110
    38693399  ,             //(2^32) / 111
    38347922  ,             //(2^32) / 112
    38008560  ,             //(2^32) / 113
    37675152  ,             //(2^32) / 114
    37347542  ,             //(2^32) / 115
    37025580  ,             //(2^32) / 116
    36709122  ,             //(2^32) / 117
    36398028  ,             //(2^32) / 118
    36092162  ,             //(2^32) / 119
    35791394  ,             //(2^32) / 120
    35495597  ,             //(2^32) / 121
    35204650  ,             //(2^32) / 122
    34918433  ,             //(2^32) / 123
    34636833  ,             //(2^32) / 124
    34359738  ,             //(2^32) / 125
    34087042  ,             //(2^32) / 126
    33818640  ,             //(2^32) / 127
    33554432  ,             //(2^32) / 128
    33294320  ,             //(2^32) / 129
    33038210  ,             //(2^32) / 130
    32786010  ,             //(2^32) / 131
    32537631  ,             //(2^32) / 132
    32292987  ,             //(2^32) / 133
    32051995  ,             //(2^32) / 134
    31814573  ,             //(2^32) / 135
    31580642  ,             //(2^32) / 136
    31350126  ,             //(2^32) / 137
    31122951  ,             //(2^32) / 138
    30899045  ,             //(2^32) / 139
    30678338  ,             //(2^32) / 140
    30460761  ,             //(2^32) / 141
    30246249  ,             //(2^32) / 142
    30034736  ,             //(2^32) / 143
    29826162  ,             //(2^32) / 144
    29620464  ,             //(2^32) / 145
    29417584  ,             //(2^32) / 146
    29217465  ,             //(2^32) / 147
    29020049  ,             //(2^32) / 148
    28825284  ,             //(2^32) / 149
    28633115  ,             //(2^32) / 150
    28443492  ,             //(2^32) / 151
    28256364  ,             //(2^32) / 152
    28071682  ,             //(2^32) / 153
    27889398  ,             //(2^32) / 154
    27709466  ,             //(2^32) / 155
    27531842  ,             //(2^32) / 156
    27356480  ,             //(2^32) / 157
    27183337  ,             //(2^32) / 158
    27012373  ,             //(2^32) / 159
    26843546  ,             //(2^32) / 160
    26676816  ,             //(2^32) / 161
    26512144  ,             //(2^32) / 162
    26349493  ,             //(2^32) / 163
    26188825  ,             //(2^32) / 164
    26030105  ,             //(2^32) / 165
    25873297  ,             //(2^32) / 166
    25718367  ,             //(2^32) / 167
    25565282  ,             //(2^32) / 168
    25414008  ,             //(2^32) / 169
    25264514  ,             //(2^32) / 170
    25116768  ,             //(2^32) / 171
    24970740  ,             //(2^32) / 172
    24826401  ,             //(2^32) / 173
    24683720  ,             //(2^32) / 174
    24542670  ,             //(2^32) / 175
    24403223  ,             //(2^32) / 176
    24265352  ,             //(2^32) / 177
    24129030  ,             //(2^32) / 178
    23994231  ,             //(2^32) / 179
    23860929  ,             //(2^32) / 180
    23729101  ,             //(2^32) / 181
    23598721  ,             //(2^32) / 182
    23469767  ,             //(2^32) / 183
    23342214  ,             //(2^32) / 184
    23216039  ,             //(2^32) / 185
    23091222  ,             //(2^32) / 186
    22967740  ,             //(2^32) / 187
    22845571  ,             //(2^32) / 188
    22724695  ,             //(2^32) / 189
    22605091  ,             //(2^32) / 190
    22486740  ,             //(2^32) / 191
    22369621  ,             //(2^32) / 192
    22253717  ,             //(2^32) / 193
    22139007  ,             //(2^32) / 194
    22025473  ,             //(2^32) / 195
    21913098  ,             //(2^32) / 196
    21801864  ,             //(2^32) / 197
    21691754  ,             //(2^32) / 198
    21582750  ,             //(2^32) / 199
    21474836  ,             //(2^32) / 200
    21367996  ,             //(2^32) / 201
    21262214  ,             //(2^32) / 202
    21157474  ,             //(2^32) / 203
    21053761  ,             //(2^32) / 204
    20951060  ,             //(2^32) / 205
    20849356  ,             //(2^32) / 206
    20748634  ,             //(2^32) / 207
    20648881  ,             //(2^32) / 208
    20550083  ,             //(2^32) / 209
    20452225  ,             //(2^32) / 210
    20355295  ,             //(2^32) / 211
    20259280  ,             //(2^32) / 212
    20164166  ,             //(2^32) / 213
    20069941  ,             //(2^32) / 214
    19976592  ,             //(2^32) / 215
    19884108  ,             //(2^32) / 216
    19792476  ,             //(2^32) / 217
    19701685  ,             //(2^32) / 218
    19611723  ,             //(2^32) / 219
    19522579  ,             //(2^32) / 220
    19434241  ,             //(2^32) / 221
    19346700  ,             //(2^32) / 222
    19259943  ,             //(2^32) / 223
    19173961  ,             //(2^32) / 224
    19088744  ,             //(2^32) / 225
    19004280  ,             //(2^32) / 226
    18920561  ,             //(2^32) / 227
    18837576  ,             //(2^32) / 228
    18755316  ,             //(2^32) / 229
    18673771  ,             //(2^32) / 230
    18592932  ,             //(2^32) / 231
    18512790  ,             //(2^32) / 232
    18433336  ,             //(2^32) / 233
    18354561  ,             //(2^32) / 234
    18276457  ,             //(2^32) / 235
    18199014  ,             //(2^32) / 236
    18122225  ,             //(2^32) / 237
    18046081  ,             //(2^32) / 238
    17970574  ,             //(2^32) / 239
    17895697  ,             //(2^32) / 240
    17821441  ,             //(2^32) / 241
    17747799  ,             //(2^32) / 242
    17674763  ,             //(2^32) / 243
    17602325  ,             //(2^32) / 244
    17530479  ,             //(2^32) / 245
    17459217  ,             //(2^32) / 246
    17388532  ,             //(2^32) / 247
    17318417  ,             //(2^32) / 248
    17248865  ,             //(2^32) / 249
    17179869  ,             //(2^32) / 250
    17111423  ,             //(2^32) / 251
    17043521  ,             //(2^32) / 252
    16976155  ,             //(2^32) / 253
    16909320  ,             //(2^32) / 254
    16843009  ,             //(2^32) / 255
    16777216  ,             //(2^32) / 256
    16711935  ,
    16647160  ,
    16582885  ,
    16519105  ,
    16455813  ,
    16393005  ,
    16330674  ,
    16268816  ,
    16207424  ,
    16146494  ,
    16086020  ,
    16025997  ,
    15966421  ,
    15907286  ,
    15848588  ,
    15790321  ,
    15732481  ,
    15675063  ,
    15618063  ,
    15561476  ,
    15505297  ,
    15449523  ,
    15394148  ,
    15339169  ,
    15284581  ,
    15230380  ,
    15176563  ,
    15123124  ,
    15070061  ,
    15017368  ,
    14965043  ,
    14913081  ,
    14861479  ,
    14810232  ,
    14759338  ,
    14708792  ,
    14658591  ,
    14608732  ,
    14559211  ,
    14510025  ,
    14461169  ,
    14412642  ,
    14364439  ,
    14316558  ,
    14268994  ,
    14221746  ,
    14174810  ,
    14128182  ,
    14081860  ,
    14035841  ,
    13990121  ,
    13944699  ,
    13899571  ,
    13854733  ,
    13810184  ,
    13765921  ,
    13721940  ,
    13678240  ,
    13634817  ,
    13591669  ,
    13548793  ,
    13506186  ,
    13463847  ,
    13421773  ,
    13379960  ,
    13338408  ,
    13297112  ,
    13256072  ,
    13215284  ,
    13174746  ,
    13134457  ,
    13094412  ,
    13054612  ,
    13015052  ,
    12975732  ,
    12936648  ,
    12897800  ,
    12859184  ,
    12820798  ,
    12782641  ,
    12744710  ,
    12707004  ,
    12669520  ,
    12632257  ,
    12595212  ,
    12558384  ,
    12521771  ,
    12485370  ,
    12449181  ,
    12413200  ,
    12377427  ,
    12341860  ,
    12306497  ,
    12271335  ,
    12236374  ,
    12201612  ,
    12167046  ,
    12132676  ,
    12098499  ,
    12064515  ,
    12030721  ,
    11997115  ,
    11963697  ,
    11930465  ,
    11897416  ,
    11864551  ,
    11831866  ,
    11799361  ,
    11767034  ,
    11734883  ,
    11702908  ,
    11671107  ,
    11639478  ,
    11608020  ,
    11576731  ,
    11545611  ,
    11514658  ,
    11483870  ,
    11453246  ,
    11422785  ,
    11392486  ,
    11362347  ,
    11332368  ,
    11302546  ,
    11272880  ,
    11243370  ,
    11214014  ,
    11184811  ,
    11155759  ,
    11126858  ,
    11098107  ,
    11069503  ,
    11041047  ,
    11012737  ,
    10984571  ,
    10956549  ,
    10928670  ,
    10900932  ,
    10873335  ,
    10845877  ,
    10818557  ,
    10791375  ,
    10764329  ,
    10737418  ,
    10710642  ,
    10683998  ,
    10657487  ,
    10631107  ,
    10604858  ,
    10578737  ,
    10552745  ,
    10526881  ,
    10501143  ,
    10475530  ,
    10450042  ,
    10424678  ,
    10399437  ,
    10374317  ,
    10349319  ,
    10324441  ,
    10299682  ,
    10275041  ,
    10250519  ,
    10226113  ,
    10201823  ,
    10177648  ,
    10153587  ,
    10129640  ,
    10105805  ,
    10082083  ,
    10058471  ,
    10034970  ,
    10011579  ,
    9988296   ,
    9965121   ,
    9942054   ,
    9919093   ,
    9896238   ,
    9873488   ,
    9850842   ,
    9828300   ,
    9805861   ,
    9783525   ,
    9761289   ,
    9739155   ,
    9717121   ,
    9695186   ,
    9673350   ,
    9651612   ,
    9629972   ,
    9608428   ,
    9586981   ,
    9565629   ,
    9544372   ,
    9523209   ,
    9502140   ,
    9481164   ,
    9460280   ,
    9439489   ,
    9418788   ,
    9398178   ,
    9377658   ,
    9357227   ,
    9336885   ,
    9316632   ,
    9296466   ,
    9276387   ,
    9256395   ,
    9236489   ,
    9216668   ,
    9196932   ,
    9177281   ,
    9157713   ,
    9138228   ,
    9118827   ,
    9099507   ,
    9080269   ,
    9061112   ,
    9042036   ,
    9023041   ,
    9004124   ,
    8985287   ,
    8966529   ,
    8947849   ,
    8929246   ,
    8910721   ,
    8892272   ,
    8873899   ,
    8855603   ,
    8837381   ,
    8819235   ,
    8801162   ,
    8783164   ,
    8765239   ,
    8747388   ,
    8729608   ,
    8711901   ,
    8694266   ,
    8676702   ,
    8659208   ,
    8641785   ,
    8624432   ,
    8607149   ,
    8589935   ,
    8572789   ,
    8555712   ,
    8538702   ,
    8521761   ,
    8504886   ,
    8488078   ,
    8471336   ,
    8454660   ,
    8438050   ,
    8421505   ,
    8405024   ,             //(2^32) / 511
};

#ifndef NOI86
#pragma warning (disable:4731) //assembly modifies ebp after preserve with push
#endif

//*************************************************************************
//  BitsNew:
//  BitsMake:
//      Creates a new uninitialized bitmap.
//      Allocates uninitialized bmPtr bitmap memory.
//      Also allocates uninitialized bmPal palette memory if needed.
//      Can be given negative pix for RGBA instead of BGRA.
//
//      The BitsNew version frees any exising bitmap first.
//      The BitsMake version can be given a BMAP with un-zeroed initial memory.
//
//  Arguments:
//      errc BitsNew (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal)
//      errc BitsMake (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal)
//      pmap    Filled in with new bitmap's information.
//      pix     Desired pixel bit size (1, 4, 8, 32, 64, 128 or -32 for RGBA).
//      x       Desired pixel width.
//      y       Desired pixel height.
//              If positive then a bottom-to-top bitmap (bmRow<0) is created.
//              If negative then a top-to-bottom bitmap (bmRow>0) is created.
//              Windows BITMAPINFOHEADER biHeight is negative for top-to-bottom.
//              Some Windows APIs and applications do not support top-to-bottom.
//      ppal    Memory to be used for palette.
//              Supplied pointer is not owned by pmap and is given KEEPPAL flag.
//              If NULL, then allocates memory as needed.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsNew (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal) {
    lint size;
    boolv bottom1;
    errc e;

    bottom1 = TRUE;
    if (y < 0) {
        y = -y;
        bottom1 = FALSE;
    }

    BitsFree(pmap);
    OSMemClear(pmap, sizeof(BMAP));
    pmap->bmX = x;
    pmap->bmY = y;
    if (pix < 0) {                              //RGBA instead of BGRA?
        pix = -pix;
        pmap->bmFlg = RGBAPIX;
    }
    pmap->bmPix = pix;
    e = BitsBytes(pix, pmap->bmX, pmap->bmY, &pmap->bmRow, &size);
    if (e) return(e);

    #ifdef XDEBUG                               //===
    size += 4096*2;
    #endif                                      //===

    pmap->bmPtr = (byte*) OSAlloc(size);
    if (pmap->bmPtr == NULL) return(ECMEMORY);

    #ifdef XDEBUG                               //===
    OSMemFillB(pmap->bmPtr, 0x55, size);
    pmap->bmPtr = ADDOFF(fbyte*, pmap->bmPtr, 4096);
    #endif                                      //===

    pmap->bmTop = pmap->bmPtr;
    if (bottom1) {
        pmap->bmTop = pmap->bmPtr + ((y-1) * pmap->bmRow);
        pmap->bmRow = -(pmap->bmRow);
    }
    if (pix <= 8) {
        if (ppal) {
            pmap->bmPal = ppal;
            pmap->bmFlg = KEEPPAL;
        } else {
            pmap->bmPal = (BGR*) OSAlloc(4 << pix);
            if (pmap->bmPtr == NULL) return(ECMEMORY);
        }
    }
    return(0);
}

// *******
// Slightly faster version which does not need or use OSMemClear.

csproc BitsMake (BMAP* pmap, lint pix, lint x, lint y, BGR* ppal) {
    lint size;
    boolv bottom1;
    errc e;

    bottom1 = TRUE;
    if (y < 0) {
        y = -y;
        bottom1 = FALSE;
    }

    pmap->bmPtr = NULL;                         //make sure everything inited
    pmap->bmTop = NULL;
    pmap->bmPal = NULL;
    pmap->bmRow = 0;
    pmap->bmFlg = 0;
    pmap->bmNow = 0;
    pmap->bmInv = 0;
    pmap->bmUse = 0;
    pmap->bmPtX = 0;
    pmap->bmPtY = 0;

    pmap->bmX = x;
    pmap->bmY = y;
    if (pix < 0) {                              //RGBA instead of BGRA?
        pix = -pix;
        pmap->bmFlg = RGBAPIX;
    }
    pmap->bmPix = pix;
    e = BitsBytes(pix, pmap->bmX, pmap->bmY, &pmap->bmRow, &size);
    if (e) return(e);

    #ifdef XDEBUG                               //===
    size += 4096*2;
    #endif                                      //===

    pmap->bmPtr = (byte*) OSAlloc(size);
    if (pmap->bmPtr == NULL) return(ECMEMORY);

    #ifdef XDEBUG                               //===
    OSMemFillB(pmap->bmPtr, 0x55, size);
    pmap->bmPtr = ADDOFF(fbyte*, pmap->bmPtr, 4096);
    #endif                                      //===

    pmap->bmTop = pmap->bmPtr;
    if (bottom1) {
        pmap->bmTop = pmap->bmPtr + ((y-1) * pmap->bmRow);
        pmap->bmRow = -(pmap->bmRow);
    }
    if (pix <= 8) {
        if (ppal) {
            pmap->bmPal = ppal;
            pmap->bmFlg = KEEPPAL;
        } else {
            pmap->bmPal = (BGR*) OSAlloc(4 << pix);
            if (pmap->bmPtr == NULL) return(ECMEMORY);
        }
    }
    return(0);
}

//*************************************************************************
//  BitsFree:
//      Frees pixel and palette memory associated with a bitmap.
//
//  Arguments:
//      errc BitsFree (BMAP* pmap)
//      pmap    Pointer to bitmap to free.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsFree (BMAP* pmap) {
    #ifdef XDEBUG                               //===
    lint* pmem;
    lint nmem;
    if (pmap->bmPtr) {
        pmem = SUBOFF(lint*, pmap->bmPtr, 4096);
        nmem = 4096/4;
        do {
            if (*pmem++ != 0x55555555) {
                OSBeep(2);
                break;
            }
        } while (--nmem);
        nmem = pmap->bmRow;
        if (nmem < 0) nmem = -nmem;
        nmem = nmem * pmap->bmY;
        pmem = ADDOFF(lint*, pmap->bmPtr, nmem);
        nmem = 4096/4;
        do {
            if (*pmem++ != 0x55555555) {
                OSBeep(2);
                break;
            }
        } while (--nmem);
        pmap->bmPtr = SUBOFF(fbyte*, pmap->bmPtr, 4096);
    }
    #endif                                      //===

    if ((pmap->bmPtr)&&(!(pmap->bmFlg & KEEPPTR)))
        OSFree(pmap->bmPtr);
    pmap->bmPtr = NULL;
    if ((pmap->bmPal)&&(!(pmap->bmFlg & KEEPPAL)))
        OSFree(pmap->bmPal);
    pmap->bmPal = NULL;
    return(0);
}

//*************************************************************************
//  BitsColor:
//      Prepares a color for copying as a bitmap pixel.
//      For non-palette bitmaps, just returns the BGRA/RGBA color value.
//      Finds a palette index for a bitmap color if bitmap uses palette.
//      For palette bitmaps, adds new palette entry to end if not found
//      Otherwise returns best matching palette entry.
//      Sets the MATCHED flag for bitmap if not enough room for exact color.
//
//      Never returns an error.  If the palette is full the finds closest
//      matching palette color.  For invisible color when palette is
//      full, finds closest matching visible color.
//
//      For 32-bit bitmaps normally fills alphas with 255 values.
//      However for CCLEAR or CINV+CRGB or CINV+CBGR colors uses 0 alphas.
//      Also uses 0 alphas if target bitmap has ALPHAS0 set in pmap->bmFlg.
//
//  Arguments:
//      lint BitsColor (BMAP* pmap, lint color)
//      pmap    Bitmap in which the color is to be used.
//      color   May either be a CRGB or CBGR color or a Standard Color number.
//              May also be CINV+? for the see-through color with alpha=0.
//              May also be CERR+i to specify a palette indexed color for pmap.
//              For 32 bits/pixel pmap, normally returns an alpha of 255.
//              Use CINV+CRGB or CINV+CBGR to return color with alpha of 0.
//              Use CCLEAR to return black with an alpha of 0.
//              Use CBLACK to return black with an alpha of 255.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//
//  Returns:
//      Either a palette index number (0=1st) for palettized bitmaps,
//      or a BGR value with alpha=255 for 32 bits/pixel bitmaps,
//      or a BGR value with alpha=0 for 32 bits/pixel when want invisible color.
//      or a RGB value if bmFlg has BGRAPIX.
//*************************************************************************

lint BitsColor (BMAP* pmap, lint color) {
    lint* ppal;
    BGR* pbgr;
    lint npal, nmax, val, delta;
    LHUE hue;

    ppal = (lint*)pmap->bmPal;

    if (color & CINV) {                         //looking for invisible color?
        color &= ~CINV;                         //clear CINV bit
        if (ppal) {                             //using palette?
            if (pmap->bmInv) {                  //already have invisible color?
                return(pmap->bmInv & 0xFF);     //return invisible index
            }
            npal = pmap->bmUse;
            nmax = 1 << pmap->bmPix;
            if (npal < nmax) {                  //room for another palette entry
                ppal += npal;
                *ppal = CINV+0;                 //add invisible black
                pmap->bmInv = CINV+npal;
                pmap->bmUse += 1;
                pmap->bmFlg |= NEWINFO;
                return(npal);
            }                                   //punt by finding visible color
        } else {                                //want invisible for full color?
            pmap->bmInv = CINV+0;               //flag that we have invisibles
            if (pmap->bmFlg & RGBAPIX) {
                hue.d = DrawRGB(color);         //get RGB color
                hue.r.ralpha = 0;               //set to invisible
            } else {
                DrawBGR(&hue.b, color);         //get BGR color
                hue.b.balpha = 0;               //set to invisible
            }
            return(hue.d);
        }
    }
    if (color & CERR) {                         //specify palette index?
        if (ppal == NULL) return(ECNULL);
        return(color & 0xFF);
    }
    if ((color == CCLEAR)&&(pmap->bmInv)) {     //CCLEAR flags invisible color
        return(pmap->bmInv & RGB_MASK);
    }

    if (pmap->bmFlg & RGBAPIX) {
        hue.d = DrawRGB(color);                 //get RGB color
        hue.r.ralpha = 255;
    } else {
        DrawBGR(&hue.b, color);                 //get BGR format color
    }
    if (ppal == NULL) {                         //no palette => return color
        if (pmap->bmFlg & ALPHAS0) hue.b.balpha = 0;
        if ((pmap->bmInv)&&((hue.d & RGB_MASK) == (pmap->bmInv & RGB_MASK))) {
            hue.b.balpha = 0;
        }
        return(hue.d);
    }

    if (pmap->bmFlg & ISALPHA) BitsAlpha(pmap, 0);
    hue.b.balpha = 0;                           //palette alphas are zero
    npal = pmap->bmUse;
    ppal -= 1;
    while (npal--) {                            //find matching palette color
        if (*(++ppal) != hue.d) continue;
        val = (lint)(ppal - (lint*)pmap->bmPal);//don't match invisible color
        if ((pmap->bmInv)&&((pmap->bmInv & 0xFF) == val)) continue;
        return (val);                           //return matching palette index
    }

    ppal += 1;
    npal = pmap->bmUse;                         //no matching palette entry
    nmax = 1 << pmap->bmPix;
    if (npal < nmax) {                          //room for new palette entry
        *ppal = hue.d;                          //add new palette entry to end
        pmap->bmUse += 1;
        pmap->bmFlg |= NEWINFO;
        return(npal);
    }

    pmap->bmFlg |= MATCHED;
    pbgr = pmap->bmPal;                         //otherwise use this not very
    delta = EMAXI;                              //good matching algorithm
    do {                                        //look for closest match
        val = ABS(pbgr->bblue - hue.b.bblue) +
              ABS(pbgr->bgreen - hue.b.bgreen) +
              ABS(pbgr->bred - hue.b.bred);
        if (val < delta) {                      //closest match to date?
            delta = val;
            ppal = (lint*)pbgr;
        }
        pbgr += 1;
    } while (--nmax);
    return ((lint)(ppal - (lint*)pmap->bmPal));
}

//*************************************************************************
//  BitsBlank:
//      Clears a bitmap region to all one color.
//
//      For 32-bit bitmaps normally fills alphas with 255 values.
//      However for CCLEAR or CINV+CRGB or CINV+CBGR colors uses 0 alphas.
//      Also uses 0 alphas if target bitmap has ALPHAS0 set in pmap->bmFlg.
//      See BitsAlphaFill to blank with color and a given alpha value.
//      Sets pmap->bmInv when used with CCLEAR for transparency.
//
//  Arguments:
//      errc BitsBlank (BMAP* pmap, IBox* pbox, lint color)
//      pmap    Bitmap to be used.
//      pbox    Region in bitmap to be cleared.
//              If NULL uses entire bitmap.
//              Otherwise clips supplied rectangle as needed.
//      color   Color to fill the bitmap retion.
//              May either be an RGB or BGR color or a Standard Color number.
//              Use CCLEAR to zero the bitmap including 0 alpha values.
//              Other Standard Color numbers set alpha values to 255.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsBlank (BMAP* pmap, IBox* pbox, lint color) {
    byte* pd;
    byte* prow;
    lint ox, oy, od, nx, ny, cnt, row, pix;
    lint blank, whole, cbyte, cpix, bit, nbytes;

    if (pbox == NULL) {                         //get the region to blank
        ox = 0;
        oy = 0;
        nx = pmap->bmX;
        ny = pmap->bmY;
    } else {
        ox = pbox->xleft;
        nx = pbox->xright - ox;
        if (ox < 0) {
            nx += ox;
            ox = 0;
        }
        if ((ox+nx) > pmap->bmX) {
            nx = pmap->bmX - ox;
        }
        if (nx <= 0) return(0);
        oy = pbox->ytop;
        ny = pbox->ybottom - oy;
        if (oy < 0) {
            ny += oy;
            oy = 0;
        }
        if ((oy+ny) > pmap->bmY) {
            ny = pmap->bmY - oy;
        }
        if (ny <= 0) return(0);
    }

    if (  (nx <= 0)                             //check for invalid arguments
        ||(ny <= 0)                             //(not a complete verification)
        ||((ox + nx) > pmap->bmX)
        ||((oy + ny) > pmap->bmY)  ) {
        return(ECARGUMENT);
    }

    row = pmap->bmRow;
    prow = pmap->bmTop + (oy * row);            //get first rows to use
    pix = pmap->bmPix;
    blank = BitsColor(pmap, color);             //get blanking color to use

    if ((color == CCLEAR)&&(pmap->bmInv == 0)) {
        pmap->bmInv = CINV + blank;             //assume CCLEAR invisible color
    }

// Handle 32 bits/pixel bitmap.

    if (pix == 32) {
        if (nx == pmap->bmX) {
            nbytes = ny * row;
            if (row < 0) {
                prow = ADDOFF(byte*, prow, nbytes-row);
                nbytes = -nbytes;
            }
            OSFILLD(prow, blank, nbytes);
        } else {
            prow += ox << 2;
            nbytes = nx << 2;
            do {
                OSFILLD(prow, blank, nbytes);
                prow = ADDOFF(byte*, prow, row);
            } while (--ny);
        }
        return(0);

// Handle 8 bits/pixel bitmap.

    } else if (pix == 8) {
        blank &= 0xFF;
        if (nx == pmap->bmX){
            nbytes = ny * row;
            if (row < 0) {
                prow = ADDOFF(byte*, prow, nbytes-row);
                nbytes = -nbytes;
            }
            OSFILLB(prow, blank, nbytes);
        } else {
            prow += ox;
            do {
                OSFILLB(prow, blank, nx);
                prow = ADDOFF(byte*, prow, row);
            } while (--ny);
        }
        return(0);

// Handle 4 bits/pixel bitmap.

    } else if (pix == 4) {
        blank &= 0x0F;
        whole = blank | (blank << 4);
        if (nx == pmap->bmX){
            nbytes = ny * row;
            if (row < 0) {
                prow = ADDOFF(byte*, prow, nbytes-row);
                nbytes = -nbytes;
            }
            OSFILLB(prow, whole, nbytes);
        } else {
            do {
                cnt = nx;
                od = ox;
                pd = prow + (od >> 1);
                do {
                    if (!(od & 1)) {            //first nibble in byte?
                        cbyte = cnt >> 1;
                        if (cbyte) {            //can fill multiple bytes?
                            OSFILLB(pd, whole, cbyte);
                            pd += cbyte;
                            cpix = cbyte + cbyte;
                            od += cpix;
                            cnt -= cpix;
                            if (cnt) continue;
                            break;
                        } else {
                            *pd = ((*pd) & 0xF) | ((byte)blank << 4);
                        }
                    } else {                    //second nibble in dest byte?
                        *pd = ((*pd) & 0xF0) | ((byte)blank);
                        pd += 1;
                    }
                    od += 1;
                } while (--cnt);
                prow += row;
            } while (--ny);
        }
        return(0);

// Handle 1 bit/pixel monochrome bitmap.

    } else if (pix == 1) {
        blank &= 0x01;
        whole = (blank) ? 0xFFFFFFFF : 0x00000000;
        if (nx == pmap->bmX){
            nbytes = ny * row;
            if (row < 0) {
                prow = ADDOFF(byte*, prow, nbytes-row);
                nbytes = -nbytes;
            }
            OSFILLB(prow, whole, nbytes);
        } else {
            do {
                cnt = nx;
                od = ox;
                pd = prow + (od >> 3);
                do {
                    bit = od & 7;
                    if (bit == 0) {             //can fill multiple bytes?
                        cbyte = cnt >> 3;
                        if (cbyte) {
                            OSFILLB(pd, whole, cbyte);
                            pd += cbyte;
                            cpix = cbyte << 3;
                            od += cpix;
                            cnt -= cpix;
                            if (cnt) continue;
                            break;
                        }
                    }
                    bit = 7 - (od & 7);
                    *pd = ((*pd) & (~(1 << bit))) | ((byte)blank << bit);
                    if (bit == 0) pd += 1;
                    od += 1;
                } while (--cnt);
                prow += row;
            } while (--ny);
        }
        return(0);

// Handle greater than 32 bits per pixel.

    } else if (pix > 32) {
        if (nx == pmap->bmX){
            nbytes = ny * row;
            if (row < 0) {
                prow = ADDOFF(byte*, prow, nbytes-row);
                nbytes = -nbytes;
            }
            OSFILLD(prow, blank, nbytes);
        } else {
            pix = pix >> 3;
            prow += ox * pix;
            nbytes = nx * pix;
            do {
                OSFILLD(prow, blank, nbytes);
                prow = ADDOFF(byte*, prow, row);
            } while (--ny);
        }
        return(0);

// Return error for other bitmap pixel sizes.

    } else {
        return(ECCANNOT);
    }
}

//*************************************************************************
//  BitsBlankInv:
//      Similar to BitsBlank but can use a clear version of the specified color.
//      Used to blank a transparent background.
//
//  Arguments:
//      errc BitsBlankInv (int mode, BMAP* pmap, IBox* pbox, lint color)
//      mode    0 = Use normal BitsBlank non-transparent color.
//            512 = Blank to transparent version of color (CLEARBG).
//      pmap    Bitmap to be used.
//      pbox    Region in bitmap to be cleared.
//              If NULL uses entire bitmap.
//              Otherwise clips supplied rectangle as needed.
//      color   Color to fill the bitmap retion.
//              May either be an RGB or BGR color or a Standard Color number.
//              Use CCLEAR to zero the bitmap including 0 alpha values.
//              Other Standard Color numbers set alpha values to 255.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsBlankInv (int mode, BMAP* pmap, IBox* pbox, lint color) {
    BMAP bmap;
    LHUE hue;
    errc e;

    if (!(mode & CLEARBG)) {
        return( BitsBlank(pmap, pbox, color) ); //clearness not required?
    }
    if (pmap->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pmap, NULL);
        if (e) return(e);
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }
    DrawBGR(&hue.b, color);
    hue.b.balpha = 0;                           //transparent version of coor
    e = BitsAlphaFill(3, pmap, NULL, 0, hue.d); //fill with that value
    pmap->bmInv = CINV + (hue.d & RGB_MASK);
    if ((pmap->bmInv == CINV)||(pmap->bmInv == B_INVIS)) {
        pmap->bmInv = B_CLEAR;                  //stop special BitsStretch stuff
    } else if (pmap->bmInv == W_INVIS) {
        pmap->bmInv = W_CLEAR;
    }
    return(e);
}

//*************************************************************************
//  BitsCopy:
//      Copies a block of bitmap pixels.
//      Can copy from any pixel size to itself.
//      Can convert from 1,4,8 to 32-bit format.
//      Can also convert from 1,4 to 8-bit format.
//      Can handle source transparency, but only fully opaque or transparent.
//      See BitsStretch for partial source transparency.
//      Can also copy 24->32 bits and 32->24 bits without transparency.
//      Does not handle palette translations but can copy whole palette.
//      Like Windows, copies to but not including the right and bottom limits.
//
//  Arguments:
//      errc BitsCopy (int mode, BMAP* pdes, IPt* pto, BMAP* psrc, IBox* pfrom)
//      mode    0 = Normal.
//              4 = Ignore source transparency.
//      pdes    Destination bitmap.
//              If bmPtr is NULL then creates bitmap using bmPix only
//              and all other entries can be NULL and are copied from source
//              (but if bmPix is 0, then uses source number of bits too).
//              If bmPtr is NULL then ignores pto and pfrom.
//      pto     Upper-left corner of destination rectangle.
//              Use NULL for upper-left corner of whole pdes bitmap (0,0).
//              If NULL, then also copies source bitmap's palette.
//              The destination rectagle size is implied by the pto argument.
//      psrc    Source bitmap.
//              If bmInv is non-zero, copies only non-transparent pixels.
//      pfrom   Target rectangle within source bitmap.
//              Use NULL for the entire source bitmap (and clips right/bottom).
//              This must be wholely within the source bitmap.
//              The destination rectangle always has the same width and height.
//              But the destination rectangle is clipped if needed.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECCANNOT if cannot do implied pixel conversion.
//      Returns ECARGUMENT if source rectangle is impossible.
//*************************************************************************

csproc BitsCopy (int mode, BMAP* pdes, IPt* pto, BMAP* psrc, IBox* pfrom) {
    BGR* ppal;
    BGR* pbgr;
    byte* prows;
    byte* prowd;
    byte* pbytes;
    byte* pbyted;
    lint* plongd;
    lint nx, ny, pix, bytes, bit, zero, one, inv;
    lint os, od, cnt, cbyte, cpix, srow, drow;
    lint fromx, fromy, xleft, ytop;
    byte val, valinv;
    errc e;
    #ifdef NOI86    //-------
    lint* plongs;
    #endif          //-------

    if (pfrom) {                                //get source upper-left corner
        fromx = pfrom->xleft;
        fromy = pfrom->ytop;
        nx = pfrom->xright - fromx;
        ny = pfrom->ybottom - fromy;
        if (  (nx <= 0)                         //check for invalid source area
            ||(ny <= 0)
            ||(fromx < 0)
            ||(fromy < 0)
            ||(pfrom->xright > psrc->bmX)
            ||(pfrom->ybottom > psrc->bmY)  ) {
            return(ECARGUMENT);
        }
    } else {
        fromx = 0;
        fromy = 0;
        nx = psrc->bmX;
        ny = psrc->bmY;
    }

    if (pdes->bmPtr) {
        if (pto) {                              //get destination rectangle
            xleft = pto->ptx;
            ytop = pto->pty;
        } else {                                //copy over entire destination?
            xleft = 0;
            ytop = 0;
        }
    } else {                                    //create destination bitmap?
        xleft = 0;
        ytop = 0;
        pix = pdes->bmPix;
        if (pix == 0) pix = psrc->bmPix;
        e = BitsNew(pdes, pix, nx, ny, NULL);
        if (e) return(e);                       //create destination bitmap
        if ((psrc->bmInv)&&(pix == 32)) pdes->bmInv = CINV;
        pdes->bmFlg |= NEWINFO;
    }

    if (xleft < 0) {                            //clip left if needed
        nx += xleft;
        if (nx <= 0) return(0);
        fromx -= xleft;
        xleft = 0;
    }
    if ((xleft + nx) > pdes->bmX) {             //clip right if needed
        nx = pdes->bmX - xleft;
        if (nx <= 0) return(0);
    }
    if (ytop < 0) {                             //clip top if needed
        ny += ytop;
        if (ny <= 0) return(0);
        fromy -= ytop;
        ytop = 0;
    }
    if ((ytop + ny) > pdes->bmY) {              //clip bottom if needed
        ny = pdes->bmY - ytop;
        if (ny <= 0) return(0);
    }

    BitsAlpha(psrc, ISALPHA);                   //set source alphas
    ppal = psrc->bmPal;

    if (  (pdes->bmPix <= 8)                    //copy source palette?
        &&(pto == NULL)
        &&(pdes->bmPix >= psrc->bmPix)  ) {
        bytes = (psrc->bmUse) ? (psrc->bmUse << 2) : (4 << psrc->bmPix);
        OSMemCopy(pdes->bmPal, psrc->bmPal, bytes);
        pdes->bmUse = psrc->bmUse;
        pdes->bmInv = psrc->bmInv;
        pdes->bmFlg |= NEWINFO + (ISALPHA & psrc->bmFlg);
    }

    srow = psrc->bmRow;
    drow = pdes->bmRow;
    prows = psrc->bmTop + (fromy * srow);       //get first rows to use
    prowd = pdes->bmTop + (ytop * drow);

// =======
// Handle transparency in the source.

    if ((psrc->bmInv)&&(!(mode & 4))) {
        inv = psrc->bmInv;
        pix = psrc->bmPix;
        if (pdes->bmPix == pix) {

// Copy 32-bit pixel block with transparency.

            if (pix == 32) {
                prows += fromx << 2;
                prowd += xleft << 2;
                do {
                    #ifdef NOI86    //-------
                    cnt = nx;
                    plongs = (lint*) prows;
                    plongd = (lint*) prowd;
                    do {
                        one = *plongs++;
                        if ((unsigned)one >= ALPHA_HALF) *plongd = one;
                        plongd += 1;
                    } while (--cnt);
                    #else           //-------
                    ASM {
                    mov     ecx,nx
                    mov     esi,prows
                    mov     edi,prowd
                    mov     edx,ALPHA_HALF
cpilpd:             mov     eax,[esi]
                    add     esi,4
                    cmp     eax,edx
                    jb      cpiskd              //alpha<0x80 => invisible?
                    mov     [edi],eax
cpiskd:             add     edi,4
                    dec     ecx
                    jnz     cpilpd
                    }
                    #endif          //-------
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Copy 8-bit pixel block with transparency.

            if (pix == 8) {
                valinv = (byte) inv;
                prows += fromx;
                prowd += xleft;
                do {
                    #ifdef NOI86    //-------
                    cnt = nx;
                    pbytes = prows;
                    pbyted = prowd;
                    do {
                        val = *pbytes++;
                        if (val != valinv) *pbyted = val;
                        pbyted += 1;
                    } while (--cnt);
                    #else           //-------
                    ASM {
                    mov     ecx,nx
                    mov     esi,prows
                    mov     edi,prowd
                    mov     dl,valinv
cpilp8:             mov     al,[esi]
                    add     esi,1
                    cmp     al,dl
                    je      cpisk8
                    mov     [edi],al
cpisk8:             add     edi,1
                    dec     ecx
                    jnz     cpilp8
                    }
                    #endif          //-------
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Copy 4-bit pixel block with transparency.

            if (pix == 4) {
                valinv = (byte) inv;
                do {
                    cnt = nx;
                    os = fromx;
                    od = xleft;
                    pbytes = prows + (os >> 1);
                    pbyted = prowd + (od >> 1);
                    do {
                        if (!(os & 1)) {        //first nibble in source byte?
                            val = (*pbytes) >> 4;
                        } else {                //second nibble in source byte?
                            val = (*pbytes) & 0xF;
                            pbytes += 1;
                        }
                        os += 1;

                        if (!(od & 1)) {        //first nibble in dest byte?
                            if (val != valinv)
                                *pbyted = ((*pbyted) & 0xF) | (val << 4);
                        } else {                //second nibble in dest byte?
                            if (val != valinv)
                                *pbyted = ((*pbyted) & 0xF0) | (val);
                            pbyted += 1;
                        }
                        od += 1;
                    } while (--cnt);
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Copy 1-bit monochrome pixel block with transparency.

            if (pix == 1) {
                valinv = (byte) inv;
                do {
                    cnt = nx;
                    os = fromx;
                    od = xleft;
                    pbytes = prows + (os >> 3);
                    pbyted = prowd + (od >> 3);
                    do {
                        bit = os & 7;
                        val = ((*pbytes) >> (7-bit)) & 1;
                        if (bit == 7) pbytes += 1;
                        os += 1;

                        bit = 7 - (od & 7);
                        if (val != valinv)
                            *pbyted = ((*pbyted) & (~(1 << bit))) | (val << bit);
                        if (bit == 0)
                            pbyted += 1;
                        od += 1;
                    } while (--cnt);
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Handle expanding pixels from 1 to 8 bits with transparency.

        } else if (pdes->bmPix == 8) {
            if (pix == 1) {
                valinv = (byte) inv;
                if (valinv) {
                    zero = BitsColor(pdes, CBGR+(((lint*)ppal)[0] & RGB_MASK));
                } else {
                    one = BitsColor(pdes, CBGR+(((lint*)ppal)[1] & RGB_MASK));
                }
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 3);
                    val = 1 << (7 - (os & 7));
                    pbyted = (byte*) (prowd + xleft);

                    #ifdef NOI86    //-------
                    do {
                        if (*pbytes & val) {
                            if (!valinv) *pbyted = (byte) one;
                        } else {
                            if (valinv) *pbyted = (byte) zero;
                        }
                        pbyted += 1;
                        val = val >> 1;
                        if (val == 0) {
                            val = 0x80;
                            pbytes += 1;
                        }
                    } while (--cnt);

                    #else           //-------
                    ASM {
                    mov     ecx,cnt
                    mov     esi,pbytes
                    mov     edi,pbyted
                    mov     dl,val
                    mov     dh,valinv
                    mov     eax,zero
                    mov     ebx,one
                    test    dh,dh
                    jnz     c8ilp1              ;one value is invisible?

c8ilp0:             test    byte ptr [esi],dl   ;loop for invisible zero value
                    jnz     c8inz0              ;not zero value?
                    add     edi,1               ;zero value is invisible
                    shr     dl,1
                    jz      c8iby0
                    dec     ecx
                    jnz     c8ilp0
                    jmp     c8idn1

c8iby0:             inc     esi                 ;come here for next 8 bits
                    mov     dl,0x80
                    dec     ecx
                    jnz     c8ilp0
                    jmp     c8idn1

c8inz0:             mov     [edi],bl            ;write one value
                    add     edi,1
                    shr     dl,1
                    jz      c8iby0
                    dec     ecx
                    jnz     c8ilp0
                    jmp     c8idn1

c8ilp1:             test    byte ptr [esi],dl   ;loop for invisible one value
                    jz      c8izr1              ;zero value?
                    add     edi,1               ;one value is invisible
                    shr     dl,1
                    jz      c8iby1
                    dec     ecx
                    jnz     c8ilp1
                    jmp     c8idn1

c8iby1:             inc     esi                 ;come here for next 8 bits
                    mov     dl,0x80
                    dec     ecx
                    jnz     c8ilp1
                    jmp     c8idn1

c8izr1:             mov     [edi],al            ;write zero value
                    add     edi,1
                    shr     dl,1
                    jz      c8iby1
                    dec     ecx
                    jnz     c8ilp1
c8idn1:
                    }
                    #endif          //-------

                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);

// Handle expanding pixels from 4 to 8 bits with transparency.

            } else if (pix == 4) {
                valinv = (byte) inv;
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 1);
                    pbyted = (byte*) (prowd + xleft);
                    do {
                        if (!(os & 1)) {        //first nibble in source byte?
                            val = (*pbytes) >> 4;
                        } else {                //second nibble in source byte?
                            val = (*pbytes) & 0xF;
                            pbytes += 1;
                        }
                        os += 1;
                        if (val != valinv) *pbyted = val;
                        pbyted += 1;
                    } while (--cnt);
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);

            } else {
                return(ECCANNOT);               //can only convert 1,4 to 8 bits
            }

// Handle expanding pixels from 8 to 32 bits with transparency.

        } else if (pdes->bmPix == 32) {
            if (pix == 8) {
                valinv = (byte) inv;
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + os;
                    plongd = (lint*) (prowd + (xleft << 2));

                    #ifdef NOI86    //-------
                    do {
                        val = *pbytes++;
                        if (val != valinv) *plongd = ((lint*)ppal)[val];
                        plongd += 1;
                    } while (--cnt);

                    #else           //-------
                    ASM {
                    mov     ecx,cnt
                    mov     esi,pbytes
                    mov     edi,plongd
                    mov     ebx,ppal
                    mov     dl,valinv
cbilp8:             xor     eax,eax
                    mov     al,[esi]
                    inc     esi
                    cmp     al,dl
                    je      cbisk8
                    mov     eax,[ebx + (eax*4)]
                    mov     [edi],eax
cbisk8:             add     edi,4
                    dec     ecx
                    jnz     cbilp8
                    }
                    #endif          //-------

                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Handle expanding pixels from 4 to 32 bits with transparency.

            if (pix == 4) {
                valinv = (byte) inv;
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 1);
                    plongd = (lint*) (prowd + (xleft << 2));

                    #ifdef NOI86    //-------
                    do {
                        if (!(os & 1)) {
                            val = (*pbytes) >> 4;
                        } else {
                            val = (*pbytes) & 0xF;
                            pbytes += 1;
                        }
                        os += 1;
                        if (val != valinv) *plongd = ((lint*)ppal)[val];
                        plongd += 1;
                    } while (--cnt);

                    #else           //-------
                    ASM {
                    mov     ecx,cnt
                    mov     esi,pbytes
                    mov     edi,plongd
                    mov     ebx,ppal
                    mov     edx,os
                    mov     dh,valinv
cbilp4:             xor     eax,eax
                    mov     al,[esi]
                    test    dl,1
                    jnz     cbitw4
                    shr     al,4
                    jmp     short cbijn4

cbitw4:             and     al,0x0F
                    inc     esi
cbijn4:             inc     dl
                    cmp     al,dh
                    je      cbisk4
                    mov     eax,[ebx + (eax*4)]
                    mov     [edi],eax
cbisk4:             add     edi,4
                    dec     ecx
                    jnz     cbilp4
                    }
                    #endif          //-------

                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }

// Handle expanding pixels from 1 to 32 bits with transparency.

            if (pix == 1) {
                valinv = (byte) inv;
                zero = ((lint*)ppal)[0];
                one = ((lint*)ppal)[1];
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 3);
                    val = 1 << (7 - (os & 7));
                    plongd = (lint*) (prowd + (xleft << 2));

                    #ifdef NOI86    //-------
                    do {
                        if (*pbytes & val) {
                            if (!valinv) *plongd = one;
                        } else {
                            if (valinv) *plongd = zero;
                        }
                        plongd += 1;
                        val = val >> 1;
                        if (val == 0) {
                            val = 0x80;
                            pbytes += 1;
                        }
                    } while (--cnt);

                    #else           //-------
                    ASM {
                    mov     ecx,cnt
                    mov     esi,pbytes
                    mov     edi,plongd
                    mov     dl,val
                    mov     dh,valinv
                    mov     eax,zero
                    mov     ebx,one
                    test    dh,dh
                    jnz     cbilp1              ;one value is invisible?

cbilp0:             test    byte ptr [esi],dl   ;loop for invisible zero value
                    jnz     cbinz0              ;not zero value?
                    add     edi,4               ;zero value is invisible
                    shr     dl,1
                    jz      cbiby0
                    dec     ecx
                    jnz     cbilp0
                    jmp     cbidn1

cbiby0:             inc     esi                 ;come here for next 8 bits
                    mov     dl,0x80
                    dec     ecx
                    jnz     cbilp0
                    jmp     cbidn1

cbinz0:             mov     [edi],ebx           ;write one value
                    add     edi,4
                    shr     dl,1
                    jz      cbiby0
                    dec     ecx
                    jnz     cbilp0
                    jmp     cbidn1

cbilp1:             test    byte ptr [esi],dl   ;loop for invisible one value
                    jz      cbizr1              ;zero value?
                    add     edi,4               ;one value is invisible
                    shr     dl,1
                    jz      cbiby1
                    dec     ecx
                    jnz     cbilp1
                    jmp     cbidn1

cbiby1:             inc     esi                 ;come here for next 8 bits
                    mov     dl,0x80
                    dec     ecx
                    jnz     cbilp1
                    jmp     cbidn1

cbizr1:             mov     [edi],eax           ;write zero value
                    add     edi,4
                    shr     dl,1
                    jz      cbiby1
                    dec     ecx
                    jnz     cbilp1
cbidn1:
                    }
                    #endif          //-------

                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);
            }
        }
    }

// =======
// Handle non-transparent source bitmap with separate code for speed.
// Handle same pixel sizes in source and destination.
// If possible, copy whole block of contiguous memory.

    pix = psrc->bmPix;
    if (pdes->bmPix == pix) {

        if ((drow == srow)&&(fromx == 0)&&(nx == pdes->bmX)) {
            bytes = ny * drow;
            if (drow < 0) {
                prows = ADDOFF(byte*, prows, bytes-drow);
                prowd = ADDOFF(byte*, prowd, bytes-drow);
                bytes = -bytes;
            }
            OSCOPYD(prowd, prows, bytes);
            return(0);
        }

// Copy 32-bit pixel block.

        if (pix == 32) {
            prows += fromx << 2;
            prowd += xleft << 2;
            bytes = nx << 2;
            do {
                OSCOPYD(prowd, prows, bytes);
                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Copy 8-bit pixel block.

        if (pix == 8) {
            prows += fromx;
            prowd += xleft;
            do {
                OSCOPY(prowd, prows, nx);
                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Copy 4-bit pixel block.

        if (pix == 4) {
            do {
                cnt = nx;
                os = fromx;
                od = xleft;
                pbytes = prows + (os >> 1);
                pbyted = prowd + (od >> 1);
                do {
                    if (!(os & 1)) {            //first nibble in source byte?
                        if (!(od & 1)) {        //can copy multiple bytes?
                            cbyte = cnt >> 1;
                            if (cbyte) {
                                OSCOPY(pbyted, pbytes, cbyte);
                                pbytes += cbyte;
                                pbyted += cbyte;
                                cpix = cbyte + cbyte;
                                os += cpix;
                                od += cpix;
                                cnt -= cpix;
                                if (cnt) continue;
                                break;
                            }
                        } else {
                            val = (*pbytes) >> 4;
                        }
                    } else {                    //second nibble in source byte?
                        val = (*pbytes) & 0xF;
                        pbytes += 1;
                    }
                    os += 1;

                    if (!(od & 1)) {            //first nibble in dest byte?
                        *pbyted = ((*pbyted) & 0xF) | (val << 4);
                    } else {                    //second nibble in dest byte?
                        *pbyted = ((*pbyted) & 0xF0) | (val);
                        pbyted += 1;
                    }
                    od += 1;
                } while (--cnt);
                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Copy 1-bit monochrome pixel block.

        if (pix == 1) {
            do {
                cnt = nx;
                os = fromx;
                od = xleft;
                pbytes = prows + (os >> 3);
                pbyted = prowd + (od >> 3);
                do {
                    bit = os & 7;
                    if ((bit == 0)&&(!(od & 7))) {
                        cbyte = cnt >> 3;       //can copy multiple bytes?
                        if (cbyte) {
                            OSCOPY(pbyted, pbytes, cbyte);
                            pbytes += cbyte;
                            pbyted += cbyte;
                            cpix = cbyte << 3;
                            os += cpix;
                            od += cpix;
                            cnt -= cpix;
                            if (cnt) continue;
                            break;
                        }
                    }
                    val = ((*pbytes) >> (7-bit)) & 1;
                    if (bit == 7) pbytes += 1;
                    os += 1;

                    bit = 7 - (od & 7);
                    *pbyted = ((*pbyted) & (~(1 << bit))) | (val << bit);
                    if (bit == 0) pbyted += 1;
                    od += 1;
                } while (--cnt);
                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Handle different pixel sizes in source and destination.
// Handle expanding pixels from 1 to 8 bits.

        } else if (pdes->bmPix == 8) {
            if (pix == 1) {
                zero = BitsColor(pdes, CBGR+(((lint*)ppal)[0] & RGB_MASK));
                one = BitsColor(pdes, CBGR+(((lint*)ppal)[1] & RGB_MASK));
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 3);
                    val = 1 << (7 - (os & 7));
                    pbyted = (byte*) (prowd + xleft);

                    #ifdef NOI86    //-------
                    do {
                        if (*pbytes & val) {
                           *pbyted = (byte) one;
                        } else {
                           *pbyted = (byte) zero;
                        }
                        pbyted += 1;
                        val = val >> 1;
                        if (val == 0) {
                            val = 0x80;
                            pbytes += 1;
                        }
                    } while (--cnt);

                    #else           //-------
                    ASM {
                    mov     ecx,cnt
                    mov     esi,pbytes
                    mov     edi,pbyted
                    mov     dl,val
                    mov     eax,zero
                    mov     ebx,one

c8lop1:             test    byte ptr [esi],dl   ;loop for invisible zero value
                    jz      c8zro1              ;zero value?
                    mov     [edi],bl            ;write one value
                    add     edi,1
                    shr     dl,1
                    jz      c8byt1
                    dec     ecx
                    jnz     c8lop1
                    jmp     c8don1

c8byt1:             inc     esi                 ;come here for next 8 bits
                    mov     dl,0x80
                    dec     ecx
                    jnz     c8lop1
                    jmp     c8don1

c8zro1:             mov     [edi],al            ;write zero value
                    add     edi,1
                    shr     dl,1
                    jz      c8byt1
                    dec     ecx
                    jnz     c8lop1
c8don1:
                    }
                    #endif          //-------

                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);


// Handle expanding pixels from 4 to 8 bits.

            } else if (pix == 4) {
                do {
                    cnt = nx;
                    os = fromx;
                    pbytes = prows + (os >> 1);
                    pbyted = (byte*) (prowd + xleft);
                    do {
                        if (!(os & 1)) {        //first nibble in source byte?
                            val = (*pbytes) >> 4;
                        } else {                //second nibble in source byte?
                            val = (*pbytes) & 0xF;
                            pbytes += 1;
                        }
                        os += 1;
                        *pbyted++ = val;
                    } while (--cnt);
                    prows += srow;
                    prowd += drow;
                } while (--ny);
                return(0);

            } else {
                return(ECCANNOT);               //can only convert 1,4 to 8 bits
            }

// Handle expanding pixels from 24 to 32 bits.

    } else if (pdes->bmPix == 32) {
        if (pix == 24) {
            do {
                cnt = nx;
                pbytes = prows + (fromx+fromx+fromx);
                pbgr = (BGR*) (prowd + (xleft << 2));

                #ifdef NOI86    //-------
                do {
                    pbgr->bblue = *pbytes++;
                    pbgr->bgreen= *pbytes++;
                    pbgr->bred  = *pbytes++;
                    pbgr->balpha= 0xFF;
                    pbgr += 1;
                } while (--cnt);

                #else           //-------
                ASM {
                xor     eax,eax
                mov     ecx,cnt
                mov     esi,pbytes
                mov     edi,pbgr
cblop2:         mov     ax,[esi]
                mov     dl,[esi+2]
                mov     dh,0xFF
                shl     edx,16
                add     esi,3
                or      edx,eax
                mov     [edi],edx
                add     edi,4
                dec     ecx
                jnz     cblop2
                }
                #endif          //-------

                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Handle expanding pixels from 8 to 32 bits.

        if (pix == 8) {
            do {
                cnt = nx;
                pbytes = prows + fromx;
                plongd = (lint*) (prowd + (xleft << 2));

                #ifdef NOI86    //-------
                do {
                    *plongd++ = ((lint*)ppal)[*pbytes++];
                } while (--cnt);

                #else           //-------
                ASM {
                mov     ecx,cnt
                mov     esi,pbytes
                mov     edi,plongd
                mov     ebx,ppal
                xor     eax,eax
cblop8:         mov     al,[esi]
                inc     esi
                mov     edx,[ebx + (eax*4)]
                mov     [edi],edx
                add     edi,4
                dec     ecx
                jnz     cblop8
                }
                #endif          //-------

                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Handle expanding pixels from 4 to 32 bits.

        if (pix == 4) {
            do {
                cnt = nx;
                os = fromx;
                pbytes = prows + (os >> 1);
                plongd = (lint*) (prowd + (xleft << 2));

                #ifdef NOI86    //-------
                do {
                    if (!(os & 1)) {
                        val = (*pbytes) >> 4;
                    } else {
                        val = (*pbytes) & 0xF;
                        pbytes += 1;
                    }
                    os += 1;
                    *plongd++ = ((lint*)ppal)[val];
                } while (--cnt);

                #else           //-------
                ASM {
                mov     ecx,cnt
                mov     esi,pbytes
                mov     edi,plongd
                mov     ebx,ppal
                mov     edx,os
cblop4:         xor     eax,eax
                mov     al,[esi]
                test    edx,1
                jnz     cbtwo4
                shr     al,4
                jmp     short cbjne4

cbtwo4:         and     al,0x0F
                inc     esi
cbjne4:         inc     edx
                mov     eax,[ebx + (eax*4)]
                mov     [edi],eax
                add     edi,4
                dec     ecx
                jnz     cblop4
                }
                #endif          //-------

                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Handle expanding pixels from 1 to 32 bits.

        if (pix == 1) {
            zero = ((lint*)ppal)[0];
            one = ((lint*)ppal)[1];
            do {
                cnt = nx;
                os = fromx;
                pbytes = prows + (os >> 3);
                val = 1 << (7 - (os & 7));
                plongd = (lint*) (prowd + (xleft << 2));

                #ifdef NOI86    //-------
                do {
                    *plongd++ = ((*pbytes) & val) ? one : zero;
                    val = val >> 1;
                    if (val == 0) {
                        val = 0x80;
                        pbytes += 1;
                    }
                } while (--cnt);

                #else           //-------
                ASM {
                mov     ecx,cnt
                mov     esi,pbytes
                mov     edi,plongd
                mov     dl,val
                mov     eax,zero
                mov     ebx,one
cblop1:         test    byte ptr [esi],dl
                jz      cbzro1                  ;zero value?
                mov     [edi],ebx               ;write one value
                add     edi,4
                shr     dl,1
                jz      cbbyt1
                dec     ecx
                jnz     cblop1
                jmp     cbdon1

cbbyt1:         inc     esi                     ;come here for next 8 bits
                mov     dl,0x80
                dec     ecx
                jnz     cblop1
                jmp     cbdon1

cbzro1:         mov     [edi],eax               ;write zero value
                add     edi,4
                shr     dl,1
                jz      cbbyt1
                dec     ecx
                jnz     cblop1

cbdon1:
                }
                #endif          //-------

                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }

// Handle converting from 32 to 24 bits.

    } else if (pdes->bmPix == 24) {
        if (pix == 32) {
            do {
                cnt = nx;
                pbgr = (BGR*) (prows + (fromx << 2));
                pbyted = prowd + (xleft+xleft+xleft);

                #ifdef NOI86    //-------
                do {
                    *pbyted++ = pbgr->bblue;
                    *pbyted++ = pbgr->bgreen;
                    *pbyted++ = pbgr->bred;
                    pbgr += 1;
                } while (--cnt);

                #else           //-------
                ASM {
                mov     ecx,cnt
                mov     esi,pbgr
                mov     edi,pbyted
cblop24:        mov     eax,[esi]
                add     esi,4
                mov     [edi],ax
                shr     eax,16
                mov     [edi+2],al
                add     edi,3
                dec     ecx
                jnz     cblop24
                }
                #endif          //-------

                prows += srow;
                prowd += drow;
            } while (--ny);
            return(0);
        }
    }

// Cannot handle other pixel sizes.

    return(ECCANNOT);
}

//*************************************************************************
//  BitsColumn:
//      Copies an array of pixels into a bitmap column.
//      Useful for adding GPR single-scan data to a bitmap.
//
//  Arguments:
//      errc BitsColumn (int mode, BMAP* pdes, IPt* pto, lint* psrc, lint npt)
//      mode    0 = Normal
//            128 = Blend partially transparent over existing bitmap    (TRI_CLEAR)
//           +256 = More transparency for TRI_CLEAR mode                (TRI_MORE)
//           +512 = Less transparency for TRI_CLEAR mode                (TRI_LESS)
//      pdes    Target bitmap where column data is written.
//              Automatically converted to 32 bits/pixel if needed.
//      pto     Top pixel in pdes target where column data begins.
//              If NULL then assumes right-most bitmap column from top.
//      psrc    Array of 32-bit pixel values to be written into pdes column.
//              Only uses a single 32-bit pixel value for TRI_CLEAR mode.
//      npt     Number of pixel values in psrc array.
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsColumn (int mode, BMAP* pdes, IPt* pto, lint* psrc, lint npt) {
    BMAP bmap;
    LHUE hue;
    LHUE* prow;
    lint ox, oy, rowinc;
    int rval, gval, bval, rclr, gclr, bclr, cshift;
    errc e;

    if (pdes->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pdes, NULL);
        if (e) return(e);
        BitsFree(pdes);
        OSMemCopy(pdes, &bmap, sizeof(BMAP));
        pdes->bmFlg |= NEWINFO;
    }
    if (pto) {                                  //start at arbitrary point?
        ox = pto->ptx;
        oy = pto->pty;
        if (ox < 0) return(0);                  //off to left cannot appear
        if (ox >= pdes->bmX) return(0);         //off to right cannot appear
        if (oy < 0) {                           //clip if off top
            npt += oy;
            oy = 0;
        } else if (oy >= pdes->bmY) {
            return(0);                          //cannot appear if off bottom
        }
    } else {                                    //use right-most column
        ox = pdes->bmY - 1;
        oy = 0;
    }
    if ((oy + npt) > pdes->bmY) {
        npt = pdes->bmY - oy;                   //clip at bottom if needed
        if (npt <= 0) return(0);
    }
    rowinc = pdes->bmRow;
    prow = ADDOFF(LHUE*, pdes->bmTop, ((oy * rowinc) + (ox << 2)));

    if (mode & TRI_CLEAR) {                     //overlay partially transparent?
        cshift = (mode & TRI_MORE) ? 3 : ((mode & TRI_LESS) ? 1 : 2);
        hue.d = *psrc;
        rclr = hue.b.bred   >> cshift;
        gclr = hue.b.bgreen >> cshift;
        bclr = hue.b.bblue  >> cshift;
        do {
            rval = (int)(prow->b.bred   - (prow->b.bred >> cshift))   + rclr;
            if ((unsigned)rval > 255) {         //blend with existing pixel color
                rval = (rval < 0) ? 0 : 255;
            }
            hue.b.bred = (byte) rval;
            gval = (int)(prow->b.bgreen - (prow->b.bgreen >> cshift)) + gclr;
            if ((unsigned)gval > 255) {
                gval = (gval < 0) ? 0 : 255;
            }
            hue.b.bgreen = (byte) gval;
            bval = (int)(prow->b.bblue  - (prow->b.bblue >> cshift))  + bclr;
            if ((unsigned)bval > 255) {
                bval = (bval < 0) ? 0 : 255;
            }
            hue.b.bblue = (byte) bval;
            prow->d = hue.d;                    //blend overlay color
            prow = ADDOFF(LHUE*, prow, rowinc); //skip to next row
        } while (--npt);
        return(0);
    }

    do {                                        //normal non-overlay case
        prow->d = *psrc++;                      //copy the column pixel
        prow = ADDOFF(LHUE*, prow, rowinc);     //skip to next row
    } while (--npt);
    return(0);
}

//*************************************************************************
//  BitsStretch:
//      Copies a block of bitmap pixels to a destination block with resizing.
//      Blends or expands pixels as needed for resizing.
//      Unlike BitsCopy handles partial transparency.
//      If source has transparency then blends with destination according to
//      the source pixels alpha and leaves the  destination alpha unchanged
//      except where the source is opaque where destination is forced opaque.
//
//      Uses BitsCopy for speed when not resizing (and if not transparent).
//      Destination bitmap must be 32 bits/pixel.
//      Source bitmap may be either 1 or 8 or 32 bits/pixel.
//      Automatically converts 4 bits/pixel source to 8 bits/pixel.
//      For 1-bit source does not do interpolate expansion (assumes mode 2).
//      Like Windows, copies to but not including the right and bottom limits.
//
//  Arguments:
//      errc BitsStretch (int mode,
//                        BMAP* pdes, IBox* pto, BMAP* psrc, IBox* pfrom)
//      mode    0 = Normal blended copy operation, skipping transparent parts
//              1 = Full copy, preserving transparency in destination
//             +2 = Do not interpolate when more destination pixels (duplicate)
//             +4 = Mask visible source with full alphas if dest is transparent
//            512 = Clear destination preserves transparency in pdes (CLEARBG)
//              Mode 0 leaves the destination alphas where non-opaque source.
//              Mode 0 sets the destination alphas to full where opaque source.
//              Mode 1 sets the destination alphas to blended source alphas.
//              Mode 4 kludge changes destination alphas to 0xFF for copied.
//              Mode 512 is much slower and is used only saving transparent PNG.
//      pdes    Destination bitmap.
//              Automatically converted to 32 bits/pixel if needed.
//              If bmPtr is NULL then creates bitmap using pto only
//              If source bitmap has bmInv transparent pixels,
//              then pdes->bmInv is set to CINV to flag possible transparency.
//      pto     Target rectangle within destination bitmap.
//              Use NULL for the entire destination bitmap.
//              Target box is automatically clipped as needed.
//      psrc    Source bitmap.
//              If bmInv is non-zero, copies only non-transparent pixels.
//              Does full blended transparency for alphas from 0-255 if bmInv.
//              May be changed from 1 or 4 to 8 bits per pixel if needed.
//      pfrom   Source rectangle within source bitmap.
//              Use NULL for the entire source bitmap.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECARGUMENT if source rectangle is impossible.
//      Returns ECCANNOT if source bitmap is not 1, 4, 8 or 32 bits/pixel.
//*************************************************************************

csproc BitsStretch (int mode, BMAP* pdes, IBox* pto, BMAP* psrc, IBox* pfrom) {
    IPt dpnt;
    IBox sbox;
    BMAP bmap;
    LHUE* phue;
    BGR* pd;
    BGR* ps;
    BGR* ptemp;
    BGR* spal;
    word mxratio[4];
    word mxone[4];
    lint bdx, bdy, ndx, ndy, bsx, bsy, nsx, nsy, cnt, sinc, dinc;
    lint invis, srow, drow, iratio, mxshift, bitm, spix, off;
    flt fratio;
    int cpu, cpux;
    errc e;

// Make sure destination is 32 bits/pixel and source is 1 or 8 or 32 bits/pixel.

    spix = psrc->bmPix;
    if (spix == 1) {
        mode |= 2;                              //can only duplicate monochrome
    } else if ((spix != 8)&&(spix != 32)) {     //convert from 4 to 8 bits?
        if (psrc->bmPix > 8) return(ECCANNOT);
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 8;
        e = BitsCopy(4, &bmap, NULL, psrc, NULL);
        if (e) return(e);
        BitsFree(psrc);
        OSMemCopy(psrc, &bmap, sizeof(BMAP));
        psrc->bmFlg |= NEWINFO;
        spix = 8;
    }
    if ((psrc->bmPal == NULL)&&(spix <= 8)) {
        return(ECDATABAD);                      //just in case
    }

    if (pdes->bmPtr == NULL) {                  //create destination bitmap?
        if (pto == NULL) return(ECARGUMENT);
        BitsFree(pdes);
        e = BitsNew(pdes,32,pto->xright-pto->xleft,pto->ybottom-pto->ytop,NULL);
        if (e) return(e);

    } else if (pdes->bmPix != 32) {             //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pdes, NULL);
        if (e) return(e);
        BitsFree(pdes);
        OSMemCopy(pdes, &bmap, sizeof(BMAP));
        pdes->bmFlg |= NEWINFO;
    }

// Handle special transparent destination mode with recursive call.

    if ((mode & 4)&&(pdes->bmInv)) {
        mode &= (~4);
        invis = pdes->bmInv;                    //use real background if non-std
        if ((invis == B_INVIS)||(invis == CINV)) {
            invis = W_INVIS;                    //assume std white background
        }
        if ((psrc->bmPix == 1)&&(invis == W_INVIS)) {
            cnt = (psrc->bmInv & 0xFF) ? 0 : 1;
            phue = (LHUE*) &psrc->bmPal[cnt];
            spix = (lint) phue->b.bblue;
            spix += (lint) phue->b.bgreen;
            spix += (lint) phue->b.bred;
            if (spix > (0xC0+0xC0+0xC0)) {      //was 0x80*3
                invis = B_INVIS;                //use black for light text
            }
        }
        //This old way may result in mistakes if bmInv used for non-clear pix.
        //if (pdes->bmInv != invis) {
        //  e = BitsAlphaFill(4, pdes, NULL, 0, (invis & RGB_MASK));
        //  if (e) return(e);                   //use unique transparent color
        //  pdes->bmInv = invis;
        //}
        //This new way slightly changes non-clear pix that use bmInv color.
        pdes->bmInv = invis;
        e = BitsAlphaFill(259, pdes, NULL, 0, (invis & RGB_MASK));
        if (e) return(e);                       //use unique transparent color

        e = BitsStretch(mode, pdes, pto, psrc, pfrom);
        if (e) return(e);                       //do stretch copy recursively
        e = BitsAlphaFill(2, pdes, NULL, 0xFF, (invis & RGB_MASK));
        return(e);                              //full alphas for copied parts
    }

// Get source and destination sizes.

    if (pto) {
        bdx = pto->xleft;
        bdy = pto->ytop;
        ndx = pto->xright - bdx;
        ndy = pto->ybottom - bdy;
        if ((ndx <= 0)||(ndy <= 0)) return(0);  //invisible destination box?
    } else {                                    //use whole destination bitmap?
        bdx = 0;
        bdy = 0;
        ndx = pdes->bmX;
        ndy = pdes->bmY;
    }

    if (pfrom) {
        bsx = pfrom->xleft;
        bsy = pfrom->ytop;
        nsx = pfrom->xright - bsx;
        nsy = pfrom->ybottom - bsy;
        if ((nsx <= 0)||(nsy <= 0)) return(0);  //invisible source box?
        if (  ( bsx < 0)                        //bad pfrom argument?
            ||((bsx + nsx) > psrc->bmX)
            ||( bsy < 0)
            ||((bsy + nsy) > psrc->bmY)  )
            return(ECARGUMENT);
    } else {                                    //use whole source bitmap?
        bsx = 0;
        bsy = 0;
        nsx = psrc->bmX;
        nsy = psrc->bmY;
    }

// Clip target box as needed.

    if (bdx < 0) {
        fratio = ((flt)nsx / (flt)ndx);
        off = (lint) (((flt)bdx * fratio) - 0.5);
        bsx -= off;
        nsx += off;
        if (nsx <= 0) return(0);
        ndx += bdx;
        if (ndx <= 0) return(0);
        bdx = 0;
    }
    if ((bdx + ndx) > pdes->bmX) {
        cnt = (bdx + ndx) - pdes->bmX;
        fratio = ((flt)nsx / (flt)ndx);
        off = (lint) (((flt)cnt * fratio) + 0.5);
        nsx -= off;
        if (nsx <= 0) return(0);
        ndx -= cnt;
        if (ndx <= 0) return(0);
    }
    if (bdy < 0) {
        fratio = ((flt)nsy / (flt)ndy);
        off = (lint) (((flt)bdy * fratio) - 0.5);
        bsy -= off;
        nsy += off;
        if (nsy <= 0) return(0);
        ndy += bdy;
        if (ndy <= 0) return(0);
        bdy = 0;
    }
    if ((bdy + ndy) > pdes->bmY) {
        cnt = (bdy + ndy) - pdes->bmY;
        fratio = ((flt)nsy / (flt)ndy);
        off = (lint) (((flt)cnt * fratio) + 0.5);
        nsy -= off;
        if (nsy <= 0) return(0);
        ndy -= cnt;
        if (ndy <= 0) return(0);
    }

    cpu = (int) BitsCPU();
    if (psrc->bmInv) {                          //possible transparency?
        if (mode & 1) {
            pdes->bmInv = CINV;                 //preserve transparency
        } else {
            cpu |= 128;                         //do not copy transparent pixels
        }
    }

// Just do copy operation if not resizing.
// But do not use BitsCopy if transparency since we do it better here.

    if ((ndx == nsx)&&(ndy == nsy)&&(psrc->bmInv == 0)) {
        dpnt.ptx = bdx;
        dpnt.pty = bdy;
        sbox.xleft = bsx;
        sbox.ytop = bsy;
        sbox.xright = bsx + nsx;
        sbox.ybottom = bsy + nsy;
        return( BitsCopy(0, pdes, &dpnt, psrc, &sbox) );
    }

// Set up temporary intermediate bitmap.

    ptemp = (BGR*) OSAlloc((ndx * nsy) << 2);
    if (ptemp == NULL) return(ECMEMORY);

// Resize the rows.

    if ((ndx <= nsx)||(mode & 2)) {
        fratio = ((flt)ndx / (flt)nsx);
    } else {
        fratio = ((flt)nsx / (flt)ndx);
        cpu |= 256;
    }
    iratio = (lint) (fratio * (flt)DFONE);
    BitsBlendInit(cpu, iratio, &mxshift, mxratio, mxone);

    ps = (BGR*) psrc->bmTop;
    srow = psrc->bmRow;
    ps = ADDOFF(BGR*, psrc->bmTop, (bsy * srow));

    pd = ptemp;
    drow = ndx << 2;

    cpux = cpu & (~128);                        //no transparency intermediate
    cnt = nsy;
    if (spix == 32) {                           //32-bit source
        ps = ADDOFF(BGR*, ps, (bsx << 2));
        do {
            BitsBlend(pd, ndx, 4, ps, nsx, 4, iratio, fratio, cpux,
                      mxshift, mxratio, mxone);
            ps = ADDOFF(BGR*, ps, srow);
            pd = ADDOFF(BGR*, pd, drow);
        } while (--cnt);

    } else if (spix == 8) {                     //8-bit source
        ps = ADDOFF(BGR*, ps, bsx);
        BitsAlpha(psrc, ISALPHA);
        spal = psrc->bmPal;
        do {
            BitsBlend8(pd,ndx,4, (byte*)ps,nsx,1,spal, iratio,fratio,cpux,
                       mxshift, mxratio, mxone);
            ps = ADDOFF(BGR*, ps, srow);
            pd = ADDOFF(BGR*, pd, drow);
        } while (--cnt);

    } else {                                    //1-bit source
        ps = ADDOFF(BGR*, ps, (bsx >> 3));
        bitm = 1 << (7 - (bsx & 7));
        BitsAlpha(psrc, ISALPHA);
        spal = psrc->bmPal;
        do {
            BitsBlend1(pd,ndx,4, (byte*)ps,nsx,bitm,spal, iratio,fratio,cpux,
                       mxshift, mxratio, mxone);
            ps = ADDOFF(BGR*, ps, srow);
            pd = ADDOFF(BGR*, pd, drow);
        } while (--cnt);
    }
    cpu &= ~256;

// Resize over the columns of resized rows.

    if ((ndy <= nsy)||(mode & 2)) {
        fratio = ((flt)ndy / (flt)nsy);
    } else {
        fratio = ((flt)nsy / (flt)ndy);
        cpu |= 256;
    }
    iratio = (lint) (fratio * (flt)DFONE);
    BitsBlendInit(cpu, iratio, &mxshift, mxratio, mxone);

    ps = ptemp;
    sinc = ndx << 2;

    dinc = pdes->bmRow;
    pd = ADDOFF(BGR*, pdes->bmTop, ((bdy * dinc)+(bdx << 2)));

    if (mode & CLEARBG) {
        cnt = ndx;                              //special alpha handling?
        do {
            BitsBlendC(pd, ndy, dinc, ps, nsy, sinc, iratio, fratio, cpu,
                       mxshift, mxratio, mxone);
            ps = ADDOFF(BGR*, ps, 4);
            pd = ADDOFF(BGR*, pd, 4);
        } while (--cnt);
        OSFree(ptemp);
        ptemp = NULL;
        return(0);
    }

    cnt = ndx;
    do {
        BitsBlend(pd, ndy, dinc, ps, nsy, sinc, iratio, fratio, cpu,
                  mxshift, mxratio, mxone);
        ps = ADDOFF(BGR*, ps, 4);
        pd = ADDOFF(BGR*, pd, 4);
    } while (--cnt);

// All done.

    OSFree(ptemp);
    ptemp = NULL;
    return(0);
}

//*************************************************************************
//  BitsAngle:
//      Copies a block of bitmap pixels at a given angle into the destination.
//      Can copy 1, 4, 8 or 32-bit source to 32-bit destination.
//      Does high-quality pixel blending for fully angled mode without A_STEP.
//      Works like BitsStretch in the normal blended transparency mode.
//      Can also create a alpha 0 bitmap just large enough for angled source.
//
//      Can optionally also do a 3D perspective transformation after rotating
//      by the desired angle.  This is faster than using BitsAngle and then
//      BitsPerspective.  But BitsPerspective is better for 3D with 0 angle.
//
//  Arguments:
//      errc BitsAngle (int mode, BMAP* pdes, IPt* pto, flt angle,
//                      BMAP* psrc, IBox* pfrom)
//      mode    0       = Normal fully-angled bitmap.
//             +A_STEP  = Stepped text where source columns stay vertical.
//                        A_STEP must be used with monochrome 1-bit source.
//                        A_STEP cannot be used with a pper 3D perspective.
//             +A_TOBOX = The pto argument is an IBox* pointer (not IPt*).
//                        A_TOBOX is ignored unless pdes->bmPtr is NULL.
//      pdes    Destination bitmap.
//              Currently must be 32-bit format.
//              If pdes->bmPtr is NULL creates bitmap which is just
//              large enough to hold the angled bitmap with zero alpha values
//              in unused portions and sets pto to its target box if A_TOBOX.
//              If pdes->bmPtr is NULL then pdes->bmInv must hold the background
//              color on entry used for blanking the new bitmap.
//      pto     Where upper-left corner of source bitmap goes on destination.
//              If mode +A_TOBOX and NULL pdes->bmPtr, pto must point to IBox or
//              or two IPt's which given target box and return new bitmap's box.
//      angle   Radian angle clockwise from horizontal pointing right.
//              Thus zero is horizontal and is equivalent to BitsCopy.
//              Pi/4 angles the bitmap 45 degrees down to the right.
//              Pi/2 angles the bitmap 90 degrees from top to bottom.
//              3*Pi/2 angles the bitmap 90 degrees from bottom to top.
//      psrc    Source bitmap.
//              If bmInv is non-zero, copies only non-transparent pixels.
//              As of V54 handles partially transparent pixels too.
//      pfrom   Target rectangle within source bitmap.
//              Use NULL for the entire source bitmap (and clips right/bottom).
//
//  Returns:
//      Non-zero code if error.
//      Returns ECCANNOT if cannot do implied pixel conversion.
//*************************************************************************

csproc BitsAngle (int mode, BMAP* pdes, IPt* pto, flt angle,
                  BMAP* psrc, IBox* pfrom) {
    #ifdef NOANGLE                              //===
    return(ECCANNOT);
    #else                                       //===
    byte* prows;
    byte* pbegd;
    byte* pbytes;
    lint* plongd;
    lint* ppal;
    flt* pflts;
    flt* pflt;
    IBox lims;
    LHUE* phue;
    LHUE* palp;
    LHUE pixel;
    LHUE value;
    BMAP blend;
    BMAP clear;
    FPt pixs[4], coor[4];
    FPt fh, fv, inc, fbeg, fpix, frac;
    IPt rto, beg, pix, ih, iv;
    IPt* plr;
    lint nx, ny, ix, iy, zero, one;
    lint os, cnt, cnty, srow, drow;
    lint fromx, fromy, fromo;
    lint xleft, ytop, spix, ival;
    lint desx, desy, limx, limy;
    byte valbit, valinv, fromv;
    flt fred, fgreen, fblue, fval;
    flt part, partx, party, tom;
    #ifndef NOI86
    lint part1, part2, part3, part4;
    word parta[4];
    word partb[4];
    #endif
    FLTTEMP(temp);
    boolv make;
    int semi, cpu, ii;
    errc e, eclip;

// Use faster BitsCopy if horizontal.

    make = FALSE;
    if (pdes->bmPtr == NULL) {
        make = TRUE;
        mode = mode & A_TOBOX;
        pdes->bmPix = 32;
    } else if (angle == 0) {
        return( BitsCopy(0, pdes, pto, psrc, pfrom) );
    }

// This setup is similar to BitsCopy for the monochrome to 32-bit case.

    spix = psrc->bmPix;
    if (pdes->bmPix != 32) {
        return(ECCANNOT);
    }

    if (pfrom) {                                //get source upper-left corner
        fromx = pfrom->xleft;
        fromy = pfrom->ytop;
        nx = pfrom->xright - fromx;
        ny = pfrom->ybottom - fromy;
    } else {
        fromx = 0;
        fromy = 0;
        nx = psrc->bmX;
        ny = psrc->bmY;
    }

    xleft = pto->ptx;                           //destination limits
    ytop = pto->pty;

    BitsAlpha(psrc, ISALPHA);                   //set source alphas
    ppal = (lint*) psrc->bmPal;

    srow = psrc->bmRow;
    drow = pdes->bmRow;
    prows = psrc->bmTop + (fromy * srow);       //get first source row to use

    valinv = (byte) psrc->bmInv;
    if (psrc->bmInv == 0) valinv = 255;         //no transparency?

    if (spix == 1) {
        zero = (ppal)[0];
        one = (ppal)[1];
    } else {                                    //currently only support A_STEP
        mode &= ~A_STEP;                        //for monochrome source bitmaps
    }
    limx = pdes->bmX;                           //high limits used for clipping
    limy = pdes->bmY;

// We use sine and cosine to get the destination bitmap fractional pixels
// between source pixels on a unit circle in its horizontal dimension (at angle)
// and between source vertical pixels (at the angle minus ninety degrees).
// The ih gives the destination X and Y increments for horizontal source pixel.
// The iv gives the destination X and Y increments for vertical source pixel.

    pbegd = pdes->bmTop;

    fh.fpx = (flt) OSCos((flx)angle);
    fv.fpy = fh.fpx;
    fval = fh.fpx * AFONE;
    FLT2INT(cnt, fval, temp);
    ih.ptx = cnt;
    iv.pty = ih.ptx;

    fh.fpy = (flt) OSSin((flx)angle);
    fv.fpx = -fh.fpy;
    fval = fh.fpy * AFONE;
    FLT2INT(cnt, fval, temp);
    ih.pty = cnt;
    iv.ptx = -ih.pty;

// *******
// For A_STEP step-wise angles we use a faster non-blended algorithm.
// First change the increments to for step-wise in the best direction.

    if (mode & A_STEP) {                        //keep source columns vertical?
        os = ABS(ih.pty);
        if (os < ih.ptx) {                      //mostly horizontal?
            ih.pty = (lint) (((flx)ih.pty * AFONE) / (flx)ih.ptx);
            ih.ptx = AFONE;
            iv.ptx = 0;
            iv.pty = AFONE;
        } else if (ih.pty < 0) {                //bottom-to-top?
            ih.ptx = (lint) (((flx)ih.ptx * -AFONE) / (flx)ih.pty);
            ih.pty = -AFONE;
            iv.ptx = AFONE;
            iv.pty = 0;
        } else {                                //top-to-bottom?
            ih.ptx = (lint) (((flx)ih.ptx * AFONE) / (flx)ih.pty);
            ih.pty = AFONE;
            iv.ptx = -AFONE;
            iv.pty = 0;
        }

        //Note that the MapX version did not subtract 1 from xleft and ytop.
        //But here we offset by one less to line up with where full blending
        //algorithm would place the same 90 degree text without A_STEP flag.
        beg.ptx = ((xleft - 1) << AFSHIFT) + (AFONE >> 1);
        beg.pty = ((ytop  - 1) << AFSHIFT) + (AFONE >> 1);

// Now we go through the source bitmap rows and through pixels for each row.
// For each pixel we compute the closest destination pixel by maintaining
// a rounded destination X and Y pixel int.fraction and adding the pixel deltas
// for horizontal source pixels for the given angle above.  We also check each
// destination pixel to see if it is off the bitmap and discard it if so.
// It would be possible to speed up the off-bitmap test by having four loops
// and testing only for the limit in the given direction (after first making
// sure that a previous pixel is on bimtap) but this does not seem worthwhile.
// We could make the result look much better by blending between destination
// pixels given the fraction but this would be very complex, especially since
// multiple hits of the different white and black text pixels could effect
// the same destination pixel and so must all be properly blended together.
// Instead, we just choose the closest pixel and set it to the source color.

        do {                                    //come here for each source row
            cnt = nx;
            os = fromx;
            pbytes = prows + (os >> 3);
            valbit = 1 << (7 - (os & 7));
            pix.ptx = beg.ptx;
            pix.pty = beg.pty;

            do {                                //go thru pixels in source row
                desx = pix.ptx >> AFSHIFT;      //get destination pixel
                if (pix.ptx & AFHALF) desx += 1;
                pix.ptx += ih.ptx;              //and advance for next time
                desy = pix.pty >> AFSHIFT;
                if (pix.pty & AFHALF) desy += 1;
                pix.pty += ih.pty;

                if ((desx >= 0)&&(desx < limx)&&(desy >= 0)&&(desy < limy)) {
                    if (*pbytes & valbit) {     //not clipped (off dest bitmap)?
                        if (valinv != 1) {
                            plongd = ADDOFF(lint*, pbegd, ((desy * drow)+(desx<<2)));
                            *plongd = one;      //fill in pix if not transparent
                        }
                    } else {
                        if (valinv != 0) {
                            plongd = ADDOFF(lint*, pbegd, ((desy * drow)+(desx<<2)));
                            *plongd = zero;
                        }
                    }
                }

                valbit = valbit >> 1;           //advance source pixel
                if (valbit == 0) {
                    valbit = 0x80;
                    pbytes += 1;
                }
            } while (--cnt);

            prows += srow;

            beg.ptx += iv.ptx;
            beg.pty += iv.pty;
        } while (--ny);

        return(0);                              //all done
    }

// *******
// Handle full blended rotation.
// Find the limits of the rotated bitmap within the destination.
// Make it a little to large just in case.
// We formerly added two extra pixels on all sides.
// The lims and ix,iy in theory have two extra pixels on all four sides.
// In tests, there is only one extra pixel presumably due to round errors.
// But there are zero extra pixels on the bottom and right of blend since 
// we add pixels into the target row plus the next row down and over.
// Moreover, there was a rotation crash where it sometimes exceeded the
// allocated number of rows by one due to ih rounding errors for small angles.
// That crash was fixed by setting ih with FLT2INT rather than (lint).
// But for a saftey margin, we add two extra pixels bottom and right.
// One pixel would be enough to match the top and left but maybe don't want odd.

    cpu = (int) BitsCPU();
    tom = (flt) (AMX_TOP * AMX_MOR);
    blend.bmPtr = NULL;
    blend.bmPal = NULL;
    clear.bmPtr = NULL;
    clear.bmPal = NULL;

    lims.xleft = xleft  - 2;
    lims.xright = xleft + 4;

    os = ((ih.ptx * nx) + (iv.ptx * ny)) >> AFSHIFT;
    os = xleft + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.xleft)  lims.xleft = os;
    else if (os > lims.xright) lims.xright = os;

    os = ((ih.ptx * nx)) >> AFSHIFT;
    os = xleft + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.xleft)  lims.xleft = os;
    else if (os > lims.xright) lims.xright = os;

    os = ((iv.ptx * ny)) >> AFSHIFT;
    os = xleft + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.xleft)  lims.xleft = os;
    else if (os > lims.xright) lims.xright = os;

    lims.ytop = ytop    - 2;
    lims.ybottom = ytop + 4;

    os = ((ih.pty * nx) + (iv.pty * ny)) >> AFSHIFT;
    os = ytop + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.ytop)    lims.ytop = os;
    else if (os > lims.ybottom) lims.ybottom = os;

    os = ((ih.pty * nx)) >> AFSHIFT;
    os = ytop + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.ytop)    lims.ytop = os;
    else if (os > lims.ybottom) lims.ybottom = os;

    os = ((iv.pty * ny)) >> AFSHIFT;
    os = ytop + os + ((os >= 0) ? (+4) : (-2));
    if      (os < lims.ytop)    lims.ytop = os;
    else if (os > lims.ybottom) lims.ybottom = os;

// If needed, make a "just large enough" destination.

    ix = lims.xright - lims.xleft;
    iy = lims.ybottom - lims.ytop;
    if (make) {
        DrawBGR(&pixel.b, pdes->bmInv);
        e = BitsMake(pdes, 32, ix, iy, NULL);   //create target bitmap
        if (e) goto error;
        pixel.b.balpha = 0;
        e = BitsAlphaFill(3, pdes, NULL, 0, pixel.d);
        if (e) goto error;                      //zero alphas

        if (mode & A_TOBOX) {
            plr = pto + 1;                      //caller must supply target box
            inc.fpx = (flt) (plr->ptx - pto->ptx);
            inc.fpy = (flt) (plr->pty - pto->pty);
            coor[1].fpx = (fh.fpx * inc.fpx);
            coor[1].fpy = (fh.fpy * inc.fpx);
            coor[2].fpx = (fh.fpx * inc.fpx) + (fv.fpx * inc.fpy);
            coor[2].fpy = (fh.fpy * inc.fpx) + (fv.fpy * inc.fpy);
            coor[3].fpx = (fv.fpx * inc.fpy);
            coor[3].fpy = (fv.fpy * inc.fpy);

            pixs[1].fpx = (fh.fpx * (flt)nx);
            pixs[1].fpy = (fh.fpy * (flt)nx);
            pixs[2].fpx = (fh.fpx * (flt)nx) + (fv.fpx * (flt)ny);
            pixs[2].fpy = (fh.fpy * (flt)nx) + (fv.fpy * (flt)ny);
            pixs[3].fpx = (fv.fpx * (flt)ny);
            pixs[3].fpy = (fv.fpy * (flt)ny);

            ii = (ABS(pixs[1].fpx) > ABS(pixs[2].fpx)) ? 1 : 2;
            if   (ABS(pixs[3].fpx) > ABS(pixs[ii].fpx)) ii = 3;
            fval = (pixs[ii].fpx) ? pixs[ii].fpx : 1;
            inc.fpx = coor[ii].fpx / fval;

            fval = (flt)(lims.xleft - xleft) * inc.fpx;
            FLT2INT(ival, fval, temp);
            pto->ptx = xleft + ival;

            fval = (flt)ix * inc.fpx;
            FLT2INT(ival, fval, temp);
            plr->ptx = pto->ptx + ival;

            ii = (ABS(pixs[1].fpy) > ABS(pixs[2].fpy)) ? 1 : 2;
            if   (ABS(pixs[3].fpy) > ABS(pixs[ii].fpy)) ii = 3;
            fval = (pixs[ii].fpy) ? pixs[ii].fpy : 1;
            inc.fpy = coor[ii].fpy / fval;

            fval = (flt)(lims.ytop - ytop) * inc.fpy;
            FLT2INT(ival, fval, temp);
            pto->pty = ytop + ival;

            fval = (flt)iy * inc.fpy;
            FLT2INT(ival, fval, temp);
            plr->pty = pto->pty + ival;
        } else {
            pto->ptx = lims.xleft;              //return corner of target
            pto->pty = lims.ytop;
        }

        if (pfrom == NULL) {                    //faster for right angles
            if        (angle == ((flt)(A_PI*0.5))) {
                return( BitsTurn90(1, pdes, psrc) );

            } else if (  (angle == -((flt)(A_PI*0.5)))
                       ||(angle == ((flt)(A_PI*1.5)))  ) {
                return( BitsTurn90(3, pdes, psrc) );

            } else if (  (angle == -((flt)(A_PI)))
                       ||(angle ==  ((flt)(A_PI)))  ) {
                return( BitsTurn90(2, pdes, psrc) );
            }
        }

        drow = pdes->bmRow;
        pbegd = pdes->bmTop;
        limx = pdes->bmX;
        limy = pdes->bmY;
        xleft = xleft - lims.xleft;
        lims.xleft = 0;
        lims.xright = ix;
        ytop = ytop - lims.ytop;
        lims.ytop = 0;
        lims.ybottom = iy;
    }

// Before V54 we did not handle partial transparency for other angles
// since we used the alphas for blending adjoining pixels.  But partial
// transparency is important for scrapbooking PNG embelishments so we
// check for it here and rotate the alphas separately.  A faster approach
// would be to expand the algorithm below to keep five floats pers pixel
// and accumultate the partial transparency alpha information in one pass.

    semi = FALSE;
    if ((psrc->bmInv)&&(psrc->bmPix == 32)) {   //maybe semi transparent?
        pbytes = prows;
        cnty = ny;
        do {                                    //come here for each source row
            cnt = nx;
            phue = ADDOFF(LHUE*, pbytes, (fromx << 2));
            do {                                //go thru pixels in source row
                if (  (phue->b.balpha < ABYTE_FULL)
                    &&(phue->b.balpha > 0)  ) {
                    semi = TRUE;                //at least one semi transparent
                    break;                      //pixel?
                }
                phue += 1;
            } while (--cnt);
            if (semi) break;
            pbytes += srow;
        } while (--cnty);
    }
    if (semi) {
        e = BitsMake(&clear, 32, pdes->bmX, pdes->bmY, NULL);
        if (e == 0) {
            clear.bmInv = CINV;
            BitsBlank(&clear, NULL, CCLEAR);
            OSMemClear(&blend, sizeof(BMAP));
            blend.bmPix = 32;                   //require 32 bits (V59)
            e = BitsCopy(4, &blend, NULL, psrc, NULL);
            if (e == 0) {
                pbytes= blend.bmTop + (fromy * blend.bmRow);
                cnty = blend.bmY;
                do {
                    cnt = blend.bmX;
                    phue = ADDOFF(LHUE*, pbytes, (fromx << 2));
                    do {
                        phue->b.bblue = phue->b.balpha;
                        phue += 1;              //get alphas in blue channel
                    } while (--cnt);
                    pbytes += blend.bmRow;
                } while (--cnty);
                blend.bmInv = 0;                //don't recurse to here
                rto.ptx = xleft;
                rto.pty = ytop;
                e = BitsAngle(0, &clear, &rto, angle, &blend, pfrom);
                BitsFree(&blend);               //rotate alphas
            }
        }
        if (e) {
            semi = FALSE;                       //do old way if problem
            BitsFree(&clear);
        }
    }

// Allocate and zero a "bitmap" to hold blend fractions.

    e = BitsMake(&blend, 128, ix, iy, NULL);
    if (e) goto error;
    e = BitsBlank(&blend, NULL, CNULL);
    if (e) goto error;

// Go through source bitmap and accumulate destination pixel blend fractions.
// Note that this code is similar to code in BitsPerspective.

    fromv = 0;
    fromo = fromx;
    if (spix == 1) {
        fromv = 1 << (7 - (fromo & 7));
        fromo = fromo >> 3;
    } else if (spix == 32) {
        fromo = fromo << 2;
    } else if (spix == 4) {
        fromo = fromo >> 1;
    }

    eclip = 0;
    fbeg.fpx = (flt)(xleft - lims.xleft);       //terms of blend temp offsets
    fbeg.fpy = (flt)(ytop - lims.ytop);
    do {
        cnt = nx;
        pbytes = prows + fromo;
        valbit = fromv;
        fpix.fpx = fbeg.fpx;
        fpix.fpy = fbeg.fpy;

        do {                                    //go thru pixels in source row
            desx = (lint)fpix.fpx;
            frac.fpx = fpix.fpx - (flt)desx;

            desy = (lint)fpix.fpy;
            frac.fpy = fpix.fpy - (flt)desy;

            fpix.fpx += fh.fpx;                 //and advance for next time
            fpix.fpy += fh.fpy;                 //so fpix is rotated next pixel

            value.d = 0;                        //assume clear source pixel
            if (spix == 1) {
                if (*pbytes & valbit) {         //not clipped (off des bitmap)?
                    if (valinv != 1) {
                        value.d = one;
                    }
                } else {
                    if (valinv != 0) {
                        value.d = zero;
                    }
                }
                valbit = valbit >> 1;           //advance source pixel
                if (valbit == 0) {
                    valbit = 0x80;
                    pbytes += 1;
                }
            } else if (spix == 32) {            //BGRA dword pixels
                value.d = *((lint*)pbytes);
                pbytes += 4;
            } else if (spix == 8) {             //byte per pixel
                value.d = *(ppal + (*pbytes));
                pbytes += 1;
            } else {                            //4 bits per pixel
                value.d = *pbytes;
                value.d = (valbit) ? (value.d & 0xF) : ((value.d >> 4) & 0xF);
                valbit = valbit ^ 1;
                value.d = *(ppal + (value.d));
                if (valbit == 0) pbytes += 1;
            }
            if (eclip) {                        //unshowable 3D location?
                eclip = 0;
                continue;
            }

            pflt = ADDOFF(flt*, blend.bmTop, ((desy * blend.bmRow)+(desx<<4)));

            if (value.b.balpha) {               //visible source pixel?
                partx = 1.0f - frac.fpx;
                party = 1.0f - frac.fpy;

                #ifndef NOI86                   //-------
                if (cpu == 5) {                 //MMX?
                    part1 = (lint) (partx * party * AMX_ONE);
                    part2 = (lint) (frac.fpx * party * AMX_ONE);
                    part3 = (lint) (partx * frac.fpy * AMX_ONE);
                    part4 = (lint) (frac.fpx * frac.fpy * AMX_ONE);
                    ASM {
                    mov         cl,0xFF;
                     mov        eax,part1
                    mov         value.b.balpha,cl
                     mov        ebx,part2
                    movd        mm7,value.d     //mm7 = value
                    pxor        mm0,mm0
                    punpcklbw   mm7,mm0         //get BGRA words for value
                     mov        edi,pflt
                    psllw       mm7,AMX_LSF     //get in 0x7F80 full intensity

                    mov         ecx,eax
                    mov         edx,ebx
                    shl         ecx,16
                    shl         edx,16
                     mov        esi,edi
                    or          eax,ecx
                    or          ebx,edx          //get fractions as int.frac
                    mov         dword ptr parta+0,eax
                    mov         dword ptr partb+0,ebx
                    mov         dword ptr parta+4,eax
                    mov         dword ptr partb+4,ebx

                    movq        mm0,parta
                     mov        eax,blend.bmRow
                    pmulhw      mm0,mm7         //mm0 = value * part1
                    movq        mm5,[edi]
                     movq       mm6,[edi+16]
                    paddusw     mm5,mm0         //mm5 = accumulated part1
                     add        esi,eax
                    movq        [edi],mm5

                    movq        mm0,partb
                    pmulhw      mm0,mm7         //mm0 = value * part2
                     mov        eax,part3
                     mov        ebx,part4
                     mov        ecx,eax
                     mov        edx,ebx
                     shl        ecx,16
                     shl        edx,16
                     or         eax,ecx
                     or         ebx,edx          //get fractions as int.frac
                     mov        dword ptr parta+0,eax
                     mov        dword ptr partb+0,ebx
                     mov        dword ptr parta+4,eax
                     mov        dword ptr partb+4,ebx
                     movq       mm1,parta
                     movq       mm2,partb
                    paddusw     mm6,mm0         //mm6 = accumulated part2
                     movq       mm5,[esi]
                    movq        [edi+16],mm6

                    pmulhw      mm1,mm7         //mm1 = value * part3
                     movq       mm6,[esi+16]
                     pmulhw     mm2,mm7         //mm2 = value * part4
                    paddusw     mm5,mm1         //mm5 = accumulated part3
                     paddusw    mm6,mm2         //mm6 = accumulated part4
                    movq        [esi],mm5
                     movq       [esi+16],mm6
                    emms
                    }

                #else                           //---
                if (FALSE) {
                #endif                          //---
                } else {                        //No MMX=> floating point blend
                    fblue  = (flt) value.b.bblue;
                    fgreen = (flt) value.b.bgreen;
                    fred   = (flt) value.b.bred;

                    part = partx * party;
                    pflt[OBBLUE]  += fblue  * part;
                    pflt[OBGREEN] += fgreen * part;
                    pflt[OBRED]   += fred   * part;
                    pflt[OBALPHA] += part;      //upper left pixel portion

                    part = frac.fpx * party;
                    pflt[OBBLUE +4] += fblue  * part;
                    pflt[OBGREEN+4] += fgreen * part;
                    pflt[OBRED  +4] += fred   * part;
                    pflt[OBALPHA+4] += part;    //upper right pixel portion

                    pflt = ADDOFF(flt*, pflt, blend.bmRow);

                    part = partx * frac.fpy;
                    pflt[OBBLUE]  += fblue  * part;
                    pflt[OBGREEN] += fgreen * part;
                    pflt[OBRED]   += fred   * part;
                    pflt[OBALPHA] += part;      //lower left pixel portion

                    part = frac.fpx * frac.fpy;
                    pflt[OBBLUE +4] += fblue  * part;
                    pflt[OBGREEN+4] += fgreen * part;
                    pflt[OBRED  +4] += fred   * part;
                    pflt[OBALPHA+4] += part;    //lower right pixel portion
                }
            }

        } while (--cnt);

        prows += srow;

        fbeg.fpx += fv.fpx;
        fbeg.fpy += fv.fpy;
    } while (--ny);

// Now go through the temporary blend pixels and transfer to our destination.
// Note that this code duplicated BitsPerspective code.

    nx = blend.bmX;
    ny = blend.bmY;
    ix = 0;
    if (lims.xleft < 0) {
        ix = -(lims.xleft);
        nx += lims.xleft;
        lims.xleft = 0;
        if (nx <= 0) goto rclip;
    }
    if (lims.xright >= limx) {
        nx -= (lims.xright - limx) + 1;
        if (nx <= 0) goto rclip;
    }
    iy = 0;
    if (lims.ytop < 0) {
        iy = -(lims.ytop);
        ny += lims.ytop;
        lims.ytop = 0;
        if (ny <= 0) goto rclip;
    }
    if (lims.ybottom >= limy) {
        ny -= (lims.ybottom - limy) + 1;
        if (ny <= 0) goto rclip;
    }
    prows = ADDOFF(byte*, pbegd, ((lims.ytop * drow)+(lims.xleft<<2)));
    pflts = ADDOFF(flt*, blend.bmTop, ((iy * blend.bmRow)+(ix<<4)));
    do {
        cnt = nx;
        desx = lims.xleft;
        pflt = pflts;
        plongd = (lint*) prows;

        #ifndef NOI86                           //---
        if (cpu == 5) {                         //MMX?
            do {
                if (*((word*)pflt + OBALPHA)) {
                    ASM {
                    xor     edx,edx
                    mov     esi,pflt
                    mov     edi,plongd
                    mov     dx,word ptr [esi+(OBALPHA*2)]
                    cmp     sdx,AMX_TOP
                    je      amxjust
                    //Kludge to stop partial transparency inside objects.
                    //At about 45 degrees a grid of less than full areas apears
                    //without next statement (JB should be for AMX_TOP only).
                    cmp     edx,AMX_TOP-(AMX_TOP >> 2);
                    jb      amxpart

                    fld         tom             //reduce to fit in pixel
                    fidiv       word ptr [esi+(OBALPHA*2)]
                    fistp       part1;          //1.0/alpha
                    fwait
                    mov         eax,part1
                    mov         edx,part1
                    shl         eax,16
                     movq       mm1,[esi]
                    or          eax,edx
                    mov         dword ptr parta+0,eax
                    mov         dword ptr parta+4,eax
                    movq        mm2,parta
                    pmulhw      mm1,mm2         //mm1 = accumulated * (1/alpha)
                    packuswb    mm1,mm1         //back BGRA words to bytes
                    movd        eax,mm1
                    mov         [edi],eax
                    jmp         amxjoin

amxjust:            movq        mm1,[esi]       //just fits in pixel
                    psrlw       mm1,AMX_RSF
                    packuswb    mm1,mm1         //back BGRA words to bytes
                    movd        eax,mm1
                    mov         [edi],eax
                    jmp         amxjoin

amxpart:            mov         eax,AMX_TOP     //blend into old destination
                     movd       mm1,[edi]
                    sub         eax,edx
                    shl         eax,AMX_DIF
                     pxor       mm0,mm0
                    mov         edx,eax
                    shl         eax,16
                     punpcklbw  mm1,mm0         //get BGRA words for old value
                    or          eax,edx
                     psllw      mm1,AMX_LSF     //get in 0x7F80 full intensity
                    mov         dword ptr parta+0,eax
                    mov         dword ptr parta+4,eax
                    movq        mm2,parta
                    pmulhw      mm1,mm2         //mm1 = old value * (1-alpha)
                    movq        mm4,[esi]
                    paddusw     mm1,mm4         //mm1 = accumulated with new
                    psrlw       mm1,AMX_RSF
                    packuswb    mm1,mm1         //back BGRA words to bytes
                    movd        eax,mm1
                    mov         [edi],eax

amxjoin:            emms
                    }

                }
                plongd += 1;
                pflt += 4;
            } while (--cnt);

        #else                                   //---
        if (FALSE) {
        #endif                                  //---
        } else {                                //No MMX => float blending
            do {
                if (pflt[OBALPHA]) {
                    pixel.d = *plongd;
                    part = pflt[OBALPHA];
                    if (part > 0.75f) {         //Should be 1.0 (Kludge to stop
                        part = 1.0f / part;     //partial transparency in objs)
                        pflt[OBBLUE]  = pflt[OBBLUE] * part;
                        pflt[OBGREEN] = pflt[OBGREEN] * part;
                        pflt[OBRED]   = pflt[OBRED] * part;
                        pflt[OBALPHA] = 0xFF;
                    } else if (part < 1.0f) {
                        part = 1.0f - part;
                        pflt[OBBLUE]  += (flt)pixel.b.bblue * part;
                        pflt[OBGREEN] += (flt)pixel.b.bgreen * part;
                        pflt[OBRED]   += (flt)pixel.b.bred * part;
                        pflt[OBALPHA] *= 255.0f;
                        pflt[OBALPHA] += (flt)pixel.b.balpha * part;
                    }
                    pixel.b.bblue  = (byte) pflt[OBBLUE];
                    pixel.b.bgreen = (byte) pflt[OBGREEN];
                    pixel.b.bred   = (byte) pflt[OBRED];
                    pixel.b.balpha = (byte) pflt[OBALPHA];
                    *plongd = pixel.d;
                }
                plongd += 1;
                pflt += 4;
            } while (--cnt);
        }

        prows = ADDOFF(byte*, prows, drow);
        pflts = ADDOFF(flt*, pflts, blend.bmRow);
    } while (--ny);

// Handle merging back in partial transparency if needed.

rclip:
    if (semi) {
        prows = (byte*) clear.bmTop;
        pbegd = (byte*) pdes->bmTop;
        cnty = clear.bmY;
        do { 
            cnt = clear.bmX;
            palp = (LHUE*) prows;
            phue = (LHUE*) pbegd;
            do {
                phue->b.balpha = palp->b.bblue;
                palp += 1;
                phue += 1;
            } while (--cnt);
            prows += clear.bmRow;
            pbegd += pdes->bmRow;
        } while (--cnty);
        BitsFree(&clear);
    }
    BitsFree(&blend);
    return(0);

error:
    BitsFree(&blend);
    BitsFree(&clear);
    return(e);
    #endif                                      //===
}

//*************************************************************************
//  BitsTurn90:
//      Similar BitsAngle but faster and only turns in 90 degree increments.
//      Can also be used to flip image horizontally exchanging left and right.
//      Only works for 32 bits per pixel and converts to 32 bits if needed.
//
//  Arguments:
//      errc BitsTurn90 (int mode, BMAP* pdes, BMAP* psrc)
//      mode    1 = Swivel 90 degrees clockwise (up to down)     0x0112 EXIF 6
//              2 = Swivel 180 degrees clockwise (upside down)               3
//              3 = Swivel 270 degrees clockwise (down to up)                8
//              8 = Flip left and right sides (rather than rotate)           2
//              9 = Swivel 90 degreess and then flip  (1+8)                  5
//              10= Swivel 90 degreess and then flip  (2+8)                  4
//              11= Swivel 90 degreess and then flip  (3+8)                  7
//      pdes    Destination bitmap.
//              Old bitmap is freed and recreated if needed.
//              Thus can be blank to create.
//      psrc    Source bitmap to be swiveled.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsTurn90 (int mode, BMAP* pdes, BMAP* psrc) {
    #ifdef NOANGLE                              //===
    return(ECCANNOT);
    #else                                       //===
    BMAP bmap;
    lint sx, sy, nx, ny, srow, cnt, incx, incy, npal;
    lint* psr;
    lint* psp;
    lint* pdr;
    lint* pdp;

    errc e;

    if (psrc->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, psrc, NULL);
        if (e) return(e);
        BitsFree(psrc);
        OSMemCopy(psrc, &bmap, sizeof(BMAP));
        psrc->bmFlg |= NEWINFO;
    }
    sx = psrc->bmX;
    sy = psrc->bmY;
    if  ((mode == 1)||(mode == 3)||(mode == (1+8))||(mode == (3+8))) {
        nx = sy;
        ny = sx;
    } else if ((mode == 2)||(mode == 8)||(mode == (2+8))) {
        nx = sx;
        ny = sy;
    } else {
        return(ECARGUMENT);
    }
    if (  (pdes->bmPtr == NULL)                 //create new destinaiton bitmap?
        ||(pdes->bmPix != psrc->bmPix)
        ||(nx != pdes->bmX)
        ||(ny != pdes->bmY)  ) {
        e = BitsNew(pdes, psrc->bmPix, nx, ny, NULL);
        if (e) return(e);
    }
    if (psrc->bmPal) {                          //duplicate palettes
        if (pdes->bmPal == NULL) return(ECNULL);
        npal = 1 << psrc->bmPix;
        OSMemCopy(pdes->bmPal, psrc->bmPal, (npal << 2));
    }
    pdes->bmInv = psrc->bmInv;                  //and transparency

    psr = (lint*) psrc->bmTop;
    srow = psrc->bmRow;
    if        (mode == 1) {                     //up to down?
        pdr = ADDOFF(lint*, pdes->bmTop, ((nx << 2) - 4));
        incx = pdes->bmRow;
        incy = -4;
    } else if (mode == 3) {                     //down to up?
        pdr = ADDOFF(lint*, pdes->bmTop, ((ny - 1) * pdes->bmRow));
        incx = -(pdes->bmRow);
        incy = 4;
    } else if (mode == 2) {                     //upside down?
        npal = ((ny - 1) * pdes->bmRow) + ((nx << 2) - 4);
        pdr = ADDOFF(lint*, pdes->bmTop, npal);
        incx = -4;
        incy = -(pdes->bmRow);
    } else if (mode == 8) {                     //flip?
        pdr = ADDOFF(lint*, pdes->bmTop, (nx << 2) - 4);
        incx = -4;
        incy = pdes->bmRow;
    } else if (mode == (1+8)) {                 //rotate 90 and then flip
        pdr = ADDOFF(lint*, pdes->bmTop, 0);
        incx = pdes->bmRow;
        incy = 4;
    } else if (mode == (2+8)) {                 //rotate 180 and then flip
        npal = ((ny - 1) * pdes->bmRow);
        pdr = ADDOFF(lint*, pdes->bmTop, npal);
        incx = 4;
        incy = -(pdes->bmRow);
    } else if (mode == (3+8)) {                 //rotate 270 and then flip
        npal = ((ny - 1) * pdes->bmRow) + ((nx << 2) - 4);
        pdr = ADDOFF(lint*, pdes->bmTop, npal);
        incx = -(pdes->bmRow);
        incy = -4;
    }
    do {                                        //go through source rows
        cnt = sx;
        psp = psr;
        pdp = pdr;
        do {                                    //go through source columns
            *pdp = *psp;
            psp += 1;
            pdp = ADDOFF(lint*, pdp, incx);
        } while (--cnt);
        psr = ADDOFF(lint*, psr, srow);
        pdr = ADDOFF(lint*, pdr, incy);
    } while (--sy);
    return(0);                                  //all done
    #endif                                      //===
}

//*************************************************************************
//  BitsSwivel:
//      Rotates a bitmap into a new larger one with invisible added areas.
//      Shifts a given offset from our upper-left corner into another bitmap.
//
//  Arguments:
//      errc BitsSwivel (int mode, BMAP* pdes, BMAP* psrc,
//                       flt angle, IPt* pctr, IBox* pto)
//      mode    Reserved for future use.  Must be zero.
//      pdes    Destination bitmap.  This must be an unused BMAP (not freed).
//              The pdes->bmInv must give background blank color.
//              On return it is replaced with CINV for transparency.
//      psrc    Source bitmap to be swiveled.
//      angle   Radian angle clockwise from horizontal pointing right.
//              Thus zero is horizontal and is equivalent to BitsCopy.
//              Pi/4 angles the bitmap 45 degrees down to the right.
//              Pi/2 angles the bitmap 90 degrees from top to bottom.
//              3*Pi/2 angles the bitmap 90 degrees from bottom to top.
//      pctr    Center point in our bitmap around which our image is swiveled.
//              Use NULL for the center of our bitmap (half width and height).
//      pto     This is current bitmap's target box in some other image.
//              It must give both the upper-left and lower-right corners.
//              On return, these cordinates are ajusted for the rotated bitmap.
//              They give the possibly stretched coordinates for the rotated
//              bitmap's upper-left and lower-right corners.
//              Warning: Code depends on IBox being equivalent to two IPt's.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsSwivel (int mode, BMAP* pdes, BMAP* psrc,
                   flt angle, IPt* pctr, IBox* pto) {
    #ifdef NOANGLE                              //===
    return(ECCANNOT);
    #else                                       //===
    BMAP copy;
    IPt ctr, ulc;
    flt aspect, ratio;
    flt fval;
    lint ival;
    FLTTEMP(temp);
    errc e;

    if (psrc->bmX == 0) return(ECNOINIT);
    copy.bmPtr = NULL;
    ratio = (flt)psrc->bmY / (flt)psrc->bmX;
    aspect = (flt)(pto->xright - pto->xleft);
    aspect = (flt)(pto->ybottom - pto->ytop) / ((aspect) ? aspect : 1.0f);
    if ((ratio < (aspect * 0.98f))||(ratio > (aspect * 1.02f))) {
        fval = (flt)psrc->bmX * aspect;         //bitmap needs right aspect
        FLT2INT(ival, fval, temp);              //in order to swivel isometric
        e = BitsMake(&copy, 32, psrc->bmX, ival, NULL);
        if (e) goto error;
        e = BitsStretch(1, &copy, NULL, psrc, NULL);
        if (e) goto error;
        psrc = &copy;
    }

    if (pctr) {
        ctr.ptx = pctr->ptx;
        ctr.pty = pctr->pty;
    } else {
        ctr.ptx = pto->xleft + ((pto->xright - pto->xleft) >> 1);
        ctr.pty = pto->ytop  + ((pto->ybottom - pto->ytop) >> 1);
    }
    ulc.ptx = pto->xleft;
    ulc.pty = pto->ytop;
    DrawAngles(0, angle, &ctr, &ulc, 1);        //rotate upper-left corner
    pto->xright  += ulc.ptx - pto->xleft;       //so that we swivel about pctr
    pto->ybottom += ulc.pty - pto->ytop;        //and offset (but do not rotate)
    pto->xleft = ulc.ptx;                       //lower-right corner similarly
    pto->ytop  = ulc.pty;

    pdes->bmPtr = NULL;                         //force creation of new bitmap
    pdes->bmPal = NULL;                         //just in case
    e = BitsAngle(0+A_TOBOX, pdes, (IPt*)pto, angle, psrc, NULL);
    if (e) goto error;                          //rotate to big enough bitmap
    if (copy.bmPtr) BitsFree(&copy);
    return(0);

error:
    BitsFree(pdes);
    if (copy.bmPtr) BitsFree(&copy);
    return(e);
    #endif                                      //===
}

//*************************************************************************
//  BitsLine:
//      Draws a single line segment.
//      Can draw wide and/or 3D-effect lines.
//      Can draw to either 8 or 32 bits/pixel bitmaps.
//
//  Arguments:
//      errc BitsLine (int mode, BMAP* pmap,
//                     lint x1, lint y1, lint x2, lint y2,
//                     lint width, lint color)
//
//      mode    Sum of various style flags:
//              Mitre directions are relative to the beginning-to-end vector.
//              Beginning and ending styles do not apply to one-pixel widths.
//              The +16 and +32 flags are ignored if the color argument is zero.
//              L_3D and L_DENT contants can be used in place of +16 and +32.
//
//              0x0 => pointed 3D-effect beginning     (L_POINT)
//              0x1 => left mitre 3D-effect beginning  (L_LEFT)
//              0x2 => square beginning                (L_END)
//              0x3 => right mitre 3D-effect beginning (L_RIGHT)
//
//              0x0 => pointed 3D-effect ending        (L_POINT << 2)
//              0x4 => left mitre 3D-effect ending     (L_LEFT << 2)
//              0x8 => square ending                   (L_END << 2)
//              0xC => right mitre 3D-effect ending    (L_RIGHT << 2)
//
//              +0  = Non-shaded line color.
//              +16 = 3D-effect line using silver-white-color-darkcolor-black.
//              +32 = Indented 3D-effect line (rather than raised).
//
//      pmap    Pointer to bitmap.
//              Must use either 8 or 32 bits/pixel.
//
//      x1, y1  Beginning pixel coordinates.
//      x2, y2  Ending pixel coordinates.
//
//      width   Pixel width of line.
//              Should be one or greater.
//              Should not be greater than LMAXW (32).
//              Odd numbers generally look better for connected lines.
//
//      color   Line color.
//              May either be a CRGB or CBGR color or a Standard Color number.
//              May be CINV+? for the see-through color.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//              If +16 mode flag, should be CSILVER through CCYAN and
//              silver-white-color-darkcolor-black is used for widths of five
//              or more (with the color repeated when width is above five).
//              For width=4, uses white-color-darkcolor-black.
//              For width=3, uses color-darkcolor-black.
//              For width=2, uses color-darkcolor.
//              Use CSILVER for 3D-effect silver-white-silver-grey-black line.
//              Use CRED for 3D-effect pink-pink-red-darkred-black line.
//
//  Returns:
//      Returns ECCANNOT if cannot handle the bitmap pixel size (not 8 or 32).
//*************************************************************************

csproc BitsLine (int mode, BMAP* pmap,
                 lint x1, lint y1, lint x2, lint y2,
                 lint width, lint color) {
    IPt pts[2];
    errc e;

    pts[0].ptx = x1;
    pts[0].pty = y1;
    pts[1].ptx = x2;
    pts[1].pty = y2;
    e = BitsLines(mode, pmap, pts, 2, width, color, NULL);
    return(e);
}

//*************************************************************************
//  BitsLines:
//      Draws a series of connected lines.
//      Can draw wide and/or 3D-effect lines.
//      Can draw to either 8 or 32 bits/pixel bitmaps.
//
//  Arguments:
//      errc BitsLines (int mode, BMAP* pmap, IPt* ppts, lint npts,
//                      lint width, lint color, lint* pcolors)
//
//      mode    Sum of various style flags:
//              Mitre directions are relative to the beginning-to-end vector.
//              Beginning and ending styles do not apply to one-pixel widths.
//              The +16 and +32 flags are ignored if the color argument is zero.
//              L_3D and L_DENT contants can be used in place of +16 and +32.
//              Use a 0 value with pointed ends for best looking non-3D lines.
//              With 0 there is less glitch as lines transition thru diagonals.
//              For 3D lines add in L_DIAG when for less glitch thru diagonals.
//
//              0x0 => pointed 3D-effect beginning     (L_POINT)
//              0x1 => left mitre 3D-effect beginning  (L_LEFT)
//              0x2 => square beginning                (L_END)
//              0x3 => right mitre 3D-effect beginning (L_RIGHT)
//
//              0x0 => pointed 3D-effect ending        (L_POINT << 2)
//              0x4 => left mitre 3D-effect ending     (L_LEFT << 2)
//              0x8 => square ending                   (L_END << 2)
//              0xC => right mitre 3D-effect ending    (L_RIGHT << 2)
//
//              +0  = Non-shaded line color.
//              +16 = 3D-effect line silver-white-color-darkcolor-black (L_3D)
//              +32 = Outline-effect line (rather than raised 3D)       (L_DENT)
//              +64 = Add to 16 or 32 for less glitch through diagonals (L_DIAG)
//                    When +64 added, must use 0 for pointed ends.
//
//      pmap    Pointer to bitmap.
//              Must use either 8 or 32 bits/pixel.
//
//      ppts    Series of X,Y line segment end point pixel coordinates.
//
//      npts    Number of IPt X,Y points in the ppts array.
//              Draws npts-1 connected line segments.
//
//      width   Pixel width of line.
//              Should be one or greater.
//              Should not be greater than LMAXW (32).
//              Odd numbers generally look better for connected lines.
//
//      color   Line color or 0 (CNULL) to use pcolors.
//              May either be a CRGB or CBGR color or a Standard Color number.
//              May be CINV+? for the see-through color.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//              If +16 mode flag, should be CSILVER through CCYAN and
//              silver-white-color-darkcolor-black is used for widths of five
//              or more (with the color repeated when width is above five).
//              For width=4, uses white-color-darkcolor-black.
//              For width=3, uses color-darkcolor-black.
//              For width=2, uses color-darkcolor.
//              Use CSILVER for 3D-effect silver-white-silver-grey-black line.
//              Use CRED for 3D-effect pink-pink-red-darkred-black line.
//
//      pcolors Ignored if the color argument is non-zero.
//              Pointer to an array of width color values.
//              These must be actual pixel values to be stored in the bitmap.
//              For 32-bits they must be BGR values with alpha set.
//              For 8-bits they must be the desired palette index.
//              Use BitsColor() to convert CRGB, CBGR or Standard Colors.
//              Using pcolors rather than color is somewhat faster if the
//              BitsLineHue() routine is used once for many lines.
//
//  Returns:
//      Returns ECCANNOT if cannot handle the bitmap pixel size (not 8 or 32).
//*************************************************************************

csproc BitsLines (int mode, BMAP* pmap, IPt* ppts, lint npts,
                  lint width, lint color, lint* pcolors) {
    lint colors[LMAXW];
    IPt* pbeg;
    IPt* pend;
    IPt* pnext;
    lint* phue;
    lint* ppre;
    lint half, less, skipbeg, skipend, skip, x, y, cnt;
    lint cx, cy, ndx, ndy, cdx, cdy, omit, omid;
    lint cdir, ndir, cedge, nedge, cinc, ninc;

    if ((pmap->bmPix != 8)&&(pmap->bmPix != 32)) return(ECCANNOT);
    if (npts < 2) return(0);
    if (width < 1) width = 1;

// Convert color to a pcolor array if needed.

    if (color) {
        if (width > LMAXW) width = LMAXW;
        pcolors = colors;
        BitsLineHue(mode, pmap, pcolors, width, color);
    }

// Go through the line segments.
// We use the following direction codes: 0=right, 1=down, 2=right, 3=up.
// We use these edge type codes: 0=point, 1=left mitre, 2=square, 3=right mitre.

    pend = ppts;
    pnext = pend+1;
    ndx = pnext->ptx - pend->ptx;
    ndy = pnext->pty - pend->pty;
    ndir = (ndx >= 0) ? ((ndy >= 0) ? ((ndx>ndy)?0:1):((ndx>(-ndy))?0:3))
                      : ((ndy >= 0) ? (((-ndx)>ndy)?2:1):((ndx<ndy)?2:3));
    npts -= 1;

// We have four different algorithms and use goto to choose to avoid deep nest.

    if (width == 1) {                           //special fast 1-pixel case?
        goto lines1;
    }
    if ((mode == 0)||(mode & (L_DIAG+L_DENT))) {//special overlap-ends mode?
        if (width == 3) goto lines3;            //three-pixel case?
        goto lines0;                            //any-width case?
    }

// =======
// Handle the general case for any width and any end style.

    nedge = mode & 0x3;
    less = (width-1) >> 1;
    half = (width-1) - less;

    do {                                        //go through the line segments
        cdx = ndx;                              //advance to next segment
        cdy = ndy;
        cdir = ndir;
        cedge = nedge;
        pbeg = pend;
        pend = pnext;

        pnext = pnext+1;                        //determine next seg's direction
        if (npts > 1) {                         //not the last segment?
            ndx = pnext->ptx - pend->ptx;
            ndy = pnext->pty - pend->pty;
            ndir = (ndx >= 0) ? ((ndy >= 0) ? ((ndx>ndy)?0:1):((ndx>(-ndy))?0:3))
                              : ((ndy >= 0) ? (((-ndx)>ndy)?2:1):((ndx<ndy)?2:3));
            nedge = (ndir - cdir) + 2;          //determine ending edge type
            if      (nedge < 1) nedge = 2 - nedge;
            else if (nedge > 3) nedge = 6 - nedge;
        } else {                                //there is no next segment
            nedge = (mode >> 2) & 0x3;
        }

        phue = pcolors + less;

        if (cdir == 0) {                        //line goes right?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            if (less) {                         //draw wide line left or above
                cinc = (cedge) ? (2 - cedge) : 1;
                ninc = (nedge) ? (2 - nedge) : 1;
                skipbeg = 0;
                skipend = 0;
                y = cy;
                ppre = phue;
                cnt = less;
                do {
                    y -= 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsHorz(pmap, cx, y, cdx, cdy, skipbeg, skipend, *(--ppre), 0);
                } while (--cnt);
            }                                   //then draw center line
            BitsHorz(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);
            if (half) {                         //draw wide line right or below
                phue += 1;
                cinc = (cedge) ? (cedge - 2) : 1;
                ninc = (nedge) ? (nedge - 2) : 1;
                skipbeg = 0;
                skipend = 0;
                y = cy;
                cnt = half;
                do {
                    y += 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsHorz(pmap, cx, y, cdx, cdy, skipbeg, skipend, *phue++, 0);
                } while (--cnt);
            }

        } else if (cdir == 1) {                 //line goes down?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            if (less) {                         //draw wide line left or above
                cinc = (cedge) ? (cedge - 2) : 1;
                ninc = (nedge) ? (nedge - 2) : 1;
                skipbeg = 0;
                skipend = 0;
                x = cx;
                ppre = phue;
                cnt = less;
                do {
                    x -= 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsVert(pmap, x, cy, cdx, cdy, skipbeg, skipend, *(--ppre), 0);
                } while (--cnt);
            }                                   //then draw center line
            BitsVert(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);
            if (half) {                         //draw wide line right or below
                phue += 1;
                cinc = (cedge) ? (2 - cedge) : 1;
                ninc = (nedge) ? (2 - nedge) : 1;
                skipbeg = 0;
                skipend = 0;
                x = cx;
                cnt = half;
                do {
                    x += 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsVert(pmap, x, cy, cdx, cdy, skipbeg, skipend, *phue++, 0);
                } while (--cnt);
            }

        } else if (cdir == 2) {                 //line goes left?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            if (less) {                         //draw wide line left or above
                cinc = (nedge) ? (nedge - 2) : 1;
                ninc = (cedge) ? (cedge - 2) : 1;
                skipbeg = 0;
                skipend = 0;
                y = cy;
                ppre = phue;
                cnt = less;
                do {
                    y -= 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsHorz(pmap, cx, y, cdx, cdy, skipbeg, skipend, *(--ppre), 0);
                } while (--cnt);
            }                                   //then draw center line
            BitsHorz(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);
            if (half) {                         //draw wide line right or below
                phue += 1;
                cinc = (nedge) ? (2 - nedge) : 1;
                ninc = (cedge) ? (2 - cedge) : 1;
                skipbeg = 0;
                skipend = 0;
                y = cy;
                cnt = half;
                do {
                    y += 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsHorz(pmap, cx, y, cdx, cdy, skipbeg, skipend, *phue++, 0);
                } while (--cnt);
            }

        } else {                                //line goes up?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            if (less) {                         //draw wide line left or above
                cinc = (nedge) ? (2 - nedge) : 1;
                ninc = (cedge) ? (2 - cedge) : 1;
                skipbeg = 0;
                skipend = 0;
                x = cx;
                ppre = phue;
                cnt = less;
                do {
                    x -= 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsVert(pmap, x, cy, cdx, cdy, skipbeg, skipend, *(--ppre), 0);
                } while (--cnt);
            }                                   //then draw center line
            BitsVert(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);
            if (half) {                         //draw wide line right or below
                phue += 1;
                cinc = (nedge) ? (nedge - 2) : 1;
                ninc = (cedge) ? (cedge - 2) : 1;
                skipbeg = 0;
                skipend = 0;
                x = cx;
                cnt = half;
                do {
                    x += 1;
                    skipbeg += cinc;
                    skipend += ninc;
                    BitsVert(pmap, x, cy, cdx, cdy, skipbeg, skipend, *phue++, 0);
                } while (--cnt);
            }

        }
    } while (--npts);
    return(0);                                  //all done

// =======
// Handle non-shaded wide lines with special better-looking ends.
// For non-3D lines, we can use triangular ends which overlap.
// This reduces the glitch as the line transitions to diagonal.
// It cannot be used for 3D-effect lines.

lines0:
    less = (width-1) >> 1;
    half = (width-1) - less;
    omit = ((mode & L_DENT)&&(width > 1)) ? (*(pcolors+1)) : 0;

    do {                                        //go through the line segments
        cdx = ndx;                              //advance to next segment
        cdy = ndy;
        cdir = ndir;
        pbeg = pend;
        pend = pnext;

        pnext = pnext+1;                        //determine next seg's direction
        if (npts > 1) {                         //not the last segment?
            ndx = pnext->ptx - pend->ptx;
            ndy = pnext->pty - pend->pty;
            ndir = (ndx >= 0) ? ((ndy >= 0) ? ((ndx>ndy)?0:1):((ndx>(-ndy))?0:3))
                              : ((ndy >= 0) ? (((-ndx)>ndy)?2:1):((ndx<ndy)?2:3));
        }

        phue = pcolors;
        omid = omit;

        if (cdir == 0) {                        //line goes right?
            cx = pbeg->ptx;
            cy = pbeg->pty - less;
            skip = 0;
            if (less) {
                cnt = less;
                do {
                    BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                    omid = 0;
                    cy += 1;
                    skip -= 1;
                } while (--cnt);
            }
            BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, 0);
            if (half) {
                cnt = half;
                do {
                    cy += 1;
                    skip += 1;
                    if (cnt == 1) omid = omit;
                    BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                } while (--cnt);
            }

        } else if (cdir == 1) {                 //line goes down?
            cx = pbeg->ptx - less;
            cy = pbeg->pty;
            skip = 0;
            if (less) {
                cnt = less;
                do {
                    BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                    omid = 0;
                    cx += 1;
                    skip -= 1;
                } while (--cnt);
            }
            BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, 0);
            if (half) {
                cnt = half;
                do {
                    cx += 1;
                    skip += 1;
                    if (cnt == 1) omid = omit;
                    BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                } while (--cnt);
            }

        } else if (cdir == 2) {                 //line goes left?
            cx = pend->ptx;
            cy = pend->pty - less;
            cdx = -cdx;
            cdy = -cdy;
            skip = 0;
            if (less) {
                cnt = less;
                do {
                    BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                    omid = 0;
                    cy += 1;
                    skip -= 1;
                } while (--cnt);
            }
            BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, 0);
            if (half) {
                cnt = half;
                do {
                    cy += 1;
                    skip += 1;
                    if (cnt == 1) omid = omit;
                    BitsHorz(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                } while (--cnt);
            }

        } else {                                //line goes up?
            cx = pend->ptx - less;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            skip = 0;
            if (less) {
                cnt = less;
                do {
                    BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                    omid = 0;
                    cx += 1;
                    skip -= 1;
                } while (--cnt);
            }
            BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, 0);
            if (half) {
                cnt = half;
                do {
                    cx += 1;
                    skip += 1;
                    if (cnt == 1) omid = omit;
                    BitsVert(pmap, cx, cy, cdx, cdy, skip, skip, *phue++, omid);
                } while (--cnt);
            }
        }

    } while (--npts);
    return(0);                                  //all done

// =======
// Handle three-pixel non-3D line as a special case.
// For non-3D lines, we can use triangular ends which overlap.
// This reduces the glitch as the line transitions to diagonal.
// This would be good for any odd width but we currently always use three.
// It cannot be used for 3D-effect lines.

lines3:
    omit = ((mode & L_DENT)&&(width > 1)) ? (*(pcolors+1)) : 0;
    do {                                        //go through the line segments
        cdx = ndx;                              //advance to next segment
        cdy = ndy;
        cdir = ndir;
        pbeg = pend;
        pend = pnext;

        pnext = pnext+1;                        //determine next seg's direction
        if (npts > 1) {                         //not the last segment?
            ndx = pnext->ptx - pend->ptx;
            ndy = pnext->pty - pend->pty;
            ndir = (ndx >= 0) ? ((ndy >= 0) ? ((ndx>ndy)?0:1):((ndx>(-ndy))?0:3))
                              : ((ndy >= 0) ? (((-ndx)>ndy)?2:1):((ndx<ndy)?2:3));
        }

        phue = pcolors;

        if (cdir == 0) {                        //line goes right?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            BitsHorz(pmap, cx, cy-1, cdx, cdy,  0,  0, *phue++, omit);
            BitsHorz(pmap, cx, cy,   cdx, cdy, -1, -1, *phue++, 0);
            BitsHorz(pmap, cx, cy+1, cdx, cdy,  0,  0, *phue++, omit);

        } else if (cdir == 1) {                 //line goes down?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            BitsVert(pmap, cx-1, cy, cdx, cdy,  0,  0, *phue++, omit);
            BitsVert(pmap, cx,   cy, cdx, cdy, -1, -1, *phue++, 0);
            BitsVert(pmap, cx+1, cy, cdx, cdy,  0,  0, *phue++, omit);

        } else if (cdir == 2) {                 //line goes left?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            BitsHorz(pmap, cx, cy-1, cdx, cdy,  0,  0, *phue++, omit);
            BitsHorz(pmap, cx, cy,   cdx, cdy, -1, -1, *phue++, 0);
            BitsHorz(pmap, cx, cy+1, cdx, cdy,  0,  0, *phue++, omit);

        } else {                                //line goes up?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            BitsVert(pmap, cx-1, cy, cdx, cdy,  0,  0, *phue++, omit);
            BitsVert(pmap, cx,   cy, cdx, cdy, -1, -1, *phue++, 0);
            BitsVert(pmap, cx+1, cy, cdx, cdy,  0,  0, *phue++, omit);
        }

    } while (--npts);
    return(0);                                  //all done

// =======
// Handle one-pixel line as a special case for speed.

lines1:
    do {                                        //go through the line segments
        cdx = ndx;                              //advance to next segment
        cdy = ndy;
        cdir = ndir;
        pbeg = pend;
        pend = pnext;

        pnext = pnext+1;                        //determine next seg's direction
        if (npts > 1) {                         //not the last segment?
            ndx = pnext->ptx - pend->ptx;
            ndy = pnext->pty - pend->pty;
            ndir = (ndx >= 0) ? ((ndy >= 0) ? ((ndx>ndy)?0:1):((ndx>(-ndy))?0:3))
                              : ((ndy >= 0) ? (((-ndx)>ndy)?2:1):((ndx<ndy)?2:3));
        }

        phue = pcolors;

        if (cdir == 0) {                        //line goes right?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            BitsHorz(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);

        } else if (cdir == 1) {                 //line goes down?
            cx = pbeg->ptx;
            cy = pbeg->pty;
            BitsVert(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);

        } else if (cdir == 2) {                 //line goes left?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            BitsHorz(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);

        } else {                                //line goes up?
            cx = pend->ptx;
            cy = pend->pty;
            cdx = -cdx;
            cdy = -cdy;
            BitsVert(pmap, cx, cy, cdx, cdy, 0, 0, *phue, 0);
        }

    } while (--npts);
    return(0);                                  //all done
}

//*************************************************************************
//  BitsLineHue:
//      Creates and array of pixel values to be used for line drawing.
//      This is called by BitsLines() if its color argument is non=zero.
//      Or the caller may use it once and then supply the array to
//      to many BitsLines() calls for speed.
//
//  Arguments:
//      void BitsLineHue (int mode, BMAP* pmap, lint* pcolors,
//                        lint width, lint color)
//      mode    Style flags:
//              +0  = Non-shaded line color.
//              +16 = 3D-effect line using silver-white-color-darkcolor-black.
//              +32 = Outline-effect line (rather than raised 3D).
//      pmap    Pointer to bitmap.
//      pcolors Pointer to where the line colors for use by BitsLines()
//              are returned.  The buffer must have space for width values.
//      width   Pixel width of line.
//              Should be one or greater.
//              Should not be greater than LMAXW (32).
//      color   Line color or 0 (CNULL) to use pcolors.
//              May either be a CRGB or CBGR color or a Standard Color number.
//              May be CINV+? for the see-through color.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//              If +16 mode flag, should be CSILVER through CCYAN and
//              silver-silver-color-darkcolor-black is used for widths of five
//              or more (with the color repeated when width is above five).
//              For width=4, uses color-color-darkcolor-black.
//              For width=3, uses color-darkcolor-black.
//              For width=2, uses color-darkcolor.
//              Use CSILVER for 3D-effect silver-white-silver-grey-black line.
//              Use CFUCHSIA for 3D-effect pink-pink-fuchsia-purple-black line.
//              Use CRED for 3D-effect pink-pink-red-darkred-black line.
//              Note that previously width=4 was color-white-darkcolor-black.
//              Note that previously width>4 was silver-white-color-dark-black.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsLineHue (int mode, BMAP* pmap, lint* pcolors,
                    lint width, lint color) {
    LHUE light, dark, bgr;
    lint* phue;
    lint cinc, hue, cnt;

    light.d = BitsColor(pmap, color);
    if ((width > 1)&&(mode & (L_3D+L_DENT))) {
        bgr.d = light.d;
        if (pmap->bmPal) {
            phue = (lint*) pmap->bmPal;
            bgr.d = phue[bgr.d];
        }
        dark.b.bblue = bgr.b.bblue >> 1;
        dark.b.bgreen = bgr.b.bgreen >> 1;
        dark.b.bred = bgr.b.bred >> 1;
        dark.b.balpha = bgr.b.balpha;
        if (pmap->bmPal) dark.d = BitsColor(pmap, CBGR | (dark.d & 0xFFFFFF));
        if (mode & L_DENT) {
            phue = (pcolors + width) - 1;
            cinc = -1;
        } else {
            phue = pcolors;
            cinc = +1;
        }
        if (width == 2) {
            *phue = light.d;
            phue += cinc;
            *phue = dark.d;
        } else if (mode & L_DENT) {
            *phue = dark.d;
            phue += cinc;
            cnt = width - 2;
            do {
                *phue = light.d;
                phue += cinc;
            } while (--cnt);
            *phue = dark.d;
        } else if (width == 3) {
            if (color == CRED) {
                dark.d = light.d;
                light.d = BitsColor(pmap, CPINK);
            }
            *phue = light.d;
            phue += cinc;
            *phue = dark.d;
            phue += cinc;
            *phue = BitsColor(pmap, CBLACK);
        } else if (width == 4) {
            if ((color == CRED)||(color == CFUCHSIA)) {
                *phue = BitsColor(pmap, CPINK);
                phue += cinc;
            } else {
                *phue = light.d;
                phue += cinc;
            }
            *phue = light.d;
            phue += cinc;
            *phue = dark.d;
            phue += cinc;
            *phue = BitsColor(pmap, CBLACK);
        } else {
            if ((color == CRED)||(color == CFUCHSIA)) {
                hue = BitsColor(pmap, CPINK);
                *phue = hue;
                phue += cinc;
                *phue = hue;
                phue += cinc;
            } else {
                hue = BitsColor(pmap, CSILVER);
                *phue = hue;
                phue += cinc;
                *phue = hue;
                phue += cinc;
            }
            cnt = width - 4;
            do {
                *phue = light.d;
                phue += cinc;
            } while (--cnt);
            *phue = dark.d;
            phue += cinc;
            *phue = BitsColor(pmap, CBLACK);
        }
    } else {
        phue = pcolors;
        cnt = width;
        do {
            *phue++ = light.d;
        } while (--cnt);
    }

    return;
}

//*************************************************************************
//  BitsPolygon:
//      Draws a color-filled polygon.
//      Can draw to either 8 or 32 bits/pixel bitmaps.
//      Note that BitsBlank is more efficient for rectangles.
//      Like Windows, fills to but not including the right and bottom limits.
//      However, mode 1 can be used to include extra pixel at right edge.
//      Can also be used to fill only alpha values within shape for 32 bits.
//
//  Arguments:
//      errc BitsPolygon (int mode, BMAP* pmap,
//                        IPt* ppts, lint npts, lint color)
//      mode    0 = Normal (do not include right/bottom pixels).
//              1 = Include extra pixel at right-hand boundry.
//             +2 = Set only alpha values in pmap. Must combine 1+2+4 = mode 7.
//             +4 = Use raw BGR color value with alpha byte.
//      pmap    Pointer to bitmap.
//              Must use either 8 or 32 bits/pixel.
//              For mode +2 must always be 32 bits.
//              For mode +2 always sets bmInv to CINV to enable transparency.
//      ppts    Series of X,Y polygon vertex point pixel coordinates.
//              The last point should be one edge before the first.
//              May use ESIGN ptx values to separate multiple loops.
//      npts    Number of IPt X,Y vertex points in the ppts array.
//      color   Polygon fill color.
//              May either be a CRGB or CBGR color or a Standard Color number.
//              May be CINV+? for the see-through color.
//              See CDraw.h for standard colors and RGB and BGR structures.
//              If mode +4 then color must be a BGR value with alpha byte.
//
//  Returns:
//      Returns ECCANNOT if cannot handle the bitmap pixel size (not 8 or 32).
//*************************************************************************

// Define structure to hold polygon edge information.

STRUCT PEP;
TYPEDEF PEP sfar fPEP;
TYPEDEF fPEP* sfar lpPEP;

BEGSTRUCT   (PEP)
DD( fPEP*,  edNext;       ) //Pointer to next edge point to right (NULL=last)
DD( lint,   edX           ) //X coordinate for edge point
ENDSTRUCT   (PEP)

#define     MAXESZ  4096    //Bytes reserved for edge info in stack

// Entry point.

csproc BitsPolygon (int mode, BMAP* pmap, IPt* ppts, lint npts, lint color) {
    fPEP* edges[MAXESZ/sizeof(fPEP*)];
    lpPEP* ptop;
    lpPEP* pmem;
    lpPEP* prow;
    fPEP* pfree;
    fPEP* psort;
    fPEP* pprev;
    fPEP* pnext;
    IPt* pbeg;
    IPt* pend;
    IPt* pnxt;
    IPt* pone;
    byte* ppix;
    byte* pbyte;
    lint* plong;
    lint cnt, nedge, top, max, size;
    lint x, y, x2, y2, y1, nx, ny, cx, nn, xadd, xinc, temp;
    lint limx, limy, fill, pix, frac, part, extra;
    boolv only;

    if ((pmap->bmPix != 8)&&(pmap->bmPix != 32)) return(ECCANNOT);
    if (npts < 3) return(0);
    pmem = NULL;

    limy = pmap->bmY;                           //bitmap limits
    limx = pmap->bmX;
    extra = mode & 1;
    if (extra) limx -= 1;

// Determine the edge array size needed and allocate if too big for stack.

    nedge = 0;
    top = EMAXI;
    max = 0;

    pnxt = ppts;
    pone = ppts;
    cnt = npts;
    do {
        pbeg = pnxt;
        pnxt += 1;
        pend = pnxt;
        if (pbeg->ptx == (signed)ESIGN) {       //start another loop?
            pone = pnxt;
            continue;
        }
        if ((cnt == 1)||(pend->ptx == (signed)ESIGN)) {
            pend = pone;                        //loop back to beginning?
        }
        y = pbeg->pty;
        y1 = y;
        y2 = pend->pty;
        ny = y2 - y1;
        if (ny < 0) {                           //consider from top to bottom
            ny = -ny;
            y1 = y2;
            y2 = y;
        }
        if ((y1 >= limy)||(y2 < 0)) {           //edge is above or below bitmap?
            continue;
        }
        if (ny > limy) ny = limy;               //won't fill in outside window
        nedge += ny;                            //count number of edge points
        if (y < top) top = y;                   //remember top and bottom
        if (y > max) max = y;                   //of polygon
    } while (--cnt);
    if (nedge == 0) return(0);                  //nothing to draw

    if (top < 0) top = 0;
    if (max > limy) max = limy;
    max = max - top;
    if (max <= 0) return(0);

    size = (max * sizeof(fPEP*)) + (nedge * sizeof(PEP));
    ptop = edges;
    if (size > MAXESZ) {                        //too big for stack area?
        pmem = (lpPEP*) OSAlloc(size);
        if (pmem == NULL) return(ECMEMORY);
        ptop = pmem;
    }
    OSMemClear(ptop, size);                     //zero lpPEP* & edNext pointers
    pfree = ADDOFF(fPEP*, ptop, (max*sizeof(lpPEP*)));

// Go through the edges and calculate all pixels along each.
// Insert thest edge points into the appropriate Y row's edges list.

    pnxt = ppts;
    pone = ppts;
    cnt = npts;
    do {
        pbeg = pnxt;
        pnxt += 1;
        pend = pnxt;
        if (pbeg->ptx == (signed)ESIGN) {       //start another loop?
            pone = pnxt;
            continue;
        }
        if ((cnt == 1)||(pend->ptx == (signed)ESIGN)) {
            pend = pone;                        //loop back to beginning?
        }

        x = pbeg->ptx;                          //get edge endpoint coordinates
        y = pbeg->pty;
        x2 = pend->ptx;
        y2 = pend->pty;
        if (y == y2) continue;                  //ignore horizontal edges

        if (y > y2) {                           //go from top to bottom of edge
            temp = x;
            x = x2;
            x2 = temp;
            temp = y;
            y = y2;
            y2 = temp;
        }
        if ((y >= limy)||(y2 < 0)) {            //edge is above or below bitmap?
            continue;
        }
        nx = x2 - x;                            //deltas over edge
        ny = y2 - y;
        prow = ptop + (y - top);                //beginning Y row to use

        if (nx > 0) {                           //get X increment direction
            xinc = +1;
        } else {
            nx = -nx;
            xinc = -1;
        }

        IDIVIDE(frac, nx, ny, pg);              //frac = nx/ny
        part = frac >> 1;                       //rounding (old ver used zero)
        xadd = (frac >> 16) * xinc;             //integer part of nx/ny
        frac = frac & 0xFFFF;                   //fraction part of nx/ny

        do {                                    //go through Y's for edge
            cx = x;                             //remember current X
            x += xadd;                          //advance X along edge's line
            part += frac;                       //for next time
            if (part & 0x10000) {
                part &= 0xFFFF;
                x += xinc;
            }
            if ((dword)y < (dword)limy) {       //Y inside of bitmap?

                if (cx < 0) cx = 0;             //clip X if needed
                else if (cx >= limx) cx = limx;

                pfree->edX = cx;                //use next free for edge point
                pprev = NULL;
                psort = *prow;
                while (psort) {                 //find sorted X location in row
                    if (cx <= psort->edX) break;
                    pprev = psort;
                    psort = psort->edNext;
                }
                pfree->edNext = psort;          //insert into linked row list
                if (pprev) {
                    pprev->edNext = pfree;      //middle or end or row
                } else {
                    *prow = pfree;              //first struct in row
                }
                pfree += 1;                     //use next free struct next time
            }
            prow += 1;                          //advance to next Y row
            y += 1;
        } while (--ny);

    } while (--cnt);

// Go through the Y rows and fill between adjoining X edge limits.

    only = FALSE;
    pix = pmap->bmPix;
    if (mode & 4) {
        fill = color;
        if (mode & 2) {
            only = TRUE;
            fill = fill & ALPHA_MASK;
            pmap->bmInv = CINV;
        }
    } else {
        fill = BitsColor(pmap, color);          //get fill color to use
    }

    size = pmap->bmRow;
    ppix = ADDOFF(byte*, pmap->bmTop, (top * size));
    prow = ptop;
    cnt = max;

    if (extra) {                                //include right edges?
        do {
            pprev = *prow;
            while (pprev) {
                pnext = pprev->edNext;          //get two X edges along row
                if (pnext == NULL) break;
                x = pprev->edX;
                nx = (pnext->edX - x) + extra;  //avoid window edge glitch
                if ((nx > 1)||((x)&&(x < limx))) {
                    if (pix == 8) {             //8 bits/pixel?
                        pbyte = ADDOFF(byte*, ppix, x);
                        OSFILLB(pbyte, fill, nx);
                    } else if (only == FALSE) { //32 bits/pixel?
                        plong = ADDOFF(lint*, ppix, (x<<2));
                        nx = nx << 2;
                        OSFILLD(plong, fill, nx);
                    } else {                    //only alpha values?
                        plong = ADDOFF(lint*, ppix, (x<<2));
                        nn = nx;
                        do {
                            *plong = (*plong & RGB_MASK) | fill;
                            plong += 1;
                        } while (--nn);
                    }
                }
                pprev = pnext->edNext;
            }

            prow += 1;                          //advance to next Y row
            ppix = ADDOFF(byte*, ppix, size);
        } while (--cnt);

    } else {                                    //do not include right edges?
        do {                                    //(normal Windows-like mode)
            pprev = *prow;
            while (pprev) {
                pnext = pprev->edNext;          //get two X edges along row
                if (pnext == NULL) break;
                x = pprev->edX;
                nx = (pnext->edX - x);
                if (pix == 8) {                 //8 bits/pixel?
                    pbyte = ADDOFF(byte*, ppix, x);
                    OSFILLB(pbyte, fill, nx);
                } else {                        //32 bits/pixel?
                    plong = ADDOFF(lint*, ppix, (x<<2));
                    nx = nx << 2;
                    OSFILLD(plong, fill, nx);
                }
                pprev = pnext->edNext;
            }

            prow += 1;                          //advance to next Y row
            ppix = ADDOFF(byte*, ppix, size);
        } while (--cnt);
    }

// All done.

    if (pmem) OSFree(pmem);
    return(0);
}

//*************************************************************************
//  BitsPen:
//      Uses a "pen" bitmap to draw.
//      The pen bitmap contains a pattern which is copied to the destination.
//      Can use special source alpha values to draw line edges and centers.
//      Can also either copy all source pixels with blended transparency.
//      Can also copy only alpha values where destination has non-zero alphas.
//
//      Unlike BitsLines, draws uniform-width wide lines.
//      Can also "stylus" lines using non-round pen patterns.
//      Can also place individual pen pattern points.
//      Like BitsPattern can draw unconnected lines with X=ESIGN separators.
//      Currently only handles 32 bits/pixel and converts to 32 bits if needed.
//
//      Note that the most common way of using the pen (mode 0) requres that
//      the destination bitmap alphas be initalized to 0's (so that the pen
//      alphas are greater).  Beware that bitmaps usually have 255 alpha values.
//      See BitsAlphaFill and bmFlg ALPHAS0.  Also see BitsBlank with CCLEAR.
//
//  Arguments:
//      errc BitsPen (int mode, BMAP* pmap, IPt* ppts, lint npts,
//                    BMAP* ppen, IBox* pfrom)
//      mode    0 = Draw source pixels with higher alphas than destination.
//              1 = Blend source pixels with destination using source alphas.
//              2 = Change only the alpha transparency values.
//              3 = Change only the alpha values where pmap has non-zero alpha.
//           +256 = Only do every other point for speed.
//           +512 = Only do every fourth point for speed.
//           +768 = Only do every eight point for speed.  
//      pmap    Destination bitmap.  Must be 32 bits/pixel.
//      ppts    Array of X,Y pixel coordinate points.
//              Normally draws the pen bitmap centered at each point
//              along the line segments between successive points.
//              Use X value set to ESIGN to start a new line with next point.
//      npts    Number of X,Y points in ppts.
//      ppen    Source bitmap pen pattern with correct alpha values.
//              For mode 0 a source pixel totally overwrites the destination,
//              but only when the source alpha is >= the desination alpha
//              and also when the source alpha is non-zero (0 is tranparent).
//              For mode 1 source pixel alphas control destination blending.
//              For mode 1 ppen->bmInv should be non-zero for transparency.
//      pfrom   Portion of the ppen source bitmap to be used.
//              Use NULL for the entire source bitmap (and clips right/bottom).
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsPen (int mode, BMAP* pmap, IPt* ppts, lint npts,
                BMAP* ppen, IBox* pfrom) {
    BMAP bmap;
    IPt* pnxt;
    IBox from;
    IBox target;
    IPt corner;
    lint xx, yy, dx, dy, absdx, absdy, xinc, xadd, yadd, yinc;
    lint index, which, part, frac, cntr, xoff, yoff, nx, ny;
    errc e;

    which = 0;
    if (mode & (256+512)) {
        which = (mode & 512) ? 3 : 1;           //every other or fourth point?
        if ((mode & (256+512)) == (256+512)) which = 7;
        mode &= (~(256+512));                   //or every eight?
    }
    index = -1;

    if (ppen->bmPix != 32) return(ECCANNOT);
    if (pmap->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pmap, NULL);
        if (e) return(e);
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }

    if (pfrom == NULL) {
        from.xleft = 0;
        from.xright = ppen->bmX;
        from.ytop = 0;
        from.ybottom = ppen->bmY;
        pfrom = &from;
    }
    nx = pfrom->xright - pfrom->xleft;
    ny = pfrom->ybottom - pfrom->ytop;
    xoff = nx >> 1;
    yoff = ny >> 1;

    while (npts--) {                            //go through the points to draw
        xx = ppts->ptx;
        yy = ppts->pty;
        if (xx == (signed)ESIGN) {              //skip new segment flag
            ppts += 1;
            continue;
        }
        pnxt = ppts;
        if (npts) {
            pnxt = ppts + 1;
            if (pnxt->ptx == (signed)ESIGN) pnxt = ppts;
        }
        ppts += 1;

        dx = pnxt->ptx - xx;
        absdx = ABS(dx);
        dy = pnxt->pty - yy;
        absdy = ABS(dy);

        if ((absdx <= 1)&&(absdy <= 1)) {       //only drawing a single point?
            cntr = 1;
            part = 0;
            frac = 0;
            xinc = 0;
            yinc = 0;
        } else if (absdx >= absdy) {            //mostly horizontal line?
            IDIVIDE(frac, absdy, absdx, lhorz); //frac = ny/nx
            part = frac >> 1;                   //rounding
            cntr = absdx;
            xinc = (dx >= 0) ? (+1) : (-1);
            yadd = (dy >= 0) ? (+1) : (-1);
            xadd = 0;
            yinc = 0;
        } else {                                //mostly vertical line?
            IDIVIDE(frac, absdx, absdy, lvert); //frac = nx/ny
            part = frac >> 1;                   //rounding
            cntr = absdy;
            yinc = (dy >= 0) ? (+1) : (-1);
            xadd = (dx >= 0) ? (+1) : (-1);
            yadd = 0;
            xinc = 0;
        }
        do {                                    //go through line segment
            index += 1;
            if (!(index & which)) {             //not every other or fourth?
                if (mode & (1+2)) {             //special modes?
                    if (mode & 2) {             //special alpha modes?
                        corner.ptx = xx - xoff;
                        corner.pty = yy - yoff;
                        BitsPenErase(pmap, &corner, ppen, pfrom, mode);
                    } else {                    //special blend mode?
                        target.xleft = xx - xoff;
                        target.xright = target.xleft + nx;
                        target.ytop = yy - yoff;
                        target.ybottom = target.ytop + ny;
                        e = BitsStretch(0, pmap, &target, ppen, pfrom);
                        if (e) return(e);       //use transparency blending
                    }
                } else {                        //normal "higher alphas" mode
                    corner.ptx = xx - xoff;
                    corner.pty = yy - yoff;
                    BitsPenPoint(pmap, &corner, ppen, pfrom);
                }                               //use selective pixel copying
            }
            xx += xinc;                         //increment along line segment
            yy += yinc;
            part += frac;
            if (part & DFONE) {
                part -= DFONE;
                xx += xadd;
                yy += yadd;
            }
        } while(--cntr);
    }
    return(0);
}

// *******
// BitsPenPoint: Used by BitsPen to draw the pen bitmap at a single point.

csvoid BitsPenPoint (BMAP* pmap, IPt* pto, BMAP* ppen, IBox* pfrom) {
    LHUE* pmaptop;
    LHUE* ppentop;
    LHUE* pmaprow;
    LHUE* ppenrow;
    lint maprow, penrow, xlim, ylim;
    lint fromx, fromy, xleft, ytop, nx, ny;
    #ifdef NOI86                                //-------
    lint cnt;
    LHUE* psrc;
    LHUE* pdes;
    #endif                                      //-------

    pmaptop = (LHUE*) pmap->bmTop;
    maprow = pmap->bmRow;
    ppentop = (LHUE*) ppen->bmTop;
    penrow = ppen->bmRow;
    xlim = pmap->bmX;
    ylim = pmap->bmY;

    xleft = pto->ptx;
    ytop  = pto->pty;
    fromx = pfrom->xleft;
    fromy = pfrom->ytop;
    nx = pfrom->xright - fromx;
    ny = pfrom->ybottom - fromy;

    if (xleft < 0) {                            //clip left if needed
        nx += xleft;
        if (nx <= 0) return;
        fromx -= xleft;
        xleft = 0;
    }
    if ((xleft + nx) > xlim) {                  //clip right if needed
        nx = xlim - xleft;
        if (nx <= 0) return;
    }
    if (ytop < 0) {                             //clip top if needed
        ny += ytop;
        if (ny <= 0) return;
        fromy -= ytop;
        ytop = 0;
    }
    if ((ytop + ny) > ylim) {                   //clip bottom if needed
        ny = ylim - ytop;
        if (ny <= 0) return;
    }

    pmaprow = ADDOFF(LHUE*, pmaptop, ((ytop * maprow) + (xleft << 2)) );
    ppenrow = ADDOFF(LHUE*, ppentop, ((fromy * penrow) + (fromx << 2)) );
    do {                                        //go through rows
        #ifdef NOI86                            //-------
        psrc = ppenrow;
        pdes = pmaprow;
        cnt = nx;
        do {                                    //go through row pixels
            if ((psrc->b.balpha)&&(psrc->b.balpha >= pdes->b.balpha)) {
                pdes->d = psrc->d;              //copy pixel if greater alpha
            }
            psrc += 1;
            pdes += 1;
        } while (--cnt);
        #else                                   //-------
        // We use an unrolled assembly loop for speed.
        // It would be possible to use MMX PCMPGT and PCMPEG to mask uncopied
        // source pixels and copy two pixels at a time.  However, the MMX
        // loop would still require 12 cycles for two pixels and this
        // regular assembly loop is estimated to use about 11.5 cycles.
        // This estimate assumes that branch prediction gives 1.5 cycle jcc's.
        ASM {
            mov     esi,ppenrow
            mov     ecx,nx
            mov     edi,pmaprow
            and     ecx,~1
            jz      penlast
penloop:    mov     al,[esi+OBALPHA]
            mov     ah,[edi+OBALPHA]
            mov     ebx,[esi]
            mov     edx,[esi+4]
            test    al,al
            jz      penskp1
            cmp     al,ah
            jb      penskp1
            mov     [edi],ebx

penskp1:    mov     al,[esi+4+OBALPHA]
            mov     ah,[edi+4+OBALPHA]
            add     esi,4+4
            add     edi,4+4
            test    al,al
            jz      penskp2
            cmp     al,ah
            jb      penskp2
            mov     [edi-4],edx
penskp2:    sub     ecx,2
            jnz     penloop

penlast:    mov     ecx,nx;
            and     ecx,1
            jz      pendone
            mov     al,[esi+OBALPHA]
            mov     ah,[edi+OBALPHA]
            mov     edx,[esi]
            add     esi,4
            test    al,al
            jz      pendone
            cmp     al,ah
            jb      pendone
            mov     [edi],edx
pendone:
        }
        #endif                                  //-------
        pmaprow = ADDOFF(LHUE*, pmaprow, maprow);
        ppenrow = ADDOFF(LHUE*, ppenrow, penrow);
    } while (--ny);

    return;
}

// *******
// BitsPenErase: Used by BitsPen to draw alphas only in pen bitmap at a point.
// Mode +1 only writes where destination already has non-zero alpha
// and only writes if source is non-zero alpha and result is a lower alpha.
// Not optimized for speed.

csvoid BitsPenErase (BMAP* pmap, IPt* pto, BMAP* ppen, IBox* pfrom, int mode) {
    LHUE* pmaprow;
    LHUE* ppenrow;
    lint fromx, fromy, xleft, ytop, nx, ny;
    lint cnt;
    LHUE* psrc;
    LHUE* pdes;

    xleft = pto->ptx;
    ytop  = pto->pty;
    fromx = pfrom->xleft;
    fromy = pfrom->ytop;
    nx = pfrom->xright - fromx;
    ny = pfrom->ybottom - fromy;

    if (xleft < 0) {                            //clip left if needed
        nx += xleft;
        if (nx <= 0) return;
        fromx -= xleft;
        xleft = 0;
    }
    if ((xleft + nx) > pmap->bmX) {             //clip right if needed
        nx = pmap->bmX - xleft;
        if (nx <= 0) return;
    }
    if (ytop < 0) {                             //clip top if needed
        ny += ytop;
        if (ny <= 0) return;
        fromy -= ytop;
        ytop = 0;
    }
    if ((ytop + ny) > pmap->bmY) {              //clip bottom if needed
        ny = pmap->bmY - ytop;
        if (ny <= 0) return;
    }

    pmaprow = ADDOFF(LHUE*, pmap->bmTop, ((ytop * pmap->bmRow) + (xleft << 2)) );
    ppenrow = ADDOFF(LHUE*, ppen->bmTop, ((fromy * ppen->bmRow) + (fromx << 2)) );
    do {                                        //go through rows
        psrc = ppenrow;
        pdes = pmaprow;
        cnt = nx;
        if (mode & 1) {                         //only overwrite non-zeros?
            do {                                //go through row pixels
                if (  (psrc->b.balpha)
                    &&(pdes->b.balpha)
                    &&(pdes->b.balpha > psrc->b.balpha)  ) {
                    pdes->b.balpha = psrc->b.balpha;
                }
                psrc += 1;
                pdes += 1;
            } while (--cnt);
        } else {                                //overwrite all alphas?
            do {
                pdes->b.balpha = psrc->b.balpha;
                psrc += 1;
                pdes += 1;
            } while (--cnt);
        }
        pmaprow = ADDOFF(LHUE*, pmaprow, pmap->bmRow);
        ppenrow = ADDOFF(LHUE*, ppenrow, ppen->bmRow);
    } while (--ny);

    return;
}

//*************************************************************************
//  BitsPenCreate:
//      Creates a source pen bitmap to be used with BitsPen.
//
//  Arguments:
//      errc BitsPenCreate (int mode, BMAP* ppen, lint width, lint* pcolors)
//      mode    1 = Round pen with one color                         (BP_ROUND1)
//              2 = Round pen with two colors                        (BP_ROUND2)
//              3 = Round pen with three colors                      (BP_ROUND3)
//              4 = Round pen with radial blending of two colors.    (BP_RADIAL)
//             16 = \ Stylus pen with one color                      (BP_PENSR1)
//             17 = / Stylus pen with one color                      (BP_PENSL1)
//             18 = \ Styles pen with two colors                     (BP_PENSR2)
//             19 = / Styles pen with two colors                     (BP_PENSL2)
//           +128 = Create 3D-effect pen with darker right and below (BP_3D)
//           +256 = Scale the pen 4x pixels wider for antialiasing   (BP_4X)
//           +512 = Specify only one *pcolor, use darker for borders (BP_1C)
//          +1024 = Use full 0xFF alpha values for BitsBlend         (BP_FF)
//          +2048 = Keep pend border color to single pixel
//      ppen    Returns created pen.
//              Caller should first BitsFree any previous bitmap.
//      width   Pixel width of pen from its center point.
//              The pen bitmap is normally twice this wide and high.
//      pcolors Array of one or more colors to be used for pen.
//              The first color is for the outermost ring.
//              But if mode +512 the first color is the fill color
//              and the outer border and midpoint are darker version.
//              May be NULL to use CSILVER and assume mode +512.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsPenCreate (int mode, BMAP* ppen, lint width, lint* pcolors) {
    FHUE fout, fmid;
    ISegs segs;
    IPt line[4];
    IPt more[4];
    IPt ctr, beg;
    byte a1, a2, a3;
    lint ox, oy, nx, ny, one, wide, half, ival, lmode;
    lint colors[3];
    flt fval, foff, finc;
    LHUE* prow;
    LHUE* ppix;
    LHUE hue;
    FLTTEMP(temp);
    int idx;
    errc e;

    if (width <= 0) return(ECARGUMENT);
    lmode = mode & 127;
    if (pcolors == NULL) {                      //no colors supplied?
        colors[0] = CSILVER;
        pcolors = colors;
        mode |= BP_1C;
    }
    if (mode & BP_1C) {                         //only single color supplied?
        DrawBGR(&hue.b, *pcolors);
        colors[1] = hue.d;
        if (mode & 2) {                         //need multiple colors?
            hue.b.bblue = hue.b.bblue >> 1;     //darker version for borders
            hue.b.bgreen = hue.b.bgreen >> 1;
            hue.b.bred = hue.b.bred >> 1;
        }
        colors[0] = hue.d;
        colors[2] = hue.d;
        pcolors = colors;
    }

    segs.isPtr = NULL;
    segs.isNpt = 0;
    one = 1;
    if (mode & BP_4X) {                         //four times wider?
        one = 4;
        width = width << 2;
    }
    if (!(mode & BP_E1)) {                      //proportion border to width?
        nx = width >> 2;
        if (one < nx) one = nx;
    }
    wide = (width << 1) + 1;

    e = BitsMake(ppen, 32, wide, wide, NULL);   //create square bitmap
    if (e) goto error;

    e = BitsAlphaFill(11, ppen, NULL, 0, 0);    //zero the bitmap;
    if (e) goto error;

    if (lmode == BP_RADIAL) {                   //round pen with blended colors
        half = wide >> 1;

        DrawBGR(&fout.hue.b, pcolors[0]);
        fout.fblue  = (flt) fout.hue.b.bblue;
        fout.fgreen = (flt) fout.hue.b.bgreen;
        fout.fred   = (flt) fout.hue.b.bred;
        fout.falpha = (flt) fout.hue.b.balpha;

        DrawBGR(&fmid.hue.b, pcolors[1]);
        if (fmid.hue.b.balpha == 0) {           //don't let alpha be zero
            fval = (flt)(fout.hue.b.balpha - fmid.hue.b.balpha) / (flt)(half+1);
            FLT2INT(ival, fval, temp);          //when used as feather brush
            fmid.hue.b.balpha = (byte) ival;    //because zero is non-copied
        }                                       //and because its too clear
        fmid.fblue  = (flt) fmid.hue.b.bblue;
        fmid.fgreen = (flt) fmid.hue.b.bgreen;
        fmid.fred   = (flt) fmid.hue.b.bred;
        fmid.falpha = (flt) fmid.hue.b.balpha;

        fout.fblue  = fout.fblue - fmid.fblue;  //use delta from fmid to fout
        fout.fgreen = fout.fgreen - fmid.fgreen;
        fout.fred   = fout.fred - fmid.fred;
        fout.falpha = fout.falpha - fmid.falpha;

        prow = (LHUE*) ppen->bmTop;
        finc = 1.0f / (flt)half;
        oy = -half;
        ny = wide;
        do {                                    //go through pen rows & cols
            ppix = prow;
            ox = -half;
            nx = ppen->bmX;
            do {
                foff = (flt)OSSqrt( ((flt)ox * (flt)ox) + ((flt)oy * (flt)oy) );
                if (foff <= (flt)half) {
                    foff = foff * finc;

                    fval = fmid.fblue + (foff * fout.fblue);
                    FLT2INT(ival, fval, temp);
                    if (ival > 255) ival = 255;
                    ppix->b.bblue = (byte) ival;

                    fval = fmid.fgreen + (foff * fout.fgreen);
                    FLT2INT(ival, fval, temp);
                    if (ival > 255) ival = 255;
                    ppix->b.bgreen = (byte) ival;

                    fval = fmid.fred + (foff * fout.fred);
                    FLT2INT(ival, fval, temp);
                    if (ival > 255) ival = 255;
                    ppix->b.bred = (byte) ival;

                    fval = fmid.falpha + (foff * fout.falpha);
                    FLT2INT(ival, fval, temp);
                    if (ival > 255) ival = 255;
                    ppix->b.balpha = (byte) ival;
                }
                ox += 1;
                ppix += 1;
            } while (--nx);

            oy += 1;
            prow = ADDOFF(LHUE*, prow, ppen->bmRow);
        } while (--ny);

        return(0);
    }

    a1 = 1;
    a2 = 2;
    a3 = 3;
    if (mode & BP_FF) {
        a1 = 0xFF;
        a2 = 0xFF;
        a3 = 0xFF;
    }

    if (lmode < 16) {                           //circular pen?
        hue.d = BitsColor(ppen, (lmode >= BP_ROUND2) ? pcolors[1] : pcolors[0]);
        hue.b.balpha = a1;
        ctr.ptx = width;
        ctr.pty = width;
        beg.ptx = 0;
        beg.pty = width;
        e = DrawCircle(0, &segs, &ctr, &beg, NULL);
        if (e) goto error;
        e = BitsPolygon(1+4, ppen, segs.isPtr, segs.isNpt, hue.d);
        if (e) goto error;                      //does not show bottom
        e = BitsPattern(4, ppen, segs.isPtr, segs.isNpt, EMASK, hue.d, hue.d);
        if (e) goto error;                      //so need outline to make round
        if (mode & BP_3D) {                     //3D-effect?
            hue.b.bblue = hue.b.bblue >> 1;     //darker version for borders
            hue.b.bgreen = hue.b.bgreen >> 1;
            hue.b.bred = hue.b.bred >> 1;
            e = BitsAlphaFill(6, ppen, NULL, 1, hue.d);
            if (e) goto error;                  //darken lower-right
        }

        if (lmode >= BP_ROUND2) {               //second color?
            hue.d = BitsColor(ppen, pcolors[0]);
            hue.b.balpha = a2;
            beg.ptx = one;                      //outer color is one pixel wide
            e = DrawCircle(0, &segs, &ctr, &beg, NULL);
            if (e) goto error;
            e = BitsPolygon(1+4, ppen, segs.isPtr, segs.isNpt, hue.d);
            if (e) goto error;                  //does not show bottom
            e = BitsPattern(4, ppen, segs.isPtr, segs.isNpt, EMASK, hue.d, hue.d);
            if (e) goto error;                  //so need outline to make round

            if (lmode >= BP_ROUND3) {           //third color?
                hue.d = BitsColor(ppen, pcolors[2]);
                hue.b.balpha = a3;
                if (one == 1) {                 //center is only one pixel?
                    e = BitsPattern(4, ppen, &ctr, 1, EMASK, hue.d, hue.d);
                    if (e) goto error;
                } else {                        //cicular center?
                    beg.ptx = width - one;
                    e = DrawCircle(0, &segs, &ctr, &beg, NULL);
                    if (e) goto error;
                    e = BitsPolygon(1+4, ppen, segs.isPtr, segs.isNpt, hue.d);
                    if (e) goto error;          //does not show bottom
                    e = BitsPattern(4, ppen, segs.isPtr, segs.isNpt, EMASK, hue.d, hue.d);
                    if (e) goto error;          //so need outline to make round
                }
            }

        }
    } else {                                    //slash pen?
        one -= 1;
        line[0].ptx = one;
        line[1].ptx = 0;
        line[2].ptx = wide-1;
        line[3].ptx = wide-(one+1);
        if (lmode & BP_ROUND1) {
            line[0].pty = wide-(one+1);
            line[1].pty = wide-1;
            line[2].pty = 0;
            line[3].pty = one;
        } else {
            line[0].pty = one;
            line[1].pty = 0;
            line[2].pty = wide-1;
            line[3].pty = wide-(one+1);
        }
        idx = 0;
        do {
            more[idx].ptx = line[idx].ptx + 1;
            more[idx].pty = line[idx].pty;
            idx += 1;
        } while (idx < 4);
        if (lmode & BP_ROUND2) {                //two colors?
            hue.d = BitsColor(ppen, pcolors[0]);
            hue.b.balpha = a2;
            e = BitsPattern(4, ppen, &line[1], 2, EMASK, hue.d, hue.d);
            if (e) goto error;
            e = BitsPattern(4, ppen, &more[1], 2, EMASK, hue.d, hue.d);
            if (e) goto error;

            hue.d = BitsColor(ppen, pcolors[1]);
            hue.b.balpha = a1;
            e = BitsPattern(4, ppen, &line[0], 2, EMASK, hue.d, hue.d);
            if (e) goto error;
            e = BitsPattern(4, ppen, &more[0], 2, EMASK, hue.d, hue.d);
            if (e) goto error;

            if (mode & BP_3D) {
                hue.b.bblue = hue.b.bblue >> 1;
                hue.b.bgreen = hue.b.bgreen >> 1;
                hue.b.bred = hue.b.bred >> 1; //darker version for borders
            }
            e = BitsPattern(4, ppen, &line[2], 2, EMASK, hue.d, hue.d);
            if (e) goto error;
            e = BitsPattern(4, ppen, &more[2], 2, EMASK, hue.d, hue.d);
            if (e) goto error;

        } else {                                //one color
            hue.d = BitsColor(ppen, pcolors[0]);
            hue.b.balpha = a1;
            e = BitsPattern(4, ppen, &line[1], 2, EMASK, hue.d, hue.d);
            if (e) goto error;
            e = BitsPattern(4, ppen, &more[1], 2, EMASK, hue.d, hue.d);
            if (e) goto error;
        }
    }
    if (segs.isPtr) OSFree(segs.isPtr);         //all done
    return(0);

error:
    if (segs.isPtr) OSFree(segs.isPtr);
    return(e);
}

//*************************************************************************
//  BitsPattern:
//      Draws a series of line segments and points using a two-color pattern.
//      Handles transparency, but not partially transparent blending.
//      Can optionally set alpha values in destination for later BitsStretch.
//      Currently only handles 32 bits/pixel and converts to 32 bits if needed.
//
//  Arguments:
//      errc BitsPattern (int mode, BMAP* pmap, IPt* ppts, lint npts,
//                        lint pattern, lint fore, lint back)
//      mode    0 = Normal.
//             +2 = Write only alpha values (leave bitmap colors unchanged).
//             +4 = Use raw BGR color value with alpha byte.
//      pmap    Destination bitmap.  Must be 32 bits/pixel.
//      ppts    Array of X,Y pixel coordinate points.
//              Normally draws line segments between successive points.
//              Use X value set to ESIGN to start a new line with next point.
//      npts    Number of X,Y points in ppts.
//      pattern Repeating pattern to be used for points (LSBit first).
//              May use LP_DASH or LP_DOT for standard dashed or dotted lines.
//              May use EMASK for all fore color or 0 for all back color.
//      fore    Foreground color used for 0 pattern bits.
//      back    Background color used for 0 pattern bits.
//              Use CCLEAR for transparent color (unless mode +4).
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsPattern (int mode, BMAP* pmap, IPt* ppts, lint npts,
                    lint pattern, lint fore, lint back) {
    BMAP bmap;
    IPt* pnxt;
    IPt* ptry;
    lint* prow;
    lint* ptop;
    lint xx, yy, dx, dy, absdx, absdy, xinc, yinc, rows, saved;
    lint part, frac, left;
    dword opaque, color;
    dword xlim, ylim;
    int npat;
    errc e;

    if (pmap->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pmap, NULL);
        if (e) return(e);
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }

    ptop = (lint*) pmap->bmTop;
    rows = pmap->bmRow;
    xlim = pmap->bmX;
    ylim = pmap->bmY;

    if (mode & 4) {
        opaque = 0;
    } else {
        opaque = ALPHA_HALF;
        fore = BitsColor(pmap, fore);
        back = BitsColor(pmap, back);
    }
    if (mode & 2) {
        fore = fore >> 24;
        back = back >> 24;
    }
    saved = pattern;
    npat = 32;

    while ((npts--) > 0) {
        xx = ppts->ptx;
        yy = ppts->pty;
        if (xx == (signed)ESIGN) {              //skip new segment flag
            ppts += 1;
            continue;
        }
        pnxt = NULL;
        ptry = ppts + 1;
        left = npts;
        while (left > 0) {
            if (ptry->ptx == (signed)ESIGN) break;
            dx = ptry->ptx - xx;
            absdx = ABS(dx);
            dy = ptry->pty - yy;
            absdy = ABS(dy);
            if ((absdx > 1)||(absdy > 1)) {
                pnxt = ptry;
                break;
            }
            ptry += 1;
            left -= 1;
        }
        if (pnxt == NULL) {                     //draw single point?
            color = (pattern & 1) ? fore : back;
            pattern = pattern >> 1;
            npat -= 1;
            if (npat == 0) {
                npat = 32;
                pattern = saved;
            }
            if (((dword)xx < xlim)&&((dword)yy < ylim)) {
                prow = ADDOFF(lint*, ptop, ((yy * rows) + (xx << 2)) );
                if (mode & 2) {
                    ((BGR*)prow)->balpha = (byte) color;
                } else {
                    if ((color & ALPHA_MASK) >= opaque) *prow = color;
                }                               //write pixel if not transparent
            }
            ppts += 1;
            continue;
        }

        xinc = (dx >= 0) ? (+1) : (-1);         //draw line seg to next point?
        yinc = (dy >= 0) ? (+1) : (-1);
        if (absdx >= absdy) {                   //mostly horizontal line?
            IDIVIDE(frac, absdy, absdx, lhorz); //frac = ny/nx
            part = frac >> 1;                   //rounding
            do {                                //go through line segment
                color = (pattern & 1) ? fore : back;
                pattern = pattern >> 1;
                npat -= 1;                      //get color
                if (npat == 0) {
                    npat = 32;
                    pattern = saved;
                }
                if (((dword)xx < xlim)&&((dword)yy < ylim)) {
                    prow = ADDOFF(lint*, ptop, ((yy * rows) + (xx << 2)) );
                    if (mode & 2) {
                        ((BGR*)prow)->balpha = (byte) color;
                    } else {
                        if ((color & ALPHA_MASK) >= opaque) *prow = color;
                    }                           //write pixel if not transparent
                }
                xx += xinc;                     //increment along line segment
                part += frac;
                if (part & DFONE) {
                    part -= DFONE;
                    yy += yinc;
                }
            } while(--absdx);

        } else {                                //mostly vertical line?
            IDIVIDE(frac, absdx, absdy, lvert); //frac = nx/ny
            part = frac >> 1;                   //rounding
            do {                                //go through line segment
                color = (pattern & 1) ? fore : back;
                pattern = pattern >> 1;
                npat -= 1;                      //get color
                if (npat == 0) {
                    npat = 32;
                    pattern = saved;
                }
                if (((dword)xx < xlim)&&((dword)yy < ylim)) {
                    prow = ADDOFF(lint*, ptop, ((yy * rows) + (xx << 2)) );
                    if (mode & 2) {
                        ((BGR*)prow)->balpha = (byte) color;
                    } else {
                        if ((color & ALPHA_MASK) >= opaque) *prow = color;
                    }                           //write pixel if not transparent
                }
                yy += yinc;                     //increment along line segment
                part += frac;
                if (part & DFONE) {
                    part -= DFONE;
                    xx += xinc;
                }
            } while(--absdy);

        }
        ppts = pnxt;
        npts = left;
    }
    return(0);
}

//*************************************************************************
//  BitsAlpha:
//      Changes a bitmap's palette to have meaningful high byte alpha
//      values (255 for opaque or 0 for transparent)
//      or changes back to the zero values required by windows.
//      Does nothing if there is no palette (32 bits/pixel).
//
//  Arguments:
//      void BitsAlpha (BMAP* pmap, lint alpha)
//      pmap    Pointer to bitmap information.
//      alpha   ISALPHA (8) if should make sure has 255 or 0 real alpha values.
//              Zero (0) if should make sure has all zero high bytes.
//              Must either be 0 or ISALPHA=8!
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsAlpha (BMAP* pmap, lint alpha) {
    lint* ppal;
    lint use;

    if (alpha == (pmap->bmFlg & ISALPHA))       //already in desired state?
        return;
    pmap->bmFlg = (pmap->bmFlg & (~ISALPHA)) | alpha;
    ppal = (lint*) pmap->bmPal;
    use = pmap->bmUse;
    if ((ppal == NULL)||(use == 0)) return;     //no palette to convert?
    if (alpha) {
        do {
            *ppal = *ppal | ALPHA_MASK;         //set alpha value to 255
            ppal += 1;
        } while (--use);
        if (pmap->bmInv) {                      //set invisible color alpha to 0
            pmap->bmPal[pmap->bmInv & 0xFF].balpha = 0;

        }
    } else {
        do {
            *ppal = *ppal & RGB_MASK;           //set alpha value to 0
            ppal += 1;
        } while (--use);
    }
    return;
}

//*************************************************************************
//  BitsAlphaFill:
//      Fills only the alpha byte values in a 32 bits/pixel bitmap.
//      Can also fill both color and alpha values (like BitsBlank with alphas).
//      Can also fill only areas with a given color or a given alpha.
//      Can also fill only the lower right diagonal area for a 3D effect.
//      Can also fade alpha values by multiplying by a 0-255 fraction.
//      Handles only 32 bits per pixel and converts to 32 bits if needed.
//
//      Note that BitsStretch can use the alpha values for blended copies.
//      Also that BitsCopy uses 0 or 255 alpha values for selective copies.
//      Also BitsPolygon can selectively fill alpha values.
//      Also BitsPattern can optionally fill only point or line alpha values.
//      Sets bmInv to CINV so BitsCopy & BitsStretch use transparency.
//      Can optionally set ALPHAS0 bitmap flag to keep background alphas at 0.
//
//  Arguments:
//      errc BitsAlphaFill (int mode, BMAP* pmap, IBox* pbox,
//                          lint alpha, lint color)
//      mode    0 = Fill with alpha value only.
//              1 = Fill with alpha value only where bitmap has given color
//              2 = Fill with alpha value only where bitmap does not have color
//              3 = Fill with color values.
//              4 = Fill with color values where bitmap has given alpha
//              5 = Fill with color values where bitmap does not have alpha
//              6 = Fill with color values where bitmap alpha and 3D low-right
//              7 = Fade alpha values by multiplying by 0-255 (0-1.0) alpha.
//             11 = Fill with color values including the color alpha byte.
//            256 = Replace color values with alpha value.
//            257 = Restore to non-transparent if fully opaque (no alpha,color).
//            258 = Stop pmap->bmInv color being used for non-transparent pix.
//                  Plus also sets all alpha values for zero (for vector draw).
//            259 = Stop pmap->bmInv color being used for non-transparent pix.
//                  Same as 258 but does not also zero all alpha values.
//                  Instead sets color for all pixels with non-zero alpha value.
//            260 = Return non-zero ECTRUE if color is used at least once.
//                  Return zero if color does not exist in bitmap.
//            261 = Fill with alpha value only but only if not fully clear.
//            262 = Makes sure color is not used anywhere (ignores alphas).
//             +8 = Fill with alpha byte of color value and ignore alpha
//                  Must not add +8 to mode 11 (which is already 3+8).
//            +16 = Set ALPHAS0 flag in pmap->bmFlg bitmap flags.
//                  This keeps 0 alphas for most future drawing.
//            +32 = Only set pmap->bmInv to CINV and ignore other arguments.
//      pmap    Target bitmap.  Must be 32 bits per pixel.
//              For alpha below 255 sets bmInv to CINV to enable transparency.
//              For mode 8 always sets bmInv to CINV regardless of ther args.
//      pbox    Portion of pmap to be filled with alpha values.
//              Use NULL for the entire bitmap.
//      alpha   Alpha value.  Must be 0 to 255.
//              Not used for mode 11.
//      color   BGR color value (with alpha value for mode 11).
//              Only the alpha byte is used for mode +8 (not mode 0 without +4).
//              The blue, green and red bytes are used for modes 1 and 2.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsAlphaFill (int mode,BMAP* pmap,IBox* pbox,lint alpha,lint color) {
    BMAP bmap;
    LHUE* ppix;
    LHUE* prow;
    LHUE one, two;
    lint ox, oy, nx, ny, cnt, lmode, rowinc, nbytes;
    byte abyte;
    flt frac;
    errc e;

    if (pmap->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pmap, NULL);
        if (e) return(e);
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }
    if (mode & 16) pmap->bmFlg |= ALPHAS0;
    if (mode & 32) {
        pmap->bmInv = CINV;
        return(0);
    }

    if (pbox) {
        ox = pbox->xleft;
        nx = pbox->xright - ox;
        if (ox < 0) {
            nx += ox;
            ox = 0;
        }
        if ((ox + nx) > pmap->bmX) {
            nx -= (ox + nx) - pmap->bmX;
        }
        if (nx <= 0) return(0);
        oy = pbox->ytop;
        ny = pbox->ybottom - oy;
        if (oy < 0) {
            ny += oy;
            oy = 0;
        }
        if ((oy + ny) > pmap->bmY) {
            ny -= (oy + ny) - pmap->bmY;
        }
        if (ny <= 0) return(0);
    } else {
        ox = 0;
        nx = pmap->bmX;
        oy = 0;
        ny = pmap->bmY;
    }
    lmode = mode & 0x107;
    rowinc = pmap->bmRow;
    abyte = (byte) alpha;
    if (mode & 8) abyte = (byte) (color >> 24);
    if ((abyte < 255)&&(pmap->bmInv == 0)) {
        pmap->bmInv = CINV;                     //bitmap will have transparency
    }
    prow = ADDOFF(LHUE*, pmap->bmTop, ((oy * rowinc) + (ox << 2)));

    if (lmode == 3) {                           //fill with color and alpha
        if (nx == pmap->bmX) {
            nbytes = ny * rowinc;
            if (rowinc < 0) {
                prow = ADDOFF(LHUE*, prow, nbytes-rowinc);
                nbytes = -nbytes;
            }
            OSFILLD(prow, color, nbytes);
            return(0);
        } else {
            nbytes = nx << 2;
            do {
                OSFILLD(prow, color, nbytes);
                prow = ADDOFF(LHUE*, prow, rowinc);
            } while (--ny);
            return(0);
        }

    } else if (lmode == 7) {                    //fade alpha values
        if (BitsCPU() != 5) {

        frac = abyte * (1/255.0f);              //fraction to fade
        do {
             ppix = prow;
             cnt = nx;
             do {
                 ppix->b.balpha = (byte) ((flt)ppix->b.balpha * frac);
                 ppix += 1;
             } while (--cnt);
             prow  = ADDOFF(LHUE*, prow, rowinc);
         } while (--ny);
         return(0);

        } else {                                //fast MMX version
        #ifndef NOI86                           //---
        ASM {
        xor     eax,eax
        mov     al,abyte
        shl     eax,AMX_FFO
        movd    mm2,eax                         //mm2 = afrac as AMX_ONE = 1.0
        mov     edi,prow
        mov     ebx,rowinc
        add     edi,OBALPHA                     //point at alpha values
        mov     edx,ny
amrow0: mov     ecx,nx                          //come here for each row
        mov     esi,edi
        and     ecx,(~1)
        jz      amskp0

amcol0: mov     ah,[edi]                        //unrolled loop for speed
        xor     al,al
        shr     eax,1                           //0x7FFF for bigest alpha
        movd    mm0,eax
        pmulhw  mm0,mm2                         //multiply by afrac AMX_ONE
        mov     ah,[edi+4]
        xor     al,al
        shr     eax,1
        movd    mm1,eax
        pmulhw  mm1,mm2
        psrlw   mm0,AMX_RSF                     //result as 0xFF for 1.0
        movd    eax,mm0
        mov     [edi],al
        psrlw   mm1,AMX_RSF
        movd    eax,mm0
        mov     [edi+4],al
        add     edi,8
        sub     ecx,2
        jnz     amcol0

amskp0: mov     ecx,nx
        and     ecx,1
        jz      amdon0
        mov     ah,[edi]                        //do odd one at end
        xor     al,al
        shr     eax,1
        movd    mm0,eax
        pmulhw  mm0,mm2
        psrlw   mm0,AMX_RSF
        movd    eax,mm0
        mov     [edi],al
amdon0: mov     edi,esi
        add     edi,ebx                         //on to next row
        dec     edx
        jnz     amrow0
        emms
        }
        #endif                                  //---
        return(0);
        }

    } else if (lmode == 0) {                    //fill all alpha values
        #ifdef NOI86                            //-------
        do {
            ppix = prow;
            cnt = nx;
            do {
                ppix->b.balpha = abyte;
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        #else                                   //-------

        ASM {
        mov     edi,prow
        mov     ebx,rowinc
        add     edi,OBALPHA                     //point at alpha values
        mov     al,abyte
        mov     edx,ny
afrow0: mov     ecx,nx                          //come here for each row
        mov     esi,edi
        and     ecx,(~3)
        jz      afskp0
afcol0: mov     [edi],al                        //unrolled column loop for speed
        mov     [edi+4],al
        mov     [edi+8],al
        mov     [edi+12],al
        add     edi,16
        sub     ecx,4
        jnz     afcol0
afskp0: mov     ecx,nx
        and     ecx,3
        jz      afdon0
afend0: mov     [edi],al                        //do odd ones at the end
        add     edi,4
        dec     ecx
        jnz     afend0
afdon0: mov     edi,esi
        add     edi,ebx                         //on to next row
        dec     edx
        jnz     afrow0
        }
        #endif                                  //-------
        return(0);

    } else  if (lmode == 1) {                   //fill alphas for given color
        color = color & RGB_MASK;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if ((ppix->d & RGB_MASK) == color) {
                    ppix->b.balpha = abyte;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 2) {                    //fill alphas without color
        color = color & RGB_MASK;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if ((ppix->d & RGB_MASK) != color) {
                    ppix->b.balpha = abyte;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 4) {                    //fill colors for given alpha
        do {
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha == abyte) {
                    ppix->d = color;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 5) {                   //fill colors without alpha
        do {
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha != abyte) {
                    ppix->d = color;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 256) {                  //change color to alpha
        color = color & RGB_MASK;
        alpha = alpha & RGB_MASK;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if ((ppix->d & RGB_MASK) == color) {
                    ppix->d = (ppix->d & ALPHA_MASK) | alpha;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 257) {                  //get rid of transparency if can
        if (pmap->bmInv == 0) return(0);
        do {
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha < ABYTE_FULL) {
                    return(0);                  //need transparency?
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        pmap->bmInv = 0;                        //unneeded transparency
        return(0);

    } else if (lmode == 258) {                  //stop bmInv non-transparency
        one.d = pmap->bmInv & RGB_MASK;         //plus set all alphas to zero
        two.d = one.d;
        two.b.balpha = 255;                     //alpha color differs from bmInv
        two.b.bblue = (two.b.bblue) ? (two.b.bblue - 1) : 1;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha) {           //not transparent color?
                    if ((ppix->d & RGB_MASK) == one.d) {
                        ppix->d = two.d;        //change if need so not bmInv
                    }
                }
                ppix->b.balpha = 0;             //plus set all alphas to zero
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 259) {                  //stop bmInv non-transparency
        one.d = pmap->bmInv & RGB_MASK;         //plus set non-clears to color
        two.d = one.d;
        two.b.balpha = 255;                     //alpha color differs from bmInv
        two.b.bblue = (two.b.bblue) ? (two.b.bblue - 1) : 1;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha) {           //not transparent color?
                    if ((ppix->d & RGB_MASK) == one.d) {
                        ppix->d = two.d;        //change if need so not bmInv
                    }
                } else {
                    ppix->d = color;            //plus set non-clears to color
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 260) {                  //see if color is used in bitmap
        one.d = color & RGB_MASK;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if ((ppix->d & RGB_MASK) == one.d) {
                    return(ECTRUE);             //color used at least once?
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 261) {                  //change non-clear alphas
        if (abyte < 1) abyte = 1;               //don't let set totally clear
        do {                                    //so can always get opaque back
            ppix = prow;
            cnt = nx;
            do {
                if (ppix->b.balpha) {
                    ppix->b.balpha = abyte;
                }
                ppix += 1;                      //only set if not fully clear
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else if (lmode == 262) {                  //make sure color not used
        one.d = color & RGB_MASK;
        two.d = one.d;                          //substitute slightly different
        two.b.bblue = (two.b.bblue) ? (two.b.bblue - 1) : 1;
        do {
            ppix = prow;
            cnt = nx;
            do {
                if ((ppix->d & RGB_MASK) == one.d) {
                    ppix->b.bred = two.b.bred;  //change slightly if match
                    ppix->b.bgreen = two.b.bgreen;
                    ppix->b.bblue = two.b.bblue;
                }                               //but keep old alpha
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);

    } else {                                    //3D lower diagonal alpha match
        ox = nx;
        do {
            ox -= 1;
            ppix = prow + ox;
            cnt = nx - ox;
            do {
                if (ppix->b.balpha == abyte) {
                    ppix->d = color;
                }
                ppix += 1;
            } while (--cnt);
            prow  = ADDOFF(LHUE*, prow, rowinc);
        } while (--ny);
        return(0);
    }
}

//*************************************************************************
//  BitsAlphaKeep:
//      Saves and restores alpha information.
//      Only works for 32 bits/pixel bitmap.
//
//  Arguments:
//      errc BitsAlphaKeep (int mode, BMAP* pmap, IBox* pbox, BMAP* ptmp)
//      mode    1 = Make copy of pmap in ptmp.
//                  The ptmp need not be initialized.
//                  The returned ptmp must be latter passed to mode 2.
//              2 = Blend ptmp alpha information back into pmap.
//                  Multiples old ptmp alpha fraction with new pmap alpha.
//                  Should have previously used mode 1.
//                  The ptmp bitmap is freed on return.
//              4 = Copy ptmp alpha information back into pmap.
//                  Copies ptmp alpha values into pmap alphas.
//                  Should have previously used mode 1.
//                  The ptmp bitmap is freed on return.
//      pmap    Bitmap whose transparency must be preserved.
//      pbox    Portion of pmap for which transparency is preserved.
//              May be NULL for whole bitmap.
//      ptmp    BMAP structure for use by this routine.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsAlphaKeep (int mode, BMAP* pmap, IBox* pbox, BMAP* ptmp) {
    LHUE* prows;
    LHUE* ppixs;
    LHUE* prowd;
    LHUE* ppixd;
    lint rowinc, srcinc, cnt, nx, ny, ox, oy;
    int alpha;
    errc e;

    if (pmap->bmPix != 32) return(ECARGUMENT);
    if (pbox) {
        ox = pbox->xleft;
        nx = pbox->xright - ox;
        if (ox < 0) {
            nx += ox;
            ox = 0;
        }
        if ((ox + nx) > pmap->bmX) {
            nx -= (ox + nx) - pmap->bmX;
        }
        if (nx <= 0) return(0);
        oy = pbox->ytop;
        ny = pbox->ybottom - oy;
        if (oy < 0) {
            ny += oy;
            oy = 0;
        }
        if ((oy + ny) > pmap->bmY) {
            ny -= (oy + ny) - pmap->bmY;
        }
        if (ny <= 0) return(0);
    } else {
        ox = 0;
        nx = pmap->bmX;
        oy = 0;
        ny = pmap->bmY;
    }

    if (mode & 1) {
        e = BitsMake(ptmp, 32, nx, ny, NULL);
        if (e) return(e);
        e = BitsCopy(4, ptmp, NULL, pmap, pbox);
        if (e) BitsFree(ptmp);
        return(e);

    } else if (mode & 4) {                      //copy back?
        if (ptmp->bmPtr == NULL) return(ECNOINIT);
        if ((ptmp->bmX != nx)||(ptmp->bmY != ny)) {
            return(ECBADCODE);                  //should not happen
        }
        rowinc = pmap->bmRow;
        srcinc = ptmp->bmRow;
        prows = ADDOFF(LHUE*, ptmp->bmTop, 0);
        prowd = ADDOFF(LHUE*, pmap->bmTop, ((oy * rowinc) + (ox << 2)));

        do {
            ppixs = prows;
            ppixd = prowd;
            cnt = nx;
            do {
                ppixd->b.balpha = ppixs->b.balpha;
                ppixs += 1;
                ppixd += 1;
            } while (--cnt);
            prows = ADDOFF(LHUE*, prows, srcinc);
            prowd = ADDOFF(LHUE*, prowd, rowinc);
        } while (--ny);
        BitsFree(ptmp);
        return(0);

    } else {                                    //blend back (mode 2)
        if (ptmp->bmPtr == NULL) return(ECNOINIT);
        if ((ptmp->bmX != nx)||(ptmp->bmY != ny)) {
            return(ECBADCODE);                  //should not happen
        }
        rowinc = pmap->bmRow;
        srcinc = ptmp->bmRow;
        prows = ADDOFF(LHUE*, ptmp->bmTop, 0);
        prowd = ADDOFF(LHUE*, pmap->bmTop, ((oy * rowinc) + (ox << 2)));

        do {
            ppixs = prows;
            ppixd = prowd;
            cnt = nx;
            do {
                if (ppixd->b.balpha >= ABYTE_FULL) {
                    ppixd->b.balpha = ppixs->b.balpha;
                } else if (ppixs->b.balpha < ABYTE_FULL) {
                    alpha = (int)ppixd->b.balpha * (int)ppixs->b.balpha;
                    alpha = alpha >> 8;
                    ppixd->b.balpha = (byte)alpha;
                }
                ppixs += 1;
                ppixd += 1;
            } while (--cnt);
            prows = ADDOFF(LHUE*, prows, srcinc);
            prowd = ADDOFF(LHUE*, prowd, rowinc);
        } while (--ny);
        BitsFree(ptmp);
        return(0);
    }
}

//*************************************************************************
//  BitsAlphaDup:
//      Copies only the alpha values from one bitmap to another.
//      Can be used to "cut out" a portion of a bitmap with non-zero alphas.
//
//  Arguments:
//      errc BitsAlphaDup (int mode,BMAP* pdes,IPt* pto,BMAP* psrc,IBox* pfrom)
//      mode    Reserved for future use and must be zero.
//      pdes    Destination bitmap.  Must be 32 bits per pixel.
//              If bmPtr is NULL then ignores pto and pfrom.
//      pto     Upper-left corner of destination rectangle.
//              Use NULL for upper-left corner of whole pdes bitmap (0,0).
//      psrc    Source bitmap.
//              Must be 1, 8, or 32 bits per pixel.
//      pfrom   Target rectangle within source bitmap.
//              Use NULL for the entire source bitmap (and clips right/bottom).
//              This must be wholely within the source bitmap.
//              The destination rectangle always has the same width and height.
//
//  Returns:
//      Non-zero code if error
//*************************************************************************

csproc BitsAlphaDup (int mode,BMAP* pdes,IPt* pto,BMAP* psrc,IBox* pfrom) {
    byte* prows;
    byte* prowd;
    LHUE* ppal;
    LHUE one, zero;
    lint xleft, ytop, fromx, fromy, nx, ny;
    lint srow, drow, spix, val, cnt;
    LHUE* phued;
    byte* pbyte;
    #ifdef NOI86    //-------
    LHUE* phues;
    #endif          //-------

    spix = psrc->bmPix;
    if ((spix != 32)&&(spix != 8)&&(spix != 1)) return(ECCANNOT);
    if (pdes->bmPix != 32) return(ECCANNOT);

    if (pfrom) {                                //get source upper-left corner
        fromx = pfrom->xleft;
        fromy = pfrom->ytop;
        nx = pfrom->xright - fromx;
        ny = pfrom->ybottom - fromy;
        if (  (nx <= 0)                         //check for invalid source area
            ||(ny <= 0)
            ||(fromx < 0)
            ||(fromy < 0)
            ||(pfrom->xright > psrc->bmX)
            ||(pfrom->ybottom > psrc->bmY)  ) {
            return(ECARGUMENT);
        }
    } else {
        fromx = 0;
        fromy = 0;
        nx = psrc->bmX;
        ny = psrc->bmY;
    }

    if (pto) {                                  //get destination rectangle
        xleft = pto->ptx;
        ytop = pto->pty;
    } else {                                    //copy over entire destination?
        xleft = 0;
        ytop = 0;
    }

    if (xleft < 0) {                            //clip left if needed
        nx += xleft;
        if (nx <= 0) return(0);
        fromx -= xleft;
        xleft = 0;
    }
    if ((xleft + nx) > pdes->bmX) {             //clip right if needed
        nx = pdes->bmX - xleft;
        if (nx <= 0) return(0);
    }
    if (ytop < 0) {                             //clip top if needed
        ny += ytop;
        if (ny <= 0) return(0);
        fromy -= ytop;
        ytop = 0;
    }
    if ((ytop + ny) > pdes->bmY) {              //clip bottom if needed
        ny = pdes->bmY - ytop;
        if (ny <= 0) return(0);
    }

    BitsAlpha(psrc, ISALPHA);                   //set source alphas
    srow = psrc->bmRow;
    drow = pdes->bmRow;
    prows = psrc->bmTop + (fromy * srow);
    prowd = pdes->bmTop + (ytop * drow) + (xleft << 2);

    if (spix == 8) {                            //8-bit source
        prows += fromx;
        ppal = (LHUE*) psrc->bmPal;
        if (ppal == NULL) return(ECNULL);
        do {
            #ifdef NOI86    //-------
            pbyte = prows;
            phued = (LHUE*) prowd;
            cnt = nx;
            do {
                phued->b.balpha = ppal[*pbyte].b.balpha;
                pbyte += 1;
                phued += 1;
            } while (--cnt);
            #else           //-------
            ASM {
            mov     ecx,nx
            mov     ebx,ppal
            mov     esi,prows
            mov     edi,prowd
            xor     eax,eax
            xor     edx,edx
            add     ebx,OBALPHA
            add     edi,OBALPHA
            and     ecx,~1
            jz      dup8ed

dup8lp:     mov     al,[esi]
            mov     dl,[esi+1]
            add     esi,2
            mov     al,[ebx+(eax*4)]
            mov     dl,[ebx+(edx*4)]
            mov     [edi],al
            mov     [edi+4],dl
            add     edi,8
            sub     ecx,2
            jnz     dup8lp

dup8ed:     mov     ecx,nx
            and     ecx,1
            jz      dup8dn
            mov     al,[esi]
            mov     al,[ebx+(eax*4)]
            mov     [edi],al
dup8dn:
            }
            #endif          //-------
            prows += srow;
            prowd += drow;
        } while (--ny);

    } else if (spix == 32) {                    //32-bit source
        prows += fromx << 2;
        do {
            #ifdef NOI86    //-------
            phues = (LHUE*) prows;
            phued = (LHUE*) prowd;
            cnt = nx;
            do {
                phued->b.balpha = phues->b.balpha;
                phues += 1;
                phued += 1;
            } while (--cnt);
            #else           //-------
            ASM {
            mov     ecx,nx
            mov     esi,prows
            mov     edi,prowd
            add     esi,OBALPHA
            add     edi,OBALPHA
            and     ecx,~1
            jz      dupded

dupdlp:     mov     al,[esi]
            mov     dl,[esi+4]
            add     esi,8
            mov     [edi],al
            mov     [edi+4],dl
            add     edi,8
            sub     ecx,2
            jnz     dupdlp

dupded:     mov     ecx,nx
            and     ecx,1
            jz      dupddn
            mov     al,[esi]
            mov     [edi],al
dupddn:
            }
            #endif          //-------
            prows += srow;
            prowd += drow;
        } while (--ny);

    } else {                                    //1-bit source
        ppal = (LHUE*) psrc->bmPal;
        zero.d = ppal[0].d;
        one.d  = ppal[1].d;
        do {
            cnt = nx;
            pbyte = prows + (fromx >> 3);
            val = 1 << (7 - (fromx & 7));
            phued = (LHUE*) prowd;

            do {
                phued->b.balpha = (*pbyte & val) ? one.b.balpha : zero.b.balpha;
                phued += 1;
                val = val >> 1;
                if (val == 0) {
                    val = 0x80;
                    pbyte += 1;
                }
            } while (--cnt);
            prows += srow;
            prowd += drow;
        } while (--ny);
    }
    return(0);
}

//*************************************************************************
//  BitsAlphaSet:
//      Sets or changes the alpha value for a given color.
//      Can be used to fade an object in or out.
//      Can also be used to set alpha values for all non-transparent colors.
//      Can also be used to switch a color value including its alpha.
//      Intended for use with palette bitmaps with 8 bits or less.
//      For palettes, just switches the palette color alpha so is very fast.
//      Works for 32 bits but is slow and not recommended.
//
//  Arguments:
//      errc BitsAlphaSet (int mode, BMAP* pmap, lint color, lint alpha)
//      mode    0 = Set alpha for given color.
//              1 = Set whole BGR alpha value as the new color.
//              2 = Set alpha values for all but given color.
//      pmap    Target bitmap.
//              Only mode 0 works for 32 bits and it is slow.
//      color   Color in pmap whose alpha transparency is to be set.
//              This can be a CSTD, CRGB or CBGR color value.
//              This can also be a CERR+index palette index for pmap.
//      alpha   New alpha value (0-255) for opaqueness of the color.
//              For mode 1 this is an entire BGR color and alpha value.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsAlphaSet (int mode, BMAP* pmap, lint color, lint alpha) {
    LHUE* phue;
    lint idx, off, use;
    errc e;

    if (pmap->bmPix <= 8) {
        idx = BitsColor(pmap, color);           //palette index to color
        BitsAlpha(pmap, ISALPHA);               //alpha values in palette
        if (mode == 0) {
            pmap->bmPal[idx].balpha = (byte) alpha;

        } else if (mode == 1) {
            phue = (LHUE*) &pmap->bmPal[idx];
            phue->d = alpha;

        } else if (mode == 2) {
            use = pmap->bmUse;
            phue = (LHUE*) &pmap->bmPal[0];
            if (phue == NULL) return(ECNULL);
            off = 0;
            while (use--) {
                if (off != idx) phue->b.balpha = (byte) alpha;
                phue += 1;
                off += 1;
            }
        }
        return(0);
    }
    if (mode == 0) {
        e = BitsAlphaFill(1, pmap, NULL, alpha, color);
    } else if (mode == 2) {
        e = BitsAlphaFill(2, pmap, NULL, alpha, color);
    } else {
        e = ECCANNOT;
    }
    return(e);
}

//*************************************************************************
//  BitsBytes:
//      Gets a bitmap total byte size and its bytes per row.
//
//  Arguments:
//      errc BitsBytes (lint pix, lint nx, lint ny, lint* prow, lint* psize)
//      pix     Number of bits per pixel (1,4,8,16,24,32,64,128).
//              Currently supports only 1,4,8,32 and returns errors for 16,24.
//              Also supports 64 (used for 4 word values for pixel).
//              Also supports 128 (used for 4 flt values for pixel).
//      nx      Pixel width of bitmap.
//      ny      Pixel height of bitmap.
//              If negative, uses absolute value.
//      prow    Where byte size of each row is returned (multiple of 4).
//              This is always a positive number.
//              It should be negatated by the caller for bottom-to-top bitmaps.
//      psize   Where byte size of whole bitmap is returned.
//
//  Returns:
//      Returns ECCANNOT if unsupported pix value.
//      Returns ECNOPTS if zero width or height.
//*************************************************************************

csproc BitsBytes (lint pix, lint nx, lint ny, lint* prow, lint* psize) {
    lint crow, csize;

    if (ny < 0) ny = -ny;

    if (pix == 32) {
        crow= nx << 2;
    } else if (pix == 24) {
        crow = ((nx + nx + nx) + 3) & (~3L);
    } else if (pix == 16) {
        crow = ((nx + nx) + 3) & (~3L);
    } else if (pix == 8) {
        crow = (nx + 3) & (~3L);
    } else if (pix == 4) {
        crow = (((nx+1)>>1) + 3) & (~3L);
    } else if (pix == 1) {
        crow = (((nx+7)>>3) + 3) & (~3L);
    } else if (pix > 32) {
        crow = nx * (pix >> 3);
    } else {
        *prow = 0;
        *psize = 0;
        return(ECCANNOT);
    }
    *prow = crow;
    csize = crow * ny;
    *psize = csize;
    if (csize) return(0);
    return(ECNOPTS);
}

//*************************************************************************
//  BitsCPU:
//      Determines the I86 CPU type and whether MMX instructions are available.
//      See also BitsCPU2 to differentiate between Pentium and Pentinum II.
//
//  Arguments:
//      errc BitsCPU ()
//
//  Returns:
//      1 = Non-Intel processor (cannot use I86 assembly).
//      2 = 386
//      3 = 486
//      4 = Pentium or better without MMX instructions.
//      5 = Pentium with MMX instructions.
//*************************************************************************

csproc BitsCPU () {
    #ifdef NOI86//-------
    return(1);
    #else       //-------
    errc rval;

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

        #ifndef NOMMX   //-------
        mov     eax,1                   ;request feature flags
        push    ecx
        CPUID                           ;0Fh, 0A2h is CPUID instr
        pop     ecx
        test    edx,0x00800000          ;bit 23 set?
        jz      havcpu                  ;do not have MMX instructions?
        inc     ecx
        #endif          //-------
havcpu: mov     rval,ecx
        pop     ebx
    }
    return(rval);
    #endif      //-------
}

//*************************************************************************
//  BitsCPU2:
//      Determines the I86 CPU type.
//      See also BitsCPU for old version.
//
//  Arguments:
//      errc BitsCPU2 ()
//
//  Returns:
//      1 = Non-Intel processor (cannot use I86 assembly).
//      2 = 386
//      3 = 486
//      4 = Pentium or better without MMX instructions.
//      5 = Pentium or Pentium Pro with MMX instructions.
//      6 = Pentium II or better (with MMX instructions).
//      Beware that in the future, may return greater than 6!
//*************************************************************************

csproc BitsCPU2 () {
    #ifdef NOI86//-------
    return(1);
    #else       //-------
    errc rval;

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
        mov     ecx,2                   ;ECX=2
        test    edx,040000h
        jz      havcpu                  ;if AC bit cannot be changed, CPU=386
        inc     ecx                     ;ECX=3
        test    edx,200000h
        jz      havcpu                  ;if ID bit cannot be changed, CPU=486
        inc     ecx                     ;ECX=4

        #ifndef NOMMX   //-------
        mov     eax,1                   ;request feature flags
        push    ecx
        CPUID                           ;0Fh, 0A2h is CPUID instr
        pop     ecx
        test    edx,0x00800000          ;bit 23 set?
        jz      havcpu                  ;do not have MMX instructions?
        inc     ecx                     ;ECX=5
        and     eax,0x00000FF0          ;bits 4-7 model, bits 8-11=family
        cmp     eax,0x00000600
        jl      havcpu                  ;not Pentium Pro or Pentium II?
        jg      havtwo                  ;better than Pentium II?
        and     eax,0x000000F0          ;bits 4-7 model, bits 8-11=family
        cmp     eax,0x00000030
        jl      havcpu                  ;old Pentium Pro model?
havtwo: inc     ecx                     ;ECX=6
        #endif          //-------
havcpu: mov     rval,ecx
        pop     ebx
    }
    return(rval);
    #endif      //-------
}

//*************************************************************************
//  BitsHorz:
//      Draws a single straight mostly-horizontal line segment.
//      Can draw to either 8 or 32 bits/pixel bitmaps.
//      Can skip points or add extra pixels at end of segment.
//
//  Arguments:
//      void BitsHorz (BMAP* pmap, lint ox, lint oy, lint nx, lint ny,
//                     lint skipbeg, lint skipend, lint value, lint omit)
//      pmap    Pointer to bitmap.
//              Must use either 8 or 32 bits/pixel.
//      ox, oy  Beginning pixel for the line segment.
//              Must be to the top of the (ox+nx, oy+ny) point.
//              The end points may be outside of the bitmap limits.
//      nx, ny  Delta X and Y pixels to the ending point.
//              The nx must be zero or positive.
//              The nx must be greater than or equal to ABS(ny).
//      skipbeg Number of pixels along the X dimension to skip
//              at the beginning of the line segment.
//              For a normal line, this is zero.
//              May be negative to add extra vertical pixels before beginning.
//              To produce 3D-like wide lines, this can be non-zero.
//      skipbeg Number of pixels along the X dimension to skip
//              at the ending of the line segment.
//              For a normal line, this is zero.
//              May be negative to add extra vertical pixels after ending.
//              To produce 3D-like wide lines, this can be non-zero.
//      color   Pixel value to write in bitmap for line.
//              This must be an BGRA color value for 32 bits/pixel.
//              This must be a palette index value for 8 bits/pixel.
//      omit    Normally zero.  If non-zero then omits line where writing over
//              this 32-bit pixel color.  Used for wide line outlines.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsHorz (BMAP* pmap, lint ox, lint oy, lint nx, lint ny,
                 lint skipbeg, lint skipend, lint value, lint omit) {
    lint* ppix;
    byte* pbyte;
    lint ex, ey, limx, limy, cnt;
    lint yadd, rowadd;
    dword frac, part;

    ex = ox + nx;
    ey = oy + ny;

    limx = pmap->bmX;
    limy = pmap->bmY;
    if (ny > 0) {                               //do nothing if entirly clipped
        if (  ((ex < 0)||(ox >= limx))
            ||((oy >= limy)||(ey < 0))  )
            return;
    } else {
        if (  ((ex < 0)||(ox >= limx))
            ||((oy < 0)||(ey >= limy))  )
            return;
    }

    cnt = nx + 1;
    if (skipbeg) {                              //skipped or extra at beginning?
        if (skipbeg < 0) {                      //extra at beginning?
            BitsHorz(pmap, ox+skipbeg, oy, -skipbeg, 0, 0, 0, value, 0);
            skipbeg = 0;
        } else {
            cnt -= skipbeg;
        }
    }
    if (skipend) {                              //skipped or extra at ending?
        if (skipend < 0) {                      //extra at ending?
            BitsHorz(pmap, ex, ey, -skipend, 0, 0, 0, value, 0);
            skipend = 0;
        } else {
            cnt -= skipend;
        }
    }

    if ((ox+skipbeg) < 0) {                     //skip clipped points at left?
        cnt += ox+skipbeg;
        skipbeg = -ox;
    }
    if ((ex-skipend) >= limx) {                 //skip clipped points at right?
        cnt -= (ex-skipend) - (limx-1);
    }
    if (cnt <= 0) return;                       //nothing to do?
    ppix = ADDOFF(lint*, pmap->bmTop, oy * pmap->bmRow);

// Handle a purely horizontal line.

    if (ny == 0) {
        if (pmap->bmPix == 8) {                 //across for 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ox+skipbeg);
            OSFILLB(pbyte, value, cnt);         //replicate bytes across line
            return;
        } else if (omit == 0) {                 //across for 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, ((ox+skipbeg)<<2));
            cnt = cnt << 2;
            OSFILLD(ppix, value, cnt);          //replicate BGR's across line
            return;
        } else {                                //handle omit where inside wide
            ppix = ADDOFF(lint*, ppix, ((ox+skipbeg)<<2));
            #ifdef NOI86    //-------
            do {
                if (*ppix != omit) *ppix = value;
                ppix += 1;
            } while (--cnt);
            return;
            #else           //-------
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edi,ppix
bhholp:     cmp     [edi],ebx
            je      bhhomt
            mov     [edi],eax
bhhomt:     add     edi,4
            dec     ecx
            jnz     bhholp
            }
            return;
            #endif          //-------
        }

// Decide whether the line is decending or acending.
// Then get fraction to increment Y with every X pixel.

    } else if (ny > 0) {
        rowadd = pmap->bmRow;
        yadd = 1;
    } else {
        ny = -ny;
        rowadd = -(pmap->bmRow);
        yadd = -1;
    }
    IFRACTION(frac, ny, nx, bh);                //frac = ny/nx
    #ifdef NOI86    //-------
    frac = frac >> 1;                           //extra overflow bit for C code
    #endif          //-------

// Skip any beginning pixels.
// The part variable is the portion of a pixel fraction (0 - 0xFFFFFFFF).
// If nx equal ny then we have a diagonal whose frac is slightly incorrectly
// represented as 0xFFFFFFFF.  We compensate for this by making the part
// a big positive number so that the 0xFFFFFFFF works just like the correct
// 0x100000000 frac that is too big to represent by overflowing 1 each pixel.
// Actually the diagonal case happens always to be handled by BitsVert
// but we correct for it here too just to be safe.

    part = frac >> 1;                           //glitch in middle not at end
    if (skipbeg) {
        #ifdef NOI86    //-------
        ox += skipbeg;
        do {
            part += frac;
            if (part & ESIGN) {
                part &= EMAXI;
                oy += yadd;
                ppix = ADDOFF(lint*, ppix, rowadd);
            }
        } while (--skipbeg);

        #else           //-------
        ASM {
        mov     eax,ox
        mov     ecx,skipbeg
        add     eax,ecx
        mov     ebx,frac
        mov     ox,eax
        mov     edx,part
        mov     esi,ppix
        mov     edi,rowadd
        mov     eax,oy
        push    ebp
        mov     ebp,yadd
bhslop: add     edx,ebx
        jc      bhsrow
        dec     ecx
        jnz     bhslop
        jmp     bhsdon

bhsrow: add     esi,edi
        add     eax,ebp
        dec     ecx
        jnz     bhslop

bhsdon: pop     ebp
        mov     oy,eax
        mov     part,edx
        mov     ppix,esi
        }
        #endif
    }

// Draw the line without clipping (when it is entirely inside the bitmap).

    if (((dword)oy < (dword)limy)&&((dword)ey < (dword)limy)) {
        if (pmap->bmPix == 8) {                 //handle 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ox);

            #ifdef NOI86//-------
            do {
                *pbyte++ = (byte)value;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    pbyte = ADDOFF(byte*, pbyte, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,pbyte
bhnblp:     mov     [edi],al                    ;come here for each pixel
            inc     edi                         ;advance to next X
            add     edx,ebx
            jc      bhnbov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhnblp
            jmp     bhnbdn

bhnbov:     add     edi,esi                     ;advance to next Y
            dec     ecx
            jnz     bhnblp
bhnbdn:
            }
            return;
            #endif      //-------

        } else if (omit == 0) {                 //handle 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                *ppix++ = value;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ppix = ADDOFF(lint*, ppix, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,ppix
bhndlp:     mov     [edi],eax                   ;come here for each pixel
            add     edi,4                       ;advance to next X
            add     edx,ebx
            jc      bhndov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhndlp
            jmp     bhnddn

bhndov:     add     edi,esi                     ;advance to next Y
            dec     ecx
            jnz     bhndlp
bhnddn:
            }
            return;
            #endif      //-------

        } else {                                //handle omit where inside wide
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if (*ppix != omit) *ppix = value;
                ppix += 1;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ppix = ADDOFF(lint*, ppix, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,ppix
bhnolp:     cmp     [edi],ebx                   ;come here for each pixel
            je      bhnomt                      ;skip if over omit color
            mov     [edi],eax                   ;install the pixel
bhnomt:     add     edi,4                       ;advance to next X
            add     edx,frac
            jc      bhnoov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhnolp
            jmp     bhnodn

bhnoov:     add     edi,esi                     ;advance to next Y
            dec     ecx
            jnz     bhnolp
bhnodn:
            }
            return;
            #endif      //-------
        }

// Draw the line with clipping.

    } else {
        if (pmap->bmPix == 8) {                 //handle 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ox);

            #ifdef NOI86//-------
            do {
                if ((dword)oy < (dword)limy) {
                    *pbyte = (byte)value;
                }
                pbyte += 1;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    oy += yadd;
                    pbyte = ADDOFF(byte*, pbyte, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,oy
            mov     edi,pbyte
bhcblp:     cmp     esi,limy
            jae     bhcbcl                      ;Y out of bounds => clip pixel?
            mov     [edi],al                    ;come here for each pixel
bhcbcl:     inc     edi                         ;advance to next X
            add     edx,ebx
            jc      bhcbov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhcblp
            jmp     bhcbdn

bhcbov:     add     edi,rowadd                  ;advance to next Y
            add     esi,yadd
            dec     ecx
            jnz     bhcblp
bhcbdn:
            }
            return;
            #endif      //-------

        } else if (omit == 0) {                 //handle 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if ((dword)oy < (dword)limy) {
                    *ppix = value;
                }
                ppix += 1;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    oy += yadd;
                    ppix = ADDOFF(lint*, ppix, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,oy
            mov     edi,ppix
bhcdlp:     cmp     esi,limy
            jae     bhcdcl                      ;Y out of bounds => clip pixel?
            mov     [edi],eax                   ;come here for each pixel
bhcdcl:     add     edi,4                       ;advance to next X
            add     edx,ebx
            jc      bhcdov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhcdlp
            jmp     bhcddn

bhcdov:     add     edi,rowadd                  ;advance to next Y
            add     esi,yadd
            dec     ecx
            jnz     bhcdlp
bhcddn:
            }
            return;
            #endif      //-------

        } else {                                //handle omit where inside wide
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if ((dword)oy < (dword)limy) {
                    if (*ppix != omit) *ppix = value;
                }
                ppix += 1;
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    oy += yadd;
                    ppix = ADDOFF(lint*, ppix, rowadd);
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edx,part
            mov     esi,oy
            mov     edi,ppix
bhcolp:     cmp     esi,limy
            jae     bhcocl                      ;Y out of bounds => clip pixel?
            cmp     [edi],ebx
            je      bhcocl                      ;omit if same as omit color
            mov     [edi],eax                   ;come here for each pixel
bhcocl:     add     edi,4                       ;advance to next X
            add     edx,frac
            jc      bhcoov                      ;advance to next whole Y row?
            dec     ecx
            jnz     bhcolp
            jmp     bhcodn

bhcoov:     add     edi,rowadd                  ;advance to next Y
            add     esi,yadd
            dec     ecx
            jnz     bhcolp
bhcodn:
            }
            return;
            #endif      //-------
        }
    }
}

//*************************************************************************
//  BitsVert:
//      Draws a single straight mostly-vertical line segment.
//      Can draw to either 8 or 32 bits/pixel bitmaps.
//      Can skip points or add extra pixels at end of segment.
//
//  Arguments:
//      void BitsVert (BMAP* pmap, lint ox, lint oy, lint nx, lint ny,
//                     lint skipbeg, lint skipend, lint value, lint omit)
//      pmap    Pointer to bitmap.
//              Must use either 8 or 32 bits/pixel.
//      ox, oy  Beginning pixel for the line segment.
//              Must be to the top of the (ox+nx, oy+ny) point.
//              The end points may be outside of the bitmap limits.
//      nx, ny  Delta X and Y pixels to the ending point.
//              The ny must be zero or positive.
//              The ny must be greater than or equal to ABS(nx).
//      skipbeg Number of pixels along the Y dimension to skip
//              at the beginning of the line segment.
//              For a normal line, this is zero.
//              May be negative to add extra vertical pixels before beginning.
//              To produce 3D-like wide lines, this can be non-zero.
//      skipbeg Number of pixels along the Y dimension to skip
//              at the ending of the line segment.
//              For a normal line, this is zero.
//              May be negative to add extra vertical pixels after ending.
//              To produce 3D-like wide lines, this can be non-zero.
//      color   Pixel value to write in bitmap for line.
//              This must be an BGRA color value for 32 bits/pixel.
//              This must be a palette index value for 8 bits/pixel.
//      omit    Normally zero.  If non-zero then omits line where writing over
//              this 32-bit pixel color.  Used for wide line outlines.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsVert (BMAP* pmap, lint ox, lint oy, lint nx, lint ny,
                 lint skipbeg, lint skipend, lint value, lint omit) {
    lint* ppix;
    byte* pbyte;
    lint ex, ey, limx, limy, cnt;
    lint xadd1, rowadd;
    dword frac, part;
    #ifndef NOI86   //-------
    lint xadd4;
    #endif          //-------

    ex = ox + nx;
    ey = oy + ny;

    limx = pmap->bmX;
    limy = pmap->bmY;
    if (nx > 0) {                               //do nothing if entirly clipped
        if (  ((ey < 0)||(oy >= limy))
            ||((ox >= limx)||(ex < 0))  )
            return;
    } else {
        if (  ((ey < 0)||(oy >= limy))
            ||((ox < 0)||(ex >= limx))  )
            return;
    }

    cnt = ny + 1;
    if (skipbeg) {                              //skipped or extra at beginning?
        if (skipbeg < 0) {                      //extra at beginning?
            BitsVert(pmap, ox, oy+skipbeg, 0, -skipbeg, 0, 0, value, 0);
            skipbeg = 0;
        } else {
            cnt -= skipbeg;
        }
    }
    if (skipend) {                              //skipped or extra at ending?
        if (skipend < 0) {                      //extra at ending?
            BitsVert(pmap, ex, ey, 0, -skipend, 0, 0, value, 0);
            skipend = 0;
        } else {
            cnt -= skipend;
        }
    }

    if ((oy+skipbeg) < 0) {                     //skip clipped points at top?
        cnt += oy+skipbeg;
        skipbeg = -oy;
    }
    if ((ey-skipend) >= limy) {                 //skip clipped points at bottom?
        cnt -= (ey-skipend) - (limy-1);
    }
    if (cnt <= 0) return;                       //nothing to do?

    rowadd = pmap->bmRow;
    ppix = ADDOFF(lint*, pmap->bmTop, oy * rowadd);

// Handle a purely vertical line.

    if (nx == 0) {
        if (pmap->bmPix == 8) {                 //down for 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ((skipbeg*rowadd)+ox));

            #ifdef NOI86//-------
            do {
                *pbyte = (byte)value;
                pbyte = ADDOFF(byte*, pbyte, rowadd);
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ecx,cnt
            mov     edx,rowadd
            mov     edi,pbyte
bvvblp:     mov     [edi],al
            add     edi,edx
            dec     ecx
            jnz     bvvblp
            }
            return;
            #endif      //-------

        } else if (omit == 0) {                 //down for 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, ((skipbeg*rowadd)+(ox<<2)));

            #ifdef NOI86//-------
            do {
                *ppix = value;
                ppix = ADDOFF(lint*, ppix, rowadd);
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ecx,cnt
            mov     edx,rowadd
            mov     edi,ppix
bvvdlp:     mov     [edi],eax
            add     edi,edx
            dec     ecx
            jnz     bvvdlp
            }
            return;
            #endif      //-------

        } else {                                //down if skip over omit color
            ppix = ADDOFF(lint*, ppix, ((skipbeg*rowadd)+(ox<<2)));

            #ifdef NOI86//-------
            do {
                if (*ppix != omit) *ppix = value;
                ppix = ADDOFF(lint*, ppix, rowadd);
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edx,rowadd
            mov     edi,ppix
bvvolp:     cmp     [edi],ebx
            je      bvvomt
            mov     [edi],eax
bvvomt:     add     edi,edx
            dec     ecx
            jnz     bvvolp
            }
            return;
            #endif      //-------
        }

// Decide whether the line ramps to right or left.
// Then get fraction to increment Y with every X pixel.

    } else if (nx > 0) {
        xadd1 = 1;
    } else {
        nx = -nx;
        xadd1 = -1;
    }
    IFRACTION(frac, nx, ny, bv);                //frac = ny/nx
    #ifdef NOI86    //-------
    frac = frac >> 1;                           //extra overflow bit for C code
    #endif          //-------

// Skip any beginning pixels.
// The part variable is the portion of a pixel fraction (0 - 0xFFFFFFFF).
// If nx equal ny then we have a diagonal whose frac is slightly incorrectly
// represented as 0xFFFFFFFF.  We compensate for this by making the part
// a big positive number so that the 0xFFFFFFFF works just like the correct
// 0x100000000 frac that is too big to represent by overflowing 1 each pixel.

    part = frac >> 1;                           //glitch in middle not at end
    if (skipbeg) {
        #ifdef NOI86    //-------
        oy += skipbeg;
        do {
            part += frac;
            ppix = ADDOFF(lint*, ppix, rowadd);
            if (part & ESIGN) {
                part &= EMAXI;
                ox += xadd1;
            }
        } while (--skipbeg);

        #else           //-------
        ASM {
        mov     eax,oy
        mov     ecx,skipbeg
        add     eax,ecx
        mov     ebx,frac
        mov     oy,eax
        mov     edx,part
        mov     esi,ppix
        mov     edi,rowadd
        mov     eax,ox
        push    ebp
        mov     ebp,xadd1
bvslop: add     esi,edi
        add     edx,ebx
        jc      bvsrow
        dec     ecx
        jnz     bvslop
        jmp     bvsdon

bvsrow: add     eax,ebp
        dec     ecx
        jnz     bvslop

bvsdon: pop     ebp
        mov     ox,eax
        mov     part,edx
        mov     ppix,esi
        }
        #endif
    }

// Draw the line without clipping (when it is entirely inside the bitmap).

    if (((dword)ox < (dword)limx)&&((dword)ex < (dword)limx)) {
        if (pmap->bmPix == 8) {                 //handle 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ox);

            #ifdef NOI86//-------
            do {
                *pbyte = (byte)value;
                pbyte = ADDOFF(byte*, pbyte, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    pbyte += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,pbyte
            push    ebp
            mov     ebp,xadd1
bvnblp:     mov     [edi],al                    ;come here for each pixel
            add     edi,esi                     ;advance to next Y
            add     edx,ebx
            jc      bvnbov                      ;advance to next whole X column?
            dec     ecx
            jnz     bvnblp
            jmp     bvnbdn

bvnbov:     add     edi,ebp                     ;advance to next X
            dec     ecx
            jnz     bvnblp

bvnbdn:     pop     ebp
            }
            return;
            #endif      //-------

        } else if (omit == 0) {                 //handle 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                *ppix = value;
                ppix = ADDOFF(lint*, ppix, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ppix += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,ppix
            push    ebp
            mov     ebp,xadd1
            shl     ebp,2
bvndlp:     mov     [edi],eax                   ;come here for each pixel
            add     edi,esi                     ;advance to next Y
            add     edx,ebx
            jc      bvndov                      ;advance to next whole X column>?
            dec     ecx
            jnz     bvndlp
            jmp     bvnddn

bvndov:     add     edi,ebp                     ;advance to next X
            dec     ecx
            jnz     bvndlp

bvnddn:     pop     ebp
            }
            return;
            #endif      //-------

        } else {                                //handle omit where inside wide
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if (*ppix != omit) *ppix = value;
                ppix = ADDOFF(lint*, ppix, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ppix += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edx,part
            mov     esi,rowadd
            mov     edi,ppix
            mov     esi,xadd1
            shl     esi,2
bvnolp:     cmp     [edi],ebx                   ;come here for each pixel
            je      bvnomt                      ;skip if same as omit color
            mov     [edi],eax                   ;install the pixel
bvnomt:     add     edi,rowadd                  ;advance to next Y
            add     edx,frac
            jc      bvnoov                      ;advance to next whole X column>?
            dec     ecx
            jnz     bvnolp
            jmp     bvnodn

bvnoov:     add     edi,esi                     ;advance to next X
            dec     ecx
            jnz     bvnolp

bvnodn:
            }
            return;
            #endif      //-------
        }

// Draw the line with clipping.

    } else {
        if (pmap->bmPix == 8) {                 //handle 8 bits/pixel
            pbyte = ADDOFF(byte*, ppix, ox);

            #ifdef NOI86//-------
            do {
                if ((dword)ox < (dword)limx) {
                    *pbyte = (byte)value;
                }
                pbyte = ADDOFF(byte*, pbyte, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ox += xadd1;
                    pbyte += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,ox
            mov     edi,pbyte
bvcblp:     cmp     esi,limx
            jae     bvcbcl                      ;X out of bounds => clip pixel?
            mov     [edi],al                    ;come here for each pixel
bvcbcl:     add     edi,rowadd                  ;advance to next Y
            add     edx,ebx
            jc      bvcbov                      ;advance to next whole X column?
            dec     ecx
            jnz     bvcblp
            jmp     bvcbdn

bvcbov:     add     edi,xadd1                   ;advance to next X
            add     esi,xadd1
            dec     ecx
            jnz     bvcblp
bvcbdn:
            }
            return;
            #endif      //-------

        } else if (omit == 0) {                 //handle 32 bits/pixel
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if ((dword)ox < (dword)limx) {
                    *ppix = value;
                }
                ppix = ADDOFF(lint*, ppix, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ox += xadd1;
                    ppix += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            xadd4 = xadd1 << 2;
            ASM {
            mov     eax,value
            mov     ebx,frac
            mov     ecx,cnt
            mov     edx,part
            mov     esi,ox
            mov     edi,ppix
bvcdlp:     cmp     esi,limx
            jae     bvcdcl                      ;X out of bounds => clip pixel?
            mov     [edi],eax                   ;come here for each pixel
bvcdcl:     add     edi,rowadd                  ;advance to next Y
            add     edx,ebx
            jc      bvcdov                      ;advance to next whole X column?
            dec     ecx
            jnz     bvcdlp
            jmp     bvcddn

bvcdov:     add     edi,xadd4                   ;advance to next X
            add     esi,xadd1
            dec     ecx
            jnz     bvcdlp
bvcddn:
            }
            return;
            #endif      //-------

        } else {                                //handle omit where inside wide
            ppix = ADDOFF(lint*, ppix, (ox<<2));

            #ifdef NOI86//-------
            do {
                if ((dword)ox < (dword)limx) {
                    if (*ppix != omit) *ppix = value;
                }
                ppix = ADDOFF(lint*, ppix, rowadd);
                part += frac;
                if (part & ESIGN) {
                    part &= EMAXI;
                    ox += xadd1;
                    ppix += xadd1;
                }
            } while (--cnt);
            return;

            #else       //-------
            xadd4 = xadd1 << 2;
            ASM {
            mov     eax,value
            mov     ebx,omit
            mov     ecx,cnt
            mov     edx,part
            mov     esi,ox
            mov     edi,ppix
bvcolp:     cmp     esi,limx                    ;come here for each pixel
            jae     bvcocl                      ;X out of bounds => clip pixel?
            cmp     [edi],ebx
            je      bvcocl                      ;skip if over omit color
            mov     [edi],eax                   ;install the pixel
bvcocl:     add     edi,rowadd                  ;advance to next Y
            add     edx,frac
            jc      bvcoov                      ;advance to next whole X column?
            dec     ecx
            jnz     bvcolp
            jmp     bvcodn

bvcoov:     add     edi,xadd4                   ;advance to next X
            add     esi,xadd1
            dec     ecx
            jnz     bvcolp
bvcodn:
            }
            return;
            #endif      //-------
        }
    }
}

//*************************************************************************
//  BitsBlendInit:
//      Sets up information needed by MMX version of BitsBlend and BitsBlend8.
//      Does nothing if not using MMX version.
//
//  Arguments:
//      void BitsBlendInit (lint cpu, lint iratio,
//                          lint* mxshift, word* mxratio, word* mxone)
//      cpu     1=Non-x86, 2=386, 3=486, 4=Pentium, 5=MMX
//      iratio  Ratio = (dnum / snum) * 0x10000  (supplied on entry)
//      mxshift Returns right shift to convert iratio to HFONE binary point
//              adjusted for high word multiply with operand left shifted by 7.
//              In other words, if you take a byte value value and left shift
//              by 7 to make a word and then multiply by the returned mxone
//              word value (just one of the four), the result can be right
//              shifted by mxshift to get back the same byte value.
//               For example, if iratio is less than 0x10000 (1.0) then
//               this routine returns mxshift=5 and mxone=0x4000.
//               If the orginal value is 0x00FF
//               The 7 leftshift word is 0x7F80
//               Multiplied by 0x4000 is 0x1FE00000
//               and PMULHW high word is 0x1FE0
//               Right mxshift by 5 is   0x00FF
//              Returns 0 if ratio out of range possible for MMX code.
//      mxratio Returns four word ratio values using HFONE binary point.
//      mxone   Returns four word 1.0 values using HFONE binary point.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsBlendInit (lint cpu, lint iratio,
                      lint* mxshift, word* mxratio, word* mxone) {
        if ((cpu & 0xF) != 5) return;
        #ifndef NOI86   //-------
        ASM {
        mov     eax,iratio                      ;ratio = dnum / snum
        mov     edx,HFSHIFT
        shr     eax,(DFSHIFT-HFSHIFT)           ;ratio with 128 binary point
        mov     edi,mxshift
        mov     dword ptr [edi],0               ;assume ratio out of range
        cmp     eax,0xF                         ;(ratio < 1/16)?
        jbe     ininot                          ;ratio too small for fixed?

inisft: cmp     eax,HFONE                       ;place binary point so that
        jb      inigot                          ;fractions fit in unsigned word
        shr     eax,1
        dec     edx
        jmp     inisft

inigot: mov     ecx,edx
        sub     ecx,1+4+4                       ;signed operand, high word mul
        jle     ininot                          ;ratio>16, too big for fixed?
        mov     [edi],ecx                       ;return right shift count

        mov     cl,DFSHIFT
        mov     ebx,DFONE
        sub     cl,dl
        mov     edi,mxone
        shr     ebx,cl                          ;get 1.0 with chosen binary pt
        mov     edx,ebx
        shl     edx,16
        or      ebx,edx                         ;duplicate 1.0 into four words
        mov     [edi+0],ebx                     ;return 1.0 value
        mov     [edi+4],ebx

        mov     edx,eax                         ;ratio with chosen binary pt
        mov     edi,mxratio
        shl     edx,16
        or      eax,edx                         ;dupicate ratio into four words
        mov     [edi+0],eax
        mov     [edi+4],eax
ininot:
        }
        #endif          //-------
        return;
}

//*************************************************************************
//  BitsBlend:
//      Reapportions a row or column pixels to a different resolution.
//      Handles cpu +256 interpolate mode where blends between pixels
//      when there are fewer source than destination pixels.
//      Handles full transparency blending.
//      Requires 32-bit BGRA values.
//
//  Arguments:
//      void BitsBlend (BGR* pd, lint dnum, lint dinc,
//                      BGR* ps, lint snum, lint sinc,
//                      lint iratio, flt fratio, int cpu,
//                      lint mxshift, word* mxratio, word* mxone);
//      pd      Destination bitmap initial pointer.
//      dnum    Number of destination pixels.
//      dinc    Bytes between destination pixels.
//      ps      Source bitmap initial pointer.
//      snum    Number of source pixels.
//      sinc    Bytes between source pixels.
//      iratio  Ratio = (dnum / snum) * 0x10000
//      fratio  Ratio = (dnum / snum)
//      cpu     CPU type and other mode information:
//              1=Non-x86, 2=386, 3=486, 4=Pentium, 5=MMX
//             +128 = Show source bitmap transparent pixels (if any).
//             +256 = Ratios are snum/dnum and snum < dnum (interpolate mode)
//      mxshift Right shift to convert iratio to HFONE binary point (MMX only).
//      mxratio Four word ratio values using HFONE binary point (MMX only).
//      mxone   Four word 1.0 values using HFONE binary point (MMX only).
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsBlend (BGR* pd, lint dnum, lint dinc,
                  BGR* ps, lint snum, lint sinc,
                  lint iratio, flt fratio, int cpu,
                  lint mxshift, word* mxratio, word* mxone) {
    int clear;
    lint per;
    flt have, left;
    #ifdef NOI86    //-------
    lint cnt;
    flt red, green, blue, alpha;
    flt dred, dgreen, dblue, dalpha, into, keep;
    #else           //-------
    lint clrneg;
    int rv, gv, bv, av;
    int ri, gi, bi, ai;
    flt rd, gd, bd, ad;
    flt s1, s2, s3;
    flt falpha, fhalf;
    word rw, gw, bw, aw;
    word afrac[4];
    word aonec[4];
    word zeroq[4];
    #endif          //-------

    clear = cpu & 128;
    cpu &= ~128;

// =======
// Interpolate: Case where there are more destination than source pixels.
// Formerly we duplicated pixels but this code now blends between pixels.

// C-Only Non-Intel version.

    if (cpu & 256) {
        cpu &= ~256;
        #ifdef NOI86//-------
        left = 0.5f;
        while (TRUE) {
            blue = (flt) ps->bblue;
            green = (flt) ps->bgreen;
            red = (flt) ps->bred;
            alpha = (flt) ps->balpha;
            snum -= 1;
            if (snum > 0) ps = ADDOFF(BGR*, ps, sinc);

            dblue = (flt)ps->bblue - blue;
            dgreen = (flt)ps->bgreen - green;
            dred = (flt)ps->bred - red;
            dalpha = (flt)ps->balpha - alpha;

            into = fratio - left;
            blue += dblue * into;
            green += dgreen * into;
            red += dred * into;
            alpha += dalpha * into;

            dblue = dblue * fratio;
            dgreen = dgreen * fratio;
            dred = dred * fratio;
            dalpha = dalpha * fratio;

            left = 1.0f + left;
            while (left >= fratio) {
                if ((clear == 0)||(alpha >= AFLT_FULL)) {
                    pd->balpha = (byte) alpha;  //fully visible? (with leeway)
                    pd->bblue  = (byte) blue;
                    pd->bgreen = (byte) green;
                    pd->bred   = (byte) red;
                } else if (alpha >= 1.0f) {
                    into = alpha * 0.00390625f; //One 256th
                    keep = 1.0f - into;
                    //leave destination alpha!
                    //pd->balpha = (byte) alpha;
                    pd->bblue  =
                        (byte) ((blue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) ((green * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) ((red   * into)+((flt)pd->bred   * keep));
                }
                pd = ADDOFF(BGR*, pd, dinc);
                blue  += dblue;
                green += dgreen;
                red   += dred;
                alpha += dalpha;
                left -= fratio;
                if ((--dnum) == 0) break;

            }
            if (dnum == 0) break;
        }
        return;
        #else       //-------

// Interpolate: MMX Version.

        ASM {
        cmp         cpu,5
        jne         biaasm                      ;not MMX processor?

        mov         edx,mxshift                 ;right shift count for ratio
        mov         ebx,mxone
        mov         ecx,mxratio
        test        edx,edx
        jle         biaasm                      ;ratio wrong for fixed?

        pxor        mm0,mm0                     ;mm0 = zero constant
                                                ;mm1 = ramp value
                                                ;mm2 = ramp increment
        movd        mm3,edx                     ;mm3 = right shift count
        movq        mm4,[ebx]                   ;mm4 = 1.0
        movq        mm5,[ecx]                   ;mm5 = ratio
                                                ;mm6 = temporary
        movq        mm7,mm4                     ;mm7 = left = 1.0>>1 = 0.5

        mov         ebx,snum                    ;EBX = snum
        psrlw       mm7,1                       ;mm7 = left = 0.5
        mov         ecx,dnum                    ;ECX = dnum
        mov         sax,clear
        mov         esi,ps
        mov         edi,pd
        test        sax,sax                     ;clear?
        jnz         bixisr                      ;source has transparent pixels?

bixsrc: movd        mm1,[esi]                   ;U mm1 = This source pixel
        mov         eax,sinc                    ;
        punpcklbw   mm1,mm0                     ;U BGRA words for next pixel
        add         esi,eax                     ;  mm1 = This RGB pixel
        psllw       mm1,7                       ;U get in signed high word
        dec         ebx                         ;U
        jg          bixhav                      ;
        sub         esi,eax                     ;
bixhav: movd        mm2,[esi]                   ;U
        punpcklbw   mm2,mm0                     ;  get BGRA words for next pix
        psllw       mm2,7                       ;U get in signed high word
        movq        mm6,mm5                     ;
         psubsw     mm2,mm1                     ;U mm2 = Next-This difference
        psubusw     mm6,mm7                     ;  into = ratio - left
        pmulhw      mm6,mm2                     ;U into = into * difference
         pmulhw      mm2,mm5                    ;U mm2  = ramp increment
         movd        edx,mm5                    ;  ratio
        pmulhw      mm1,mm4                     ;U This in multiplied position
         paddusw    mm7,mm4                     ;  left = left + 1.0
        paddsw      mm1,mm6                     ;U beginning of source ramp
         movd        eax,mm7                    ;  ratio

bixlop: cmp         eax,edx                     ;U left >= ratio? (high words)
        jb          bixsrc                      ;  no, exit inner loop

        movq        mm6,mm1                     ;U
        psrlw       mm6,mm3                     ;U right shift into byte values
        packuswb    mm6,mm6                     ;U back BGRA words to bytes
         mov        eax,dinc                    ;
        movd        [edi],mm6                   ;U save bytes to dest pixel
         add        edi,eax
        paddsw      mm1,mm2                     ;U increment source ramp
        psubusw     mm7,mm5                     ;  left = left - ratio
        movq        mm6,mm1                     ;U
        pcmpgtw     mm1,mm0                     ;
        pand        mm1,mm6                     ;U stop round error underflow
         dec        ecx                         ;
        movd        eax,mm7                     ;U left
         jnz        bixlop                      ;

        emms
        jmp         bixdon

// Interpolate: MMX Version with Transparency

bixisr: sub         edi,dinc
        mov         eax,0x40004000
        movq        zeroq,mm0
        mov         word ptr afrac+6,0;         ;leave destination alpha!
        mov         dword ptr aonec+0,eax
        mov         dword ptr aonec+4,eax

bixisl: movd        mm1,[esi]                   ;U mm1 = This source pixel
        mov         eax,sinc                    ;
        punpcklbw   mm1,mm0                     ;U BGRA words for next pixel
        add         esi,eax                     ;  mm1 = This RGB pixel
        psllw       mm1,7                       ;U get in signed high word
        dec         ebx                         ;U
        jg          bixihv                      ;
        sub         esi,eax                     ;
bixihv: movd        mm2,[esi]                   ;U
        punpcklbw   mm2,mm0                     ;  get BGRA words for next pix
        psllw       mm2,7                       ;U get in signed high word
        movq        mm6,mm5                     ;
         psubsw     mm2,mm1                     ;U mm2 = Next-This difference
        psubusw     mm6,mm7                     ;  into = ratio - left
        pmulhw      mm6,mm2                     ;U into = into * difference
         pmulhw      mm2,mm5                    ;U mm2  = ramp increment
         movd        edx,mm5                    ;  ratio
        pmulhw      mm1,mm4                     ;U This in multiplied position
         paddusw    mm7,mm4                     ;  left = left + 1.0
        paddsw      mm1,mm6                     ;U beginning of source ramp
         movd        eax,mm7                    ;  ratio

bixilp: cmp         eax,edx                     ;U left >= ratio? (high words)
        jb          bixisl                      ;  no, exit inner loop

        movq        mm6,mm1                     ;U
         mov        eax,dinc                    ;
        psrlw       mm6,mm3                     ;U right shift into byte values
         add        edi,eax                     ;
        packuswb    mm6,mm6                     ;U back BGRA words to bytes
        movd        eax,mm6                     ;U
        and         eax,ALPHA_MASK              ;U
        jz          bixisk                      ;  fully transparent?
        cmp         eax,ALPHA_FULL
        jae         bixall                      ;  fully visible? (with leeway)

        shr         eax,16+2                    ;U make 0x4000=1.0 alpha frac
        mov         afrac+0,ax                  ;U get RGB alpha frac words
        mov         afrac+2,ax                  ;
        mov         afrac+4,ax                  ;U
        //leave destination alpha!
        //mov       afrac+6,0x4000              ;  0 for alpha factor (was 1.0)
        punpcklbw   mm6,mm0                     ;U BGRA words for next pixel
         movd       mm0,[edi]                   ;  old destination pixel
        psllw       mm6,7                       ;U get in signed high word
         punpcklbw  mm0,zeroq                   ;
        pmulhw      mm6,afrac                   ;U lighten source by alpha
         psllw      mm0,7                       ;
        psrlw       mm6,5                       ;U
        packuswb    mm6,mm6                     ;U back BGRA words to bytes
        movd        eax,mm6                     ;U
         movq       mm6,aonec                   ;  1.0
         psubsw     mm6,afrac                   ;U 1.0 - alpha fraction
         pmulhw     mm6,mm0                     ;U lighten dest by 1-alpha
         psrlw      mm6,5                       ;U
         packuswb   mm6,mm6                     ;U back BGRA words to bytes
        movd        mm0,eax                     ;
        paddusb     mm6,mm0                     ;U
        pxor        mm0,mm0                     ;  restore zero

bixall: movd        eax,mm6                     ;U
        mov         [edi],eax                   ;U save bytes to dest pixel
bixisk: paddsw      mm1,mm2                     ;U increment source ramp
        psubusw     mm7,mm5                     ;  left = left - ratio
        movq        mm6,mm1                     ;U
        pcmpgtw     mm1,mm0                     ;
        pand        mm1,mm6                     ;U stop round error underflow
         dec        ecx                         ;
        movd        eax,mm7                     ;U left
         jnz        bixilp                      ;

        emms
        jmp         bixdon

// Interpolate: I86 Assembly Version

biaasm: mov     sax,clear
        xor     edx,edx
        test    sax,sax
        jz      briioz
        mov     edx,ESIGN
        fld1
        mov     bw,255
        fidiv   bw
        mov     bi,0
        mov     gi,0
        mov     ri,0
        mov     ai,0
        fstp    falpha                          ;1/255 constant
briioz: mov     clrneg,edx                      ;negative if (clear == 0)

        mov     bv,0                            ;zero high bytes
        mov     gv,0
        mov     rv,0
        mov     av,0
        mov     bi,0
        mov     gi,0
        mov     ri,0
        mov     ai,0
        mov     dword ptr fhalf,0x3F000000      ;0.5 floating

        mov     ebx,snum
        mov     ecx,dnum
        mov     esi,ps
        mov     edi,pd
        fld     fhalf                           ;  Left=0.5
        jmp     biabeg

biasrc: fstp    ST                              ;  clean up stack from last
        fstp    ST
        fstp    ST
        fstp    ST
        fstp    ST

biabeg: mov     edx,[esi]                       ;  Left must be on 8087 stack
        mov     byte ptr bv,dl
        mov     byte ptr gv,dh
        shr     edx,16
        mov     byte ptr rv,dl
        mov     byte ptr av,dh
        mov     eax,sinc
        dec     ebx
        jle     biadon
        add     esi,eax
biadon: mov     edx,[esi]
        mov     byte ptr bi,dl
        mov     byte ptr gi,dh
        shr     edx,16
        mov     byte ptr ri,dl
        mov     byte ptr ai,dh

        fld     fratio                          ;  Ratio, Left
        fsub    ST,ST(1)                        ;  Into = Ratio-Left, Left
        fild    bv                              ;  Blue, Into, Left
        fild    bi                              ;  Blue2, Blue, Into, Left
        fsub    ST,ST(1)                        ;  BlueD, Blue, Into, Left
        fild    gv                              ;  G,BD,B,Into,Left
        fild    gi                              ;  G2,G,BD,B,Into,Left
        fxch    ST(2)                           ;  BD,G,G2,B,Into,Left
        fst     bd                              ;  BD,G,G2,B,Into,left
        fmul    ST,ST(4)                        ;  BIn,G,G2,B,Into,Left
        fxch    ST(2)                           ;  G2,G,BIn,B,Into,Left
        fsub    ST,ST(1)                        ;  GD,G,BIn,B,Into,Left
        fild    rv                              ;  R,GD,G,BIn,B,Into,Left
        fxch    ST(3)                           ;  BIn,GD,G,R,B,Into,Left
        faddp   ST(4),ST                        ;  GD,G,R,B,Into,Left
        fild    ri                              ;  R2,GD,G,R,B,Into,Left
        fsub    ST,ST(3)                        ;  RD,GD,G,R,B,Into,Left
        fxch    ST(1)                           ;  GD,RD,G,R,B,Into,Left
        fst     gd                              ;  GD,RD,G,R,B,Into,Left
        fmul    ST,ST(5)                        ;  GIn,RD,G,R,B,Into,Left
        fxch    ST(1)                           ;  RD,GIn,G,R,B,Into,Left
        fst     rd                              ;  RD,GIn,G,R,B,Into,Left
        fmul    ST,ST(5)                        ;  RIn,GIn,G,R,B,Into,Left
        fxch    ST(1)                           ;  GIn,RIn,G,R,B,Into,Left
        faddp   ST(2),ST                        ;  RIn,G,R,B,Into,Left
        fild    av                              ;  A,RIn,G,R,B,Into,Left
        fild    ai                              ;  A2,A,RIn,G,R,B,Into,Left
        fsub    ST,ST(1)                        ;  AD,A,RIn,G,R,B,Into,Left
        fxch    ST(2)                           ;  RIn,A,AD,G,R,B,Into,Left
        faddp   ST(4),ST                        ;  A,AD,G,R,B,Into,Left
        fxch    ST(1)                           ;  AD,A,G,R,B,Into,Left
        fst     ad                              ;  AD,A,G,R,B,Into,Left
        fmulp   ST(5),ST                        ;  A,G,R,B,AIn,Left
        fld     fratio                          ;  Ratio,A,G,R,B,AIn,Left
        fmul    bd                              ;  BD,A,G,R,B,AIn,Left
        fxch    ST(1)                           ;  A,BD,G,R,B,AIn,Left
        faddp   ST(5),ST                        ;  BD,G,R,B,A,Left

        fstp    bd                              ;  G,R,B,A,Left
        fld     fratio                          ;  Ratio,G,R,B,A,Left
        fmul    gd                              ;  GD,G,R,B,A,Left
        fld     fratio                          ;  Ratio,GD,G,R,B,A,Left
        fmul    rd                              ;  RD,GD,G,R,B,A,Left
        fld     fratio                          ;  Ratio,RD,GD,G,R,B,A,Left
        fmul    ad                              ;  AD,RD,GD,G,R,B,A,Left
        fxch    ST(2)                           ;  GD,RD,AD,G,R,B,A,Left
        fstp    gd                              ;  RD,AD,G,R,B,A,Left
        fstp    rd                              ;  AD,G,R,B,A,Left
        fstp    ad                              ;  G,R,B,A,Left

        fld1                                    ;  G,R,B,A,Left
        faddp   ST(5),ST                        ;  G,R,B,A,Left+1.0

bialop: fld     fratio                          ;  Ratio,G,R,B,A,Left
        fcom    ST(5)                           ;  Left >= Ratio?
        fnstsw  ax
        mov     edx,dinc
        fwait
        sahf
        ja      biasrc                          ;  Left < Ratio => end loop
        fsubp   ST(5),ST                        ;  G,R,B,A,Left-Ratio
        mov     eax,clrneg

        fist    gv
        fxch    ST(1)                           ;  R,G,B,A,Left
        fist    rv
        fxch    ST(2)                           ;  B,G,R,A,Left
        fist    bv
        fxch    ST(3)                           ;  A,G,R,B,Left
        fist    av
        mov     al,byte ptr bv
        mov     dl,byte ptr rv
        mov     ah,byte ptr gv
        fwait
        mov     dh,byte ptr av
        test    eax,eax
        jge     baivis                          ;  clear set to 0?
        test    dh,dh                           ;  possible transparent source
        jz      baiinv                          ;  invisible alpha?
        cmp     dh,ABYTE_FULL
        jae     baivis                          ;  fully visible? (with leeway)

        fstp    s1                              ;need 6 of stack for blending
        fstp    s2                              ;so save A,G,R to memory
        fstp    s3                              ;but leave B,Left on stack
        fld     falpha;                         ;1/255
        fimul   av                              ;alpha as a fraction
        fld1                                    ;1, afrac
        mov     edx,[edi]                       ;get old destination pixel
        mov     byte ptr bi,dl
        mov     byte ptr gi,dh
        shr     edx,16
        fsub    ST,ST(1)                        ;1-afrac, afrac
        mov     byte ptr ri,dl
        fild    bv                              ;src blue, 1-afrac, afrac
        fmul    ST,ST(2)                        ;sblue*afrac, 1-afrac, afrac
        fild    bi                              ;dst blue, sblue, 1-a,a
        fmul    ST,ST(2)                        ;dblue*1-afrac, sblue, 1-a,a
        fild    gv                              ;src green, dblue, sblue, 1-a,a
        fmul    ST,ST(4)                        ;sgreen*afrac
        fild    gi                              ;dst green, sgreen,db,sb,1-a,a
        fmul    ST,ST(4)                        ;dgreen*1-afrac,sg,db,sb,1-a,a
        fxch    ST(2)                           ;db,dg,sg,sb,1-a,a
        faddp   ST(3),ST                        ;dg,sg,sb+db,1-a,a
        faddp   ST(1),ST                        ;sg+db,sb+db,1-a,a
        fild    rv                              ;src red, green, blue, 1-a, a
        fmulp   ST(4),ST                        ;green, blue,1-a, sred*afrac
        fild    ri                              ;dst red, green, blue, 1-a, sr
        fmulp   ST(3),ST                        ;green, blue, dred*1-afrac,sr
        fxch    ST(1)                           ;blue, green, dr, sr
        fistp   bv                              ;green, dr, sr
        fistp   gv                              ;dr, sr
        faddp   ST(1),ST                        ;sr+dr
        mov     al,byte ptr bv                  ;red
        mov     ah,byte ptr gv                  ;red
        fistp   rv
        //mov   dh,byte ptr av
        mov     dh,[edi+OBALPHA]                ;leave destination alpha!
        fld     s3                              ;restore R
        fld     s2                              ;restore G,R
        fld     s1                              ;restore A,G,R
        fwait
        mov     dl,byte ptr rv

baivis: mov     [edi],ax                        ;  save interpolated pixel
        mov     [edi+2],dx
baiinv: fadd    ad                              ;  A+AD,G,R,B,Left
        fxch    ST(3)                           ;  B,G,R,A,Left
        fadd    bd                              ;  B+BD,G,R,A,Left
        fxch    ST(2)                           ;  R,G,B,A,Left
        fadd    rd                              ;  R+RD,G,B,A,Left
        fxch    ST(1)                           ;  G,R,B,A,Left
        fadd    gd                              ;  G+GD,R,B,A,Left
        mov     eax,dinc
        add     edi,eax
        dec     ecx
        jnz     bialop

        fstp    ST                              ;  clean up stack
        fstp    ST
        fstp    ST
        fstp    ST
        fstp    ST

bixdon:
        }
        return;
        #endif      //-------
    }

// =======
// Handle case where there are an even number of source pixels per destination.
// For speed, we just duplicate source pixels for this case.

    if ((iratio & (DFONE-1)) == 0) {            //integer ratio?
        per = iratio >> DFSHIFT;
        #ifdef NOI86//-------
        do {
            cnt = per;
            do {
                if ((clear == 0)||(ps->balpha >= ABYTE_FULL)) {
                    pd->balpha= ps->balpha;     //fully visible? (with leeway)
                    pd->bblue = ps->bblue;
                    pd->bgreen= ps->bgreen;
                    pd->bred  = ps->bred;
                } else if (ps->balpha >= 1) {
                    into = (flt)ps->balpha * 0.00390625f;
                    keep = 1.0f - into;         //One 256th
                    //leave destination alpha!
                    //pd->balpha = ps->balpha;
                    pd->bblue  =
                        (byte) (((flt)ps->bblue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) (((flt)ps->bgreen * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) (((flt)ps->bred   * into)+((flt)pd->bred   * keep));
                }
                pd = ADDOFF(BGR*, pd, dinc);
            } while (--cnt);
            ps = ADDOFF(BGR*, ps, sinc);
        } while (--snum);

        #else       //-------
        ASM {
        mov     esi,ps
        mov     edi,pd
        mov     ebx,snum
        mov     scx,clear
        mov     edx,dinc
        mov     sax,cpu
        test    scx,scx
        jz      brelop                          ;no transparency?
        cmp     sax,5
        je      brtmmx                          ;can use mmx instructions?

        fld1
        mov     bw,255
        fidiv   bw
        mov     bi,0
        mov     gi,0
        mov     ri,0
        mov     ai,0
        mov     bv,0
        mov     gv,0
        mov     rv,0
        mov     av,0
        fstp    falpha                          ;1/255 constant

brtlop: mov     ecx,per                         ;handle transparent pixels
        mov     eax,[esi]                       ;without mmx
brtlp2: test    eax,ALPHA_MASK                  ;fully transparent?
        jz      brtlp3
        cmp     eax,ALPHA_FULL                  ;fully visible? (with leeway)
        jae     brtlpo

        fld     falpha;                         ;1/255
        mov     byte ptr bv,al
        mov     byte ptr gv,ah
        shr     eax,16
        mov     byte ptr rv,al
        mov     byte ptr av,ah
        fimul   av                              ;alpha as a fraction
        fld1                                    ;1, afrac
        mov     eax,[edi]                       ;get old destination pixel
        mov     byte ptr bi,al
        mov     byte ptr gi,ah
        shr     eax,16
        fsub    ST,ST(1)                        ;1-afrac, afrac
        mov     byte ptr ri,al
        fild    bv                              ;src blue, 1-afrac, afrac
        fmul    ST,ST(2)                        ;sblue*afrac, 1-afrac, afrac
        fild    bi                              ;dst blue, sblue, 1-a,a
        fmul    ST,ST(2)                        ;dblue*1-afrac, sblue, 1-a,a
        fild    gv                              ;src green, dblue, sblue, 1-a,a
        fmul    ST,ST(4)                        ;sgreen*afrac
        fild    gi                              ;dst green, sgreen,db,sb,1-a,a
        fmul    ST,ST(4)                        ;dgreen*1-afrac,sg,db,sb,1-a,a
        fxch    ST(2)                           ;db,dg,sg,sb,1-a,a
        faddp   ST(3),ST                        ;dg,sg,sb+db,1-a,a
        faddp   ST(1),ST                        ;sg+db,sb+db,1-a,a
        fild    rv                              ;src red, green, blue, 1-a, a
        fmulp   ST(4),ST                        ;green, blue,1-a, sred*afrac
        fild    ri                              ;dst red, green, blue, 1-a, sr
        fmulp   ST(3),ST                        ;green, blue, dred*1-afrac,sr
        fxch    ST(1)                           ;blue, green, dr, sr
        fistp   bv                              ;green, dr, sr
        fistp   gv                              ;dr, sr
        faddp   ST(1),ST                        ;sr+dr
        fistp   rv                              ;red
        //mov   ah,byte ptr av
        mov     ah,[edi+OBALPHA]                ;leave destination alpha!
        fwait
        mov     al,byte ptr rv
        shl     eax,16
        mov     al,byte ptr bv
        mov     ah,byte ptr gv

brtlpo: mov     [edi],eax
brtlp3: add     edi,edx
        dec     ecx
        jnz     brtlp2
        add     esi,sinc
        dec     ebx
        jnz     brtlop
        jmp     short breend

brtmmx: pxor        mm7,mm7                     ;mm7 = 0
        mov         eax,0x40004000
        mov         word ptr afrac+6,0;         ;leave destination alpha!
        mov         dword ptr aonec+0,eax
        mov         dword ptr aonec+4,eax
        movq        mm4,aonec                   ;mm5 = 1.0
brmlop: mov     ecx,per                         ;handle transparent pixels
        mov     eax,[esi]                       ;with mmx
brmlp2: test    eax,ALPHA_MASK                  ;fully transparent?
        jz      brmlp3
        cmp     eax,ALPHA_FULL                  ;fully visible? (with leeway)
        jae     brmlpo
        movd        mm6,eax                     ;U
        shr         eax,16+2                    ;  make 0x4000=1.0 alpha frac
        punpcklbw   mm6,mm7                     ;U BGRA words for next pixel
        mov         afrac+0,ax                  ;U get RGB alpha frac words
        mov         afrac+2,ax                  ;
        mov         afrac+4,ax                  ;U
        //leave destination alpha!
        //mov       afrac+6,0x4000              ;  0 alpha factor (was 1.0!)
        movq        mm5,afrac
         movd       mm0,[edi]                   ;  old destination pixel
        psllw       mm6,7                       ;U get in signed high word
         punpcklbw  mm0,mm7                     ;
        pmulhw      mm6,mm5                     ;U lighten source by alpha
         psllw      mm0,7                       ;
        psrlw       mm6,5                       ;U
        packuswb    mm6,mm6                     ;U back BGRA words to bytes
         movq       mm2,mm4                     ;  1.0
         psubsw     mm2,mm5                     ;U 1.0 - alpha fraction
         pmulhw     mm2,mm0                     ;U lighten dest by 1-alpha
         psrlw      mm2,5                       ;U
         packuswb   mm2,mm2                     ;U back BGRA words to bytes
        paddusb     mm6,mm2                     ;U
        movd        eax,mm6                     ;U
brmlpo: mov     [edi],eax
brmlp3: add     edi,edx
        dec     ecx
        jnz     brmlp2
        add     esi,sinc
        dec     ebx
        jnz     brmlop
        emms
        jmp     short breend

brelop: mov     ecx,per                         ;source has not transparency
        mov     eax,[esi]                       ;so quickly copy without blend
brelp2: mov     [edi],eax
        add     edi,edx
        dec     ecx
        jnz     brelp2
        add     esi,sinc
        dec     ebx
        jnz     brelop
breend:
        }
        #endif      //-------
        return;
    }

// =======
// Handle case where there are fewer destination than source pixels.
// C-Only Non-Intel version.

    #ifdef NOI86//-------
        left = 1.0f;
        have = fratio;
        blue  = 0.0f;
        green = 0.0f;
        red   = 0.0f;
        alpha = 0.0f;
        while (TRUE) {
            if (left > have) {                  //part of next dest pixel?
                blue  += (flt)ps->bblue * have; //use all of source pixel
                green += (flt)ps->bgreen* have;
                red   += (flt)ps->bred  * have;
                alpha += (flt)ps->balpha* have;
                left -= have;
                have = fratio;
                ps = ADDOFF(BGR*, ps, sinc);
                if (--snum) continue;           //should not run out of points
                ps = ADDOFF(BGR*, ps, -sinc);   //just in case of rounding error
                continue;
            } else {                            //end dest pixel?
                alpha += (flt)ps->balpha * left;
                if ((clear == 0)||(alpha >= AFLT_FULL)) {
                    pd->balpha = (byte) alpha;  //fully visible? (with leeway)
                    pd->bblue  = (byte) (blue + ((flt)ps->bblue * left));
                    pd->bgreen = (byte) (green+ ((flt)ps->bgreen* left));
                    pd->bred   = (byte) (red  + ((flt)ps->bred  * left));
                } else if (alpha >= 1.0f) {
                    into = alpha * 0.00390625f; //One 256th
                    keep = 1.0f - into;
                    //leave destination alpha!
                    //pd->balpha = (byte) alpha;
                    blue  = blue + ((flt)ps->bblue * left);
                    green = green+ ((flt)ps->bgreen* left);
                    red   = red  + ((flt)ps->bred  * left);
                    pd->bblue  =
                        (byte) ((blue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) ((green * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) ((red   * into)+((flt)pd->bred   * keep));
                }
                pd = ADDOFF(BGR*, pd, dinc);
                if ((--dnum) == 0) break;       //done with all dest points?
                have -= left;                   //use rest of source pixel next
                left = 1.0f;
                blue  = 0.0f;                   //starting new dest pixel
                green = 0.0f;
                red   = 0.0f;
                alpha = 0.0f;
                continue;
            }
        }
        return;
    #else       //-------

// =======
// Intel MMX Assembly version.

    if (cpu == 5) {
        ASM {
        mov         edx,mxshift                 ;right shift count for ratio
        mov         ebx,mxone
        mov         ecx,mxratio
        test        edx,edx
        jle         braasm                      ;ratio wrong for fixed?

        movd        mm3,edx                     ;mm3 = right shift count
        movq        mm4,[ebx]                   ;mm4 = 1.0
        movq        mm5,[ecx]                   ;mm5 = ratio

        movq        mm6,mm5                     ;mm6 = have = ratio
        movq        mm7,mm4                     ;mm7 = left = 1.0

        mov         ebx,snum                    ;EBX = snum
        mov         ecx,dnum                    ;ECX = dnum
        mov         esi,ps
        mov         edi,pd

        pxor        mm0,mm0                     ;mm0 = BGRA accumulators = 0
        pxor        mm2,mm2                     ;mm2 = 0

        mov         edx,dinc
        mov         sax,clear
        sub         edi,edx                     ;will increment before using

        test        sax,sax                     ;clear?
        jnz         brxibg                      ;source has transparent pixels?

// This version assumes all source pixels are opaque.

brxlop: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        jbe         brxend                      ;  no

brxmid: movd        mm1,[esi]                   ;U get source pixel
        mov         eax,sinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         esi,eax                     ;  ps += sinc
        pmulhw      mm1,mm6                     ;U BGRA * have
        psubusw     mm7,mm6                     ;  left -= have
        paddusw     mm0,mm1                     ;U accumulate BGRA
        movq        mm6,mm5                     ;  have = ratio
        dec         ebx                         ;U snum -= 1
        jle         brxsub                      ;

brxtwo: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        ja          brxmid                      ;  yes (maybe better prediction)

brxend: movd        mm1,[esi]                   ;U zero
        mov         eax,dinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         edi,eax                     ;
        pmulhw      mm1,mm7                     ;U BGRA * left
        psubusw     mm6,mm7                     ;  have -= left
        paddusw     mm1,mm0                     ;U accumulate BGRA
        movq        mm7,mm4                     ;  left = 1.0
        psrlw       mm1,mm3                     ;U right shift into byte values
        pxor        mm0,mm0                     ;  BGRA accumulators = 0
        packuswb    mm1,mm1                     ;U back BGRA words to bytes
        dec         ecx                         ;
        movd        [edi],mm1                   ;U save bytes to dest pixel
        jnz         brxlop                      ;
        jmp         brxdon

brxsub: sub         esi,eax                     ;  just in case of rouding error
        jmp         brxtwo

// This version checks for transparent pixels.

brxisb: sub         esi,eax                     ;  just in case of rouding error
        jmp         brxitw

brxibg: movq        zeroq,mm2
        mov         eax,0x40004000
        mov         word ptr afrac+6,0;         ;leave destiantion alpha!
        mov         dword ptr aonec+0,eax
        mov         dword ptr aonec+4,eax

brxilp: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        jbe         brxied                      ;

brximd: movd        mm1,[esi]                   ;U get source pixel
        mov         eax,sinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         esi,eax                     ;  ps += sinc
        pmulhw      mm1,mm6                     ;U BGRA * have
        psubusw     mm7,mm6                     ;  left -= have
        paddusw     mm0,mm1                     ;U accumulate BGRA
        movq        mm6,mm5                     ;  have = ratio
        dec         ebx                         ;U snum -= 1
        jle         brxisb                      ;

brxitw: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        ja          brximd                      ;  yes (maybe better prediction)

brxied: movd        mm1,[esi]                   ;U zero
        mov         eax,dinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         edi,eax                     ;
        pmulhw      mm1,mm7                     ;U BGRA * left
        psubusw     mm6,mm7                     ;  have -= left
        paddusw     mm1,mm0                     ;U accumulate BGRA
        movq        mm7,mm4                     ;  left = 1.0
        psrlw       mm1,mm3                     ;U right shift into byte values
        pxor        mm0,mm0                     ;  BGRA accumulators = 0
        packuswb    mm1,mm1                     ;U back BGRA words to bytes
        movd        eax,mm1                     ;U
        and         eax,ALPHA_MASK              ;U
        jz          brxisk                      ;  fully transparent?
        cmp         eax,ALPHA_FULL
        jae         brxall                      ;  fully visible? (with leeway)

        shr         eax,16+2                    ;U make 0x4000=1.0 alpha frac
        mov         afrac+0,ax                  ;U get RGB alpha frac words
        mov         afrac+2,ax                  ;
        mov         afrac+4,ax                  ;U
        //leave destination alpha!
        //mov       afrac+6,0x4000              ;  0 for alpha factor (was 1.0)
        punpcklbw   mm1,mm2                     ;U BGRA words for next pixel
         movd       mm2,[edi]                   ;  old destination pixel
        psllw       mm1,7                       ;U get in signed high word
         punpcklbw  mm2,zeroq                   ;
        pmulhw      mm1,afrac                   ;U lighten source by alpha
         psllw      mm2,7                       ;
        psrlw       mm1,5                       ;U
        packuswb    mm1,mm1                     ;U back BGRA words to bytes
        movd        eax,mm1                     ;U
         movq       mm1,aonec                   ;  1.0
         psubsw     mm1,afrac                   ;U 1.0 - alpha fraction
         pmulhw     mm1,mm2                     ;U lighten dest by 1-alpha
         psrlw      mm1,5                       ;U
         packuswb   mm1,mm1                     ;U back BGRA words to bytes
        movd        mm2,eax                     ;
        paddusb     mm1,mm2                     ;U
        pxor        mm2,mm2                     ;  restore zero

brxall: movd        eax,mm1                     ;U
        mov         [edi],eax                   ;U save bytes to dest pixel
brxisk: dec         ecx                         ;U
        jnz         brxilp                      ;

brxdon: emms
        }
        return;

// =======
// Intel I86 Non-MMX version.

    } else {
        ASM {
braasm: mov     sax,clear
        xor     edx,edx
        test    sax,sax
        jz      braioz
        mov     edx,ESIGN
        fld1
        mov     bw,255
        fidiv   bw
        mov     bi,0
        mov     gi,0
        mov     ri,0
        mov     ai,0
        fstp    falpha                          ;1/255 constant
braioz: mov     clrneg,edx                      ;negative if (clear == 0)

        mov     esi,ps
        mov     edi,pd
        mov     ebx,snum
        mov     ecx,dnum

        mov     left,VONE                       ;left = 1.0
        mov     eax,fratio
        mov     have,eax                        ;have = fratio
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        mov     bw,0                            ;zero high bytes
        mov     gw,0
        mov     rw,0
        mov     aw,0

bralop: fld     left
        fcomp   have                            ;(left > have)?
        fnstsw  ax
        mov     edx,[esi]
        fwait
        sahf
        jbe     braend

        mov     byte ptr bw,dl                  ;come here if left>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    have                            ;bblue*have, a, r, g, b
        shr     edx,16
        mov     eax,sinc
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b
        add     esi,eax
        fmul    have                            ;bgreen*have, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    have                            ;bred*have,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    have                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fld     left
        mov     eax,fratio
        fsub    have                            ;left -= have
        mov     have,eax                        ;have = fratio
        fstp    left
        dec     ebx
        jnz     bralop
        sub     esi,sinc                        ;just in case of rounding error
        jmp     bralop

braend: mov     byte ptr bw,dl                  ;come here if left<=>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    left                            ;bblue*left, a, r, g, b
        shr     edx,16
        mov     eax,clrneg
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b
        fmul    left                            ;bgreen*left, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    left                            ;bred*left,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    left                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fxch    ST(3)                           ;b, r, g, a
        fistp   bv                              ;r, g, a
        fxch    ST(1)                           ;g, r, a
        fistp   gv                              ;r, a
        fistp   rv                              ;a
        fistp   av
        mov     al,byte ptr bv
        mov     dl,byte ptr rv
        mov     ah,byte ptr gv
        fwait
        mov     dh,byte ptr av

        test    eax,eax
        jl      brainv                          ;use transparency?

        mov     [edi],ax
        mov     [edi+2],dx
        dec     ecx                             ;dnum -= 1
        jz      bradon
        fld     have
        mov     eax,dinc
        fsub    left                            ;have -= left
        mov     left,VONE                       ;left = 1.0
        fstp    have
        add     edi,eax
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        jmp     bralop

brainv: test    dh,dh                           ;possible transparent source
        jz      braclr                          ;alpha=0 => totally invisible?
        cmp     dh,ABYTE_FULL
        jae     braone                          ;fully visible? (with leeway)
        fld     falpha;                         ;1/255
        fimul   av                              ;alpha as a fraction
        fld1                                    ;1, afrac
        mov     edx,[edi]                       ;get old destination pixel
        mov     byte ptr bi,dl
        mov     byte ptr gi,dh
        shr     edx,16
        fsub    ST,ST(1)                        ;1-afrac, afrac
        mov     byte ptr ri,dl
        fild    bv                              ;src blue, 1-afrac, afrac
        fmul    ST,ST(2)                        ;sblue*afrac, 1-afrac, afrac
        fild    bi                              ;dst blue, sblue, 1-a,a
        fmul    ST,ST(2)                        ;dblue*1-afrac, sblue, 1-a,a
        fild    gv                              ;src green, dblue, sblue, 1-a,a
        fmul    ST,ST(4)                        ;sgreen*afrac
        fild    gi                              ;dst green, sgreen,db,sb,1-a,a
        fmul    ST,ST(4)                        ;dgreen*1-afrac,sg,db,sb,1-a,a
        fxch    ST(2)                           ;db,dg,sg,sb,1-a,a
        faddp   ST(3),ST                        ;dg,sg,sb+db,1-a,a
        faddp   ST(1),ST                        ;sg+db,sb+db,1-a,a
        fild    rv                              ;src red, green, blue, 1-a, a
        fmulp   ST(4),ST                        ;green, blue,1-a, sred*afrac
        fild    ri                              ;dst red, green, blue, 1-a, sr
        fmulp   ST(3),ST                        ;green, blue, dred*1-afrac,sr
        fxch    ST(1)                           ;blue, green, dr, sr
        fistp   bv                              ;green, dr, sr
        fistp   gv                              ;dr, sr
        faddp   ST(1),ST                        ;sr+dr
        mov     al,byte ptr bv                  ;red
        mov     ah,byte ptr gv                  ;red
        fistp   rv
        //mov   dh,byte ptr av
        mov     dh,[edi+OBALPHA]                ;leave destination alpha!
        fwait
        mov     dl,byte ptr rv

braone: mov     [edi],ax
        mov     [edi+2],dx
braclr: dec     ecx                             ;dnum -= 1
        jz      bradon
        fld     have
        mov     eax,dinc
        fsub    left                            ;have -= left
        mov     left,VONE                       ;left = 1.0
        fstp    have
        add     edi,eax
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        jmp     bralop

bradon:
        }
        return;
    }
    #endif      //-------
}

//*************************************************************************
//  BitsBlendC:
//      Works like BitsBlend for 32-bit BGRA values.
//      This is a slower C-only version that specially handles clear alphas.
//      It blends alphas to preserve a transparent destination bitmap.
//
//      While BitsBlend does: Ades = Asrc if Asrc=1 for BitsStretch mode 0,
//      this BitsBlendC does: Ades = Ades + Asrc*(1-Ades)
//      where A values are 0.0 - 1.0 fractions.
//
//      It also blends the RGB values differently when Ades is not opaque.
//      While BitsBlend does: Rdes = (Rsrc * Asrc) + (Rdes * (1-Asrc))
//      this BitsBlendC does: Rdes = (Rsrc * Asrc) + (Rdes * Ades * (1-Asrc))
//
//  Arguments:
//      void BitsBlendC (BGR* pd, lint dnum, lint dinc,
//                       BGR* ps, lint snum, lint sinc,
//                       lint iratio, flt fratio, int cpu,
//                       lint mxshift, word* mxratio, word* mxone);
//      pd      Destination bitmap initial pointer.
//      dnum    Number of destination pixels.
//      dinc    Bytes between destination pixels.
//      ps      Source bitmap initial pointer.
//      snum    Number of source pixels.
//      sinc    Bytes between source pixels.
//      iratio  Ratio = (dnum / snum) * 0x10000
//      fratio  Ratio = (dnum / snum)
//      cpu     CPU type and other mode information:
//              1=Non-x86, 2=386, 3=486, 4=Pentium, 5=MMX
//             +128 = Show source bitmap transparent pixels (if any).
//             +256 = Ratios are snum/dnum and snum <  dnum (interpolate mode)
//      mxshift Right shift to convert iratio to HFONE binary point (MMX only).
//      mxratio Four word ratio values using HFONE binary point (MMX only).
//      mxone   Four word 1.0 values using HFONE binary point (MMX only).
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsBlendC (BGR* pd, lint dnum, lint dinc,
                   BGR* ps, lint snum, lint sinc,
                   lint iratio, flt fratio, int cpu,
                   lint mxshift, word* mxratio, word* mxone) {
    int clear;
    lint per;
    flt have, left;
    lint cnt;
    flt red, green, blue, alpha;
    flt dred, dgreen, dblue, dalpha, into, keep, ades;

    clear = cpu & 128;
    cpu &= ~128;

// =======
// Interpolate: Case where there are more destination than source pixels.
// Formerly we duplicated pixels but this code now blends between pixels.

    if (cpu & 256) {
        cpu &= ~256;
        left = 0.5f;
        while (TRUE) {
            blue = (flt) ps->bblue;
            green = (flt) ps->bgreen;
            red = (flt) ps->bred;
            alpha = (flt) ps->balpha;
            snum -= 1;
            if (snum > 0) ps = ADDOFF(BGR*, ps, sinc);

            dblue = (flt)ps->bblue - blue;
            dgreen = (flt)ps->bgreen - green;
            dred = (flt)ps->bred - red;
            dalpha = (flt)ps->balpha - alpha;

            into = fratio - left;
            blue += dblue * into;
            green += dgreen * into;
            red += dred * into;
            alpha += dalpha * into;

            dblue = dblue * fratio;
            dgreen = dgreen * fratio;
            dred = dred * fratio;
            dalpha = dalpha * fratio;

            left = 1.0f + left;
            while (left >= fratio) {
                if (clear == 0) {
                    pd->balpha = (byte) alpha;  //BitsStretch mode 1?
                    pd->bblue  = (byte) blue;
                    pd->bgreen = (byte) green;
                    pd->bred   = (byte) red;
                } else if (alpha >= 1.0f) {
                    into = alpha * 0.00390625f; //One 256th
                    ades = (flt)pd->balpha * 0.00390625f;
                    keep = ades * (1.0f - into);
                    pd->bblue  =
                        (byte) ((blue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) ((green * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) ((red   * into)+((flt)pd->bred   * keep));
                    pd->balpha =
                        (byte) ((flt)pd->balpha + (alpha * (1.0f - ades)));
                }
                pd = ADDOFF(BGR*, pd, dinc);
                blue  += dblue;
                green += dgreen;
                red   += dred;
                alpha += dalpha;
                left -= fratio;
                if ((--dnum) == 0) break;
            }
            if (dnum == 0) break;
        }
        return;

// =======
// Handle case where there are an even number of source pixels per destination.
// For speed, we just duplicate source pixels for this case.

    } else if ((iratio & (DFONE-1)) == 0) {     //integer ratio?
        per = iratio >> DFSHIFT;
        do {
            cnt = per;
            do {
                if (clear == 0) {
                    pd->balpha= ps->balpha;     //BitsStretch mode 1?
                    pd->bblue = ps->bblue;
                    pd->bgreen= ps->bgreen;
                    pd->bred  = ps->bred;
                } else if (ps->balpha >= 1) {
                    into = (flt)ps->balpha * 0.00390625f;
                    ades = (flt)pd->balpha * 0.00390625f;
                    keep = ades * (1.0f - into);
                    pd->bblue  =
                        (byte) (((flt)ps->bblue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) (((flt)ps->bgreen * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) (((flt)ps->bred   * into)+((flt)pd->bred   * keep));
                    pd->balpha =
                        (byte) ((flt)pd->balpha + ((flt)ps->balpha * (1.0f - ades)));
                }
                pd = ADDOFF(BGR*, pd, dinc);
            } while (--cnt);
            ps = ADDOFF(BGR*, ps, sinc);
        } while (--snum);
        return;

// =======
// Handle case where there are fewer destination than source pixels.
// C-Only Non-Intel version.

    } else {
        left = 1.0f;
        have = fratio;
        blue  = 0.0f;
        green = 0.0f;
        red   = 0.0f;
        alpha = 0.0f;
        while (TRUE) {
            if (left > have) {                  //part of next dest pixel?
                blue  += (flt)ps->bblue * have; //use all of source pixel
                green += (flt)ps->bgreen* have;
                red   += (flt)ps->bred  * have;
                alpha += (flt)ps->balpha* have;
                left -= have;
                have = fratio;
                ps = ADDOFF(BGR*, ps, sinc);
                if (--snum) continue;           //should not run out of points
                ps = ADDOFF(BGR*, ps, -sinc);   //just in case of rounding error
                continue;
            } else {                            //end dest pixel?
                alpha += (flt)ps->balpha * left;
                if (clear == 0) {
                    pd->balpha = (byte) alpha;  //fully visible? (with leeway)
                    pd->bblue  = (byte) (blue + ((flt)ps->bblue * left));
                    pd->bgreen = (byte) (green+ ((flt)ps->bgreen* left));
                    pd->bred   = (byte) (red  + ((flt)ps->bred  * left));
                } else if (alpha >= 1.0f) {
                    into = alpha * 0.00390625f; //One 256th
                    ades = (flt)pd->balpha * 0.00390625f;
                    keep = ades * (1.0f - into);
                    blue  = blue + ((flt)ps->bblue * left);
                    green = green+ ((flt)ps->bgreen* left);
                    red   = red  + ((flt)ps->bred  * left);
                    pd->bblue  =
                        (byte) ((blue  * into)+((flt)pd->bblue  * keep));
                    pd->bgreen =
                        (byte) ((green * into)+((flt)pd->bgreen * keep));
                    pd->bred   =
                        (byte) ((red   * into)+((flt)pd->bred   * keep));
                    pd->balpha =
                        (byte) ((flt)pd->balpha + (alpha * (1.0f - ades)));
                }
                pd = ADDOFF(BGR*, pd, dinc);
                if ((--dnum) == 0) break;       //done with all dest points?
                have -= left;                   //use rest of source pixel next
                left = 1.0f;
                blue  = 0.0f;                   //starting new dest pixel
                green = 0.0f;
                red   = 0.0f;
                alpha = 0.0f;
                continue;
            }
        }
        return;
    }
}

//*************************************************************************
//  BitsBlend8:
//      Reapportions a row or column pixels to a different resolution.
//      Handles cpu +256 interpolate mode where blends between pixels
//      when there are fewer source than destination pixels.
//      This 8-bit version does not do transparency blending because it is only
//      used to a 32-bit intermediate which is then blended to the destination.
//      Requires 8-bit source bimap and 32-bit destination BGRA values.
//
//  Arguments:
//      void BitsBlend8 (BGR* pd, lint dnum, lint dinc,
//                       byte* ps, lint snum, lint sinc, BGR* spal,
//                       lint iratio, flt fratio, int cpu,
//                       lint mxshift, word* mxratio, word* mxone);
//      pd      Destination bitmap initial pointer.
//      dnum    Number of destination pixels.
//      dinc    Bytes between destination pixels.
//      ps      Source bitmap initial pointer.
//      snum    Number of source pixels.
//      sinc    Bytes between source pixels.
//      spal    Pointer to source palette entries.
//      iratio  Ratio = (dnum / snum) * 0x10000
//      fratio  Ratio = (dnum / snum)
//      cpu     CPU type and other mode information:
//              1=Non-x86, 2=386, 3=486, 4=Pentium, 5=MMX
//             +128 = Show source bitmap transparent pix (NEVER USED=>IGNORED!)
//             +256 = Ratios are snum/dnum and snum <  dnum (interpolate mode)
//      mxshift Right shift to convert iratio to HFONE binary point (MMX only),
//      mxratio Four word ratio values using HFONE binary point (MMX only).
//      mxone   Four word 1.0 values using HFONE binary point (MMX only).
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsBlend8 (BGR* pd, lint dnum, lint dinc,
                   byte* ps, lint snum, lint sinc, BGR* spal,
                   lint iratio, flt fratio, int cpu,
                   lint mxshift, word* mxratio, word* mxone) {
    lint per;
    flt have, left;
    #ifdef NOI86    //-------
    BGR sbgr;
    lint cnt;
    flt red, green, blue, alpha;
    flt dred, dgreen, dblue, dalpha, into;
    #else           //-------
    int rv, gv, bv, av;
    int ri, gi, bi, ai;
    flt rd, gd, bd, ad, fhalf;
    word rw, gw, bw, aw;
    #endif          //-------

    cpu &= ~128;                                //ignore transparency

// =======
// Handle case where there are more destination than source pixels.
// Formerly we duplicated pixles but this code now blends between pixels.

    if (cpu & 256) {
        cpu &= ~256;
        #ifdef NOI86//-------
        left = 0.5;
        while (TRUE) {
            sbgr = spal[*ps];
            blue = (flt) sbgr.bblue;
            green = (flt) sbgr.bgreen;
            red = (flt) sbgr.bred;
            alpha = (flt) sbgr.balpha;
            snum -= 1;
            if (snum > 0) ps = ADDOFF(byte*, ps, sinc);

            sbgr = spal[*ps];
            dblue = (flt)sbgr.bblue - blue;
            dgreen = (flt)sbgr.bgreen - green;
            dred = (flt)sbgr.bred - red;
            dalpha = (flt)sbgr.balpha - alpha;

            into = fratio - left;
            blue += dblue * into;
            green += dgreen * into;
            red += dred * into;
            alpha += dalpha * into;

            dblue = dblue * fratio;
            dgreen = dgreen * fratio;
            dred = dred * fratio;
            dalpha = dalpha * fratio;

            left = 1.0f + left;
            while (left >= fratio) {
                pd->balpha = (byte) alpha;
                pd->bblue  = (byte) blue;
                pd->bgreen = (byte) green;
                pd->bred   = (byte) red;

                pd = ADDOFF(BGR*, pd, dinc);
                blue  += dblue;
                green += dgreen;
                red   += dred;
                alpha += dalpha;
                left -= fratio;
                if ((--dnum) == 0) break;

            }
            if (dnum == 0) break;
        }
        return;
        #else       //-------

// Interpolate: MMX Version.

        ASM {
        cmp         cpu,5
        jne         biaasm                      ;not MMX processor?

        mov         edx,mxshift                 ;right shift count for ratio
        mov         ebx,mxone
        mov         ecx,mxratio
        test        edx,edx
        jle         biaasm                      ;ratio wrong for fixed?

        pxor        mm0,mm0                     ;mm0 = zero constant
                                                ;mm1 = ramp value
                                                ;mm2 = ramp increment
        movd        mm3,edx                     ;mm3 = right shift count
        movq        mm4,[ebx]                   ;mm4 = 1.0
        movq        mm5,[ecx]                   ;mm5 = ratio
                                                ;mm6 = temporary
        movq        mm7,mm4                     ;mm7 = left = 1.0 >> 1 = 0.5

        mov         ebx,snum                    ;EBX = snum
        psrlw       mm7,1                       ;mm7 = left = 0.5
        mov         ecx,dnum                    ;ECX = dnum
        mov         esi,ps
        mov         edi,pd

bi8src: xor         eax,eax                     ;U
        mov         edx,spal                    ;
        mov         al,[esi]                    ;U
        movd        mm1,[edx+(eax*4)]           ;2 AGI delay
        mov         eax,sinc                    ;
        punpcklbw   mm1,mm0                     ;U BGRA words for next pixel
        add         esi,eax                     ;  mm1 = This RGB pixel
        psllw       mm1,7                       ;U get in signed high word
        dec         ebx                         ;U
        jg          bi8hav                      ;
        sub         esi,eax                     ;
bi8hav: xor         eax,eax                     ;U
        mov         al,[esi]                    ;U
        movd        mm2,[edx+(eax*4)]           ;2 AGI delay
        punpcklbw   mm2,mm0                     ;  get BGRA words for next pix
        psllw       mm2,7                       ;U get in signed high word
        movq        mm6,mm5                     ;
         psubsw     mm2,mm1                     ;U mm2 = Next-This difference
        psubusw     mm6,mm7                     ;  into = ratio - left
        pmulhw      mm6,mm2                     ;U into = into * difference
         pmulhw      mm2,mm5                    ;U mm2  = ramp increment
         movd        edx,mm5                    ;  ratio
        pmulhw      mm1,mm4                     ;U This in multiplied position
         paddusw    mm7,mm4                     ;  left = left + 1.0
        paddsw      mm1,mm6                     ;U beginning of source ramp
         movd        eax,mm7                    ;  ratio

bi8lop: cmp         eax,edx                     ;U left >= ratio? (high words)
        jb          bi8src                      ;  no, exit inner loop

        movq        mm6,mm1                     ;U
        psrlw       mm6,mm3                     ;U right shift into byte values
        packuswb    mm6,mm6                     ;U back BGRA words to bytes
         mov        eax,dinc                    ;
        movd        [edi],mm6                   ;U save bytes to dest pixel
         add        edi,eax
        paddsw      mm1,mm2                     ;U increment source ramp
        psubusw     mm7,mm5                     ;  left = left - ratio
        movq        mm6,mm1                     ;U
        pcmpgtw     mm1,mm0                     ;
        pand        mm1,mm6                     ;U stop round error underflow
         dec        ecx                         ;
        movd        eax,mm7                     ;U left
         jnz        bi8lop                      ;

        emms
        jmp         bi8don

// Interpolate: I86 Assembly Version

biaasm: mov     bv,0                            ;zero high bytes
        mov     gv,0
        mov     rv,0
        mov     av,0
        mov     bi,0
        mov     gi,0
        mov     ri,0
        mov     ai,0
        mov     dword ptr fhalf,0x3F000000      ;0.5 floating

        mov     ebx,snum
        mov     ecx,dnum
        mov     esi,ps
        mov     edi,pd
        fld     fhalf                           ;  Left=0.5
        jmp     bi8beg

b8asrc: fstp    ST                              ;  clean up stack from last
        fstp    ST
        fstp    ST
        fstp    ST
        fstp    ST

bi8beg: xor     eax,eax
        mov     edx,spal
        mov     al,[esi]
        mov     edx,[edx+(eax*4)]
        mov     byte ptr bv,dl
        mov     byte ptr gv,dh
        shr     edx,16
        mov     byte ptr rv,dl
        mov     byte ptr av,dh
        mov     eax,sinc
        dec     ebx
        jle     b8adon
        add     esi,eax
b8adon: xor     eax,eax
        mov     edx,spal
        mov     al,[esi]
        mov     edx,[edx+(eax*4)]
        mov     byte ptr bi,dl
        mov     byte ptr gi,dh
        shr     edx,16
        mov     byte ptr ri,dl
        mov     byte ptr ai,dh

        fld     fratio                          ;  Ratio, Left
        fsub    ST,ST(1)                        ;  Into = Ratio-Left, Left
        fild    bv                              ;  Blue, Into, Left
        fild    bi                              ;  Blue2, Blue, Into, Left
        fsub    ST,ST(1)                        ;  BlueD, Blue, Into, Left
        fild    gv                              ;  G,BD,B,Into,Left
        fild    gi                              ;  G2,G,BD,B,Into,Left
        fxch    ST(2)                           ;  BD,G,G2,B,Into,Left
        fst     bd                              ;  BD,G,G2,B,Into,left
        fmul    ST,ST(4)                        ;  BIn,G,G2,B,Into,Left
        fxch    ST(2)                           ;  G2,G,BIn,B,Into,Left
        fsub    ST,ST(1)                        ;  GD,G,BIn,B,Into,Left
        fild    rv                              ;  R,GD,G,BIn,B,Into,Left
        fxch    ST(3)                           ;  BIn,GD,G,R,B,Into,Left
        faddp   ST(4),ST                        ;  GD,G,R,B,Into,Left
        fild    ri                              ;  R2,GD,G,R,B,Into,Left
        fsub    ST,ST(3)                        ;  RD,GD,G,R,B,Into,Left
        fxch    ST(1)                           ;  GD,RD,G,R,B,Into,Left
        fst     gd                              ;  GD,RD,G,R,B,Into,Left
        fmul    ST,ST(5)                        ;  GIn,RD,G,R,B,Into,Left
        fxch    ST(1)                           ;  RD,GIn,G,R,B,Into,Left
        fst     rd                              ;  RD,GIn,G,R,B,Into,Left
        fmul    ST,ST(5)                        ;  RIn,GIn,G,R,B,Into,Left
        fxch    ST(1)                           ;  GIn,RIn,G,R,B,Into,Left
        faddp   ST(2),ST                        ;  RIn,G,R,B,Into,Left
        fild    av                              ;  A,RIn,G,R,B,Into,Left
        fild    ai                              ;  A2,A,RIn,G,R,B,Into,Left
        fsub    ST,ST(1)                        ;  AD,A,RIn,G,R,B,Into,Left
        fxch    ST(2)                           ;  RIn,A,AD,G,R,B,Into,Left
        faddp   ST(4),ST                        ;  A,AD,G,R,B,Into,Left
        fxch    ST(1)                           ;  AD,A,G,R,B,Into,Left
        fst     ad                              ;  AD,A,G,R,B,Into,Left
        fmulp   ST(5),ST                        ;  A,G,R,B,AIn,Left
        fld     fratio                          ;  Ratio,A,G,R,B,AIn,Left
        fmul    bd                              ;  BD,A,G,R,B,AIn,Left
        fxch    ST(1)                           ;  A,BD,G,R,B,AIn,Left
        faddp   ST(5),ST                        ;  BD,G,R,B,A,Left

        fstp    bd                              ;  G,R,B,A,Left
        fld     fratio                          ;  Ratio,G,R,B,A,Left
        fmul    gd                              ;  GD,G,R,B,A,Left
        fld     fratio                          ;  Ratio,GD,G,R,B,A,Left
        fmul    rd                              ;  RD,GD,G,R,B,A,Left
        fld     fratio                          ;  Ratio,RD,GD,G,R,B,A,Left
        fmul    ad                              ;  AD,RD,GD,G,R,B,A,Left
        fxch    ST(2)                           ;  GD,RD,AD,G,R,B,A,Left
        fstp    gd                              ;  RD,AD,G,R,B,A,Left
        fstp    rd                              ;  AD,G,R,B,A,Left
        fstp    ad                              ;  G,R,B,A,Left

        fld1                                    ;  G,R,B,A,Left
        faddp   ST(5),ST                        ;  G,R,B,A,Left+1.0

b8alop: fld     fratio                          ;  Ratio,G,R,B,A,Left
        fcom    ST(5)                           ;  Left >= Ratio?
        fnstsw  ax
        mov     edx,dinc
        fwait
        sahf
        ja      b8asrc                          ;  Left < Ratio => end loop
        fsubp   ST(5),ST                        ;  G,R,B,A,Left-Ratio

        fist    gv
        fxch    ST(1)                           ;  R,G,B,A,Left
        fist    rv
        fxch    ST(2)                           ;  B,G,R,A,Left
        fist    bv
        fxch    ST(3)                           ;  A,G,R,B,Left
        fist    av
        mov     al,byte ptr bv
        mov     dl,byte ptr rv
        mov     ah,byte ptr gv
        fwait
        mov     dh,byte ptr av

        mov     [edi],ax                        ;  save interpolated pixel
        mov     [edi+2],dx
        fadd    ad                              ;  A+AD,G,R,B,Left
        fxch    ST(3)                           ;  B,G,R,A,Left
        fadd    bd                              ;  B+BD,G,R,A,Left
        fxch    ST(2)                           ;  R,G,B,A,Left
        fadd    rd                              ;  R+RD,G,B,A,Left
        fxch    ST(1)                           ;  G,R,B,A,Left
        fadd    gd                              ;  G+GD,R,B,A,Left
        mov     eax,dinc
        add     edi,eax
        dec     ecx
        jnz     b8alop

        fstp    ST                              ;  clean up stack
        fstp    ST
        fstp    ST
        fstp    ST
        fstp    ST

bi8don:
        }
        return;
        #endif      //-------
    }

// =======
// Handle case where there are an even number of source pixels per destination.
// For speed, we just duplicate source pixels for this case.

    if ((iratio & (DFONE-1)) == 0) {            //integer ratio?
        per = iratio >> DFSHIFT;
        #ifdef NOI86//-------
        do {
            cnt = per;
            do {
                sbgr = spal[*ps];
                pd->balpha= sbgr.balpha;
                pd->bblue = sbgr.bblue;
                pd->bgreen= sbgr.bgreen;
                pd->bred  = sbgr.bred;
                pd = ADDOFF(BGR*, pd, dinc);
            } while (--cnt);
            ps = ADDOFF(byte*, ps, sinc);
        } while (--snum);

        #else       //-------
        ASM {
        mov     esi,ps
        mov     edi,pd
        mov     ebx,snum
        mov     edx,dinc

bre8lp: xor     eax,eax                         ;source has no transparency
        mov     ecx,spal
        mov     al,[esi]
        mov     eax,[ecx+(eax*4)]
        mov     ecx,per
bre8l2: mov     [edi],eax
        add     edi,edx
        dec     ecx
        jnz     bre8l2
        add     esi,sinc
        dec     ebx
        jnz     bre8lp
        }
        #endif      //-------
        return;
    }

// =======
// Handle case where there are fewer destination than source pixels.
// C-Only Non-Intel version.

    #ifdef NOI86//-------
        left = 1.0f;
        have = fratio;
        blue  = 0.0f;
        green = 0.0f;
        red   = 0.0f;
        alpha = 0.0f;
        while (TRUE) {
            sbgr = spal[*ps];                   //get next source pixel color
            if (left > have) {                  //part of next dest pixel?
                blue  += (flt)sbgr.bblue * have;//use all of source pixel
                green += (flt)sbgr.bgreen* have;
                red   += (flt)sbgr.bred  * have;
                alpha += (flt)sbgr.balpha* have;
                left -= have;
                have = fratio;
                ps = ADDOFF(byte*, ps, sinc);
                if (--snum) continue;           //should not run out of points
                ps = ADDOFF(byte*, ps, -sinc);  //just in case of rounding err
                continue;
            } else {                            //end dest pixel?
                alpha += (flt)sbgr.balpha * left;
                pd->balpha = (byte) alpha;
                pd->bblue  = (byte) (blue + ((flt)sbgr.bblue * left));
                pd->bgreen = (byte) (green+ ((flt)sbgr.bgreen* left));
                pd->bred   = (byte) (red  + ((flt)sbgr.bred  * left));
                pd = ADDOFF(BGR*, pd, dinc);
                if ((--dnum) == 0) break;       //done with all dest points?
                have -= left;                   //use rest of source pixel next
                left = 1.0f;
                blue  = 0.0f;                   //starting new dest pixel
                green = 0.0f;
                red   = 0.0f;
                alpha = 0.0f;
                continue;
            }
        }
        return;
    #else       //-------

// =======
// Intel MMX Assembly version.

    if (cpu == 5) {
        ASM {
        mov         edx,mxshift                 ;right shift count for ratio
        mov         ebx,mxone
        mov         ecx,mxratio
        test        edx,edx
        jle         br8asm                      ;ratio wrong for fixed?

        movd        mm3,edx                     ;mm3 = right shift count
        movq        mm4,[ebx]                   ;mm4 = 1.0
        movq        mm5,[ecx]                   ;mm5 = ratio

        movq        mm6,mm5                     ;mm6 = have = ratio
        movq        mm7,mm4                     ;mm7 = left = 1.0

        mov         ebx,snum                    ;EBX = snum
        mov         ecx,dnum                    ;ECX = dnum
        mov         esi,ps
        mov         edi,pd

        pxor        mm0,mm0                     ;mm0 = BGRA accumulators = 0
        pxor        mm2,mm2                     ;mm2 = 0

        mov         edx,dinc
        sub         edi,edx                     ;will increment before using

// This version assumes all source pixels are opaque.

br8lop: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        jbe         br8end                      ;  no

br8mid: xor         eax,eax                     ;U
        mov         edx,spal                    ;
        mov         al,[esi]                    ;U get source palette index
        movd        mm1,[edx+(eax*4)]           ;2 AGI delay
        mov         edx,sinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         esi,edx                     ;  ps += sinc
        pmulhw      mm1,mm6                     ;U BGRA * have
        psubusw     mm7,mm6                     ;  left -= have
        paddusw     mm0,mm1                     ;U accumulate BGRA
        movq        mm6,mm5                     ;  have = ratio
        dec         ebx                         ;U snum -= 1
        jle         br8sub                      ;

br8two: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        ja          br8mid                      ;  yes (maybe better prediction)

br8end: xor         eax,eax                     ;U
        mov         edx,spal                    ;
        mov         al,[esi]                    ;U get source palette index
        movd        mm1,[edx+(eax*4)]           ;2 AGI delay
        mov         edx,dinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         edi,edx                     ;
        pmulhw      mm1,mm7                     ;U BGRA * left
        psubusw     mm6,mm7                     ;  have -= left
        paddusw     mm1,mm0                     ;U accumulate BGRA
        movq        mm7,mm4                     ;  left = 1.0
        psrlw       mm1,mm3                     ;U right shift into byte values
        pxor        mm0,mm0                     ;  BGRA accumulators = 0
        packuswb    mm1,mm1                     ;U back BGRA words to bytes
        dec         ecx                         ;
        movd        [edi],mm1                   ;U save bytes to dest pixel
        jnz         br8lop                      ;
        jmp         br8don

br8sub: sub         esi,edx                     ;  just in case of rouding error
        jmp         br8two

br8don: emms
        }
        return;

// =======
// Intel I86 Non-MMX version.

    } else {
        ASM {
br8asm: mov     esi,ps
        mov     edi,pd
        mov     ebx,snum
        mov     ecx,dnum

        mov     left,VONE                       ;left = 1.0
        mov     eax,fratio
        mov     have,eax                        ;have = fratio
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        mov     bw,0                            ;zero high bytes
        mov     gw,0
        mov     rw,0
        mov     aw,0

bra8lp: fld     left
        xor     eax,eax
        fcomp   have                            ;(left > have)?
        mov     al,[esi]                        ;get source palette index
        mov     edx,spal
        mov     edx,[edx+(eax*4)]               ;get palette color for index
        fnstsw  ax
        fwait
        sahf
        jbe     bra8ed

        mov     byte ptr bw,dl                  ;come here if left>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    have                            ;bblue*have, a, r, g, b
        shr     edx,16
        mov     eax,sinc
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b
        add     esi,eax
        fmul    have                            ;bgreen*have, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    have                            ;bred*have,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    have                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fld     left
        mov     eax,fratio
        fsub    have                            ;left -= have
        mov     have,eax                        ;have = fratio
        fstp    left
        dec     ebx
        jnz     bra8lp
        sub     esi,sinc                        ;just in case of rounding error
        jmp     bra8lp

bra8ed: mov     byte ptr bw,dl                  ;come here if left<=>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    left                            ;bblue*left, a, r, g, b
        shr     edx,16
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b
        fmul    left                            ;bgreen*left, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    left                            ;bred*left,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    left                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fxch    ST(3)                           ;b, r, g, a
        fistp   bv                              ;r, g, a
        fxch    ST(1)                           ;g, r, a
        fistp   gv                              ;r, a
        fistp   rv                              ;a
        fistp   av
        mov     al,byte ptr bv
        mov     dl,byte ptr rv
        mov     ah,byte ptr gv
        fwait
        mov     dh,byte ptr av

        mov     [edi],ax
        mov     [edi+2],dx
        dec     ecx                             ;dnum -= 1
        jz      bra8dn
        fld     have
        mov     eax,dinc
        fsub    left                            ;have -= left
        mov     left,VONE                       ;left = 1.0
        fstp    have
        add     edi,eax
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        jmp     bra8lp

bra8dn:
        }
        return;
    }
    #endif      //-------
}

//*************************************************************************
//  BitsBlend1:
//      Reapportions a row or column pixels to a different resolution.
//      This 1-bit version does not do transparency blending because it is only
//      used to a 32-bit intermediate which is then blended to the destination.
//      Requires 1-bit mono source bimap and 32-bit destination BGRA values.
//      Only handles the case where there are fewer destination pixels (or =).
//
//  Arguments:
//      void BitsBlend1 (BGR* pd, lint dnum, lint dinc,
//                       byte* ps, lint snum, lint bitm, BGR* spal,
//                       lint iratio, flt fratio, int cpu,
//                       lint mxshift, word* mxratio, word* mxone);
//      pd      Destination bitmap initial pointer.
//      dnum    Number of destination pixels.
//      dinc    Bytes between destination pixels.
//      ps      Source bitmap initial pointer.
//      snum    Number of source pixels.
//      bitm    Mask for pixel bit to start with in *ps.
//              Note that there is sinc and we always advance one pixel right.
//      spal    Pointer to source palette entries.
//      iratio  Ratio = (dnum / snum) * 0x10000
//      fratio  Ratio = (dnum / snum)
//      cpu     CPU type and other mode information:
//              1=Non-x86, 2=386, 3=486, 4=Pentium, 5=MMX
//      mxshift Right shift to convert iratio to HFONE binary point (MMX only),
//      mxratio Four word ratio values using HFONE binary point (MMX only).
//      mxone   Four word 1.0 values using HFONE binary point (MMX only).
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid BitsBlend1 (BGR* pd, lint dnum, lint dinc,
                   byte* ps, lint snum, lint bitm, BGR* spal,
                   lint iratio, flt fratio, int cpu,
                   lint mxshift, word* mxratio, word* mxone) {
    lint one, zero;
    flt have, left;
    #ifdef NOI86    //-------
    LHUE sbgr;
    flt red, green, blue, alpha;
    #else           //-------
    int rv, gv, bv, av;
    word rw, gw, bw, aw;
    #endif          //-------

    cpu &= ~128;                                //ingnore transparency
    zero = ((lint*)spal)[0];
    one  = ((lint*)spal)[1];

// =======
// Handle case where there are fewer destination than source pixels.
// C-Only Non-Intel version.

    #ifdef NOI86//-------
    left = 1.0f;
    have = fratio;
    blue  = 0.0f;
    green = 0.0f;
    red   = 0.0f;
    alpha = 0.0f;
    while (TRUE) {
        sbgr.d = (*ps & bitm) ? one : zero;
        if (left > have) {
            blue  += (flt)sbgr.b.bblue * have;  //use all of source pixel
            green += (flt)sbgr.b.bgreen* have;
            red   += (flt)sbgr.b.bred  * have;
            alpha += (flt)sbgr.b.balpha* have;
            left -= have;
            have = fratio;
            snum -= 1;
            if (snum == 0) continue;
            bitm = bitm >> 1;                   //advance to next source pixel
            if (bitm == 0) {
                bitm = 0x80;
                ps += 1;
            }
        } else {                                //end dest pixel?
            alpha += (flt)sbgr.b.balpha * left;
            pd->balpha = (byte) alpha;
            pd->bblue  = (byte) (blue + ((flt)sbgr.b.bblue * left));
            pd->bgreen = (byte) (green+ ((flt)sbgr.b.bgreen* left));
            pd->bred   = (byte) (red  + ((flt)sbgr.b.bred  * left));
            pd = ADDOFF(BGR*, pd, dinc);
            if ((--dnum) == 0) break;           //done with all dest points?
            have -= left;                       //use rest of source pixel next
            left = 1.0f;
            blue  = 0.0f;                       //starting new dest pixel
            green = 0.0f;
            red   = 0.0f;
            alpha = 0.0f;
            continue;
        }
    }
    return;
    #else       //-------

// =======
// Intel MMX Assembly version.

    if (cpu == 5) {
        ASM {
        mov         edx,mxshift                 ;right shift count for ratio
        mov         ebx,mxone
        mov         ecx,mxratio
        test        edx,edx
        jle         br1asm                      ;ratio wrong for fixed?

        movd        mm3,edx                     ;mm3 = right shift count
        movq        mm4,[ebx]                   ;mm4 = 1.0
        movq        mm5,[ecx]                   ;mm5 = ratio

        movq        mm6,mm5                     ;mm6 = have = ratio
        movq        mm7,mm4                     ;mm7 = left = 1.0

        mov         ebx,snum                    ;EBX = snum
        mov         ecx,dnum                    ;ECX = dnum
        mov         esi,ps
        mov         edi,pd

        pxor        mm0,mm0                     ;mm0 = BGRA accumulators = 0
        pxor        mm2,mm2                     ;mm2 = 0

        mov         edx,dinc
        sub         edi,edx                     ;will increment before using

// This version assumes all source pixels are opaque.

br1lop: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        jbe         br1end                      ;  no

br1mid: mov         al,[esi]                    ;U get source palette index
        mov         edx,bitm
        and         al,dl                       ;U
        mov         edx,one                     ;U
        jnz         br1mi1
        mov         edx,zero

br1mi1: movd        mm1,edx                     ;U
        mov         edx,bitm                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word

        shr         edx,1                       ;U advance source pixel
        jnz         br1mor
        mov         edx,0x80                    ;U
        inc         esi
br1mor: mov         bitm,edx                    ;U

        pmulhw      mm1,mm6                     ;U BGRA * have
        psubusw     mm7,mm6                     ;  left -= have
        paddusw     mm0,mm1                     ;U accumulate BGRA
        movq        mm6,mm5                     ;  have = ratio
        dec         ebx                         ;U snum -= 1
        jle         br1sub                      ;

br1two: movd        eax,mm7                     ;U left
        movd        edx,mm6                     ;  have
        cmp         eax,edx                     ;U left>have? (high words)
        ja          br1mid                      ;  yes (maybe better prediction)

br1end: mov         al,[esi]                    ;U get source palette index
        mov         edx,bitm
        and         al,dl                       ;U
        mov         edx,one                     ;U
        jnz         br1one
        mov         edx,zero

br1one: movd        mm1,edx                     ;U
        mov         edx,dinc                    ;
        punpcklbw   mm1,mm2                     ;U get BGRA words for next pixel
        psllw       mm1,7                       ;U get in signed high word
        add         edi,edx                     ;
        pmulhw      mm1,mm7                     ;U BGRA * left
        psubusw     mm6,mm7                     ;  have -= left
        paddusw     mm1,mm0                     ;U accumulate BGRA
        movq        mm7,mm4                     ;  left = 1.0
        psrlw       mm1,mm3                     ;U right shift into byte values
        pxor        mm0,mm0                     ;  BGRA accumulators = 0
        packuswb    mm1,mm1                     ;U back BGRA words to bytes
        dec         ecx                         ;
        movd        [edi],mm1                   ;U save bytes to dest pixel
        jnz         br1lop                      ;
        jmp         br1don

br1sub: shl         edx,1                       ;  just in case of rouding error
        mov         bitm,edx
        cmp         edx,0x100
        jne         br1two
        mov         edx,1
        dec         esi
        mov         bitm,edx
        jmp         br1two

br1don: emms
        }
        return;

// =======
// Intel I86 Non-MMX version.

    } else {
        ASM {
br1asm: mov     esi,ps
        mov     edi,pd
        mov     ebx,snum
        mov     ecx,dnum

        mov     left,VONE                       ;left = 1.0
        mov     eax,fratio
        mov     have,eax                        ;have = fratio
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        mov     bw,0                            ;zero high bytes
        mov     gw,0
        mov     rw,0
        mov     aw,0

bra1lp: fld     left
        fcomp   have                            ;(left > have)?
        mov     al,[esi]                        ;get source palette index
        mov     edx,bitm
        and     al,dl
        mov     edx,one                         ;get palette color for src bit
        jnz     bra1i1
        mov     edx,zero

bra1i1: fnstsw  ax
        fwait
        sahf
        jbe     bra1ed

        mov     byte ptr bw,dl                  ;come here if left>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    have                            ;bblue*have, a, r, g, b
        shr     edx,16
        mov     eax,bitm
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b

        shr     eax,1                           ;advance source pixel
        jnz     bra1mo
        mov     eax,0x80
        inc     esi
bra1mo: mov     bitm,eax

        fmul    have                            ;bgreen*have, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    have                            ;bred*have,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    have                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fld     left
        mov     eax,fratio
        fsub    have                            ;left -= have
        mov     have,eax                        ;have = fratio
        fstp    left
        dec     ebx
        jnz     bra1lp

        mov     edx,bitm                        ;back up source bit
        shl     edx,1
        cmp     edx,0x100                       ;just in case of rounding error
        jne     bra1in
        mov     edx,0x01
        dec     esi
bra1in: mov     bitm,edx
        jmp     bra1lp

bra1ed: mov     byte ptr bw,dl                  ;come here if left<=>have
        mov     byte ptr gw,dh
        fild    bw                              ;ps->bblue, a, r, g, b
        fmul    left                            ;bblue*left, a, r, g, b
        shr     edx,16
        fild    gw                              ;ps->bgreen, bblue, a, r, g, b
        fmul    left                            ;bgreen*left, bblue, a, r, g, b
        mov     byte ptr rw,dl
        mov     byte ptr aw,dh
        fild    rw                              ;ps->bred
        fmul    left                            ;bred*left,bgreen,bblue,a,r,g,b
        fild    aw                              ;ps->balpha
        fmul    left                            ;ba,br,bg,bb,a,r,g,b
        fxch    ST(3)                           ;bb,br,bg,ba,a,r,g,b
        faddp   ST(7),ST                        ;br,bg,ba,a,r,g,b+bb
        fxch    ST(1)                           ;bg,br,ba,a,r,g,b+bb
        faddp   ST(5),ST                        ;br,ba,a,r,g+bg,b+bb
        faddp   ST(3),ST                        ;ba,a,r+br,g+bg,b+bb
        faddp   ST(1),ST                        ;a+ba,r+br,g+bg,b+bb

        fxch    ST(3)                           ;b, r, g, a
        fistp   bv                              ;r, g, a
        fxch    ST(1)                           ;g, r, a
        fistp   gv                              ;r, a
        fistp   rv                              ;a
        fistp   av
        mov     al,byte ptr bv
        mov     dl,byte ptr rv
        mov     ah,byte ptr gv
        fwait
        mov     dh,byte ptr av

        mov     [edi],ax
        mov     [edi+2],dx
        dec     ecx                             ;dnum -= 1
        jz      bra1dn
        fld     have
        mov     eax,dinc
        fsub    left                            ;have -= left
        mov     left,VONE                       ;left = 1.0
        fstp    have
        add     edi,eax
        fldz                                    ;blue=0
        fldz                                    ;green=0, blue
        fldz                                    ;red=0, green, blue
        fldz                                    ;alpha=0, red, green, blue
        jmp     bra1lp

bra1dn:
        }
        return;
    }
    #endif      //-------
}

//*************************************************************************
//  BitsText:
//      Allows for OS-independent text to be written onto TriGL bitmaps.
//      Uses a simple mono-spaced font image of Ascii chars 0x20 - 0x59
//      which must be saved as a PNG file with background transparency
//      and white foreground characters. Requires PNG so /D "NOPNG" diables.
//
//  Arguments:
//      errc BitsText (int mode, BFont* pfont, BMAP* pmap, lint x, lint y,
//                     lint color, text* ptxt, lint ctxt, IPt* psiz)
//      mode    0 = Close pfont.                                     (BT_CLOSE)
//              1 = Open ptxt font file into pfont.                  (BT_OPEN)
//                  Does NOT close old pfont first. If ptxt is NULL,
//                  then uses TriFont_T9.png in exe's dir.
//                  +256 uses TriFont_T12.png larger size.
//                  Must later call mode 0 to free resources.
//              2 = Write ptxt text to x,y on given target pmap.     (BT_TEXT)
//              3 = Write text with one pixel grey drop shadow.      (BT_TEXTS)
//              4 = Write text with surround white feathering.       (BT_TEXTF)
//              5 = Just get pixel extent of ptxt text to *psiz.     (BT_EXTENT)
//           +256 = Add to mode 1 to use larger TriFont_T12.png font (BT_BIG)     
//      pfont   Caller supplied structure for open font.
//      pmap    Target bitmap for modes 2 and 3.
//      x,y     Upper left corner where text is placed in pmap target (pixels).
//      color   Target color for mode 2. Maybe 0 to use font's default color.
//      ptxt    Text to be drawn for modes 2,3,4,5.
//              Font path name for mode 1 or NULL for default.
//      ctxt    Number of characters in ptxt to use or 0 if zero-terminated.
//      psiz    Returns X and Y pixel extent of drawn text for modes 2,3,4,5.
//              Returns max single character width and height for mode 1.
//              May be NULL if not needed.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc BitsText (int mode, BFont* pfont, BMAP* pmap, lint x, lint y,
                 lint color, text* ptxt, lint ctxt, IPt* psiz) {
    #ifdef NOPNG                                //---
    return(ECCANNOT);
    #else                                       //---
    text name[SZPATH];
    text exe[SZPATH];
    fStm stm;
    IBox box;
    IPt pos;
    int idx;
    errc e;

    switch (mode & BT_MASK) {
    case BT_TEXT:                               //draw given text to bitmap?
        if (pfont->bfMap.bmPtr == NULL) return(ECNOINIT);
        if (ctxt == 0) {
            ctxt = OSTxtSize(ptxt) - 1;
        }
        if (psiz) {
            psiz->ptx = ctxt * pfont->bfWide;
            psiz->pty = pfont->bfHigh;
        }
        if (  (color)
            &&(color != pfont->bfColor)  ) {    //must change color?
            e = BitsAlphaFill(4, &pfont->bfMap, NULL, 255, BitsColor(pmap, color));
            if (e) return(e);
            pfont->bfColor = color;
        }
        pos.pty = y;
        pos.ptx = x;
        box.ytop = 0;
        box.ybottom = pfont->bfHigh;
        while (ctxt--) {                        //go through characters
            idx = (*ptxt++) - BF_CHARBEG;       //index to char in font file
            if ((idx < 0)||(idx >= BF_CHARCNT)) {
                idx = 0;                        //substitute space if non-std
            }                                   //character not in our font
            box.xleft = idx * pfont->bfWide;
            box.xright = box.xleft + pfont->bfWide;
            BitsCopy(0, pmap, &pos, &pfont->bfMap, &box);
            pos.ptx += pfont->bfWide;           //copy box for given character
        }
        return(0);

    case BT_TEXTS:                              //draw shadowed text?
        BitsText(BT_TEXT, pfont, pmap, x+1, y+1, CGREY, ptxt, ctxt, NULL);
        e = BitsText(BT_TEXT, pfont, pmap, x, y, color, ptxt, ctxt, psiz);
        return(e);
        
    case BT_TEXTF:                              //draw feathered text?
        BitsText(BT_TEXT, pfont, pmap, x-1, y, CSILVER, ptxt, ctxt, NULL);
        BitsText(BT_TEXT, pfont, pmap, x+1, y, CSILVER, ptxt, ctxt, NULL);
        BitsText(BT_TEXT, pfont, pmap, x, y-1, CSILVER, ptxt, ctxt, NULL);
        BitsText(BT_TEXT, pfont, pmap, x, y+1, CSILVER, ptxt, ctxt, NULL);
        e = BitsText(BT_TEXT, pfont, pmap, x, y, color, ptxt, ctxt, psiz);
        return(e);

    case BT_EXTENT:                             //get get pixel extent?
        if (ctxt == 0) {
            ctxt = OSTxtSize(ptxt) - 1;
        }
        if (psiz) {
            psiz->ptx = ctxt * pfont->bfWide;
            psiz->pty = pfont->bfHigh;
        }
        return(0);

    case BT_OPEN:                               //open font bitmap?
        OSMemClear(pfont, sizeof(BFont));
        if (ptxt == NULL) {                     //use default file?
            OSLocation(exe, OS_LOCEXE);         //in exe's dir
            ptxt = name + OSFileDir(name, exe); //named TriFont_T9.png (or T12)
            OSTxtCopy(ptxt, (text*)((mode & BT_BIG) ? BF_FBIG : BF_FILE));
            ptxt = name;
        }
        e = stm.StmOpen(OS_READ, ptxt);
        if (e) return(e);
        idx = ((pmap)&&(pmap->bmFlg & RGBAPIX)) ? JM_DORGB : 0;
        e = PNGRead(idx, &stm, &pfont->bfMap);
        if (e) return(e);
        pfont->bfWide = pfont->bfMap.bmX / BF_CHARCNT;
        pfont->bfHigh = pfont->bfMap.bmY;
        if (psiz) {
            psiz->ptx = pfont->bfWide;
            psiz->pty = pfont->bfHigh;
        }
        return(0);

    case BT_CLOSE:                              //close opened bitmap?
        BitsFree(&pfont->bfMap);
        return(0);
    }
    return(ECARGUMENT);
    #endif                                      //---
}

//*************************************************************************
//  DrawStd:
//      Converts a CRGB or CBGR color into a standard color index if possible.
//
//  Arguments:
//      lint DrawStd (lint color)
//      color   CRGB+RGB or CBGR+BGR color or a Standard Color number.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//
//  Returns:
//      Returns the equivalent CBLACK through CCLEAR standard color.
//      Or returns CNULL=0 if there is no matching standard color.
//*************************************************************************

cslint DrawStd (lint color) {
    const lint sfar* pstd;
    lint rgb;
    int std;

    if (color & CINV) return(CCLEAR);           //all invisibles are CCLEAR
    rgb = DrawRGB(color);                       //get RGB version of color
    pstd = &StdColors[CMAXIMUM];
    std = CMAXIMUM;
    do {                                        //look for matching standard
        if (rgb == *pstd--) return(std);
    } while (--std);
    return(CNULL);                              //no matching color
}

//*************************************************************************
//  DrawRGB:
//      Converts either an RGB or a BGR or a standard color number
//      into a RGB-format color with alpha=0.
//
//  Arguments:
//      lint DrawRGB (lint color)
//      color   CRGB+RGB or CBGR+BGR color or a Standard Color number.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//
//  Returns:
//      Returns the 0x00BBGGRR RGB color.
//      Always sets the top byte alpha value to zero.
//*************************************************************************

cslint DrawRGB (lint color) {
    LHUE hue;

    if (!(color & 0xFFFFFF00)) {                //standard color number?
        return( StdColors[color] );
    }
    if (color & CBGR) {                         //convert BGR to RGB?
        hue.r.ralpha = 0;
        hue.r.rblue  = (*(BGR*)(&color)).bblue;
        hue.r.rgreen = (*(BGR*)(&color)).bgreen;
        hue.r.rred   = (*(BGR*)(&color)).bred;
        return(hue.d);
    }
    return(color & RGB_MASK);                   //zero alpha in RGB color
}

//*************************************************************************
//  DrawBGR:
//      Converts either an RGB or BGR or a standard color number
//      into a BGR-format color with alpha=255.
//
//  Arguments:
//      void DrawBGR (BGR* pbgr, lint color)
//      pbgr    Where the BGR color value is returned.
//              Always returns pbgr->balpha set to 255 (fully opaque).
//      color   CRGB+RGB or CBGR+BGR color or a Standard Color number.
//              See CDraw.h for standardard colors and RGB and BGR structures.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid DrawBGR (BGR* pbgr, lint color) {
    if (!(color & ALPHA_MASK)) {                //standard color number?
        color = StdColors[color];
    }
    if (color & CBGR) {                         //already in BGR format?
        ((LHUE*)pbgr)->d = color;
    } else {                                    //convert RGB to BGR format
        pbgr->bblue  = (*(RGB*)(&color)).rblue;
        pbgr->bgreen = (*(RGB*)(&color)).rgreen;
        pbgr->bred   = (*(RGB*)(&color)).rred;
    }
    pbgr->balpha = (color & CINV) ? 0x00 : 0xFF;//return alpha=255 unless clear
    return;
}

//*************************************************************************
//  DrawHLS:
//      Converts Hue, Lightness, Saturation color to RGB.
//      Algorithm from Foley page 596.
//
//  Arguments:
//      lint DrawHLS (flx hue, flx lum, flx sat)
//      hue     Hue value: 0-360 degrees.
//      lum     Luminosity: 0.0 - 1.0 fraction.
//      sat     Saturation: 0.0 - 1.0 fraction.
//
//  Returns:
//      RGB color value with CRGB alpha value.
//*************************************************************************

cslint DrawHLS (flx hue, flx lum, flx sat) {
    LHUE bgr, color;

    DrawHLS2BGR(&bgr.b, (flt)hue, (flt)lum, (flt)sat);
    color.r.rred   = bgr.b.bred;
    color.r.rgreen = bgr.b.bgreen;
    color.r.rblue  = bgr.b.bblue;
    color.r.ralpha = CRGB_BYTE;
    return(color.d);
}

//*************************************************************************
//  DrawHLS2BGR:
//      Converts Hue, Lightness, Saturation color to BGR.
//      Algorithm from Foley page 596.
//
//  Arguments:
//      lint DrawHLS2BGR (BGR* pbgr, flt hue, flt lum, flt sat)
//      pbgr    Returns BGR color value with unchanged alpha.
//      hue     Hue value: 0-360 degrees.
//      lum     Luminosity: 0.0 - 1.0 fraction.
//      sat     Saturation: 0.0 - 1.0 fraction.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid DrawHLS2BGR (BGR* pbgr, flt hue, flt lum, flt sat) {
    flt red, green, blue, m1, m2;
    lint huev, ival;
    FLTTEMP(temp);

    if (FLTZERO(sat)) {
        red   = lum;
        green = lum;
        blue  = lum;

    } else {
        m2 = (lum <= 0.5f) ? (lum + (lum * sat)) : ((lum + sat) - (lum * sat));
        m1 = (2.0f * lum) - m2;

        FLT2INT(huev, hue, temp);
        huev = huev + 120;
        if      (huev > 360) huev -= 360;
        else if (huev < 0)   huev += 360;
        if        (huev < 60) {
            red = m1 + ((m2 - m1) * (flt)huev * (1.0f/60.0f));
        } else if (huev < 180) {
            red = m2;
        } else if (huev < 240) {
            red = m1 + ((m2 - m1) * (240.0f - (flt)huev) * (1.0f/60.0f));
        } else {
            red = m1;
        }

        huev = huev - 120;
        if (huev < 0) huev += 360;
        if        (huev < 60) {
            green = m1 + ((m2 - m1) * (flt)huev * (1.0f/60.0f));
        } else if (huev < 180) {
            green = m2;
        } else if (huev < 240) {
            green = m1 + ((m2 - m1) * (240.0f - (flt)huev) * (1.0f/60.0f));
        } else {
            green = m1;
        }

        huev = huev - 120;
        if (huev < 0) huev += 360;
        if        (huev < 60) {
            blue = m1 + ((m2 - m1) * (flt)huev * (1.0f/60.0f));
        } else if (huev < 180) {
            blue = m2;
        } else if (huev < 240) {
            blue = m1 + ((m2 - m1) * (240.0f - (flt)huev) * (1.0f/60.0f));
        } else {
            blue = m1;
        }
    }
    red = red * 256.0f;
    FLT2INT(ival, red, temp);
    if (ival > 255) ival = 255;
    pbgr->bred   = (byte) ival;

    green = green * 256.0f;
    FLT2INT(ival, green, temp);
    if (ival > 255) ival = 255;
    pbgr->bgreen = (byte) ival;

    blue = blue* 256.0f;
    FLT2INT(ival, blue, temp);
    if (ival > 255) ival = 255;
    pbgr->bblue  = (byte) ival;
    return;
}

//*************************************************************************
//  DrawBGR2HLS:
//      Converts BGR to Hue, Lightness, Saturation values.
//      Algorithm from Foley page 595.
//
//  Arguments:
//      void DrawBGR2HLS (flt* phue, flt* plum, flt* psat, lint bgr)
//      phue    Returns Hue value: 0-360 degrees.
//      plum    Returns Luminosity: 0.0 - 1.0 fraction.
//      psat    Returns Saturation: 0.0 - 1.0 fraction.
//      bgr     BGR color value to be converted to HLS.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid DrawBGR2HLS (flt* phue, flt* plum, flt* psat, lint bgr) {
    LHUE hue;
    byte red, green, blue, delta, max, min;

    hue.d = bgr;
    blue = hue.b.bblue;
    green = hue.b.bgreen;
    red = hue.b.bred;

    max = (blue > green) ? blue : green;
    max = (max > red) ? max : red;

    min = (blue < green) ? blue : green;
    min = (min < red) ? min : red;

    *plum = ((max + min) >> 1) * (1.0f/256.0f); //lightness is average RGB val

    if (max == min) {
        *psat = 0.0f;
        *phue = 0.0f;
    } else {
        delta = max - min;
        *psat = (*plum <= 0.5f) ? ((flt)delta / (flt)(max+min))
                                : ((flt)delta / (flt)(512 - (max+min)));
        if (red == max) {
            *phue = 60.0f *         ((flt)(green - blue) / (flt)delta);
        } else if (green == max) {
            *phue = 60.0f * (2.0f + ((flt)(blue - red) / (flt)delta));
        } else {
            *phue = 60.0f * (4.0f + ((flt)(red - green) / (flt)delta));
        }
        if (*phue < 0.0f) *phue += 360.0f;
    }
    return;
}

//*************************************************************************
//  DrawAngles:
//      Transforms a series of points by a given angle.
//
//  Arguments:
//      void DrawAngles (lint mode, flt angle, IPt* pctr, IPt* ppts, lint npts)
//      mode    0 = Normal.
//              1 = Skip over any points with ESIGN X (used to start new line).
//      angle   Radian angle clockwise from horizontal pointing right.
//              Thus zero is horizontal and is equivalent to BitsCopy.
//              Pi/4 angles the bitmap 45 degrees down to the right.
//              Pi/2 angles the bitmap 90 degrees from top to bottom.
//              3*Pi/2 angles the bitmap 90 degrees from bottom to top.
//              Use negative angle for reverse transformation.
//      pctr    Center point about which the points are to be rotated.
//      ppts    Input plane pixel coordinates to be transformed.
//      npts    Number of X,Y coordinate pairs in ppts input and output.
//
//  Returns:
//      Nothing.
//*************************************************************************

csvoid DrawAngles (lint mode, flt angle, IPt* pctr, IPt* ppts, lint npts) {
    FPt fh, fv;
    IPt off, ctr;
    flt value;
    lint ival;
    FLTTEMP(temp);

    fh.fpx = (flt) OSCos((flx)angle);
    fv.fpy = fh.fpx;

    fh.fpy = (flt) OSSin((flx)angle);
    fv.fpx = -fh.fpy;

    ctr.ptx = pctr->ptx;
    ctr.pty = pctr->pty;
    while (npts--) {
        if (((unsigned)ppts->ptx == ESIGN)&&(mode & 1)) {
            ppts += 1;
            continue;
        }

        off.ptx = ppts->ptx - ctr.ptx;
        off.pty = ppts->pty - ctr.pty;

        value = ((flt)off.ptx * fh.fpx) + ((flt)off.pty * fv.fpx);
        FLT2INT(ival, value, temp);
        ppts->ptx = ctr.ptx + ival;

        value = ((flt)off.ptx * fh.fpy) + ((flt)off.pty * fv.fpy);
        FLT2INT(ival, value, temp);
        ppts->pty = ctr.pty + ival;

        ppts += 1;
    }
    return;
}

//*************************************************************************
//  DrawCircle:
//      Gets a list of X,Y pixel coordinates needed to draw a circle.
//      Can also get only the pixels needed to draw an arc from the circle
//      with given beginning and ending points.
//      Can optionally add points to the end of the existing list.
//      Can use DrawLines to add line segment points to the same output list.
//      The result can be used with BitsLines, BitsPattern or BitsPolygon.
//      Algorithm from Foley page 87.
//
//  Arguments:
//      errc DrawCircle (lint mode, ISegs* ppts,
//                       IPt* pctr, IPt* pbeg, IPt* pend)
//      mode    0 = Normal.
//              1 = Just return the required output buffer IPt point size.
//             +2 = Do not reallocate the ppts array, return error if too small.
//             +4 = Do arc clockwise from pbeg to pend even if over 180 degrees.
//                  Otherwise reverses pend and pbeg if counterclockwise.
//             +8 = Use ppts->isMax buffer size & add to end of buffer at isNpt.
//                  Otherwise ppts->isMax is ignored.
//      ppts    Returns an array of X,Y points used to draw the circle.
//              On entry, gives an array available to receive points.
//              On entry, ppts->isPtr may be NULL if no array is available.
//              On entry, if isPtr is not NULL, ppts->isNpt gives buffer size.
//              On entry, if mode +8 isMax is buffer size & add to isPtr+isNpt.
//              On return, may realloc memory if needed for mode 0.
//              On return, ppts->isNpt gives number of points returned in isPtr.
//              On return, caller owns any ppts->isPtr memory and must free it.
//              Note that for efficiency, we require the buffer to be somewhat
//              bigger than it really needs to be.  We may return an ECMEMORY
//              error if the provided buffer is estimated to be too small
//              for mode 2 even if in reality there would be enough space.
//              For whole circles, we over estimate needed size by about 5%.
//              For arcs we may over estimate needed space by up to 30%.
//              Note that for mode 1, the ppts argument is ignored.
//      pctr    X,Y pixel coordinates for center of the circle.
//      pbeg    X,Y pixel coordinates for one point on the circle.
//              This determines the radius of the circle.
//      pend    This is normall NULL for the entire circle.
//              To draw an arc, this should give the arc's end point.
//              The arc is drawn clockwise from pbeg to pend.
//              But if mode +4 not set and pbeg to pend is counterclockwise then
//              draws a clockwise arc of less than 180 deg from pend to pbeg.
//
//  Returns:
//      Non-zero code if error (ECMEMORY is the only possible error).
//      For mode 1 returns positive max buffer point size needed.
//*************************************************************************

#define CIRSMX 1024

csproc DrawCircle (lint mode, ISegs* ppts, IPt* pctr, IPt* pbeg, IPt* pend) {
    IPt octpts[CIRSMX];
    lint lolim[9];
    lint hilim[9];
    IPt* pout;
    IPt* puse;
    IPt* poct;
    IPt* pmid;
    IPt* pall;
    fvoid* pv;
    boolv almost;
    lint radius, octmax, temp, need, min, max, dif, dse, de, xx, yy;
    int octnum, begoct, endoct;

// Initialize.  Assume that we will draw all octants of the circle.

    pall = NULL;
    almost = FALSE;

    temp = 9 * sizeof(lint);
    pv = lolim;
    OSFILLD(pv, ESIGN, temp);                   //init most negative possible
    pv = hilim;
    OSFILLD(pv, EMAXI, temp);                   //init most positive possible

// Get the circle's radius.

    if (pctr->pty == pbeg->pty) {
        radius = pbeg->ptx - pctr->ptx;
        radius = ABS(radius);
    } else if (pctr->ptx == pbeg->ptx) {
        radius = pbeg->pty - pctr->pty;
        radius = ABS(radius);
    } else {
        radius = pbeg->ptx - pctr->ptx;
        radius = radius * radius;
        temp = pbeg->pty - pctr->pty;
        temp = temp * temp;
        radius = (lint) (OSSqrt((flx)radius + (flx)temp) + 0.5);
    }

// Estimate the number of points needed for one octant and for whole circle.
// We over estimate the number of points in each octant as .75 times the radius
// since the real number is Sin(45deg) ~= 0.7071.

    octmax = (radius >> 1) + (radius >> 2) + 2; //.75*radius+1 > sin(45)=0.7071
    need = octmax << 3;                         //assume times 8 for all octants

    begoct = 1;                                 //assume drawing whole circle
    endoct = 8;

// If drawing an arc with begin and end points, we need to draw clockwise.
// Use cross products to determine direction and reverse endpoints if needed.
// The normal of two vectors is given by the cross product:
//      V1xV2 = [X,Y,Z] = [Y1*Z2-Y2*Z1, Z1*X2-Z2*X1, X1*Y2-Y1*X2]
// If V1 and V2 lie on the XY plane then their Z values are zero:
//      V1xV2 = [X,Y,Z] = [Y1*0-Y2*0, 0*X2-0*X1, X1*Y2-Y1*X2]
//      V1xV2 = [0,0,Z] = [0, 01, X1*Y2-Y1*X2]
// The cross product gives the normal using the right handed rule
// (swing fingers of right hand from V1's direction to V2's direction and
// the thumb points toward the normal, up from the surface V1 and V2 define).
// Thus we look at the sign of BegX*EndY-BegY*EndX and if positive then
// the arc from Beg to End is clockwise.  Otherwise reverse them.

    if (pend) {                                 //drawing an arc?
        if (  (!(mode & 4))
            &&(((pbeg->ptx * pend->pty) - (pbeg->pty * pend->ptx)) < 0)  ) {
            puse = pbeg;                        //drawing counterclockwise arc?
            pbeg = pend;                        //reverse to make clockwise
            pend = puse;
        }

// We divide the circle into eight octants:
//       8 1
//      7   2
//      6   3
//       5 4
// The octant numbers with the 2 bit set (2,3,6,7) are mostly vertical.
// For mostly vertical octants we use min and max Y values.
// For the other octants (1,4,5,8) we use min and max X values.
// The min and max values are only needed if drawing an arc with beg/end points.
// We will draw clockwise through the octants from 1 through 8.
// But if drawing an arc, we will skip portions before pbeg and after pend.
// Decide which octant the arc endpoints are in and set up limits.
// Note that octants 1,4,5,8 are mostly horizontal and so we use X limits.
// Note that octants 2,3,6,7 are mostly vertical and so we use Y limits.
// We can test for mostly-vertical octants by testing 2 in the octant number.

// Classify the arc endpoint octants and set limits within their octants.

        xx = pbeg->ptx - pctr->ptx;             //first classify arc begin point
        yy = pbeg->pty - pctr->pty;             //octant and set octant limits
        if ((xx >= 0)&&(yy < 0)) {              //octants 1 or 2?
            if (xx < (-yy)) {
                begoct = 1;
                lolim[1] = pbeg->ptx;
            } else {
                begoct = 2;
                lolim[2] = pbeg->pty;
            }
        } else if ((yy >= 0)&&(xx > 0)) {       //octants 3 or 4?
            if (yy < xx) {
                begoct = 3;
                lolim[3] = pbeg->pty;
            } else {
                begoct = 4;
                hilim[4] = pbeg->ptx;
            }
        } else if ((xx <= 0)&&(yy > 0)) {       //octants 5 or 6?
            if ((-xx) < yy) {
                begoct = 5;
                hilim[5] = pbeg->ptx;
            } else {
                begoct = 6;
                hilim[6] = pbeg->pty;
            }

        } else {                                //octants 7 or 8?
            if (yy > xx) {
                begoct = 7;
                hilim[7] = pbeg->pty;
            } else {
                begoct = 8;
                lolim[8] = pbeg->ptx;
            }
        }

        xx = pend->ptx - pctr->ptx;             //now classify the arc endpoint
        yy = pend->pty - pctr->pty;             //octant and set octant limits
        if ((xx >= 0)&&(yy < 0)) {              //octants 1 or 2?
            if (xx < (-yy)) {
                endoct = 1;
                hilim[1] = pend->ptx;
            } else {
                endoct = 2;
                hilim[2] = pend->pty;
            }
        } else if ((yy >= 0)&&(xx > 0)) {       //octants 3 or 4?
            if (yy < xx) {
                endoct = 3;
                hilim[3] = pend->pty;
            } else {
                endoct = 4;
                lolim[4] = pend->ptx;
            }
        } else if ((xx <= 0)&&(yy > 0)) {       //octants 5 or 6?
            if ((-xx) < yy) {
                endoct = 5;
                lolim[5] = pend->ptx;
            } else {
                endoct = 6;
                lolim[6] = pend->pty;
            }

        } else {                                //octants 7 or 8?
            if (yy > xx) {
                endoct = 7;
                lolim[7] = pend->pty;
            } else {
                endoct = 8;
                hilim[8] = pend->ptx;
            }
        }

// If need to draw almost entire circle from one octant back around to itself,
// we need to detect and handle this case specially.

        if ((mode & 4)&&(begoct == endoct)&&(lolim[begoct] >= hilim[begoct])) {
            almost = TRUE;
            lolim[0] = lolim[begoct];
            hilim[0] = hilim[begoct];
            if (begoct == 1) {
                lolim[0] = ESIGN;
                hilim[1] = EMAXI;
            } else if (begoct == 2) {
                lolim[0] = ESIGN;
                hilim[2] = EMAXI;
            } else if (begoct == 3) {
                lolim[0] = ESIGN;
                hilim[3] = EMAXI;
            } else if (begoct == 4) {
                hilim[0] = EMAXI;
                lolim[4] = ESIGN;
            } else if (begoct == 5) {
                hilim[0] = EMAXI;
                lolim[5] = ESIGN;
            } else if (begoct == 6) {
                hilim[0] = EMAXI;
                lolim[6] = ESIGN;
            } else if (begoct == 7) {
                hilim[0] = EMAXI;
                lolim[7] = ESIGN;
            } else {
                lolim[0] = ESIGN;
                hilim[8] = EMAXI;
            }

// Subtract unused octants from the maximum circle points needed.
// But only if not special=TRUE case which uses all octants.

        } else {
            temp = begoct - 1;
            while (TRUE) {                      //go counterclockwise beg to end
                if (temp == 0) temp = 8;        //wrap around the circle
                if (temp == endoct) break;
                need -= octmax;
                temp -= 1;
            }
        }
    }

// Just return needed buffer point size for mode 1.

    if (mode & 1) {
        return(need);
    }

// If needed, allocate an output buffer large enough for all points.

    temp = ppts->isNpt;
    if (mode & 8) {
        if (ppts->isPtr == NULL) ppts->isNpt = 0;
        temp = ppts->isMax - ppts->isNpt;
        need = need + ppts->isNpt;
    }
    if ((ppts->isPtr == NULL)||(temp < need)) {
        if (mode & 2) return(ECMEMORY);         //not allowed to allocate?
        pout = (IPt*) OSAlloc(need << IPTP2);
        if (pout == NULL) return(ECMEMORY);     //prevent realloc smaller
        if ((mode & 8)&&(ppts->isPtr)) {        //since may crash Vista
            temp = need;
            if (ppts->isNpt < temp) temp = ppts->isNpt;
            if (temp > 0) OSMemCopy(pout, ppts->isPtr, (temp << IPTP2));
        }
        if (ppts->isPtr) OSFree(ppts->isPtr);
        ppts->isPtr = pout;
        ppts->isMax = need;
    }

// We generate circle points for one octant and then use them for all octants.
// If we are drawing a limited arc, we must have a separate buffer for the
// octant one points.  We use the stack if possible for speed.

    poct = octpts;
    if (octmax > CIRSMX) {
        pall = (IPt*) OSAlloc(octmax << IPTP2);
        if (pall == NULL) return(ECMEMORY);
        poct = pall;
    }

// Generate the pixels for one octant with postive X and Y values (Foley Pg 87).

    xx = 0;
    yy = radius;
    dif = 1 - radius;
    dse = 5 - (radius << 1);
    de = 3;

    puse = poct;
    while (TRUE) {
        puse->ptx = xx;                         //remember X,Y point in array
        puse->pty = yy;
        if (yy <= xx) break;                    //done with octant?
        puse += 1;

        if (dif < 0) {                          //compute point for next time
            dif += de;
            dse += 2;
            de += 2;
        } else {
            dif += dse;
            dse += 4;
            de += 2;
            yy -= 1;
        }
        xx += 1;
    }
    pmid = puse;                                //remember last octant point

// Go through all the octants (1-8) and record the circle pixels.

    pout = ppts->isPtr;
    if (mode & 8) pout += ppts->isNpt;
    xx = pctr->ptx;                             //keep center point for speed
    yy = pctr->pty;

    octnum = begoct;
    while (TRUE) {                              //go through octants
        if (octnum == 1) {                      //do octant 1
            min = lolim[1];
            max = hilim[1];
            puse = poct;
            do {
                pout->ptx = xx + puse->ptx;
                pout->pty = yy - puse->pty;
                if ((pout->ptx >= min)&&(pout->ptx <= max)) {
                    pout += 1;
                }
                puse += 1;
            } while (puse < pmid);

        } else if (octnum == 2) {               //do octant 2
            min = lolim[2];
            max = hilim[2];
            puse = pmid;
            do {
                pout->ptx = xx + puse->pty;
                pout->pty = yy - puse->ptx;
                if ((pout->pty >= min)&&(pout->pty <= max)) {
                    pout += 1;
                }
                puse -= 1;
            } while (puse > poct);

        } else if (octnum == 3) {               //do octant 3
            min = lolim[3];
            max = hilim[3];
            puse = poct;
            do {
                pout->ptx = xx + puse->pty;
                pout->pty = yy + puse->ptx;
                if ((pout->pty >= min)&&(pout->pty <= max)) {
                    pout += 1;
                }
                puse += 1;
            } while (puse < pmid);

        } else if (octnum == 4) {               //do octant 4
            min = lolim[4];
            max = hilim[4];
            puse = pmid;
            do {
                pout->ptx = xx + puse->ptx;
                pout->pty = yy + puse->pty;
                if ((pout->ptx >= min)&&(pout->ptx <= max)) {
                    pout += 1;
                }
                puse -= 1;
            } while (puse > poct);

        } else if (octnum == 5) {               //do octant 5
            min = lolim[5];
            max = hilim[5];
            puse = poct;
            do {
                pout->ptx = xx - puse->ptx;
                pout->pty = yy + puse->pty;
                if ((pout->ptx >= min)&&(pout->ptx <= max)) {
                    pout += 1;
                }
                puse += 1;
            } while (puse < pmid);

        } else if (octnum == 6) {               //do octant 6
            min = lolim[6];
            max = hilim[6];
            puse = pmid;
            do {
                pout->ptx = xx - puse->pty;
                pout->pty = yy + puse->ptx;
                if ((pout->pty >= min)&&(pout->pty <= max)) {
                    pout += 1;
                }
                puse -= 1;
            } while (puse > poct);

        } else if (octnum == 7) {               //do octant 7
            min = lolim[7];
            max = hilim[7];
            puse = poct;
            do {
                pout->ptx = xx - puse->pty;
                pout->pty = yy - puse->ptx;
                if ((pout->pty >= min)&&(pout->pty <= max)) {
                    pout += 1;
                }
                puse += 1;
            } while (puse < pmid);

        } else {                                //do octant 8
            min = lolim[8];
            max = hilim[8];
            puse = pmid;
            do {
                pout->ptx = xx - puse->ptx;
                pout->pty = yy - puse->pty;
                if ((pout->ptx >= min)&&(pout->ptx <= max)) {
                    pout += 1;
                }
                puse -= 1;
            } while (puse > poct);
        }

        if (octnum == endoct) {                 //finished last octant?
            if (almost != TRUE) break;
            almost = TRUENEG;                   //handle special case octant
            lolim[octnum] = lolim[0];           //by doing twice, once to begin
            hilim[octnum] = hilim[0];           //and again after rest of circle
        }
        octnum += 1;                            //on to next octant
        if (octnum > 8) octnum = 1;             //wrap around the circle
    }

// All done.

    if (pall) OSFree(pall);
    ppts->isNpt = (lint)(pout - ppts->isPtr);   //return number of output points
    return(0);
}

//*************************************************************************
//  DrawOval:
//      Gets a list of X,Y pixel coordinates needed to draw an oval.
//      Derives the oval by stretching a circle in X or Y dimension.
//
//      Can optionally add points to the end of the existing list.
//      Can use DrawLines to add line segment points to the same output list.
//      The result can be used with BitsLines, BitsPattern or BitsPolygon.
//      Algorithm from Foley page 87.
//
//  Arguments:
//      errc DrawOval (lint mode, ISegs* ppts, IPt* pctr, IPt* poff)
//      mode    0 = Normal.
//              1 = Just return the required output buffer IPt point size.
//             +2 = Do not reallocate the ppts array, return error if too small.
//             +8 = Use ppts->isMax buffer size & add to end of buffer at isNpt.
//                  Otherwise ppts->isMax is ignored.
//      ppts    Returns an array of X,Y points used to draw the oval.
//              On entry, gives an array available to receive points.
//              On entry, ppts->isPtr may be NULL if no array is available.
//              On entry, if isPtr is not NULL, ppts->isNpt gives buffer size.
//              On entry, if mode +8 isMax is buffer size & add to isPtr+isNpt.
//              On return, may realloc memory if needed for mode 0.
//              On return, ppts->isNpt gives number of points returned in isPtr.
//              On return, caller owns any ppts->isPtr memory and must free it.
//              Note that for efficiency, we require the buffer to be somewhat
//              bigger than it really needs to be.  We may return an ECMEMORY
//              error if the provided buffer is estimated to be too small
//              for mode 2 even if in reality there would be enough space.
//              For whole circles, we over estimate needed size by about 5%.
//              For arcs we may over estimate needed space by up to 30%.
//              Note that for mode 1, the ppts argument is ignored.
//      pctr    X,Y pixel coordinates for center of the oval.
//      poff    X,Y pixel coordinates in  lower-right corner of a rectangle
//              which defines the oval.  (Actually can be any corner.)
//
//  Returns:
//      Non-zero code if error (ECMEMORY is the only possible error).
//      For mode 1 returns positive max buffer point size needed.
//*************************************************************************

csproc DrawOval (lint mode, ISegs* ppts, IPt* pctr, IPt* poff) {
    IPt* ppt;
    IPt delt, corn;
    lint center, cnt;
    boolv stretchx;
    flt grow;
    errc e;

    corn.ptx = pctr->ptx;
    corn.pty = pctr->pty;
    delt.ptx = poff->ptx - corn.ptx;
    delt.ptx = ABS(delt.ptx);
    delt.pty = poff->pty - corn.pty;
    delt.pty = ABS(delt.pty);
    if ((delt.ptx == 0)||(delt.pty == 0)) return(ECARGUMENT);

    if (delt.ptx >= (delt.pty << 3)) {          //circumscribed circle radius
        stretchx = FALSE;                       //super wide?
        grow = (flt)delt.pty / (flt)delt.ptx;
        corn.ptx += delt.ptx;

    } else if (delt.ptx >= delt.pty) {          //normally wide?
        stretchx = TRUE;
        grow = (flt)delt.ptx / (flt)delt.pty;
        corn.ptx += delt.pty;

    } else if (delt.pty >= (delt.ptx << 3)) {   //super narrow?
        stretchx = TRUE;
        grow = (flt)delt.ptx / (flt)delt.pty;
        corn.ptx += delt.pty;

    } else {                                    //normally narrow?
        stretchx = FALSE;
        grow = (flt)delt.pty / (flt)delt.ptx;
        corn.ptx += delt.ptx;
    }

    e = DrawCircle(mode, ppts, pctr, &corn, NULL);
    if (e) return(e);                           //get points for a circle

    ppt = ppts->isPtr;
    cnt = ppts->isNpt;
    if (cnt == 0) return(0);

    if (stretchx) {                             //stretch circle to make oval
        center = pctr->ptx;
        do {
            ppt->ptx = (lint)(((flt)(ppt->ptx - center) * grow) + 0.5f) + center;
            ppt += 1;
        } while (--cnt);

    } else {
        center = pctr->pty;
        do {
            ppt->pty = (lint)(((flt)(ppt->pty - center) * grow) + 0.5f) + center;
            ppt += 1;
        } while (--cnt);
    }
    return(0);
}

//*************************************************************************
//  DataHues:
//      Converts 32-bit integer values to shaded color values.
//
//  Arguments:
//      errc DataHues (int mode, lint* pdat, lint npts, LHUE* phue, IPt plim)
//      mode    0 = Two color blending
//              1 = Three color blending (TRI_HUES)
//      pdat    32-bit integer data on entry.
//              BGRA or RGBA color values on return.
//              Assumes BGRA pixel colors but works with RGBA if phue uses.
//      npts    Number of data points in pdat.
//      phue    Must point to two or three LHUE colors
//              for low and high data values or low, mid, high.
//              These must already be in the final BGRA or RGBA pixel format
//              and caller should have already used BitsColor if needed.
//              If NULL then uses grey scale.
//      plim    The ptx gives lowest 32-bit integer value for source data.
//              The pty gives highest 32-bit integer vlaue for source data.
//              If NULL uses 24 bit range.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc DataHues (int mode, lint* pdat, lint npts, LHUE* phue, IPt* plim) {
    lint dlow, dmid, ddlt;
    lint dval, blow, glow, rlow, bmid, gmid, rmid, ival;
    flt rdlt, gdlt, bdlt, rtop, gtop, btop, fval;
    LHUE hue;
    FLTTEMP(temp);

    if (phue) {                                 //get color limits
        blow = phue->b.bblue;
        glow = phue->b.bgreen;
        rlow = phue->b.bred;
        phue += 1;
        if (mode & TRI_HUES) {
            bmid = phue->b.bblue;
            gmid = phue->b.bgreen;
            rmid = phue->b.bred;
            phue += 1;
        }
        bdlt = (flt) phue->b.bblue;
        gdlt = (flt) phue->b.bgreen;
        rdlt = (flt) phue->b.bred;
    } else {
        blow = glow = rlow = 0;                 //two-color grey scale?
        bdlt = gdlt = rdlt = 255.0f;
        mode &= (~1);
    }
    if (plim) {
        dlow = plim->ptx;
        ddlt = plim->pty;
    } else {
        ddlt = 0x7FFFFF;
        dlow = -ddlt;
    }
    ddlt = ddlt - dlow;
    if (ddlt <= 0) return(ECARGUMENT);
    fval = 1.0f / (flt) ddlt;
    hue.d = ALPHA_MASK;

// Handle three colors.

    if (mode & TRI_HUES) {
        dmid = dlow + (ddlt >> 1);
        fval = fval * 2.0f;                     //half the range for each half

        btop = (bdlt - (flt)bmid) * fval;       //get delta per source value
        gtop = (gdlt - (flt)gmid) * fval;       //if in upper half of range
        rtop = (rdlt - (flt)rmid) * fval;

        bdlt = (bmid - (flt)blow) * fval;       //get delta per source value
        gdlt = (gmid - (flt)glow) * fval;       //if in lower half of range
        rdlt = (rmid - (flt)rlow) * fval;

        do {                                    //go though data
            if (*pdat >= dmid) {                //upper half?
                dval = *pdat - dmid;            //get delta from limit floor

                fval = (flt)dval * btop;        //convert to blue color
                FLT2INT(ival, fval, temp);
                ival += bmid;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bblue = (byte) ival;

                fval = (flt)dval * gtop;        //and green color
                FLT2INT(ival, fval, temp);
                ival += gmid;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bgreen = (byte) ival;

                fval = (flt)dval * rtop;        //and red color
                FLT2INT(ival, fval, temp);
                ival += rmid;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bred = (byte) ival;

            } else {                            //lower half?
                dval = *pdat - dlow;            //get delta from limit floor

                fval = (flt)dval * bdlt;        //convert to blue color
                FLT2INT(ival, fval, temp);
                ival += blow;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bblue = (byte) ival;

                fval = (flt)dval * gdlt;        //and green color
                FLT2INT(ival, fval, temp);
                ival += glow;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bgreen = (byte) ival;

                fval = (flt)dval * rdlt;        //and red color
                FLT2INT(ival, fval, temp);
                ival += rlow;
                if (ival < 0) ival = 0;
                if (ival > 255) ival = 255;
                hue.b.bred = (byte) ival;
            }

            *pdat++ = hue.d;                    //and replace data with BGR
        } while (--npts);
        return(0);
    }

// Handle two colors.

    bdlt = (bdlt - (flt)blow) * fval;           //get delta per source value
    gdlt = (gdlt - (flt)glow) * fval;
    rdlt = (rdlt - (flt)rlow) * fval;

    do {                                        //go though data
        dval = *pdat - dlow;                    //get delta from limit floor

        fval = (flt)dval * bdlt;                //convert to blue color
        FLT2INT(ival, fval, temp);
        ival += blow;
        if (ival < 0) ival = 0;
        if (ival > 255) ival = 255;
        hue.b.bblue = (byte) ival;

        fval = (flt)dval * gdlt;                //and green color
        FLT2INT(ival, fval, temp);
        ival += glow;
        if (ival < 0) ival = 0;
        if (ival > 255) ival = 255;
        hue.b.bgreen = (byte) ival;

        fval = (flt)dval * rdlt;                //and red color
        FLT2INT(ival, fval, temp);
        ival += rlow;
        if (ival < 0) ival = 0;
        if (ival > 255) ival = 255;
        hue.b.bred = (byte) ival;

        *pdat++ = hue.d;                        //and replace data with BGR
    } while (--npts);
    return(0);
}

//*************************************************************************
//  DataPlot:
//      Used for XY plots of evenly spaced data.
//      Converts an array of integer numbers to Y pixel coordinates.
//      Interpolates array to match pixel spacing in X graph direction.
//      Places Y pixel heights in each X pixel position of the output array.
//
//      Note that this function does not actually draw.
//      It just converts a data array to pixel coordinates for easy drawing.
//      Use the BitsGraph function to actually draw the graph to a bitmap.
//
//      Compile with /D PLOTAVG to use average of sections not max/min.
//
//  Arguments:
//      errc DataPlot (int mode, lint* pout, lint* pdat, lint npts, FPt* yfac)
//      mode    Reserved for future use and must be zero.
//      pout    Output pixel Y heights are returned here.
//      nout    Desired number of output values in pout buffer.
//      pdat    Input array of integer data points.
//      npts    Number of input ata points in pdat array.
//      yfac    There are scl pixels per per input unit in Y dimension.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc DataPlot (int mode,lint* pout,lint nout,lint* pdat,lint npts,flt yfac) {
    #ifndef PLOTAVG                             //---
    lint ibest, inext, ilast, delta, dbest;
    #endif                                      //---
    lint ival, xinc, xpos, icnt, ipix;
    flt fval, fbeg, finc, xscl;
    FLTTEMP(temp);

    if (npts <= 1) return(ECFEWPTS);
    if (nout <= 1) return(ECBADNPTS);

// Handle easy case where the X scaling is one-to-one.

    if (nout == npts) {
        do {                                    //go through output pixels
            fval = (flt)(*pdat++) * yfac;
            FLT2INT(ival, fval, temp);          //scale Y output
            *pout++ = ival;
        } while (--nout);
        return(0);

// Handle case where there are more input points than output pixels.
// We must compress multiple input points to each output pixel.

    } else if (npts > nout) {
        xscl = (flt)(nout - 1) / (flt)(npts - 1);
        fval = (flt)DFONE / xscl;               //input points per out pixel
        FLT2INT(xinc, fval, temp);              //use int.frac arithmetic

        #ifndef PLOTAVG                         //---
        ilast = *pdat;
        #endif                                  //---
        xpos = xinc;                            //end of first output interval
        ipix = DFONE;                           //ipix is +DFONE so comp next
        do {                                    //go through output pixels
            #ifndef PLOTAVG                     //---
            ibest = *pdat++;
            dbest = ibest - ilast;
            dbest = ABS(dbest);
            #else                               //---
            fval = (flt) *pdat++;               //unroll first iteration
            icnt = 1;
            #endif                              //---
            if ((--npts) <= 0) pdat -= 1;       //cope if need a few more at end
            ipix += DFONE;
            while (ipix < xpos) {               //go though ins per next out pix
                #ifndef PLOTAVG                 //---
                inext = *pdat++;
                delta = inext - ilast;
                delta = ABS(delta);
                if (delta > dbest) {            //farthest from last point?
                    dbest = delta;
                    ibest = inext;
                }
                #else                           //---
                fval += (flt) *pdat++;
                icnt += 1;
                #endif                          //---
                if ((--npts) <= 0) pdat -= 1;
                ipix += DFONE;
            }
            #ifndef PLOTAVG                     //---
            fval = (flt)ibest * yfac;
            ilast = inext; 
            #else                               //---
            fval = (fval / (flt) icnt) * yfac;
            #endif                              //---
            FLT2INT(ival, fval, temp);          //scale Y output
            *pout++ = ival;
            xpos += xinc;                       //next X interval end
        } while (--nout);
        return(0);

// Handle case where there are more output pixels than input points.
// Here we must draw lines between input points.

    } else {
        xscl = (flt)(npts - 1) / (flt)(nout - 1);
        fval = (flt)DFONE / xscl;               //input points per out pixel
        FLT2INT(xinc, fval, temp);              //use int.frac arithmetic

        fbeg = (flt)(*pdat++) * yfac;
        FLT2INT(ival, fbeg, temp);
        *pout++ = ival;                         //first point
        npts -= 1;
        nout -= 1;
        if (nout == 0) return(0);
        ipix = DFONE;
        xpos = 0;
        do {                                    //go through input pixels
            icnt = 0;
            xpos += xinc;
            while (ipix <= xpos) {              //how many pixels per in point?
                ipix += DFONE;
                icnt += 1;
            }
            if (icnt > nout) icnt = nout;       //just in case
            nout -= icnt;

            fval = (flt)(*pdat++) * yfac;       //difference per output point
            finc = (fval - fbeg) / (flt)((icnt)? icnt : 1);

            do {                                //travel between input points
                fbeg += finc;                   //ramping Y values between
                FLT2INT(ival, fbeg, temp);
                *pout++ = ival;
            } while (--icnt);

            npts -= 1;
            if (npts <= 0) {                    //just in case
                npts = 1;                       //repeat last input if we
                pdat -= 1;                      //are slightly off at end
            }

            fbeg = fval;                        //current becomes last point
        } while (nout);
        return(0);
    }
}

//*************************************************************************
//  BitsGraph:
//      Does an XY plot of an array of evenly-spaced data.
//
//  Arguments:
//      errc BitsGraph (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts,
//                      lint ydelt, lint yzero, lint* pcolor)
//      mode    1 = Vertical XY plot (Y values run down pmap pbox)      GR_VER
//              2 = Horizontal XY plot                                  GR_HOR
//             +4 = Horizontal strip graph with varying color intensity GR_STRIP
//             +8 = Filled  plot colored from Y=0 up or down to Y point GR_FILL
//            +16 = Use pcolor array of color data values               GR_HUES
//                  Must be correct pbox size with bitmap colors
//                  Only supported for GR_FILL and GR_FILL+GR_CLEAR
//           +128 = Partially transparent where underlying shows thru   GR_CLEAR
//           +256 = More transparency for GR_CLEAR mode                 GR_MORE
//           +512 = Strip input data goes from -ydelt to +ydelt         GR_NEGS
//                  Otherwise strip input assumed from 0 to +ydelt
//              Not all possible mode plot types are supported yet.
//              GR_VER vertical plots cannot yet use strip, fill or clear.
//              GR_HOR horizontal plots cannot use clear unless also use fill.
//      pmap    Output bitmap where plot is drawn.
//              Must already be blanked and caller must provide any labeling.
//      pbox    Portion of pmap to be plotted.
//              May be NULL to use the whole bitmap.
//      pdat    Input array of integer data points.
//      npts    Number of data points in pdat input array.
//      ydelt   Delta input value per full Y output box height (width if vert).
//              Delta input value for full color intensity if strip plot.
//      yzero   Y position for zero input value. May be outside pbox.
//              Relative to pbox->ytop if horizontal or pbox->xleft if vert.
//              For vertical plots, this is a pixel offset in X dimension.
//      color   Color to be plotted. Unless +GR_HUES is added to the mode,
//              then *pcolor must be single color to be used for whole plot and
//              may either be a CRGB or CBGR color or a Standard Color number.
//              If +GR_HUES then must be bitmap RGBA/BGRA values
//              already reduced to pbox width or height plotted pixel size.
//
//  Returns:
//      Non-zero code if error.
//      Returns ECNOROOM if too many pixels (more than 4K).
//      Returns ECARGUMENT if unsupported mode plot type combination.
//*************************************************************************

csproc BitsGraph (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts,
                  lint ydelt, lint yzero, lint* pcolor) {
    lint outs[BLKSIZE];
    lint rows, begx, begy, lowy, offs, cntr;
    lint last, next, ival, icnt, xpix, ypix, dx, dy;
    int rval, gval, bval, rclr, gclr, bclr, cshift;
    dword ylim;
    lint* pout;
    LHUE* ppix;
    LHUE* prow;
    LHUE hue, two[2];
    lint color;
    IPt lim;
    flt fac;
    errc e;

// Get commonly needed information and convert data to pixel scale.

    if (!(mode & GR_HUES)) {
        color = BitsColor(pmap, *pcolor);       //convert color to bitmap value
    }
    rows = pmap->bmRow;

    if (pbox) {
        dx = pbox->xright - pbox->xleft;
        dy = pbox->ybottom - pbox->ytop;
        begx = pbox->xleft;
        begy = pbox->ytop;
        lowy = pbox->ybottom;
    } else {
        dx = pmap->bmX;
        dy = pmap->bmY;
        begx = 0;
        begy = 0;
        lowy = pmap->bmY;
    }
    xpix = dx;
    ypix = dy;
    if (mode & GR_VER) {                        //vertical?
        xpix = dy;
        ypix = dx;
    }
    if (xpix > BLKSIZE) return(ECNOROOM);
    ylim = (dword) ypix;

    fac = (mode & GR_STRIP) ? 1 : ((flt)ypix / ydelt);
    e = DataPlot(0, outs, xpix, pdat, npts, fac);
    if (e) return(e);                           //scale to pmap pixels

// Jump to plot type handler.

    switch (mode & GR_TYPE) {             
    default:
        return(ECARGUMENT);                     //bad mode?
        
// Handle horizontal XY line plot.

    case GR_HOR:
        offs = begx << SSZP2;
        pout = outs;
        cntr = xpix;
        last = *pout + yzero;
        do {
            ival = (*pout++) + yzero;           //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(lowy-ival)) + offs));
            if ((unsigned)ival < ylim) {        //clip if outside box
                ppix->d = color;                //plot the Y pixel
            }
            if (ival > last) {                  //plot down to last value?
                icnt = (ival - last) - 1;
                while (icnt--) {
                    ppix = ADDOFF(LHUE*, ppix, rows);
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color down to previous point
                    }                           //on same column
                }
            } else if (ival < last) {           //plot up to last value?
                icnt = (last - ival) - 1;
                while (icnt--) {
                    ppix = SUBOFF(LHUE*, ppix, rows);
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color up to previous point
                    }                           //on same column
                }
            }
            last = next;
            offs += SSZ;                        //next X pixel over next time
        } while (--cntr);
        return(0);

// Handle horizontal XY filled plot.

    case GR_HOR+GR_FILL:
        offs = begx << SSZP2;
        pout = outs;
        cntr = xpix;
        do {
            ival = (*pout++) + yzero;           //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(lowy-ival)) + offs));
            if ((unsigned)ival < ylim) {        //clip if outside box
                ppix->d = color;                //plot the Y pixel
            }
            if (ival > yzero) {                 //plot down to zero value?
                icnt = ival - yzero;
                do {
                    ppix = ADDOFF(LHUE*, ppix, rows);
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color down to zero point
                    }                           //on same column
                } while (--icnt);
            } else {                            //plot up to zero value?
                icnt = yzero - ival;
                while (icnt--) {
                    ppix = SUBOFF(LHUE*, ppix, rows);
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color up to zero point
                    }                           //on same column
                }
            }
            offs += SSZ;                        //next X pixel over next time
        } while (--cntr);
        return(0);

// Handle horizontal XY filled partially transparent plot.

    case GR_HOR+GR_FILL+GR_CLEAR:
        cshift = (mode & GR_MORE) ? 3 : 2;
        hue.d = color;
        rclr = hue.b.bred   >> cshift;
        gclr = hue.b.bgreen >> cshift;
        bclr = hue.b.bblue  >> cshift;

        offs = begx << SSZP2;
        pout = outs;
        cntr = xpix;
        do {
            ival = (*pout++) + yzero;           //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(lowy-ival)) + offs));
            if ((unsigned)ival < ylim) {        //clip if outside box
                rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                if ((unsigned)rval > 255) {     //blend with existing pixel color
                    rval = (rval < 0) ? 0 : 255;
                }
                hue.b.bred = (byte) rval;
                gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                if ((unsigned)gval > 255) {
                    gval = (gval < 0) ? 0 : 255;
                }
                hue.b.bgreen = (byte) gval;
                bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                if ((unsigned)bval > 255) {
                    bval = (bval < 0) ? 0 : 255;
                }
                hue.b.bblue = (byte) bval;
                ppix->d = hue.d;                //plot the Y pixel
            }
            if (ival > yzero) {                 //plot down to zero value?
                icnt = ival - yzero;
                do {
                    ppix = ADDOFF(LHUE*, ppix, rows);
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                        if ((unsigned)rval > 255) {
                            rval = (rval < 0) ? 0 : 255;
                        }                       //blend with existing pixel color
                        hue.b.bred = (byte) rval;
                        gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                        if ((unsigned)gval > 255) {
                            gval = (gval < 0) ? 0 : 255;
                        }
                        hue.b.bgreen = (byte) gval;
                        bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                        if ((unsigned)bval > 255) {
                            bval = (bval < 0) ? 0 : 255;
                        }
                        hue.b.bblue = (byte) bval;
                        ppix->d = hue.d;        //color down to zero point
                    }                           //on same column
                } while (--icnt);
            } else {                            //plot up to zero value?
                icnt = yzero - ival;
                while (icnt--) {
                    ppix = SUBOFF(LHUE*, ppix, rows);
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                        if ((unsigned)rval > 255) {
                            rval = (rval < 0) ? 0 : 255;
                        }                       //blend with existing pixel color
                        hue.b.bred = (byte) rval;
                        gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                        if ((unsigned)gval > 255) {
                            gval = (gval < 0) ? 0 : 255;
                        }
                        hue.b.bgreen = (byte) gval;
                        bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                        if ((unsigned)bval > 255) {
                            bval = (bval < 0) ? 0 : 255;
                        }
                        hue.b.bblue = (byte) bval;
                        ppix->d = hue.d;        //color up to zero point
                    }                           //on same column
                }
            }
            offs += SSZ;                        //next X pixel over next time
        } while (--cntr);
        return(0);

// Handle horizontal XY filled plot with array of colors.

    case GR_HOR+GR_FILL+GR_HUES:
        offs = begx << SSZP2;
        pout = outs;
        cntr = xpix;
        do {
            color = *pcolor++;                  //get next pixel's color
            ival = (*pout++) + yzero;           //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(lowy-ival)) + offs));
            if ((unsigned)ival < ylim) {        //clip if outside box
                ppix->d = color;                //plot the Y pixel
            }
            if (ival > yzero) {                 //plot down to zero value?
                icnt = ival - yzero;
                do {
                    ppix = ADDOFF(LHUE*, ppix, rows);
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color down to zero point
                    }                           //on same column
                } while (--icnt);
            } else {                            //plot up to zero value?
                icnt = yzero - ival;
                while (icnt--) {
                    ppix = SUBOFF(LHUE*, ppix, rows);
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color up to zero point
                    }                           //on same column
                }
            }
            offs += SSZ;                        //next X pixel over next time
        } while (--cntr);
        return(0);

// Handle horizontal XY filled partially transparent plot with array of colors.

    case GR_HOR+GR_FILL+GR_CLEAR+GR_HUES:
        cshift = (mode & GR_MORE) ? 3 : 2;

        offs = begx << SSZP2;
        pout = outs;
        cntr = xpix;
        do {
            hue.d = *pcolor++;                  //get next pixel's color
            rclr = hue.b.bred   >> cshift;
            gclr = hue.b.bgreen >> cshift;
            bclr = hue.b.bblue  >> cshift;
            ival = (*pout++) + yzero;           //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(lowy-ival)) + offs));
            if ((unsigned)ival < ylim) {        //clip if outside box
                rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                if ((unsigned)rval > 255) {     //blend with existing pixel color
                    rval = (rval < 0) ? 0 : 255;
                }
                hue.b.bred = (byte) rval;
                gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                if ((unsigned)gval > 255) {
                    gval = (gval < 0) ? 0 : 255;
                }
                hue.b.bgreen = (byte) gval;
                bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                if ((unsigned)bval > 255) {
                    bval = (bval < 0) ? 0 : 255;
                }
                hue.b.bblue = (byte) bval;
                ppix->d = hue.d;                //plot the Y pixel
            }
            if (ival > yzero) {                     //plot down to zero value?
                icnt = ival - yzero;
                do {
                    ppix = ADDOFF(LHUE*, ppix, rows);
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                        if ((unsigned)rval > 255) {
                            rval = (rval < 0) ? 0 : 255;
                        }                       //blend with existing pixel color
                        hue.b.bred = (byte) rval;
                        gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                        if ((unsigned)gval > 255) {
                            gval = (gval < 0) ? 0 : 255;
                        }
                        hue.b.bgreen = (byte) gval;
                        bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                        if ((unsigned)bval > 255) {
                            bval = (bval < 0) ? 0 : 255;
                        }
                        hue.b.bblue = (byte) bval;
                        ppix->d = hue.d;        //color down to zero point
                    }                           //on same column
                } while (--icnt);
            } else {                            //plot up to zero value?
                icnt = yzero - ival;
                while (icnt--) {
                    ppix = SUBOFF(LHUE*, ppix, rows);
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift))   + rclr;
                        if ((unsigned)rval > 255) {
                            rval = (rval < 0) ? 0 : 255;
                        }                       //blend with existing pixel color
                        hue.b.bred = (byte) rval;
                        gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift)) + gclr;
                        if ((unsigned)gval > 255) {
                            gval = (gval < 0) ? 0 : 255;
                        }
                        hue.b.bgreen = (byte) gval;
                        bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift))  + bclr;
                        if ((unsigned)bval > 255) {
                            bval = (bval < 0) ? 0 : 255;
                        }
                        hue.b.bblue = (byte) bval;
                        ppix->d = hue.d;        //color up to zero point
                    }                           //on same column
                }
            }
            offs += SSZ;                        //next X pixel over next time
        } while (--cntr);
        return(0);

// Handle horizontal strip plot.

    case GR_HOR+GR_STRIP:
        two[1].d = color;
        two[0].d = color;
        two[0].b.bred   = 255 - two[0].b.bred;  //blend from opposite color
        two[0].b.bgreen = 255 - two[0].b.bgreen;
        two[0].b.bblue  = 255 - two[0].b.bblue;
        lim.pty = ydelt;
        lim.ptx = (mode & GR_NEGS) ? (-ydelt) : 0;
        e = DataHues(0, outs, xpix, two, &lim); //convert pixels to colors
        if (e) return(e);

        ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*begy) + (begx << SSZP2)));
        icnt = xpix << SSZP2;
        cntr = ypix;                            //go through output rows
        do {
            OSMemCopy(ppix, outs, icnt);        //copy row of pixel colors
            ppix = ADDOFF(LHUE*, ppix, rows);
        } while (--cntr);
        return(0);

// Handle horizontal partially transparent strip plot.

    case GR_HOR+GR_STRIP+GR_CLEAR:
        cshift = (mode & GR_MORE) ? 3 : 2;

        two[1].d = color;
        two[0].d = color;
        two[0].b.bred   = 255 - two[0].b.bred;  //blend from opposite color
        two[0].b.bgreen = 255 - two[0].b.bgreen;
        two[0].b.bblue  = 255 - two[0].b.bblue;
        lim.pty = ydelt;
        lim.ptx = (mode & GR_NEGS) ? (-ydelt) : 0;
        e = DataHues(0, outs, xpix, two, &lim); //convert pixels to colors
        if (e) return(e);

        prow = ADDOFF(LHUE*, pmap->bmTop, ((rows*begy) + (begx << SSZP2)));
        cntr = ypix;                            //go through output rows
        do {
            pout = outs;
            ppix = prow;
            icnt = xpix;
            do {
                hue.d = *pout++;
                rval = (int)(ppix->b.bred   - (ppix->b.bred >> cshift));
                rval += (int)(hue.b.bred >> cshift);
                if ((unsigned)rval > 255) {     //blend with existing pixel color
                    rval = (rval < 0) ? 0 : 255;
                }
                hue.b.bred = (byte) rval;

                gval = (int)(ppix->b.bgreen - (ppix->b.bgreen >> cshift));
                gval += (int)(hue.b.bgreen >> cshift);
                if ((unsigned)gval > 255) {
                    gval = (gval < 0) ? 0 : 255;
                }
                hue.b.bgreen = (byte) gval;

                bval = (int)(ppix->b.bblue  - (ppix->b.bblue >> cshift));
                bval += (int)(hue.b.bblue >> cshift);
                if ((unsigned)bval > 255) {
                    bval = (bval < 0) ? 0 : 255;
                }
                hue.b.bblue = (byte) bval;

                ppix->d = hue.d;                //overlay strip color
                ppix += 1;
            } while (--icnt);
            prow = ADDOFF(LHUE*, prow, rows);
        } while (--cntr);
        return(0);

// Handle vertical XY line plot.

    case GR_VER:
        offs = 0;
        pout = outs;
        cntr = xpix;
        last = *pout + yzero + begx;
        do {
            ival = (*pout++) + yzero + begx;    //next Y pixel box offset
            next = ival;
            ppix = ADDOFF(LHUE*, pmap->bmTop, ((rows*(offs+begy))+(ival<<SSZP2)));
            if ((unsigned)ival < ylim) {        //clip if outside box
                ppix->d = color;                //plot the Y pixel
            }
            if (ival > last) {                  //plot down to last value?
                icnt = (ival - last) - 1;
                while (icnt--) {
                    ppix -= 1;
                    ival -= 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color down to previous point
                    }                           //on same column
                }
            } else if (ival < last) {           //plot up to last value?
                icnt = (last - ival) - 1;
                while (icnt--) {
                    ppix += 1;
                    ival += 1;
                    if ((unsigned)ival < ylim) {
                        ppix->d = color;        //color up to previous point
                    }                           //on same column
                }
            }
            last = next;
            offs += 1;                          //next X pixel over next time
        } while (--cntr);
        return(0);
    }
}

//*************************************************************************
//  Bits2DAdd:
//      Colorizes and adds a scan line to end of 2D data bitmap.
//      Can also add a overlaid partially transparent graph on second call.
//      Normally adds from left to right as with the GPR display standard.
//      Limits npts and bitmap height to BLKSIZE (4096) max.
//
//  Arguments:
//      errc Bits2DAdd (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts, 
//                      LHUE* phue, IPt* plim)
//      mode    0 = Two color blending
//              1 = Three color blending                    (TRI_HUES)
//             +4 = Okay to modify pdat data for speed      (TRI_MODSRC)
//             +8 = Return error rather than grow pmap      (TRI_NOGROW)
//            +16 = Add scans to right rather than left     (TRI_RIGHT)
//            +64 = Supply bitmap column pix ready to use   (TRI_READY)
//                  The pdata must point to BGRA or RGBA
//           +128 = Partially transparent graph at bottom   (TRI_CLEAR)
//                  The npts gives graph height for point
//                  and *phue gives graph color at point
//                  Only use just after normal Bits2DAdd
//           +256 = More transparency for TRI_CLEAR mode    (TRI_MORE)
//           +512 = More transparency for TRI_CLEAR mode    (TRI_LESS)
//      pmap    Target bitmap where column data is written.
//              Automatically converted to 32 bits/pixel if needed.
//              The pmap->bmPtr may be NULL to create target bitmap.
//              The pmap->bmPtX is used to keep track last column used.
//              If pmap->bmPtr is NULL the creates initial bitmap.
//      pbox    Allows writing to only a portion of the vertical bitmap.
//              The ytop gives the top Y where writing starts.
//              The ybottom gives just past the bottom Y where writing stops.
//              The xleft and xright members are currently ignored
//              and added lines are always the rightmost bitmaps column,
//              May be NULL to target the entire bitmap vertically.
//      pdat    32-bit integer data on entry.
//              Unless TRI_READY values are colored as BGRA/RGBA pixels.
//              Assumes BGRA pixel colors but works with RGBA if phue uses.
//              If TRI_MODSRC mode than array data is modified on return.
//              Ignored for TRI_CLEAR mode.
//      npts    Number of data points in pdat.
//              Must be curent graph height for TRI_CLEAR mode.
//      phue    Must point to two or three LHUE colors
//              for low and high data values or low, mid, high.
//              These must already be in the final BGRA or RGBA pixel format
//              and caller should have already used BitsColor if needed.
//              If NULL then uses grey scale.
//              Must be single graph color for TRI_CLEAR mode.
//      plim    The ptx gives lowest 32-bit integer value for source data.
//              The pty gives highest 32-bit integer value for source data.
//              If NULL uses 24 bit range.
//              Ignored for TRI_CLEAR mode.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc Bits2DAdd (int mode, BMAP* pmap, IBox* pbox, lint* pdat, lint npts,
                  LHUE* phue, IPt* plim) {
    lint one[BLKSIZE];
    lint two[BLKSIZE];
    BMAP bmap;
    word mxratio[4];
    word mxone[4];
    lint mxshift, iratio, offs, ndes;
    flt fratio;
    int cpu;
    IPt to;
    errc e;

    if (pmap->bmPtr == NULL) {                  //create target bitmap?
        OSMemCopy(&bmap, pmap, sizeof(BMAP));
        if (pmap->bmY == 0) pmap->bmY = npts;
        e = BitsMake(pmap, 32, ADD_COLS, pmap->bmY, NULL);
        if (e) return(e);
        pmap->bmPtX = bmap.bmX;
        pmap->bmPtY = bmap.bmPtY;
        BitsBlank(pmap, NULL, CWHITE);
    }
    if (pmap->bmPix != 32) {                    //convert to 32 bits/pixel?
        OSMemClear(&bmap, sizeof(BMAP));
        bmap.bmPix = 32;
        e = BitsCopy(4, &bmap, NULL, pmap, NULL);
        if (e) return(e);
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }
    if (pmap->bmPtX >= pmap->bmX) {             //need to grow bitmap?
        offs = (pmap->bmFlg & RGBAPIX) ? (-32) : 32;
        e = BitsMake(&bmap, offs, pmap->bmX + ADD_COLS, pmap->bmY, NULL);
        if (e) return(e);                       //create larger bitmap
        BitsBlank(&bmap, NULL, CWHITE);
        to.ptx = (mode & TRI_RIGHT) ? 0 : ADD_COLS;
        to.pty = 0;
        e = BitsCopy(0, &bmap, &to, pmap, NULL);
        if (e) return(e);                       //and copy former
        bmap.bmPtX = pmap->bmPtX;
        bmap.bmPtY = pmap->bmPtY;
        BitsFree(pmap);
        OSMemCopy(pmap, &bmap, sizeof(BMAP));
        pmap->bmFlg |= NEWINFO;
    }
       
    if (pbox) {
        offs = pbox->ytop;
        if ((offs < 0)||(offs >= pmap->bmY)) return(0);
        ndes = pbox->ybottom - pbox->ytop;
        if (ndes <= 0) ndes = pmap->bmY - offs;
    } else {
        offs = 0;
        ndes = pmap->bmY;
    }

    if (mode & TRI_CLEAR) {                     //overlay graphed point at bottom?
        if (npts <= 0) npts = 1;
        if (npts > ndes) npts = ndes;
        to.pty = (offs + ndes) - npts;          //graph from bottom up
        offs = pmap->bmPtX - 1;                 //back up to previous add point
        to.ptx = (mode & TRI_RIGHT) ? (offs) : (pmap->bmX - offs);
        e = BitsColumn((mode & (TRI_CLEAR+TRI_MORE+TRI_LESS)), pmap, &to, &phue->d, ndes);
        return(e);                              //overlay last column at end
    }

    if (!(mode & TRI_READY)) {
        if (!(mode & TRI_MODSRC)) {             //don't modify caller's array
            if (npts > BLKSIZE) return(ECBADNPTS);
            OSMemCopy(one, pdat, (npts << SSZP2));
            pdat = one;
        }
        e = DataHues(mode, pdat, npts, phue, plim);
        if (e) return(e);                       //convert data to colored pixels
    }

    if (npts != ndes) {                         //target bitmap different size?
        if (ndes > BLKSIZE) return(ECBADNPTS);
        cpu = (int) BitsCPU();
        if (ndes <= npts) {
            fratio = ((flt)ndes / (flt)npts);
        } else {
            fratio = ((flt)npts / (flt)ndes);
            cpu |= 256;
        }
        iratio = (lint) (fratio * (flt)DFONE);
        BitsBlendInit(cpu, iratio, &mxshift, mxratio, mxone);

        BitsBlend((BGR*)two, ndes, 4, (BGR*)pdat, npts, 4, iratio, fratio, cpu,
                  mxshift, mxratio, mxone);     //resample pix to match target
        pdat = two;
    }

    to.ptx = (mode & TRI_RIGHT) ? pmap->bmPtX : (pmap->bmX - pmap->bmPtX);
    to.pty = offs;
    e = BitsColumn(0, pmap, &to, pdat, ndes);   //insert new column at end
    if (e) return(e);
    pmap->bmPtX += 1;
    return(0);
}

#ifndef NOJPEG                                  //===
//*************************************************************************
// JErrMgr: Used by JPEG handler below.
// Define extended error handler with setjmp_buffer at the end.

struct JErrMgr {
  struct jpeg_error_mgr pub;    //"public" fields
  jmp_buf setjmp_buffer;        //used for return to caller
};

// Static maximum image dimension for avaiable RAM to avoid huge memory freeze.

dword _MaxIDimJPG = 7200;

//*************************************************************************
//  JPEGWrite:
//      Writes a BMAP bitmap to a JPEG file image.
//
//  Arguments:
//      errc JPEGWrite (lint mode, fStm* pstream, BMAP* pmap, IBox* pbox, byte* pexifs)
//      mode    Compression mode:
//              JM_QUAL  (0xFF) Quality level mask (1=low,100=high, 0=default)
//              JM_FAST  (512)  Fastest integer computation (poor quality)
//              JM_FLOAT (1024) Floating computation (maybe faster than normal)
//              JM_ALPHA (4096) Save four-channels with alpha information
//              Beware that JM_ALPHA usually makes the file about 250% larger
//              than normal three-channel RGB (not 33% larger as expected).
//              Also JM_ALPHA writes a non-standard "unknown" colorspace file.
//              Therefore the VIB format is prefered for transparent 32bit data.
//              Add in neigher JM_FAST or JM_FLOAT for default algorithm.
//              In tests on a 192,058 byte 32-bit bitmap, the default
//              quality (0) was equivalent to somewhat less than 80 and
//              QUAL 0 = 17699 bytes, 1 = 3160, 17 = 7072, 33 = 10307,
//              50 = 13317, 67 = 15931, 83=20713, 100 = 51888 bytes.
//              The QUAL 17=6828 bytes was blocky but OK and QUAL 33 was better.
//      pstream Output stream to receive the JPEG file image.
//      pmap    Input bitmap to be written.
//      pbox    Portion of pmap to be saved or NULL to save entire image.
//      pexifs  EXIF block to save with the file beginning with byte size word.
//              May be NULL to not save EXIF.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc JPEGWrite (lint mode, fStm* pstream, BMAP* pmap, IBox* pbox, byte* pexifs) {
    #ifdef NOWRITE                              //-------
    return(ECCANNOT);
    #else                                       //-------
    struct jpeg_compress_struct cinfo;
    struct JErrMgr jerr;
    JSAMPROW row_pointer[1];
    BMAP bmap;
    BMAP btmp;
    byte* prow;
    byte* ppix;
    byte value;
    lint left, cnt, len, nx, ny, stride;
    byte chans, space;
    errc e;

    if ((pmap->bmX == 0)||(pmap->bmY == 0)) return(ECFEWPTS);

// Handle palettes by converting to 32 bits.

    btmp.bmPtr = NULL;
    if (pmap->bmPix <= 8) {                     //cannot convert directly?
        OSMemClear(&btmp, sizeof(BMAP));
        btmp.bmPix = 32;                        //then go via 32 bits
        e = BitsCopy(0, &btmp, NULL, pmap, NULL);
        if (e) return(e);
        pmap = &btmp;
    }

// Convert bitmap to 24 bits/pixel and reverse order to get R,G,B pixels.
// If doing four channels, convert to reveres 32 bits/pixel.

    OSMemClear(&bmap, sizeof(BMAP));
    if ((mode & JM_ALPHA)&&(pmap->bmPix == 32)&&(pmap->bmInv)) {
        chans = 4;                              //four channels?
        space = JCS_UNKNOWN;                    //JCS_CMYK
        bmap.bmPix = 32;
    } else {                                    //not four channels?
        chans = 3;
        space = JCS_RGB;
        bmap.bmPix = 24;
    }
    e = BitsCopy(4, &bmap, NULL, pmap, pbox);
    if (btmp.bmPtr) BitsFree(&btmp);            //maybe need 3 bytes/pixel
    if (e) return(e);
    stride = bmap.bmRow;

    if (!(pmap->bmFlg & RGBAPIX)) {
        ny = bmap.bmY;
        nx = bmap.bmX;
        left = ny;
        prow = bmap.bmTop;
        do {                                    //convert from B,G,R to R,G,B
            cnt = nx;
            ppix = prow;
            do {
                value = ppix[0];
                ppix[0] = ppix[2];
                ppix[2] = value;
                ppix += chans;
            } while (--cnt);
            prow += stride;
        } while (--left);
    }

// Set up for error return.
// Due to the JPEG library's use of exiting for errors,
// we must use setjmp() and longjmp() to handle errors.

    cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);
    jerr.pub.error_exit = JPEGErrorExit;
    if (setjmp(jerr.setjmp_buffer)) {           //establish longjmp return point
        e = StmErrors(1, ECBADFILE);
        goto error;
    }

// Allocate and initialize JPEG compression object.
// Also, specify the destination file.  Note we pretend our StmFile* is a FILE*.

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, (FILE*)pstream);

// Set parameters for compression.

    cinfo.image_width = bmap.bmX;
    cinfo.image_height = bmap.bmY;
    cinfo.input_components = chans;
    cinfo.in_color_space = (J_COLOR_SPACE) space;
    jpeg_set_defaults(&cinfo);
    if (mode & JM_FAST) cinfo.dct_method = JDCT_IFAST;
    if (mode & JM_FLOAT) cinfo.dct_method = JDCT_FLOAT;
    if (mode & JM_QUAL) {
        jpeg_set_quality(&cinfo, (int)mode & JM_QUAL, TRUE);
    }
    if (pexifs) {
        cinfo.write_JFIF_header = FALSE;        //Need EXIF to come first
    }

// Start the compressor and output the bitmap row scan lines.

    jpeg_start_compress(&cinfo, TRUE);

    if (pexifs) {                               //saving EXIF?
        len = (lint) (*((word*)pexifs));
        if (len > 0) {                          //just in case
            jpeg_write_marker(&cinfo, (JPEG_APP0 + 1), (pexifs+2), (len-2));
        }
    }

    ny = bmap.bmY;
    prow = bmap.bmTop;
    do {
        row_pointer[0] = prow;                  //write one row at a time
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        prow += stride;
    } while (--ny);

// Finish compression and release JPEG compression object.

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

// Free the temporary bitmap copy and return.

    BitsFree(&bmap);                            //free B,G,R bitmap
    return(0);

error:
    jpeg_destroy_compress(&cinfo);
    BitsFree(&bmap);                            //free B,G,R bitmap
    return(e);
    #endif                                      //-------
}

//*************************************************************************
//  JPEGRead:
//      Reads a BMAP bitmap from a JPEG file image.
//
//  Arguments:
//      errc JPEGRead (lint mode, fStm* pstream, BMAP* pmap)
//      mode    Compression mode:
//              JM_8BIT    (256) Decompress to 8 bit/pixel bitmap (rather 32)
//              JM_FAST    (512) Fastest integer computation (poor quality)
//              JM_FLOAT  (1024) Floating computation (maybe faster than normal)
//              JM_DITHER (2048) Use pixel dithering (but only when JM_8BIT)
//              JM_DORGB         Use RGB order instead of BGA in pmap.
//              Add in neigher JM_FAST or JM_FLOAT for default algorithm.
//              The JM_8BIT 8-bit mode is not recommended.
//      pstream Input stream with the JPEG file image.
//      pmap    Where output bitmap is returned.
//              This is overwritten with a newly created bitmap.
//              It must not contain an existing bitmap.
//
//  Returns:
//      Non-zero code if error.
//      The pmap output bitmap is freed and zeroed if error.
//*************************************************************************

csproc JPEGRead (lint mode, fStm* pstream, BMAP* pmap) {
    struct jpeg_decompress_struct cinfo;
    struct JErrMgr jerr;
    JSAMPARRAY pprows;
    BGR* ppal;
    byte* prow;
    byte* pout;
    byte* ppix;
    lint stride, colors, cnt, idx, nx, ny, nc, ns;
    int i0, i1, i2;
    errc e;

// Initialize in case of error.

    OSMemClear(pmap, sizeof(BMAP));

// Set up for error return.
// Due to the JPEG library's use of exiting for errors,
// we must use setjmp() and longjmp() to handle errors.

    cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);
    jerr.pub.error_exit = JPEGErrorExit;
    if (setjmp(jerr.setjmp_buffer)) {           //establish longjmp return point
        e = ECBADFILE;
        goto error;
    }

// Allocate and initialize JPEG decompression object.
// Also, specify the source file.  Note we pretend our StmFile* is a FILE*.

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, (FILE*)pstream);

// Read file parameters (including bitmap size).
// Set parameters for decompression.

    jpeg_read_header(&cinfo, TRUE);
    if ((cinfo.image_width >_MaxIDimJPG)||(cinfo.image_height > _MaxIDimJPG)) {
        if (_MaxIDimJPG >= 7200) {              //just in case
            e = ECBADSIZE;                      //try to prevent freeze (V62)
            goto error;
        }
    }

    if (mode & JM_FAST) cinfo.dct_method = JDCT_IFAST;
    if (mode & JM_FLOAT) cinfo.dct_method = JDCT_FLOAT;
    if (mode & JM_8BIT) {
        cinfo.quantize_colors = TRUE;
        cinfo.two_pass_quantize = TRUE;
        cinfo.desired_number_of_colors = 256;
        cinfo.dither_mode = (mode & JM_DITHER) ? JDITHER_FS : JDITHER_NONE;
    }

// Start the compressor and output the bitmap row scan lines.
// Create scan line buffer and output bitmap.

    jpeg_start_decompress(&cinfo);

    nx = cinfo.image_width;
    ny = cinfo.image_height;
    if (((dword)nx > _MaxIDimJPG)||((dword)ny > _MaxIDimJPG)) {
        if (_MaxIDimJPG >= 7200) {              //just in case
            e = ECBADSIZE;                      //try to prevent freeze (V62)
            goto error;
        }
    }
    nc = cinfo.output_components;
    if ((nc != 3)&&(nc != 4)&&(nc != 1)) {
        e = ECBADFILE;
        goto error;
    }
    ns = nx * nc;                               //allocate row buf and ptr to it
    pprows = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo,JPOOL_IMAGE,ns,1);

    if (mode & JM_8BIT) {
        e = BitsNew(pmap, 8, nx, ny, NULL);     //8-bit bitmap with palette
        if (e) goto error;
        ppal = pmap->bmPal;
        cnt = cinfo.actual_number_of_colors;
        if (cnt > 256) cnt = 256;
        idx = 0;
        while (idx < cnt) {                     //fill in our palette colors
            ppal->bblue = cinfo.colormap[2][idx];
            ppal->bblue = cinfo.colormap[1][idx];
            ppal->bblue = cinfo.colormap[0][idx];
            ppal->balpha = 0;
            ppal += 1;
            idx += 1;
        }
        pmap->bmUse = cnt;

    } else {
        e = BitsNew(pmap, 32, nx, ny, NULL);    //normal 32-bit bitmap
        if (e) goto error;                      //(better suited for JPEG data)
    }

    i0 = 2;
    i1 = 1;
    i2 = 0;
    if (mode & JM_DORGB) {                      //RGB?
        pmap->bmFlg |= RGBAPIX;
        i0 = 0;
        i1 = 1;
        i2 = 2;
    }

// Go though the rows and fill in our bitmap.

    pout = pmap->bmTop;
    stride = pmap->bmRow;
    do {
        jpeg_read_scanlines(&cinfo, pprows, 1); //read one row

        prow = pprows[0];
        ppix = pout;
        cnt = nx;
        if (mode & JM_8BIT) {                   //8-bit output?
            OSMemCopy(ppix, prow, cnt);
        } else if (nc == 3) {                   //RGB pixels?
            do {
                *ppix++ = prow[i0];             //blue
                *ppix++ = prow[i1];             //green
                *ppix++ = prow[i2];             //red
                *ppix++ = 0xFF;                 //alpha
                prow += 3;
            } while (--cnt);
        } else if (nc == 4) {                   //RGBA pixels?
            if ((cinfo.out_color_space == JCS_CMYK)&&(cinfo.saw_Adobe_marker)) {
                do {                            //Adobe CMYK
                    colors = prow[3];           //Adobe has bug: 0 used for 100%
                    *ppix++ = (byte) ((colors * (prow[i0])) >> 8);
                    *ppix++ = (byte) ((colors * (prow[i1])) >> 8);
                    *ppix++ = (byte) ((colors * (prow[i2])) >> 8);
                    *ppix++ = 0xFF;                 //alpha
                    prow += 4;
                } while (--cnt);
            } else if (cinfo.out_color_space == JCS_CMYK) {
                do {                            //Adobe CMYK
                    colors = 255 - prow[3];     //Ghostscript CMYK
                    *ppix++ = (byte) ((colors * (255 - prow[i0])) >> 8);
                    *ppix++ = (byte) ((colors * (255 - prow[i1])) >> 8);
                    *ppix++ = (byte) ((colors * (255 - prow[i2])) >> 8);
                    *ppix++ = 0xFF;                 //alpha
                    prow += 4;
                } while (--cnt);
            } else {
                do {                            //RGBA is never really used
                    *ppix++ = prow[i0];         //blue
                    *ppix++ = prow[i1];         //green
                    *ppix++ = prow[i2];         //red
                    *ppix++ = prow[3];          //alpha
                    if (prow[3] < 255) pmap->bmInv = CINV;
                    prow += 4;
                } while (--cnt);
            }
        } else {                                //monochrome pixels?
            do {
                *ppix++ = prow[0];              //blue
                *ppix++ = prow[0];              //green
                *ppix++ = prow[0];              //red
                *ppix++ = 0xFF;                 //alpha
                prow += 1;
            } while (--cnt);
        }
        pout += stride;
    } while (--ny);
    pmap->bmFlg |= NEWINFO;

// Finish compression and release JPEG compression object.

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return(0);

error:
    jpeg_destroy_decompress(&cinfo);
    BitsFree(pmap);
    OSMemClear(pmap, sizeof(BMAP));
    return(e);
}

// *******
// JPEGErrorExit:
// Internal routine used to override the error handler.

METHODDEF(void) JPEGErrorExit (j_common_ptr cinfo) {
    // The cinfo->err really points to a my_error_mgr struct, so coerce pointer.
    JErrMgr* myerr = (JErrMgr*) cinfo->err;

    // Always display the message.
    (*cinfo->err->output_message) (cinfo);

    // Return control to the setjmp point with a non-zero value.
    longjmp(myerr->setjmp_buffer, 1);
}
#endif                                          //=== NOJPEG

#ifndef NOPNG                                   //===
//*************************************************************************
//  my_read_data:
//  my_write_data:
//  my_flush_data:
//  png_my_error:
//  png_my_warning
//  _MaxIDimPNG:
//      Substitute routines we implement to read and write PNG files,
//      Plus PNG error handlers.
//      Plus max image size to avoid running out of RAM for bad PNG files.
//*************************************************************************

// Static maximum image dimension for avaiable RAM to avoid huge memory freeze.

dword _MaxIDimPNG = 7200;

// *******
// png_my_read: Reads from PNG file.

void my_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    StmFRead((fStm*)png_ptr->io_ptr, data, (lint)length);
    return;                                     //ignore errors
}

// *******
// png_my_write: Writes to PNG file.

errc  _PNGError = 0;
void my_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    dword done;
    done = StmFWrite((fStm*)png_ptr->io_ptr, data, (lint)length);
    if (done != length) {
        _PNGError = StmErrors(1, ECWRITE);      //remember write errors
    }
    return;
}

// *******
// png_my_read: Flush writes to disk not needed.

void my_flush_data(png_structp png_ptr) {
    return;                                     //no need to flush
}

// *******
// png_my_error: Define extended error handler with setjmp_buffer at the end.

void png_my_error (png_structp png_ptr, png_const_charp message) {
    //xprintf("ERROR(libpng): %s - %s\n", message,(char *)png_get_error_ptr(png_ptr));
    longjmp(png_ptr->jmpbuf, 1);
}

// *******
// png_my_warning: Do nothing for warnings.

void png_my_warning(png_structp png_ptr, png_const_charp message) {
    //xprintf( "WARNING(libpng): %s - %s\n", message,(char *)png_get_error_ptr(png_ptr));
    return;
}

//*************************************************************************
//  PNGWrite:
//      Writes a BMAP bitmap to a PNG file image.
//      Only handles 8 or 32 bits/pixel formats.
//
//  Arguments:
//      errc PNGWrite (lint mode, fStm* pstream, BMAP* pmap)
//      mode    Compression mode. 1-9 where 9 is highest compression.
//              Use 0 for default of 6.
//              Add in +16 for interlaced image.
//      pstream Output stream to receive the JPEG file image.
//      pmap    Input bitmap.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc PNGWrite (lint mode, fStm* pstream, BMAP* pmap) {
    #ifdef NOWRITE                              //-------
    return(ECCANNOT);
    #else                                       //-------
    byte trans[256];
    png_structp png_ptr;
    png_infop info_ptr;
    png_color_16 scx;
    png_color* pcolor;
    png_color colors[256];
    BMAP bmap;
    LHUE hue;
    BGR* ppal;
    lpbyte* pprows;
    lpbyte* pprow;
    byte* prow;
    byte* ppix;
    byte value;
    lint left, cnt, nx, ny, stride;
    int val, npal, nadd, ctype, interlace;
    errc e;

    if ((pmap->bmX == 0)||(pmap->bmY == 0)) return(ECFEWPTS);
    OSMemClear(&scx, sizeof(png_color_16));
    _PNGError = 0;

// Allocate and initialize PNG compression object.

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
                                      png_my_error, png_my_warning);
    if (png_ptr == NULL) {
        e = ECMEMORY;
        goto error;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        e = ECMEMORY;
        goto error;
    }

// Set up for error return.
// Due to the PNG library's use of exiting for errors,
// we must use setjmp() and longjmp() to handle errors.

    if (setjmp(png_ptr->jmpbuf)) {              //establish longjmp return point
        png_destroy_write_struct(&png_ptr, &info_ptr);
        e = _PNGError;                          //if get hear, library had error
        if (e == 0) e = ECBADFILE;
        goto error;
    }

// Specify the destination file.  Note we pretend or fStm* is a FILE*.
// Also Set up for compression.

    //Seems to cause warning (probably not needed to write image).
    //png_set_read_fn(png_ptr, (FILE*)pstream, my_read_data);

    png_set_write_fn(png_ptr, (FILE*)pstream, my_write_data, my_flush_data);

    //This has already been done during the png_set_write_fn call.
    //png_init_io(png_ptr, (FILE*)pstream);

    val = mode & 0xF;
    if ((val < 1)||(val > 9)) val = 6;
    png_set_compression_level(png_ptr, val);    //1-9 (6 is usual default)

    interlace = (mode & 16) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;

// *******
// Handle 8-bit bitmaps.

    if (pmap->bmPix == 8) {
        png_set_IHDR(png_ptr, info_ptr, pmap->bmX, pmap->bmY, 8,
                     PNG_COLOR_TYPE_PALETTE, interlace,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        if (pmap->bmInv) {
            val = pmap->bmInv & 0xFF;
            npal = val + 1;
            ppix = trans;
            while (val--) {
                *ppix++ = 0xFF;
            }
            *ppix = 0;
            png_set_tRNS(png_ptr, info_ptr, trans, npal, &scx);
        }
        ppal = pmap->bmPal;
        npal = (int) pmap->bmUse;
        if ((npal == 0)||(npal > 256)) npal = 256;
        pcolor = colors;
        val = npal;
        while (val--) {
            pcolor->blue = ppal->bblue;
            pcolor->green = ppal->bgreen;
            pcolor->red = ppal->bred;
            ppal += 1;
            pcolor += 1;
        }
        png_set_PLTE(png_ptr, info_ptr, colors, npal);

        png_write_info(png_ptr, info_ptr);      //write the header info

        pprows = (lpbyte*) OSAlloc(pmap->bmY * sizeof(byte*));
        pprow = pprows;
        ppix = pmap->bmTop;
        stride = pmap->bmRow;
        cnt = pmap->bmY;                        //make array of row pointers
        do {
            *pprow++ = ppix;                    //remember rows top to bottom
            ppix += stride;
        } while (--cnt);

        png_write_image(png_ptr, pprows);       //write the bitmap image
        png_write_end(png_ptr, info_ptr);

        OSFree(pprows);
        pprows = NULL;
        png_destroy_write_struct( &png_ptr, &info_ptr );
        return(_PNGError);                      //all done
    }

// *******
// Handle 32-bit bitmaps.
// Convert bitmap to 24 bits/pixel and reverse order to get R,G,B pixels.

    pprows = NULL;
    OSMemClear(&bmap, sizeof(BMAP));
    bmap.bmPix = 24;
    ctype = PNG_COLOR_TYPE_RGB;
    nadd = 3;
    if ((pmap->bmInv)&&(pmap->bmPix == 32)) {   //alpha channel transparency?
        bmap.bmPix = 32;
        ctype = PNG_COLOR_TYPE_RGB_ALPHA;
        nadd = 4;
    }
    e = BitsCopy(4, &bmap, NULL, pmap, NULL);   //get 3 bytes/pixel
    if (e) goto abter;

    stride = bmap.bmRow;
    if (!(pmap->bmFlg & RGBAPIX)) {
        ny = bmap.bmY;
        nx = bmap.bmX;
        left = ny;
        prow = bmap.bmTop;
        do {                                    //convert from B,G,R to R,G,B
            cnt = nx;
            ppix = prow;
            do {
                value = ppix[0];
                ppix[0] = ppix[2];
                ppix[2] = value;
                ppix += nadd;
            } while (--cnt);
            prow += stride;
        } while (--left);
    }

// Setup and write PNG file header.
// We always use 8-bit color values with 3 bytes per RGB pixel.

    png_set_IHDR(png_ptr, info_ptr, bmap.bmX, bmap.bmY, 8,
                 ctype, interlace,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (bmap.bmInv) {
        hue.d = bmap.bmInv;
        scx.blue = hue.b.bblue;
        scx.green = hue.b.bgreen;
        scx.red = hue.b.bred;
        trans[0] = 0;
        npal = 0;
        png_set_tRNS(png_ptr, info_ptr, trans, npal, &scx);
    }
    png_write_info(png_ptr, info_ptr);

    //We want the RGB order.
    //png_set_bgr(png_ptr);                     //set for BGR order

// Set up array of pointers to all rows.
// Writing all rows with one call lets the library handle interlacing.

    pprows = (lpbyte*) OSAlloc(bmap.bmY * sizeof(byte*));

    pprow = pprows;
    ppix = bmap.bmTop;
    cnt = bmap.bmY;
    do {
        *pprow++ = ppix;                        //remember rows top to bottom
        ppix += stride;
    } while (--cnt);

// Now write the bitmap image.

    png_write_image(png_ptr, pprows);
    png_write_end(png_ptr, info_ptr);

// Free the temporary bitmap copy and return.

    OSFree(pprows);
    pprows = NULL;
    png_destroy_write_struct( &png_ptr, &info_ptr );
    BitsFree(&bmap);                            //free B,G,R bitmap
    return(_PNGError);                          //all done

abter:
    png_destroy_write_struct(&png_ptr, &info_ptr);
error:
    if (pprows) OSFree(pprows);
    BitsFree(&bmap);                            //free B,G,R bitmap
    return(e);
    #endif                                      //-------
}

//*************************************************************************
//  PNGRead:
//      Reads a BMAP bitmap from a PNG file image.
//
//  Arguments:
//      errc PNGRead (lint mode, fStm* pstream, BMAP* pmap)
//      mode    0 = Normal.
//              JM_DORGB = Use RGB order instead of BGA in pmap.
//      pstream Output stream from which to read the PNG file image.
//      pmap    Output bitmap.
//
//  Returns:
//      Non-zero code if error.
//*************************************************************************

csproc PNGRead (lint mode, fStm* pstream, BMAP* pmap) {
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_color_16* pcx;
    png_color* pcolor;
    lpbyte* pprows;
    byte* prow;
    byte* ppix;
    byte* pval;
    BGR* ppal;
    LHUE hue;
    lint stride;
    dword nrow, ncol, nx, ny, npix, channels;
    int bit_depth, color_type, interlace_type, compression_type, filter_type;
    int npal, val;
    errc e;

// Initialize.

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                     png_my_error, png_my_warning);
    if (png_ptr == NULL) {
        e = ECMEMORY;
        goto error;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
        e = ECMEMORY;
        goto error;
    }

    end_info = png_create_info_struct(png_ptr);
    if (end_info == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        e = ECMEMORY;
        goto error;
    }

// Set up for error return.
// Due to the PNG library's use of exiting for errors,
// we must use setjmp() and longjmp() to handle errors.

    if (setjmp(png_ptr->jmpbuf)) {              //establish longjmp return point
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        e = ECBADFILE;                          //if get hear, library had error
        goto error;
    }

// Specify the source file.  Note we pretend or fStm* is a FILE*.

    png_set_read_fn(png_ptr, (FILE*)pstream, my_read_data);
    //This has already been done during the png_set_write_fn call.
    //png_init_io(png_ptr, (FILE*)pstream);

// Read whole image into memory.

    val = PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING
        | PNG_TRANSFORM_PACKSWAP | PNG_TRANSFORM_BGR;
    if (mode & JM_DORGB) {
        val &= (~PNG_TRANSFORM_BGR);            //RGB output?
    }
    png_read_png(png_ptr, info_ptr, val, NULL);

// Copy the image into our bitmap.

    png_get_IHDR(png_ptr, info_ptr, &nx, &ny, &bit_depth,
                 &color_type, &interlace_type,
                 &compression_type, &filter_type);
    if ((bit_depth != 8)||(nx <= 0)||(ny <= 0)) goto bader;
    if (((dword)nx > _MaxIDimPNG)||((dword)ny > _MaxIDimPNG)) {
        if (_MaxIDimPNG >= 7200) {              //just in case
            e = ECBADSIZE;                      //try to prevent freeze (V62)
            goto ender;
        }
    }
    channels = png_get_channels(png_ptr, info_ptr);

    npix = (color_type == PNG_COLOR_TYPE_PALETTE) ? 8 : 32;
    e = BitsNew(pmap, npix, nx, ny, NULL);
    if (e) goto ender;
    if (mode & JM_DORGB) pmap->bmFlg |= RGBAPIX;

    npal = 0;
    pcx = NULL;
    png_get_tRNS(png_ptr, info_ptr, &pval, &npal, &pcx);
    if ((npal > 0)&&(pcx)) {
        if (npix == 8) {
            val = -1;
            while (npal--) {
                val += 1;
                if (*pval++ == 0) {
                    pmap->bmInv = CINV + val;
                    break;
                }
            }
        } else {
            hue.d = CINV;
            hue.b.bblue = (byte) pcx->blue;
            hue.b.bgreen = (byte) pcx->green;
            hue.b.bred = (byte) pcx->red;
            pmap->bmInv = hue.d;
        }
    } else if (channels == 4) {                 //alpha channel?
        pmap->bmInv = CINV;                     //allow transparency
    }

    if (npix == 8) {
        ppal = pmap->bmPal;
        pcolor = NULL;
        png_get_PLTE(png_ptr, info_ptr, &pcolor, &npal);
        if ((pcolor == NULL)||(npal > 256)) goto bader;
        pmap->bmUse = npal;
        while (npal--) {
            ppal->bblue = pcolor->blue;
            ppal->bgreen = pcolor->green;
            ppal->bred = pcolor->red;
            ppal->balpha = 0xFF;
            ppal += 1;
            pcolor += 1;
        }
    }

    pprows = png_get_rows(png_ptr, info_ptr);
    ppix = pmap->bmTop;
    stride = pmap->bmRow;
    nrow = ny;
    do {
        prow = *pprows;
        pval = ppix;
        ncol = nx;
        do {
            if (npix == 8) {
                *pval++ = *prow;
                prow += channels;
            } else if (channels == 4) {
                pval[0] = *prow++;
                pval[1] = *prow++;
                pval[2] = *prow++;
                pval[3] = *prow++;
                pval += 4;
            } else if (channels == 3) {
                pval[0] = *prow++;
                pval[1] = *prow++;
                pval[2] = *prow++;
                pval[3] = 0xFF;
                pval += 4;
            } else if (channels == 2) {
                pval[0] = *prow;
                pval[1] = *prow;
                pval[2] = *prow++;
                pval[3] = *prow++;
                pval += 4;
            } else {
                pval[0] = *prow;
                pval[1] = *prow;
                pval[2] = *prow++;
                pval[3] = 0xFF;
                pval += 4;
            }
        } while (--ncol);

        ppix += stride;
        pprows += 1;
    } while (--nrow);

// All done.

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return(0);

bader:
    e = ECBADFILE;
ender:
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
error:
    return(e);
}
#endif                                          //=== NOPNG

//*************************************************************************
//  GLTest:
//      Test for TriGL.cpp functions.
//      This is a general prototype for function-driven module tests.
//      Test functions should use the same parameters.
//      Compile with /D NO_TESTS to exclude this function.
//
//  Arguments:
//      errc GLTest (int mode, text* pdir)
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

#ifndef NO_TESTS                                //---
#include "TriEx.h"

csproc GLTest (int mode, text* pdir) {
    lint colors[2];
    text name[SZPATH];
    text line[SZPATH];
    text* pend;
    BFont font;
    StmFile file;
    BMAP map, pen;
    IBox box;
    IPt corn, rect[4];
    aint fone, ftwo;
    errc ee;

    OSPrint((mode|T_ASIS), "GLTest: TriGL.cpp Testing", 0);
    ee = 0;
    if (pdir) {
        pend = OSTxtCopy(name, pdir);
    } else {
        pend = OSLocation(name, OS_CURDIR);     //get current working directory
    }

    while (ee == 0) {
        OSPrint(mode, "Testing BitsMake BitsBlank BitsCopy...", 0);
        ee = ECERROR;

        ee = BitsMake(&map, 32, 400, 300, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsMake error", ee);
            break;
        }
        ee = BitsBlank(&map, NULL, CSILVER);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsBlank error", ee);
            break;
        }
        box.xleft = 0;
        box.xright = 100;
        box.ytop = 0;
        box.ybottom = 75;
        ee = BitsBlank(&map, &box, CYELLOW);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsBlank error", ee);
            break;
        }
        corn.ptx = 300;
        corn.pty = 225;
        ee = BitsCopy(0, &map, &corn, &map, &box);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsCopy error", ee);
            break;
        }
        box.xleft = 300;
        box.xright = 400;
        box.ytop = 0;
        box.ybottom = 75;
        ee = BitsBlank(&map, &box, CAQUA);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsBlank error", ee);
            break;
        }
        corn.ptx = 0;
        corn.pty = 225;
        ee = BitsCopy(0, &map, &corn, &map, &box);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsCopy error", ee);
            break;
        }
        ee = BitsLine(0, &map, 50,50, 350,50, 9,CRED);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsLine #1 error", ee);
            break;
        }
        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing BitsLine BitsPen BitsPolygon...", 0);
        ee = ECERROR;

        ee = BitsLine(0, &map, 50,50, 50,250, 9,CBLUE);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsLine #2 error", ee);
            break;
        }
        colors[0] = CGREEN;
        colors[1] = CBLACK;
        ee = BitsPenCreate(2, &pen, 5, colors);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsPenCreate error", ee);
            break;
        }
        ee = BitsAlphaFill(0, &map, NULL, 0, 0);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsAlphaFill error", ee);
            break;
        }
        rect[0].ptx =  50;
        rect[0].pty = 250;
        rect[1].ptx = 350;
        rect[1].pty = 250;
        rect[2].ptx = 350;
        rect[2].pty =  50;
        ee = BitsPen(0, &map, rect, 3, &pen, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsPen error", ee);
            break;
        }                   
        rect[0].ptx = 25;
        rect[0].pty = 25;
        rect[1].ptx = 50;
        rect[1].pty = 25;
        rect[2].ptx = 50;
        rect[2].pty = 50;
        rect[3].ptx = 25;
        rect[3].pty = 50;
        ee = BitsPolygon(0, &map, rect, 4, CGREEN);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsPolygon error", ee);
            break;
        }
        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing BitsText...", 0);
        ee = ECERROR;

        map.bmInv = 0;                          //allows CBLACK not to be clear
        ee = BitsText((BT_OPEN+BT_BIG), &font, NULL, 0,0,0, NULL,0, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BT_OPEN error (TriFont_T12.png must be in exe dir)", ee);
            break;
        }
        ee = BitsText(BT_EXTENT, &font, &map, 170,280, 0, "Test Text",0, &corn);
        if (ee) {
            OSPrint(mode+T_FAIL, "BT_EXTENT error", ee);
            break;
        }
        ee = BitsText(BT_TEXTS, &font, &map, (200-(corn.ptx>>1)),280, CBLACK, "Test Text",0, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BT_TEXTS error", ee);
            break;
        }
        ee = BitsText(BT_CLOSE, &font, NULL, 0,0,0, NULL,0, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BT_CLOSE error", ee);
            break;
        }
        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing BitsStretch...", 0);
        ee = ECERROR;

        ee = BitsAlphaFill(0, &map, NULL, 128, 0);
        box.xleft = 100;
        box.xright = 300;
        box.ytop = 75;
        box.ybottom = 225;
        ee = BitsStretch(0, &map, &box, &map, NULL);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsStretch error", ee);
            break;
        }
        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing PNG save and compare...", 0);
        ee = ECERROR;

        OSTxtCopy(pend, "Test.png");
        ee = file.StmOpen(OS_CREATE, name);
        if (ee) {
            OSPrintS(mode+T_FAIL, "StmOpen PNG error", name, ee);
            break;
        }
        ee = PNGWrite(0, &file, &map);
        if (ee) {
            OSPrintS(mode+T_FAIL, "PNGWrite error", name, ee);
            break;
        }
        StmSeek(&file, 0, OS_SET, NULL);
        ee = PNGRead(0, &file, &map);
        if (ee) {
            OSPrintS(mode+T_FAIL, "PNGRead error", name, ee);
            break;
        }
        file.StmClose();

        OSTxtCopy(line, name);
        OSTxtCopy(pend, "Data.png");
        fone = OSOpen(OS_READ, name);
        if (fone == -1) {
            if  (mode & T_NEW) {
                ee = FileCopy(CF_OVERWT+CF_TONAME, NULL, name, line, NULL);
                if (ee) {
                    OSPrint(mode+T_FAIL, "FileCopy data.png error", ee);
                    break;
                }
                fone = OSOpen(OS_READ, name);
            }
            if (fone == -1) {
                OSPrintS(mode+T_FAIL, "OSOpen Data.png error", name, ee);
                break;
            }
            OSPrintS(mode+T_FAIL, "Created Data.png for this and future tests", name, ee);
        }
        ftwo = OSOpen(OS_READ, line);
        if (ftwo == -1) {
            OSPrint(mode+T_FAIL, "OSOpen Test.png error", ee);
            break;
        }
        ee = FileComp(fone, ftwo);
        if (ee) {
            OSPrint(mode+T_FAIL, "Test.png FileComp error", ee);
            break;
        }
        if (fone != -1) OSClose(fone);
        if (ftwo != -1) OSClose(ftwo);
        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing JPEG save and compare...", 0);
        ee = ECERROR;

        OSTxtCopy(pend, "Test.jpg");
        ee = file.StmOpen(OS_CREATE, name);
        if (ee) {
            OSPrintS(mode+T_FAIL, "StmOpen JPG error", name, ee);
            break;
        }
        ee = JPEGWrite(0, &file, &map, NULL, NULL);
        if (ee) {
            OSPrintS(mode+T_FAIL, "JPEGWrite error", name, ee);
            break;
        }
        StmSeek(&file, 0, OS_SET, NULL);
        ee = JPEGRead(0, &file, &map);
        if (ee) {
            OSPrintS(mode+T_FAIL, "JPEGRead error", name, ee);
            break;
        }
        StmSeek(&file, 0, OS_SET, NULL);
        ee = JPEGWrite(0, &file, &map, NULL, NULL);
        if (ee) {
            OSPrintS(mode+T_FAIL, "JPEGWrite #2 error", name, ee);
            break;
        }
        file.StmClose();

        OSTxtCopy(line, name);
        OSTxtCopy(pend, "Data.jpg");
        fone = OSOpen(OS_READ, name);
        if (fone == -1) {
            if  (mode & T_NEW) {
                ee = FileCopy(CF_OVERWT+CF_TONAME, NULL, name, line, NULL);
                if (ee) {
                    OSPrint(mode+T_FAIL, "FileCopy data.jpg error", ee);
                    break;
                }
                fone = OSOpen(OS_READ, name);
            }
            if (fone == -1) {
                OSPrintS(mode+T_FAIL, "OSOpen Data.jpg error", name, ee);
                break;
            }
            OSPrintS(mode+T_FAIL, "Created Data.png for this and future tests", name, ee);
        }
        ftwo = OSOpen(OS_READ, line);
        if (ftwo == -1) {
            OSPrint(mode+T_FAIL, "OSOpen Test.jpg error", ee);
            break;
        }
        ee = FileComp(fone, ftwo);
        if (ee) {
            OSPrint(mode+T_FAIL, "Test.jpg FileComp error", ee);
            break;
        }

        ee = 0;
        break;
    }

    while (ee == 0) {
        OSPrint(mode, "Testing BitsFree...", 0);
        ee = BitsFree(&map);
        if (ee) {
            OSPrint(mode+T_FAIL, "BitsFree error", ee);
            break;
        }
        ee = 0;
        break;
    }

    return(ee);
}
#endif                                          //---
CSTYLE_END
