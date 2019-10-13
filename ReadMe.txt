TriOS Project
Portable OS functions with Extra and Graphics functions

The "Tri" refers to Triscape Inc, the company founded by Stacey Simonoff
which formerly published Triscape Map Explorer, the first 3D navigable
geographic mapping application, the idea (but not code) behind Google Maps, 
and Triscape FxFoto, popular photography editing and collage layout software.
TriOS does not refer to 3 operating systems and only supports Windows and Linux.

This is a fairly complete non-GUI API for applications which can be
built to run on Windows or Linux. It has been used for a number of Windows
applications and DLLs. It has also been used for one fairly sophisticated
embedded Linux application, as well as Linux CGI server applications.
Overall, it is better tested on Windows.

The TriOS.cpp and TriOS.h sources contain the core portable OS API
with a large variety of functions to handle memory allocation, file I/O,
text formatting and handling, and much more. There are also functions to
handle multi-tasking (threading) with mutex and semaphore functions (OSTask,
OSLock, OSWait), starting other executables (OSExec), transcendental numerical
functions (square root, sine, cosine, tangent, powers), walking directory
trees (OSFindFile and OSExists), mapped memory (OSMapMem) encryption and
random numbers (OSCipher, OSRandom), UTF16 wide to/from byte UTF8 text
conversion (OSWideUTF8), serial port I/O (OSComPort), TCP or UDP sockets
(OSConnect, OSConSend, OSConRecv). There are also some common Windows
dialog handlers not yet supported from Linux. Some low level functions are
optionally implemented in assembly for speed and can replace C/C++ runtime
functions. 

See the list of functions at the top of TriOS.cpp. Search for function call
documentation in the same file using the function name followed by a : char.

The TriOS.c source is a C-only version of TriOS.cpp, because the TriOS API
does not heavily depend on C++ and TriOS.c is convenient for embedded Linux
applications where you may want to work exclusively in C.

The TriEx.cpp and TriEx.h sources contain extensions that are portable because
they use the TriOS.cpp API and do not change between Windows and Linux.
These include functions to serialize and de-serialize binary data, which
are usually used for saving application parameter files. There are functions
for copying disk files. There are also FIFO APIs for handling communication
between tasks, one of which is bullet proof without mutex locking when used
on single core systems (though it is usually built with locking).
There are also functions to handle CRC and Windows Registry I/O.

The TriGL.cpp and TriGL.h sources contain a fairly complete bitmap-based
graphics API. This is not a GUI to support building windowed applications.
Instead it provides functions to read and write common image file formats
like PNG and JPEG, and provides an API to do fairly sophisticated image
manipulations for bitmap-based images. Applications using this API typically
use low level Windows calls to render bitmaps to the app's screen windows.

Note that TriGL JPEG handling depends on a specially built version of the
standard JPEG library where I/O calls are directed to the StmRead, StmWrite
file stream I/O APIs defined by TriOS.h.


Builds TriOS.lib library to link into other projects.
Prebuilt Windows libraries are included in Release and Debug.
However, the C/C++ sources are normally just included in larger projects.
See TriOS.cpp, TriEx.cpp, TriGL.cpp comments for documentation.

Currently builds with Visual Studio 2008
Currently defines OSBASE,NOJPEG,NOPNG

Does not include JPEG and PNG image file support given NOJPEG and NOPNG defs.
If enabled, Inc and Lib folders hold Windows JPEG and PNG implementations
and the jpeg.lib uses TriOS compatible StmRead/StmWrite interfaces as required.
To enable JPEG and PNG remove NOJPEG and/or NOPNG defintions.

There is currently no customized JPEG library for Linux.
ToDo: Dig out the old slightly modified JPEG sources and recompile for Linux.

For Windows, define:
    WIN32 (or WIN64)
For Windows, link with:
    zlib.lib;wpcap.lib;ws2_32.lib;iphlpapi.lib;htmlhelp.lib;wininet.lib;
Without OSBASE define, must also link with:
    ws2_32.lib and wininet.lib
Without NOPNG define, must also link with:
    libpng.lib
Without NOJPEG define, must also link with:
    jpeg.lib (built with I/O calls linked to StmRead/StmWrite)
The required zlib.lib, jpeg.lib, libpng.lib are available in the Lib folder. 
The included Visual Studio project files are for VS2008.

For Linux, compile with:
    -Wno-write-strings -Wno-format
For Linux, link with:
    -lpthread -lrt -lm
Without NOPNG define, must also link with:
    -lpng
Without NOJPEG define, must also link with:
    -ljpeg (built with I/O calls linked to StmRead/StmWrite)
