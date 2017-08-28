Slapped-together notes. NEEDS editing, proofreading.

RemGlk far far away
=======================
The design of this app is that the RemGlk interactive fiction interpreter ("engine") runs as another operating system process and stdin/stdout JSON is exchanged. This app is a JSON consumer and JSON emitter.

This is a very flexible design, as the classic IF engine is decoupled from the toolchain and can be remote or local - and there is absolutely no linking of any kind regarding software license conflicts or copyright concerns.

This also opens up the possibility of "plugin apps" that use intermediate pipes that intercept the JSON before it reaches this app. For example, a profanity filter or language translation app could be developed that consumes RemGlk's JSON and re-emits it - allowing chains of apps to act upon the data stream before it gets here to the presentation layer app.

NOTE: This project assumes you compile and link the interpreters + RemGlk in outside projects.  This app is a client to the RemGlk API.


The base code of this app
===========================
This app started from the Qt 5.9.1 example app for text-editor. It is a rich-text and subset of HTML QTextEdit.

The assumption was that a rich text editor already knew how to do all the text formatting that Glk needs: change text colors, alignment of paragraphs, bold, underline, font size changes, etc.

This has proven, so far, to be a decent enough assumption. During development, it can be useful to re-enable all the toobar icons for the original text edit, do not define HIDE_TEXT_EDIT_TOOLBARS in .pro file.


Incomplete
==============
Some things were barely started, such as StoryRunParameters class. This was intended to define a session of data file + engine (Glulx vs. Z-machine) selection. Class was created but not wired up.

The RemGlkProcess class needs more work to pick multiple engine backends (like Gargoyle or Thunderword). Right now it's pretty much one-engine app level of completeness in it's logic of which remglk binary to launch.

FindFilesWindow class and FindStoriesWindow class are variations. One is classical file searching taking from the Qt 5.9.1 example apps. A dedicated thread class was added to allow completely hidden background file finding.  FindFilewsWindow class could be removed if desired, it's mostly kept in the source tree for reference. The FindStoriesWindow class is reworked to emit an HTML list instead of a QTable as the HTML output works much better on 5" mobile phones with large sets of file. Intelligent grouping is done to cut the folder path name out of individual story file links.  Right now the story data file searching logic is Glulx only. It works pretty well on a Linux desktop system and Android device, but it needs to be fleshed out a lot more to support more than Glulx and to cache/save results so it isn't as frequently doing a full scan of the folders.

The app lacks any code to save or remember user settings. Size of the window, last story played, enabling or disabling rewriting of the Glk TextGrid lines, etc.  All preference-saving code needs to be added.


State of this App code as a Glk front-end
===========================================

Essentially, here is a ToDo list of what needs to be added to make this a full fledge front-end app to RemGlk:

1. Glk Timer support isn't complete. There is only a whisper of code to satisfy a timer response in the opening of the Glulx story "Six".
2. Glk windows are not properly parsed into a maintained data structure. A simple object has been started to hold each window, but it isn't populated from the JSON and it isn't really referenced yet. Right now the code assumes "class Zork-like two-windows" of a main Glk TextBuffer and a Glk TextGrid for the top status line.  The logic will essentially merge multiple Glk TextBuffer windows into the main output.
3. Glk style hints are not supported in RemGlk and nothing has been done to add that. This means colored text like in the Glulx story "Rover's Day Out" will not be colored.  There is some hacky code out there for Thunderword's fork of RemGlk to add style hints to the JSON output of RemGlk.
4. Glk player input system needs work. autocomplete typing was experimental and can be annoying. The code needs an option to disable that autocomplete logic.
5. Glk player input system does not track multiple Glk windows and only tracks if there is character or line input mode, or a combination of both.  If two windows have line input, or one window has line and another char, it does not know which window to send the input to when sending input back into RemGlk.  It blindly assumes one Glk TextBuffer window and sends the input to that.
6. Glk hyperlinks are working in RemGlk but this code does not yet account for them. Code needs to be added to render them and to assign a handler to send clicks back into RemGlk.
7. Clearing the screen (Glk TextBuffer window) probably doesn't work but that's pretty easy to do. Some stories will clear the screen after there Glk TextBuffer content after opening, find some examples to run that do that and add the logic.
8. Glk inline graphic images (inline to a Glk TextBuffer window), figures, are not coded yet. RemGlk does send the data about the images but there is no Qt C++ code yet to process that information and create a HTML img tag.
9. Glk graphics windows are entirely not working. Code needs to be added. Glk graphics windows are often used to create colored boxes or image graphics. The Glulx stores "Dead Cities by John Ingold" and "Six" are difficult test cases to get working.
10. Glk filename picking for save/restore and other data files needs to be added.  RemGlk supports triggering a user dialog to ask for a filename, but the Qt C++ code ignores it.
11. RemGlk's big absent feature is Glk sound. That's one area where contribution to RemGlk itself would be needed to add this Glk feature.  RemGlk already does a lot of what it needs to parse the BLORB file for graphics images and emit the JSON - in theory basic sound file support isn't a massive task.
12. The 12 standard Glk text styles are not really recognized and accounted for.  Heading and subheading are kind of there, but really it needs to be fleshed out and other styles accounted for.
13. Glk TextGrid support does have a primitive logic for multiple lines.  However, it doesn't account for RemGlk incremental frames (generations). If only 3 lines change of a 10 line TextGrid are revised - it doesn't have any concept of this. An array or QMap of the lines needs to be added and logic to update the proper index line. For development coding of this, a Z-machine interpreter bound to RemGlk with the stories Freefall/Tetris and Z-Racer are recommended.
14. Glk text alignment and justification of text isn't coded. Pretty rare to encounter stories that use this feature (or is it?).
15. Glk input in char mode special keystrokes such as cursor keys, ESC, F1..F12 function keys, etc. are not encoded into RemGlk's expectation. This can cause troubles with certain stories that expect those keys to be mapped, typically for menu systems.
16. Relating to #10 - the RemGlkProcess QProcess - the current directory, the working directory, needs to be set to a sensible place such as the user's home folder to dump story data files. The Glulx story "Six" is an example of a story that creates data files that it loads on repeat plays.
17. Relating to #8, a path can be sent to RemGlk via args that set the path it uses to reference image, sound, etc assets from blorb files.  This app, Thunderquake, will need to independently access those assets from the blorb file - as RemGlk does not currently have an option to extract them for consumer apps and expects that to be done independently.
18. Relating to #2, the RemGlk INIT string sent at story is fixed currently and some stories may not like the relatively small screen it passes along. This init string would need to be parameter-driven and not just a hard-coded line.
19. RemGlkProcess class has no concept of loss of engine. If it were remote and disconnected - it does not track nor does it present any user interface indication.

Kind of depressing to see how long that ToDo list was ;) It was fun to start from scratch knowing much more about Glk and RemGlk - but looking over this list you can see there is a lot of work to do.


Bugs
=============
This is an incomplete prototype app with lots of bugs still, but some mentioned:

1. Glux story "Six" ends up with bold text output stuck on. Other stories like Glulx "Bronze" don't seem to have this trouble? Test with cold-start of app each time.
2. Glux story "Six" has a 3-line Glk TextGrid status window. However, it expects a monospace font. Set a monospace font on the QLabel.


Design issue with Glk input
==============================
Right now there are two places to input into the story

1. QComboBox at the top that also allows free-form typing. Pressing [enter]/[return] or the "go" button next to it sends to RemGlk. it is only coded for Glk line input and has no concept of char input.
2. The entire canvas of the main Glk TextBuffer window is a Qt TextEdit that can process Glk line or character input.

Related to #2: this TextEdit allows you to cursor or type anywhere and even destroy the story output text by deleting it, etc. Design work and code need to be done as to make this more usable / user friendly.


Android
============
All the technical side to compile the Qt app and run RemGlk on Android has been extensively tested and works great in testing.  However, worth mentioning some very obvious issues:

1. What's missing is a lot of user-interface work.  For example: A QTextEdit scrolling presents a scroll-bar on the right in desktop PC fashion. It does not have normal Android app touch-screen scrolling.
2. The HTML presentation of the File Open found story files is very flexible with different screen sizes. However, it's too tightly packed and it would be difficult to use a touch-screen to click the link without accidentally hitting links on other lines of output.
3. BUG: on arm device the TextEdit input system isn't working correctly? However, it works fine on the x86 Android 7.1.1 emulator with both soft and hard keyboard. why?


Macintosh desktop / Apple iPhone / Windows desktop
=====================================================
Not yet tested in any way shape or form. May have compile problems or runtime problems due to assumptions.


Qt project / makefile system
===================================
Unsolved issues:

1. Unable to get the .pro file to understand anroid-arm vs. android-x86 as to which binary of the RemGlk engine app to bundle into the APK for Android builds.  remglkprocess.h has values of enginePickA set based on this need.
