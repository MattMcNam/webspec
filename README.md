webspec
=======

Spectate a TF2 server from a browser via WebSockets & Canvas.

Usage
-
**Server**  
Build the plugin by following `BUILD.md` in the server folder, then copy it to your TF2 server's addons folder.  
Load it via `plugin_load addons\webspec`, or create a VDF file.
**Client**  
In webspec.js, change `WebSpec.host` to your server's IP and the WebSpec port. (default localhost:28020)  
Note that Chrome requires the `--allow-file-access-from-files` launch option if running from disk.

To-do
-
See the Issues tab on GitHub, and use the tags to filter what you want.

License
-
Unless otherwise specified in source file, all of WebSpec is provided under the BSD 2-Clause License, which is available in full in LICENSE.md  
WebSpec is based in part on the work of the [libwebsockets project](http://libwebsockets.org)
