TriOS Project
Portable OS functions with Extra and Graphics functions

The "Tri" refers to Triscape Inc, the company founded by Stacey Simonoff
which formerly published Triscape Map Explorer, the first 3D navigable
geographic mapping application, the idea (but not code) behind Google Maps, 
and Triscape FxFoto, photography editing and collage layout software. TriOS
does not refer to three operating systems and only supports Windows and Linux.

This is a fairly complete non-GUI API for building applications that can be
built to run on Windows or Linux. It has be used by for a number of Windows
applications and DLLs. It has also been used for some fairly sophisticated
embedded Linux applications, as well as a few Linux CGI server applications.
Overall, it is better tested on Windows.

The TriOS.cpp and TriOS.h sources contain the core transportable OS API
with a large variety of functions to handle memory allocation, files I/O,
text formatting and handle, plus much more. There are also functions to
handle multi-tasking (threading) with mutex and semaphore functions (OSTask,
OSLock, OSWait), starting other executables (OSExec), transcendental numerical
functions (square root, sine, cosine, tangent, powers), walking directory
trees (OSFindFile and OSExists), mapped memory (OSMapMem) encryption and
random numbers (OSCipher, OSRandom), UTF16 wide to/from byte UTF8 text
conversion (OSWideUTF8), serial port I/O (OSComPort), TCP or UDP sockets
(OSConnect, OSConSend, OSConRecv). There are also some common Windows
dialog handlers not yet available from Linux. Some low level functions are
optionally implemented in assembly for speed. See the list of functions
at the top of TriOS.cpp and search for the function call documentation
below in the same file using the function name followed by a colon character.

The TriOS.c source is a C-only version of TriOS.cpp, because the TriOS API
does not heavily depend on C++ and TriOS.c is convenient for embedded Linux
applications where you may want to work exclusively in C.

The TriEx.cpp and TriEx.h sources contain extensions that are portable because
they use the TriOS.cpp API and do not change between Windows and Linux.
This includes functions to serialize and de-serialize binary data, which
I usually use for saving application parameter files. There are also functions
for copying disk files. There are also two FIFO APIs for handling communication
between tasks, one of which is bullet proof without semaphore locking when
used on single core Linux systems (though it is usually built with locking).
There are also functions to handle CRC and Windows Registry I/O.

The TriGL.cpp and TriGL.h sources contain a fairly complete bitmap-based
graphics API. This is not a GUI which allows building windowed application.
Instead it provides functions to read and write common image file formats
like PNG and JPEG and provides an API to do fairly sophisticated image
manipulations for bitmap-based images. Applications using this API typically
use low level Windows calls to render bitmaps to the app's screen window.

Note that TriGL JPEG handling depends on a specially built version of the
standard JPEG library where I/O calls are directed to the StmRead, StmWrite
file stream I/O APIs defined by TriOS.h.


Builds TriOS.lib library to link into other projects.
See TriOS.cpp, TriEx.cpp, TriGL.cpp comments for documentation.

Currently builds with VisualStudio 2008
Currently defines OSBASE,NOJPEG,NOPNG


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


For Linux, compile with:
    -Wno-write-strings -Wno-format
For Linux, link with:
    -lpthread -lrt -lm
Without NOPNG define, must also link with:
    -lpng
Without NOJPEG define, must also link with:
    -ljpeg (built with I/O calls linked to StmRead/StmWrite)
