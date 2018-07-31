
REminiscence README
Release version: 0.3.2
-------------------------------------------------------------------------------


About:
------

REminiscence is a re-implementation of the engine used in the game Flashback
made by Delphine Software and released in 1992. More informations about the
game can be found at [1], [2] and [3].


Compiling:
----------

Update the defines in the Makefile if needed. The SDL, zlib and modplug
libraries are required.


Data Files:
-----------

You will need the original files of the PC (DOS or CD) or Amiga release.

To hear background music during polygonal cutscenes with the PC version,
you'll need to copy the .mod files of the Amiga version :

	mod.flashback-ascenseur
	mod.flashback-ceinturea
	mod.flashback-chute
	mod.flashback-desintegr
	mod.flashback-donneobjt
	mod.flashback-fin
	mod.flashback-fin2
	mod.flashback-game_over
	mod.flashback-holocube
	mod.flashback-introb
	mod.flashback-jungle
	mod.flashback-logo
	mod.flashback-memoire
	mod.flashback-missionca
	mod.flashback-options1
	mod.flashback-options2
	mod.flashback-reunion
	mod.flashback-taxi
	mod.flashback-teleport2
	mod.flashback-teleporta
	mod.flashback-voyage

To hear voice during in-game dialogues, you'll need to copy the 'VOICE.VCE'
file from the SegaCD version to the DATA directory.


Running:
--------

By default, the engine will try to load the game data files from the 'DATA'
directory (as the original game did). The savestates are saved in the current
directory. These paths can be changed using command line switches :

    Usage: rs [OPTIONS]...
    --datapath=PATH   Path to data files (default 'DATA')
    --savepath=PATH   Path to save files (default '.')
    --levelnum=NUM    Level to start from (default '0')
    --fullscreen      Fullscreen display
    --scaler=INDEX    Graphics scaler
    --language=LANG   Language (fr,en,de,sp,it)

In-game hotkeys :

    Arrow Keys      move Conrad
    Enter           use the current inventory object
    Shift           talk / use / run / shoot
    Escape          display the options
    Backspace       display the inventory
    Alt Enter       toggle windowed/fullscreen mode
    Alt + and -     change video scaler
    Alt S           write screenshot as .bmp
    Ctrl S          save game state
    Ctrl L          load game state
    Ctrl + and -    change game state slot

Debug hotkeys :

    Ctrl F          toggle fast mode
    Ctrl I          Conrad 'infinite' life
    Ctrl B          toggle display of updated dirty blocks
    Ctrl M          mirror mode (right - left swapped)


Credits:
--------

Delphine Software, obviously, for making another great game.
Yaz0r, Pixel and gawd for sharing information they gathered on the game.
Nicolas Bondoux for sound fixes.


Contact:
--------

Gregory Montoir, cyx@users.sourceforge.net


URLs:
-----

[1] http://www.mobygames.com/game/flashback-the-quest-for-identity
[2] http://en.wikipedia.org/wiki/Flashback:_The_Quest_for_Identity
[3] http://ramal.free.fr/fb_en.htm
