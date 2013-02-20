TF2 WebSpec To-do list
======================

GitHub contributors can edit this list as they wish.  
Otherwise you can create a new 'issue' via GitHub and tag it as `enhancement` or `todo`  

--

**Server**

* Feature parity with SourceMod base, `webspec.sp` (Remove once these are implemented)
    * Send message when player connects or disconnects
    * Send message when players chat (only during pause and only to casters??)
    * Send message when player changes name
    * Send message when player changes team
    * Ditch chat between WebSpec spectators ?
    * Ditch sending amount of connected spectators to all spectators
* Fix cmake with Windows
* Clean up code in various headers
    * Clean code style to more closely match Valve's
* Move from virtual function indexes to signature scanning where possible
* Ideally, make all code as 'update-proof' as possible
    * Eg. Signature scanning rather than virtual function indexes
* Look into sending binary WebSocket messages

**Client**

* _Basically, a full rewrite from SrcTV2D base_
* _Added to repo to allow developers to test with_
* Redo rendering code to use variable framerate
* Add Pyro portrait (currently uses Heavy's)
    * Get transparant class icons
* Replace player dots with class emblems
* Toggles for all HUD elements, & names above players
* Look into binary message parsing with JavaScript
