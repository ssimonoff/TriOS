//*************************************************************************
//  FILENAME:   TriGL.h
//  AUTHOR:     SCSimonoff  4/3/14
//
//  Definitions for TriGL.cpp Graphics Library.
//
//  This software was created 1997-2012 by SCSimonoff.
//  It may be freely used and distributed on the condition that this
//  message remains and no copyright is added to this or derived copies.
//*************************************************************************

#ifndef _TriGL                  //Only define once if include multiple times
#define _TriGL
CSTYLE_BEG                      //Keep it simple and do not require C++ caller

// This structure is used to define a color.  It must always be four bytes!
// Note that this must match the Windows PALETTEENTRY structure.
// Beware that BITMAPINFO colors are in the opposite byte order: Blue,Green,Red!
// For 8-bit colors, the index is stored in the second byte so that the
// least significant byte can be used for fractional index values.
// The ralpha byte can be used for CRGB and CINV flags.
// Guaranteed to have a byte size of four.
// Note that we assume HIENDIAN machines use the same order!

DEFSTRUCT   (RGB)           //RGB:
DB( byte,   rred      )     //Red intensity
DB( byte,   rgreen    )     //Green intensity
DB( byte,   rblue     )     //Blue intensity
DB( byte,   ralpha    )     //Flags for RGB or Standard color value (see above)
DEFENDING   (RGB)
TYPEDEF RGB* lpRGB;

// This structure defines a color in DIB order (Blue,Green,Red).
// Must match Microsoft's RGBQUAD structure.
// The balpha byte can be used for CRGB and CINV flags.
// It can also be an alpha channel pixel coverage (0=transparent,255=full).
// Guaranteed to have a byte size of four.
// Note that we assume HIENDIAN machines use the same order!

DEFSTRUCT   (BGR)           //BGR:
DB( byte,   bblue     )     //Blue intensity
DB( byte,   bgreen    )     //Green intensity
DB( byte,   bred      )     //Red intensity
DB( byte,   balpha    )     //Alpha channel value
DEFENDING   (BGR)
TYPEDEF BGR* lpBGR;

// Define HLS Hue,Luminosity,Saturation dword values.
// Note that we assume HIENDIAN machines use the same order!

DEFSTRUCT   (HLS)           //BGR:
DB( byte,   hhue      )     //Hue (color)
DB( byte,   hlum      )     //Luminosity (lightness)
DB( byte,   hsat      )     //Saturation
DB( byte,   halpha    )     //Alpha channel value
DEFENDING   (HLS)
TYPEDEF HLS* lpHLS;

// For RGB and BGR arrays, define component offsets.

#define ORRED   0           //Blue is first
#define ORGREEN 1           //Green is next
#define ORBLUE  2           //Red is next
#define ORALPHA 3           //Alpha is last

#define OBBLUE  0           //Blue is first
#define OBGREEN 1           //Green is next
#define OBRED   2           //Red is next
#define OBALPHA 3           //Alpha is last

// This structure defines the colors in a bitmap (which unfortunately are
// reversed from the COLORS order due to a Windows inconsistency).
// It also gives a way of equating COLORS values with dwords.
// The c version is equivalent to our native COLORS RGB order.
// The m version is equivalent to Microsoft's RGBQUAD and RGBTRIPLE order.

typedef struct {            //LHue:
    union   {
        lint d;             //32-bit dword version of color value
        RGB  r;             //RGB version of value
        BGR  b;             //BGR version of value
        HLS  h;             //HLS version of value
    };
} LHUE;
TYPEDEF LHUE* lpLHUE;

// Define structure to hold floating RGB color information.

DEFSTRUCT   (FHUE)          //FHUE:
DS( LHUE,   hue       )     //Non-floating color equivalent or 0
DB( flt,    fblue     )     //Blue intensity
DB( flt,    fgreen    )     //Green intensity
DB( flt,    fred      )     //Red intensity
DB( flt,    falpha    )     //Alpha channel value
DEFENDING   (FHUE)
TYPEDEF FHUE* lpFHUE;

// This structure defines a standardized bitmap (in lieu of BITMAPINFOHEADER).
// Top-to-bottom bitmaps have bmTop set to bmPtr and a postive bmRow value.
// Bottom-to-top bitmaps have bmTop near end of bmPtr memory and negative bmRow.
// Bottom-to-top is preferred as some applications do not support top-to-bottom.
// The bmRow value should be a multiple of four (dword aligned).
// The bmX value may have fewer pixels than implied by bmRow.
// For bmPix 1 or 4, the most significant bits in a byte are left most.
// For bmPix 24, there are three bytes per pixel: blue, green, then red.
// For bmPix 32, there are four bytes per pixel: blue, green, red, then alpha.
// For bmPix 64, there are four words per pixel: BGRA WFONE=1.0 int.frac values.
// For bmPix 128, there are four flts per pixel: BGRA floating values.
// Normally, the alpha byte denotes transparency (not coverage) for the pixel.
// Note that not all functions support all bmPix pixel sizes or bmInv.
// For 1,4,8 bits/pixel, the low byte of CINV gives the palette index for clear.
// For 32 bits/pixel, a pixel is clear if its alpha value is less than 0x80.
// Actually, BitsStretch pixel is clear if alpha is zero and opaque if 0xFF.

// To allow our VBits object to include VIF vector images as well as bitmaps,
// we allow its m_map BMAP to point to a Vector Image File block (see Vector.h).
// The bmFlg is set to VIMGPTR and (fVImg*)bmPtr points to the vector image.
// All other members of the BMAP structure must be zeroed for vector images.
// Most routines that handle BMAP bitmaps cannot handle vector images!

DEFSTRUCT   (BMAP)          //BMAP:
DD( byte*,  bmPtr         ) //Pointer to bitmap BGR or palette index memory
DD( byte*,  bmTop         ) //Pointer to top line of bitmap (inside bmPtr mem)
DD( lint,   bmRow         ) //Bytes between bitmap rows (neg if bottom-to-top)
DD( lint,   bmPix         ) //Bits per bitmap pixel: 1, 4, 8, 24, 32
DD( lint,   bmX           ) //Number of horizontal pixels in bitmap
DD( lint,   bmY           ) //Number of vertical pixels in bitmap
DD( lint,   bmFlg         ) //Flags (see below)
DD( lint,   bmNow         ) //Time delay for sequenced image (millisec) or 0
DD( lint,   bmInv         ) //Invisible BGR plus CINV (zero=>no transparency)
DD( lint,   bmUse         ) //Number of beginning palette colors currently used
DD( BGR*,   bmPal         ) //Pointer to BGR palette color dwords or NULL
DD( lint,   bmPtX         ) //Extra X axis info (columns used so far)
DD( lint,   bmPtY         ) //Extra Y axis info (rows used or columns discarded)
DEFENDING   (BMAP)
TYPEDEF BMAP* lpBMAP;

#define KEEPPTR 1           //Set in bmFlg if we do not own bmPtr memory
#define KEEPPAL 2           //Set in bmFlg if we do not own bmPal memory
#define KEEPALL 3           //Set in bmFlg if we do not own bmPtr or bmPal
#define NEWINFO 4           //Set in bmFlg if bmPix,bmX,bmY,bmUse have changed
#define ISALPHA 8           //Set in bmFlg if bmPal has real alphas (not 0's)
#define VIMGPTR 16          //Set in bmFlg if bmPtr points to VImg vector image
#define ALPHAS0 32          //Set in bmFlg to use 0 background alphas, not 0xFF
#define MATCHED 128         //Set in bmFlg if bmPal non-exactly matched colors
#define ZPACKED 256         //Set in bmFlg if bmPtr and bmPal are ZLib packed
#define CLEARBG 512         //Reserved for Clear Background (same as IS_CLEARB)
#define RGBAPIX 1024        //Set in bmFlg if RGBA instead of BGRA byte order

#define PALSZ   256         //Maximum palette size (max of 8 bits/pixel)
#define PALHI   8           //Number of most frequently used BitsMatch +8 colors

// Predefined Standard Colors.
// A color value can hold a standard color index or CRGB and a RGB value.
// The standard Windows VGA palette is 0=BLACK - 7=GREY, 8=SILVER - 15=WHITE.
// SuperVGA adds 8=SAGE,9=SKY,248=CREAM,247=STEEL then 248=SILVER - 255=WHITE.
// The Non-Windows colors go in application-specific palette additions (10+).
// The CRGB bit can be omitted if the Green or Blue bytes are non-zero.
// Note the CCREAM was made darker for FX and was formerly 0x0F0FBFF.
// Note that ALPHA_FULL was previousy 0xFE... but BitsStretch creates 0xFD...

#if (HIENDIAN==0)               //=======
#define CSTD        0x00000000  //No flag if 0x00000SS Standard color index
#define CRGB        0x01000000  //Bit set if 0x1BBGGRR RGB color
#define CBGR        0x02000000  //Bit set if 0x2RRGGBB BGR color
#define CINV        0x04000000  //Bit set if transparent (invisible) pixel
#define CERR        0x08000000  //Bit set if unused or invalid color
#define RGB_MASK    0x00FFFFFF  //Mask for RGB or BGR byte values
#define ALPHA_MASK  0xFF000000  //Mask for alpha byte value (or fully opaque)
#define ALPHA_FULL  0xFC000000  //Assumed fully opaque alpha after rounding
#define ALPHA_HALF  0x80000000  //BGR or RGB comparision with 50% alpha
#else                           //=======
#define CSTD        0x00000000
#define CRGB        0x00000001
#define CBGR        0x00000002
#define CINV        0x00000004
#define CERR        0x00000008
#define RGB_MASK    0xFFFFFF00
#define ALPHA_MASK  0x000000FF
#define ALPHA_FULL  0x000000FC
#define ALPHA_HALF  0x00000080
#endif                          //=======
#define ABYTE_FULL  0xFC        //ALPHA_FULL byte value
#define AFLT_FULL   252.0f      //ALPHA_FULL as floating value

#define CRGB_BYTE   0x01        //Alpha byte if 0x1BBGGRR RGB color
#define CBGR_BYTE   0x02        //Alpha byte if 0x2RRGGBB BGR color
#define CINV_BYTE   0x04        //Alpha byte if transparent (invisible) pixel

#define CNULL       0           //No Color                      0x0BBGGRR

#define CBLACK      1           //Windows Black                 0x0000000
#define CMAROON     2           //Windows Dark Red              0x0000080
#define CGREEN      3           //Windows Dark Green            0x0008000
#define COLIVE      4           //Windows Dark Yellow           0x0008080
#define CNAVY       5           //Windows Dark Blue             0x0800000
#define CPURPLE     6           //Windows Dark Magenta          0x0800080
#define CTEAL       7           //Windows Dark Cyan             0x0808000
#define CGREY       8           //Windows Dark Grey             0x0808080

#define CSAGE       9           //Windows Money Green (Not VGA) 0x0C0DCC0
#define CSKY        10          //Windows Sky Blue    (Not VGA) 0x0F0CAA6

#define CBROWN      11          //Non-Windows Dark Dull Red     0x0404080
#define CDUSK       12          //Non-Windows Dull Red          0x0AAAAC5
#define CFOREST     13          //Non-Windows Dull Green        0x0AAD5AA
#define CSALMON     14          //Non-Windows Light Dull Red    0x0DFF7FF
#define CORANGE     15          //Non-Windows Yellow-Red        0x00080FF
#define CKHAKI      16          //Non-Windows Yellow-Green      0x0B0FFF0
#define CSEA        17          //Non-Windows Cyan-Green        0x080FF00
#define CCYAN       18          //Non-Windows Cyan-Blue         0x0FF8000
#define CINDIGO     19          //Non-Windows Magenta-Blue      0x0FF0080
#define CROSE       20          //Non-Windows Magenta-Red       0x08000FF
#define CPINK       21          //Non-Windows Bright Magenta    0x0C080FF

#define CCREAM      22          //Windows Cream       (Not VGA) 0x0EAFAFF
#define CSTEEL      23          //Windows Medium Grey (Not VGA) 0x0A4A0A0

#define CSILVER     24          //Windows Light Grey            0x0C0C0C0
#define CRED        25          //Windows Red                   0x00000FF
#define CLIME       26          //Windows Green                 0x000FF00
#define CYELLOW     27          //Windows Yellow                0x000FFFF
#define CBLUE       28          //Windows Blue                  0x0FF0000
#define CFUCHSIA    29          //Windows Magenta               0x0FF00FF
#define CAQUA       30          //Windows Cyan                  0x0FFFF00
#define CWHITE      31          //Windows White                 0x0FFFFFF

#define CCLEAR      32          //Transparent (No Color)        0x0400000
#define CMAXIMUM    32          //Maximum Standard Color Number

// Define some non-standard RGB colors.

#define H_SRED      (CRGB+0x9090FF) //Saturated Red
#define H_SGREEN    (CRGB+0x90FF90) //Saturated Green
#define H_SBLUE     (CRGB+0xFF9090) //Saturated Blue
#define H_SFUCH     (CRGB+0xFF90FF) //Saturated Magenta
#define H_SAQUA     (CRGB+0xFFFF90) //Saturated Cyan
#define H_SPARK     (CRGB+0xC9E4C9) //Saturated Forest
#define H_SGREY     (CRGB+0xE0E0E0) //Saturated Grey
#define H_CHARCOAL  (CRGB+0x404040) //Very Dark Grey

// This color is sometimes reserved for a unique invisible color.
// B_INVIS is near black and is in BGR order.  W_INVIS is near white.
// The B_CLEAR and W_CLEAR do not trigger special BitsStretch handling.

#define B_INVIS     (CINV+0x010203) //Unique color reserved for invisible
#define W_INVIS     (CINV+0xFEFDFC) //Unique white reserved for invisible

#define B_CLEAR     (CINV+0x010101) //Black background clear feather color
#define W_CLEAR     (CINV+0xFEFEFE) //White background clear feather color

// Define three flavors of fixed point numbers that are stored in integers.

#define WFONE   0x00100 //1.0 in a fraction for low word multiply result
#define HFONE   0x04000 //1.0 in a fraction for high word multiply result
#define DFONE   0x10000 //1.0 in a signed dword fraction
#define AFONE   0x01000 //1.0 in a signed dword fraction used by BitsAngle
#define AFHALF  0x00800 //0.5 in a signed dword fraction used by BitsAngle
#define WFSHIFT 8       //Left shift for WFONE
#define HFSHIFT 14      //Left shift for HFONE
#define DFSHIFT 16      //Left shift for DFONE
#define AFSHIFT 12      //Left shift for AFONE

// Define one way to represent fractions multiplied by integers in MMX words.

#define AMX_ONE 0x800   //1.0 for MMX int.frac word
#define AMX_TOP 0x3FF   //1.0 after multiply high word
#define AMX_MOR 0x4000  //Remultiply 1.0 for 0xFF high
#define AMX_LSF 7       //Left shift for source pixel
#define AMX_RSF 2       //Right shift for top product
#define AMX_FFO 3       //Left shift from 0xFF=1.0 to AMX_ONE
#define AMX_DIF 1       //Left shift from AMX_TOP to AMX_ONE

// Define useful constants.

#define FMAX    64      //Maximum number of filter values (notches) (power of 2)
#define LMAXW   256     //Maximum width of lines

#define L_POINT 0       //Pointed line end
#define L_LEFT  1       //Left mitrer line end
#define L_END   2       //Square line end
#define L_RIGHT 3       //Right mitrer line end

#define L_END2  (8+2)   //Square line end at both ends

#define L_3D    16      //Raised 3D-effect line
#define L_DENT  32      //Outlined with darker color line
#define L_DIAG  64      //3D-effect when near diagonals are common

#define A_STEP  128     //Draw angle text as staircase horizontal/vertical chars
#define A_VDN   64      //Mostly vertical downward angle
#define A_VUP   32      //Mostly vertical upward angle
#define A_TOBOX 0x20000 //BitsAngle pto points to IBox rather than IPt struct

#define A_PI    3.1415926535897932384626433832795

#define LP_DOT  0x55555555 //Dot pattern for BitsPattern
#define LP_DDOT 0x27272727 //DashDot pattern for BitsPattern
#define LP_DASH 0x0F0F0F0F //Dash pattern for BitsPattern
#define LP_DSH2 0x3C3C3C3C //Second in animated moving dash sequence
#define LP_DSH3 0xF0F0F0F0 //Third in animated moving dash sequence
#define LP_DSH4 0xC3C3C3C3 //Fourth in animated moving dash sequence
#define LP_BIGD 0x00FF00FF //Big dash pattern for BitsPattern
#define LP_BIG2 0x03FC03FC //Second in animated moving big dash sequence
#define LP_BIG3 0x0FF00FF0 //Third in animated moving big dash sequence
#define LP_BIG4 0x3FC03FC0 //Fourth in animated moving big dash sequence

#define BP_ROUND1     1 //Round pen with one color (code depends on numbers!)
#define BP_ROUND2     2 //Round pen with two colors
#define BP_ROUND3     3 //Round pen with three colors
#define BP_RADIAL     4 //Round pen with radial blending of two colors
#define BP_PENSR1    16 //Left to right / stylus with one color
#define BP_PENSL1    17 //Right to left \ stylus with one color
#define BP_PENSR2    18 //Left to right / stylus with one color
#define BP_PENSL2    19 //Right to left \ stylus with one color
#define BP_3D       128 //Create 3D-effect pen with darker colors right & below
#define BP_4X       256 //Scale the pen 4x pixels wider for antialiasing
#define BP_1C       512 //Specify only one color and use darker version borders
#define BP_FF      1024 //Use full 0xFF alpha values for pen (for blending)
#define BP_E1      2048 //Keep pen border color a single pixel wide

#define GR_HOR        1 //BitsGraph horizontal plot mode
#define GR_VER        2 //BitsGraph vertical plot mode
#define GR_STRIP      4 //BitsGraph strip plot mode
#define GR_FILL       8 //BitsGraph filled plot mode
#define GR_HUES      16 //BitsGraph mode to allow second color array of data
#define GR_CLEAR    128 //BitsGraph partially transparent mode
#define GR_TYPE    0xFF //BitsGraph basic type mask
#define GR_MORE   0x100 //BitsGraph mode for more transparency
#define GR_NEGS   0x200 //BitsGraph mode to allow negative input values

#define TRI_HUES      1 //Bits2DAdd and DataHues mode for three colors
#define TRI_MODSRC    4 //Bits2DAdd mode to allow modifying pdat source array
#define TRI_NOGROW    8 //Bits2DAdd mode to return error instead of grow bitmap
#define TRI_RIGHT    16 //Bits2DAdd mode to add to right rather than left
#define TRI_READY    64 //Bits2DAdd mode to supply ready to use bitmap column
#define TRI_CLEAR   128 //Bits2DAdd partially transparent mode
#define TRI_MORE    256 //Bits2DAdd for more transparency
#define TRI_LESS    512 //Bits2DAdd for less transparency
#define ADD_COLS   1024 //Bits2DAdd columns to add when expand

// Definitions used by inline assembly.

#define VONE   0x3F800000   //floating value for 1.0 (low dword is zero)

#define sax     eax
#define sbx     ebx
#define scx     ecx
#define sdx     edx
#define sbp     ebp
#define ssi     esi
#define sdi     edi
#define ssp     esp

#define dss     ds
#define ess     ds
#define fss     ds
#define gss     ds
#define sss     ds
#define css     cs

#define movsxs  mov
#define movzxs  mov
#define movsxw  movsw
#define movzxw  movzw
#define jscxz   jecxz

// Define the DrawDivTab sizes.

#define DIVMAX  512
#define DIVOVER (~(DIVMAX-1))

// IFRACTION: Macro to quickly divide two numbers and produce a fraction.
// The lesser and greater args must be positive numbers with lesser<=greater.
// The assembly version is much faster if greater is less than DIVMAX (512).
// The frac result returns an integer value which is 2^32 * lesser/greater.
// The frac result is 2^32-1 (0xFFFFFFFF) if lesser>=greater.
// The lab argument must be a unique label for the module used internally.

#ifdef NOI86    //-------
#define IFRACTION(frac, lesser, greater, lab) {                 \
    if (lesser >= greater)                                      \
        frac = 0xFFFFFFFF;                                      \
    else                                                        \
        frac = (dword) (4294967296.0*(flx)lesser/(flx)greater); \
}
#else           //-------
#define IFRACTION(frac, lesser, greater, lab) {                 \
    __asm       mov     eax,lesser                              \
    __asm       mov     ecx,greater                             \
    __asm       cmp     eax,ecx                                 \
    __asm       jge     lab##one                                \
    __asm       test    ecx,DIVOVER                             \
    __asm       jnz     lab##big                                \
    __asm       mov     ecx,DrawDivTab[ecx*4]                   \
    __asm lab##ok:                                              \
    __asm       mul     ecx                                     \
    __asm       mov     frac,eax                                \
    __asm       jmp     lab##done                               \
    __asm lab##one:                                             \
    __asm       mov     frac,0xFFFFFFFF                         \
    __asm       jmp     lab##done                               \
    __asm lab##big:                                             \
    __asm       push    eax                                     \
    __asm       xor     eax,eax                                 \
    __asm       mov     edx,1                                   \
    __asm       div     ecx                                     \
    __asm       mov     ecx,eax                                 \
    __asm       pop     eax                                     \
    __asm       jmp     lab##ok                                 \
    __asm lab##done:                                            \
}
#endif          //-------

// IDIVIDE: Macro to quickly divide two numbers and produce a int.frac value.
// The numerator and denominator arguments must be positive numbers.
// The assembly version is much faster if denominator is less than DIVMAX (512).
// The result is set to numerator/denominator as a lint with DFSHIFT binary pt.
// This means that the high word is integer and the low word is fraction.
// The result returns an integer value which is 2^16 * numer/denom.
// The lab argument must be a unique label for the module used internally.

#ifdef NOI86    //-------
#define IDIVIDE(result, numerator, denominator, lab) {          \
    result =  (lint)(65536.0*(flx)numerator / (flx)denominator);\
}
#else           //-------
#define IDIVIDE(result, numerator, denominator, lab) {          \
    __asm       mov     eax,numerator                           \
    __asm       mov     ecx,denominator                         \
    __asm       test    ecx,DIVOVER                             \
    __asm       jnz     lab##big                                \
    __asm       mov     ecx,DrawDivTab[ecx*4]                   \
    __asm lab##ok:                                              \
    __asm       mul     ecx                                     \
    __asm       shr     eax,16                                  \
    __asm       shl     edx,16                                  \
    __asm       or      eax,edx                                 \
    __asm       mov     result,eax                              \
    __asm       jmp     lab##done                               \
    __asm lab##big:                                             \
    __asm       push    eax                                     \
    __asm       xor     eax,eax                                 \
    __asm       mov     edx,1                                   \
    __asm       div     ecx                                     \
    __asm       mov     ecx,eax                                 \
    __asm       pop     eax                                     \
    __asm       jmp     lab##ok                                 \
    __asm lab##done:                                            \
}
#endif          //-------

// MDIVIDE: Macro to quickly divide two numbers and produce a integer value.
// The numerator and denominator arguments must be positive numbers.
// The assembly version is much faster if denominator is less than DIVMAX (512).
// The result is set to numerator/denominator as a lint.
// Currently we limit the denominator to DIVOVER=511 maximum!

#ifdef NOI86    //-------
#define MDIVIDE(result, numerator, denominator) {               \
    result =  (lint)((flx)numerator / (flx)denominator);        \
}
#else           //-------
#define MDIVIDE(result, numerator, denominator) {               \
    __asm       mov     eax,numerator                           \
    __asm       mov     ecx,denominator                         \
    __asm       mov     ecx,DrawDivTab[ecx*4]                   \
    __asm       mul     ecx                                     \
    __asm       mov     result,eax                              \
}
#endif          //-------

// DDIVIDE: Divides a 64-bit int (two DWords) by DWord: result = high,low / over
// The high,low arguments define the 64-bit integer to be divied by over.
// The result returns the integer dividend value.
// Does a unsigned division and operands should be dword values.

#ifdef NOI86    //-------
#define DDIVIDE(result, high, low, over) {                              \
    result = (lint)(((flx)high * 4294967296.0) + (flx)low) / (flx)over; \
}
#else           //-------
#define DDIVIDE(result, high, low, over) {                      \
    __asm       mov     edx,high                                \
    __asm       mov     eax,low                                 \
    __asm       mov     ecx,over                                \
    __asm       div     ecx                                     \
    __asm       mov     result,eax                              \
}
#endif          //-------

// DSUBTRACT: Subtracts two 64-bit integers: one = one - two
// Operands should be dword values (or lints for signed high values).

#ifdef NOI86    //-------
#define DSUBTRACT(onehigh, onelow, twohigh, twolow) {           \
    onelow = onelow - twolow;                                   \
    onehigh = onehigh - twohigh;                                \
    if ((lint)onelow < 0) {                                     \
        onehigh -= 1;                                           \
    }                                                           \
}
#else           //-------
#define DSUBTRACT(onehigh, onelow, twohigh, twolow) {           \
    __asm       mov     edx,onehigh                             \
    __asm       mov     eax,onelow                              \
    __asm       sub     eax,twolow                              \
    __asm       sbb     edx,twohigh                             \
    __asm       mov     onehigh,edx                             \
    __asm       mov     onelow,eax                              \
}
#endif          //-------

// Structure used to hold two X,Y points or a rectangle.
// Notice that this corresponds exactly to the Win32 RECT structure.
// Code depends on IBox being two IPt's for upper-left and lower-right corners.

DEFSTRUCT   (IBox)      //IBox:
DD(lint,    xleft     ) //left X coordinate
DD(lint,    ytop      ) //top Y coordinate
DD(lint,    xright    ) //right X coordinate
DD(lint,    ybottom   ) //bottom Y coordinate
DEFENDING   (IBox)
TYPEDEF IBox* lpIBox;

// This structure also holds a rectangle but uses width and height.

DEFSTRUCT   (DBox)      //DBox:
DD(lint,    ileft     ) //left X coordinate
DD(lint,    itop      ) //top Y coordinate
DD(lint,    iwidth    ) //width of box
DD(lint,    iheight   ) //height of box
DEFENDING   (DBox)
TYPEDEF DBox* lpDBox;

// Structure used to hold a single X,Y point or size.
// Notice that this corresponds exactly to the Win32 POINT and SIZE structures.
// Also notice that it corresponds exactly with the upper-left corner of IBox.
// The IPTSAME macro gives true if the two IPt's are the same point.

DEFSTRUCT   (IPt)       //IPt:
DD(lint,    ptx       ) //X coordinate or size
DD(lint,    pty       ) //Y coordinate or size
DEFENDING   (IPt)
TYPEDEF IPt* lpIPt;

#define IPTP2   3       //Byte size of IPt as a power of two
#define IPTSAME(p1,p2)  (((p1)->ptx == (p2)->ptx)&&((p1)->pty == (p2)->pty))

// Structure used to define a series of integer X,Y points.

DEFSTRUCT   (ISegs)     //ISegs:
DD(IPt*,    isPtr     ) //Pointer to an array of X,Y points
DD(lint,    isNpt     ) //Number of X,Y points in the array
DD(lint,    isMax     ) //Total number of X,Y points in buffer (optional)
DD(lint,    isVal     ) //Available for special uses
DEFENDING   (ISegs)
TYPEDEF ISegs* lpISegs;

// Floating point structures.

DEFSTRUCT   (FPt)       //FPt:
DD(flt,     fpx       ) //X single precision coordinate or size
DD(flt,     fpy       ) //Y single precision coordinate or size
DEFENDING   (FPt)
TYPEDEF FPt* lpFPt;

DEFSTRUCT   (FBox)      //FBox:
DD(flt,     fleft     ) //left X coordinate
DD(flt,     ftop      ) //top Y coordinate
DD(flt,     fwidth    ) //width of box (right X coordinate is fleft+fwidth)
DD(flt,     fheight   ) //height of box (bottom Y coordinate is ftop+fheight)
DEFENDING   (FBox)
TYPEDEF FBox* lpFBox;

// Structure to hold a 3D X,Y,Z points or a volume.

DEFSTRUCT   (IVol)      //IVol:
DD(lint,    ix        ) //left X begin coordinate
DD(lint,    iy        ) //top Y begin coordinate
DD(lint,    iz        ) //first Z begin coordinate
DD(lint,    nx        ) //X width of volume
DD(lint,    ny        ) //Y height of volume
DD(lint,    nz        ) //Z depth of volume
DD(lint,    how       ) //Optional flag values
DD(void*,   vol       ) //Volume matrix pointer or NULL
DEFENDING   (IVol)
TYPEDEF IVol* lpIVol;

#define IV_X    1       //Optional how flag for X
#define IV_Y    2       //Optional how flag for Y
#define IV_Z    4       //Optional how flag for Z

// Structure used to open BitsText font.

DEFSTRUCT   (BFont)     //BFont:
DD(BMAP,    bfMap     ) //Font bitmap for ascii 0x20 - 0x59 loaded into memory
DD(lint,    bfWide    ) //Pixel width of all font characters
DD(lint,    bfHigh    ) //Pixel height of all font characters
DD(lint,    bfColor   ) //Current text color
DEFENDING   (BFont)
TYPEDEF BFont* lpBfont;

#define BT_CLOSE    0   //BitsText mode to close BFont
#define BT_OPEN     1   //BitsText mode to open new font file into BFont
#define BT_TEXT     2   //BitsText mode to draw text into target font
#define BT_TEXTS    3   //BitsText mode to draw grey shadow text
#define BT_TEXTF    4   //BitsText mode to draw grey feathered text
#define BT_EXTENT   5   //BitsText mode to just get extent of text to be drawn
#define BT_MASK  0xFF   //BitsText mode mask for basic operation
#define BT_BIG    256   //BitsText mode added for bigger default font

#define BT_SHADOW   CGREY
#define BT_FEATHER  CSILVER

#define BF_FILE    "TriFont_T9.png"
#define BF_FBIG    "TriFont_T12.png"
#define BF_CHARBEG  ' ' //BFont assumed first character in font file
#define BF_CHAREND  '~' //BFont assumed last character in font file
#define BF_CHARCNT ((BF_CHAREND - BF_CHARBEG)+1)

// Floating point structures.

// Define normal and high JPEG Quality levels for application.

#define JPEG_QNORM   0  //Normal quality level
#define JPEG_QHIGH  95  //High quality level

// These are the mode argument bit definitions.

#define JM_QUAL   0x0FF //Compress quality level mask (1=low,7=high, 0=default)
#define JM_8BIT   0x100 //Decompress to 8 bit/pixel bitmap (rather 32 bits/pix)
#define JM_FAST   0x200 //Fastest integer computation (poor quality)
#define JM_FLOAT  0x400 //Floating computation (maybe faster than normal)
#define JM_DITHER 0x800 //Use pixel dithering (but only when JM_8BIT)
#define JM_ALPHA 0x1000 //Use four-channel color with alpha information
#define JM_DORGB 0x2000 //Use RGB instead of BGR bitmap order

// External functions.

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

#ifdef      __cplusplus                         //---
csproc      JPEGWrite (lint mode, fStm* pstream, BMAP* pmap, IBox* pbox, byte* pexifs);
csproc      JPEGRead (lint mode, fStm* pstream, BMAP* pmap);

csproc      PNGWrite (lint mode, fStm* pstream, BMAP* pmap);
csproc      PNGRead (lint mode, fStm* pstream, BMAP* pmap);
#endif                                          //---

csproc      GLTest (int mode, text* pdir);

CSTYLE_END
#endif                          //_TriGL
