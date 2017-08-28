# Thunderquake_Qt
Thunderquake::Qt technology preview &amp; proof-of-concept app, for Qt 5.9.1 to run RemGlk

This is my first Qt app, but it turned out as a decent enough working example.

# Running on PC desktop

This was tested on Ubuntu 17.04 x64 desktop system with Qt Creator version 4.3.1 and Qt version 5.9.1

To make a desktop app to run Glulxe interpreter stories:

1. check out from github
2. run the bash script desktop_build_RemGlk_and_terp_r0.sh to comple RemGlk + Glulxe
3. Open the Thunderquake_Qt_GitHub.pro file in Qt Creator
4. Try to run on your desktop

It should automatically scan your entire hard drive for .gblorb files for Glulx stories. Download some story files before you start the app. If the search does not work on your system, grep the code for "APPDEV_TEST_SETA" and please ask for help and I can give some tips on revising the starting path logic.

The bash script in #2 is pretty simple, if you are on non-Linux system you can easily run these steps by hand.

# Android instead of PC desktop

It can run on Android - but you have to use another project to compile the RemGlk + Glulxe interpreters.

There is another project called Thunderfall that is for using the Google Android NDK to compile RemGlk + Glulxe and other interpreters:
  https://github.com/WakeRealityDev/IFTerpCrossCompileAndroidCMake

The Thunderquake_Qt_GitHub.pro will need to be edited with what path to pull the compiled Glulxe binary exectuable from, etc. Ask for help if you need it.  Again, this is a proof-of-concept, usability on Android is pretty poor, especially the scrolling touchscreen support.


# ToDo list based on prior RemGlk experience

The file [README_state_of_this_app_code_as_Glk_frontend.txt](README_state_of_this_app_code_as_Glk_frontend.txt) has extensive comments on what is missing in terms of Glk features.
