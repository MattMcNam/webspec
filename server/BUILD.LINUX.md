Building on Linux
==============

Requirements
-
* gcc (Valve uses 4.6)
* cmake 2.6+
* hl2sdk-ob-valve - http://hg.alliedmods.net/hl2sdks/hl2sdk-ob-valve/
* libwebsockets-1.22 - http://git.warmcat.com/cgi-bin/cgit/libwebsockets/
    * only needed for headers, prebuilt library is included

Building
-
* Craete a directory named build and `cd` into it.  
* `cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Unix Makefiles" ..`  
* `make`  
* There should now be a webspec.so file at the root of the build directory.  
* While not necessary, running the Clang Static Analyzer, `scan-build`, is recommended.  
