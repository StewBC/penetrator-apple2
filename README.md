# penetrator
 Remake of the ZX Spectrum game for the Apple II.

1. INTRODUCTION

This is a "port" of the Commander X16 version that's also in my repo's as just
penetrator.  I had to rewrite quite a bit of the drawing code and make some
other changes to make it work on the Apple II, and to make it fast enough.

This is also my very first program for the Apple II.  It was a learning
experience :)

2. CURRENT STATUS

The game is a pretty good replica of the original.  It is missing sound.  It's
also missing the demo attract mode, and the fireworks.  

3. TECHNICAL DETAILS

The game is written in 6502 assembly using the ca65 assembler.

The other interesting thing about the original is that it ran the AI and user
input twice for every world move. This version does not do it the same way. The
bullet update ran twice for each world update but now that the world moves while
the bullets move twice, the bullets could pass through the walls.  To fix this
issue, I have made the bullets 50% longer.

4. KEYS and JOYSTICK

The Apple 2 gameplay works only with a joystick since you cannot detect multiple
keys down on the Apple II.  The UI works with the keyboard or joystick and the
world editor uses both.

The menus all show the keys to advance.  There's also a help screen in the
editor.

5. THE FILES

I tried to comment all the code very thoroughly in the CX16 version.  Since this
version is derived from that version, the code is also well documented, but to
be honest, the comments may not always be accurate.  I did not do a pass to see
if they are accurate.

There are actually 2 programs in this.  The 1st is the game, and it's in
src/apple2.

* defs.inc       - Constants and definitions used throughout
* draw.inc       - VERA layer drawing.  drawSprite, for example
* edit.inc       - The built-in world editor
* file.inc       - Load and Save code
* fontdata.inc   - A ZX Spectrum font in 16x8
* game.inc       - The in-game logic, AI etc.  The bulk of the "game"
* input.inc      - User controls for game and editor
* logo.hgr       - 8Kb splash screen in HGR format
* logo.inc       - file that simply does an incbin on logo.hgr
* logodata.inc   - Line data for writing Penetrator and logo image data
* macros.inc     - print macros mostly
* penetrator.asm - Where the game starts, initial setup, etc.
* penetrator.cfg - ca65 configuration file
* rodata.inc     - Read-only data such as lookup tables, sprites etc.
* terrain.inc    - The terrain drawing routines
* text.inc       - In game text and print functions
* trndata.inc    - The world data (triplet - top, bottom and enemy pos)
* ui.inc         - User facing screens, name input, etc.
* variables.inc  - All non zero page variables
* zpvars.inc     - The zero page variables

The second is the ProDos loader that will auto-load the game.  It's in the
src/apple2.loader folder.  It has these files (all provided to me by Oliver
Schmidt)

* loader.cfg     - ca65 configuration file
* loader.s       - file to load and start the game

6. BUILDING THE GAME

Making the game has a few steps.  Use make and the Makefile on all OSs, that would be the easiest.  

Start by making the loader - this needs to be done once only.  
make TARGETS=apple2.loader

Next, make the game with:  
make

The next step is to make a bootable disk image.  For this, you will need some
other piece of 3rd party software.  It's called AppleCommander.  Once
downloaded, put it in the apple2 folder in the root (apple2 folder at same level
as Makefile).  This software will put the loader and game into the disk image.

Since I use Windows to develop, I have a file called build.cmd next to the
Makefile (and checked in here as well).  The contents of this file is listed
below (where the line ends with "\\" isn't actually a line-end, I just cut the
line there for readability, the next line is actually on the same line as the
line ending with "\\").

copy apple2\\template.dsk Penetrator.dsk  
java -jar apple2\\AppleCommander-win64-1.5.0.jar -p  Penetrator.dsk \\  
    penetrat.system sys < penetrator.apple2.loader  
java -jar apple2\\AppleCommander-win64-1.5.0.jar -as Penetrator.dsk \\  
    penetrat        bin < penetrator.apple2  
apple2\\Applewin.exe -d1 penetrator.dsk  

You will need to install Java to use AppleCommander.  The apple2/template.dsk is
a "blank ProDos floppy" that has the loader and the game placed on it by
AppleCommander.  I use the AppleWin emulator to run the game (-d1 penetrator.dsk
mounts the floppy in drive one, and then boots from it automatically).  In
powershell, when developing the game, I would just use this command to do it
all:  
make; .\\build.cmd

Find AppleCommander here (I used Version 1.5.0):  
https://github.com/AppleCommander/AppleCommander/releases

Find AppleWin here:  
https://github.com/AppleWin/AppleWin/releases

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
