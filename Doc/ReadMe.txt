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
Without NONPG define, must also link with:
    libpng.lib
Without NOJPEG define, must also link with:
    jpeg.lib (built with I/O calls linked to StmRead/StmWrite)

For Linux, compile with:
    -Wno-write-strings -Wno-format
For Linux, link with:
    -lpthread -lrt -lm
Without NONPG define, must also link with:
    -lpng
Without NOJPEG define, must also link with:
    -ljpeg (built with I/O calls linked to StmRead/StmWrite)
