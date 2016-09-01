-----------------------------------------

 Nintendo NES (8-bit 6502 processor)
 project by Chessforeva

http://chessforeva.blogspot.com/2016/09/nes-chess-project.html
http://github.com/Chessforeva/ChessNES

 Play beginners chess with a very noob AI.

 This is a freesource C project designed for
awesome cc64 compiler - a very good NES development platform.

Tools used:
 a) cc65 compiler for 6502 CPU (www.cc65.org)
 b) nesst tileset editor tool (http://shiru.untergrund.net/files/nesst.zip)
 c) yy-chr v0.99 tileset editor
 d) n2.exe tool for tileset writing into existing .nes-file
 e) other sources on internet
 f) Notepad++

Keyboard input:
 Arrows and Select (usually [F] in emulators)


Emulators tested:

FCEUX 2.2.2 (keys: arrows,F)
JNES 1.1 (arrows,F)
Nestopia v.1.40 (arrows,L)
javascript Nezulator (arrows,X)

Online version:
http://sisenis-1193.appspot.com/ChessNES/NezulatorChess.htm

Developer remarks:

NES graphics is too awkward. Should fit into framerate.
Use waitvblank() function wherever it is possible to get safe and free CPU resource
and avoid screen blinking glitches caused by drawing interrupts.

No sprites in this project, no scrollings, sorry.
2-bank 8Kb tileset chr-file edited in editor tool.
One bank (charset) is for texts, the other - for chess board and pieces.

AI is fast and dumb. Tries to capture, to checkmate and move forward.
No calculations at all.
A small openings base for first chess moves.

Chessforeva
2016,sep.