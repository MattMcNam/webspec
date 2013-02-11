TF2 WebSpec To-do list
======================

GitHub contributors can edit this list as they wish; otherwise you can either:

1. Create a new 'issue' via GitHub and tag it as `enhancement` or `todo` (**preferred**)
2. Modify this file in your own fork and submit a pull request, *along with relevant/useful code changes*

--

**Server**

* Feature parity with SourceMod base, `webspec.sp` (Remove once these are implemented)
    * Send message when player connects or disconnects
    * Send message when players chat
    * Send message when player dies, add extra flag if uber was dropped
    * Send message when player spawns
    * Send message when player changes name
    * Send message when player changes team
    * Send message every *x* milliseconds of player locations (x to be determined, 300 worked with SourceMod)
    * Ditch chat between WebSpec spectators ?
    * Ditch sending amount of connected spectators to all spectators
* **Linux build**, considering using CMake
* Move `webspec-dumping.h` functionality to another repo
* Clean up code in various headers
    * Clean code style to more closely match Valve's
* Move from virtual function indexes to signature scanning where possible
* Ideally, make all code as 'update-proof' as possible
* Look into sending binary WebSocket messages

**Client**

* Redo rendering code to use variable framerate (adding to repo after this)
* Add Pyro portrait (currently uses Heavy's)
    * Get transparant class icons
* Replace player dots with class emblems
* Toggles for all HUD elements, & names above players
* Look into binary message parsing with JavaScript
