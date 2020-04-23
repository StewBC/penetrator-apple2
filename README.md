# penetrator
 Remake of the ZX Spectrum game for the Apple II.

1. INTRODUCTION

This is a "port" of the Commander X16 version that's also in my repo's as just
penetrator.  I had to rewrite quite a bit of the drawing code and make some
other changes to make it work on the Apple II, and to make it fast enough.

This is also my very first program for the Apple II.  It was a learning
experience :)

2. CURRENT STATUS

The game is a pretty good replica of the original.  It is missing the demo 
attract mode, and the fireworks.  

Video https://youtu.be/fVms0nNVDtY and here's Oliver Schmidt's son, Pascal,
playing the game on real hardware: https://www.youtube.com/watch?v=rXC2JKuN594

There's also a pre-built Apple II disk of the project, Penetrator.dsk under the
releases tab.  This can be played directly in emulators.

3. TECHNICAL DETAILS

The game is written in 6502 assembly using the ca65 assembler.

Below is a CPU cycle profile of 1 game frame in stage 4.  It's a busy frame with
bullets, bombs and enemies.  Most importantly, it has a busy terrain at the top
and bottom.  

Hex | Dec | Frame % | Item
--- | --- | --- | ---
148F3 | 84211 | 100% | Total Frame
61F | 1567 | 2.0% | inputGame
0A | 10 | 0.0% | Misc
73 | 115 | 0.1% | gameWorldMove
05 | 5 | 0.0% | Misc
19 | 25 | 0.0% | gameCheckStage
08 | 8 | 0.0% | Misc
B80 | 2944 | 3.5% | gameAI
17 | 23 | 0.0% | Misc
7267 | 29287 | 34.8% | drawClearRows
6721 | 26401 | 31.4% | terrainDraw
38A8 | 14504 | 17.2% | drawEnemies
985 | 2437 | 2.9% | drawPlayer
581 | 1409 | 1.7% | drawBullets
6E3 | 1763 | 2.1% | drawBombs
E14 | 3604 | 4.3% | drawExplosions
06 | 6 | 0.0% | Misc
1A | 26 | 0.0% | drawPresent
11 | 17 | 0.0% | Misc
3C | 60 | 0.1% | Emulator error

  
As can be seen, clearing the area where the world will be drawn takes almost 35%
of the frame and drawing the terrain takes another 31% of the frame.  The
Emulator Error are unaccounted for cycles -discrepency between total cycles at
the start and end, vs. the cycles reported in each step.  I modifies the
emulator myself to do step reporting and I assume I must have missed someting.

4. KEYS and JOYSTICK

The Apple 2 gameplay works only with a joystick since you cannot detect multiple
keys down on the Apple II.  The UI works with the keyboard or joystick and the
world editor uses both.

The joystick can also drive the menu's.  Pressing right, for example, is
equivalent to pressing 1 (the UI menu options are selected through numbers).

The menus all show the keys to advance.  There's also a help screen in the
editor.

5. THE FILES

I tried to comment all the code very thoroughly in the CX16 version.  Since this
version is derived from that version, the code is also well documented, but to
be honest, the comments may not always be accurate.  I did not do a pass to see
if they are accurate.

There are actually 2 programs in this.  The 1st is the game, and it's in
src/apple2.

* audio.inc      - Routines to make the speaker beep
* defs.inc       - Constants and definitions used throughout
* draw.inc       - HGR buffer drawing.  drawSprite, for example
* edit.inc       - The built-in world editor
* file.inc       - Load and Save code
* fontdata.inc   - A ZX Spectrum font in reverse 8x8 - Apple II format
* game.inc       - The in-game logic, AI etc.  The bulk of the "game"
* input.inc      - User controls for game and editor
* logo.hgr       - 8Kb splash screen in HGR format
* logo.inc       - File that simply does an incbin on logo.hgr
* logodata.inc   - Line data for writing Penetrator and logo image data
* macros.inc     - Print macros mostly
* penetrator.asm - Where the game starts, initial setup, etc.
* penetrator.cfg - ca65 configuration file
* rodata.inc     - Read-only data such as lookup tables, sprites etc.
* terrain.inc    - The terrain drawing routines
* text.inc       - In game text and print functions
* trndata.inc    - The world data (triplet - top, bottom and enemy pos)
* ui.inc         - User facing screens, name input, etc.
* variables.inc  - All non zero page variables (stats, scores, draw buffers)
* zpvars.inc     - The zero page variables (most variables)

The second is the ProDos loader that will auto-load the game.  It's in the
src/apple2.loader folder.  It has these files (all provided to me by Oliver
Schmidt)

* loader.cfg     - ca65 configuration file
* loader.s       - file to load and start the game

6. BUILDING THE GAME

Making the game has a few steps.  Use make and the Makefile on all OSs, that
would be the easiest.  

Start by making the loader - this needs to be done once only.  
make TARGETS=apple2.loader

Next, make the game with:  
make

The next step is to make a bootable disk image.  For this, you will need 3rd
party software.  I use AppleCommander.  This software will put the loader and
game into the disk image. You will need to install Java to use AppleCommander.  

The apple2/template.dsk is a "blank ProDos floppy" that has the loader and the
game placed on it by AppleCommander.

To make the disc image, set an environment variable to point at apple commander
(see notes) and then use the command:  
make dsk

This will make a disc named Penetrator.dsk which can be loaded up in an
emulator.

If you want to edit the code and get into some iterative development/testing,
you can edit the Makefile and fill in the commands commands neccesary to launch
an emulator as well.  Look for apple2_EMUCMD and set that as neccesary.  The
simplest is to call test.sh or test.bat and fill in a complete command in the
batch file or shell script.  However you choose to do this, once done, you can
do everything with the command:  
make dsk test

NOTES:
1) Find AppleCommander here (I used Version 1.5.0):
https://github.com/AppleCommander/AppleCommander/releases  
2) Set the environment variable (or change the Makefile-dsk.md) to point at the
apple commander jar file.  Here's how it's done for different shell's:  
 Powershell:  
   $env:AC = "path to apple commander.jar"  
 cmd.exe   
   set AC="path to apple commander.jar"  
 bash (Unix or MacOS terminal):  
   export AC="path to apple commander.jar"  

7. CREDITS

* Phillip Mitchell and Beam Software for creating and publishing the game in
  1982.  
* A special call-out to Oliver Schmidt who provided me with invaluable
  assistance to get me going on the Apple II.
* Everyone involved in the Apple II projects (AppleWin | AppleCommander).
* Everyone involved in making the cc65 tools, it's very good.

8. CONTACT

Feel free to contact me at swessels@email.com if you have thoughts or
suggestions.

Thank you  
Stefan Wessels  
16 December 2019 - Initial Revision  
