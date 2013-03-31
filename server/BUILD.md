Building WebSpec server plugin
==============================

Requirements
-
* **[Windows]** Visual Studio or Visual Express C++
* **[Linux]** gcc 4.6+ or clang
* **[OSX]** Xcode, or a manually install gcc4.6+ / clang
* cmake 2.6+
* hl2sdk-ob-valve - http://hg.alliedmods.net/hl2sdks/hl2sdk-ob-valve/
* libwebsockets-1.22 - http://git.warmcat.com/cgi-bin/cgit/libwebsockets/
    * only needed for headers, prebuilt library is included

Building
-
From your system's Command Prompt or Terminal:  
`cd <path-to-server-folder>`
`mkdir build`
`cd build`
  
**Windows**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Visual Studio 10" ..`  
Then open in Visual Studio/C++ Express  

**Linux**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -DCMAKE_BUILD_TYPE="Debug" -G "Unix Makefiles" ..`
`make`  
  
**Mac**  
`cmake -DHL2SDK=<path-to-hl2sdk...> -DLWS_DIR=<path-to-libwebsockets> -G "Xcode" ..`  
Then open in Xcode. Or, use the Linux commands