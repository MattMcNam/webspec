Building WebSpec server plugin
==============================

Notice
-
WebSpec will be moving to the new [2013 Source SDK](https://github.com/ValveSoftware/source-sdk-2013). These instructions will be updated once it is compiling and tested on all platforms.

Requirements
-
* **[Windows]** Visual Studio or Visual Express C++
* **[Linux]** gcc 4.6+ or clang, zlib-dev
* **[OSX]** Xcode, or a manually install gcc4.6+ / clang
* cmake 2.6+
* hl2sdk-ob-valve - http://hg.alliedmods.net/hl2sdks/hl2sdk-ob-valve/
* libwebsockets v1.22 - https://github.com/warmcat/libwebsockets, `checkout v1.22-chrome26-firefox18`
    * only needed for headers, prebuilt library is included

Building
-
From your system's Command Prompt or Terminal:  
`cd <path-to-server-folder>`
`mkdir build`
`cd build`
  
**Windows**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Visual Studio 10" ..`  
Then open in Visual Studio/C++ Express, choose Debug or Release configuration and build.  

**Linux**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -DCMAKE_BUILD_TYPE="Debug" -G "Unix Makefiles" ..`
`make`  
  
**Mac**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Xcode" ..`  
Then open in Xcode. Or, use the Linux commands