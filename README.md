webspec
=======

Spectate a TF2 server from a browser via WebSockets & Canvas.

Usage
-
**Server**  
Transitioning to CMake, please refer to `BUILD.LINUX.md` or `BUILD.WINDOWS.md` in the server directory.  
For Linux builds: GCC & Clang are more pedantic, so expect new warnings and errors. Running Clang Static Analyzer is also recommended.  
**Client**  
Change `WebSpec.host` to your server's IP and the WebSpec port. (default localhost:28020)  
Note that Chrome requires the `--allow-file-access-from-files` launch option if running from disk.

To-do
-
See `TODO.md`

License
-
Unless otherwise specified in souce file, all of WebSpec is provided under the BSD 2-Clause License. It is available in full in LICENSE.md  
WebSpec is based in part on the work of the libwebsockets  project (http://libwebsockets.org)
