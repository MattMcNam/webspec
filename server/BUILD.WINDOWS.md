Building on Windows
==============

Requirements
-
* Visual Studio or Visual Express C++
* cmake 2.6+
* hl2sdk-ob-valve - http://hg.alliedmods.net/hl2sdks/hl2sdk-ob-valve/
* libwebsockets-1.22 - http://git.warmcat.com/cgi-bin/cgit/libwebsockets/
    * only needed for headers, prebuilt library is included

Building (Provided VS2010 sln)
-
* Set up environment variables for HL2SDK, pointing to the hl2sdk-ob-valve directory, and LWS_DIR, pointing to the libwebsockets directory.
* Open the provided webspec.sln file, choose Release or Debug and Build Solution.

Building (CMake, not fully functioning)
-
* Create a directory named build and `cd` into it.  
* `cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Visual Studio 10" ..`  
    * Note that the paths require UNIX `/` directory separators, Windows `\` separators will cause a cmake error.  
* Open webspec.sln in the build directory and, for both Release and Debug builds, change the 'webspec' project's Properties as follows:
    * Release: C/C++ -> Code Generation -> Change 'Runtime Library' from 'Multi-threaded DLL' to 'Multi-threaded'  
	* Debug: Change from 'Multi-threaded Debug DLL' to 'Multi-threaded Debug'  
	* This seems to be a problem with cmake, investigating a workaround.
* There should now be a Debug or Release directory in the build directory, containing webspec.dll.
