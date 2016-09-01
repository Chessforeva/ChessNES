/*
 Chess project for NES.
*/


/* Includes */

#include <nes.h>

#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <device.h>
#include <time.h>


/* 16 bytes generated in editor tool and pasted here*/
const unsigned char paletteBoard[16]=
{ 0x0f,0x17,0x27,0x30,0x0f,0x01,0x15,0x31,0x0f,0x19,0x16,0x11,0x0f,0x09,0x19,0x29 };
const unsigned char paletteTitle[16]=
{ 0x0f,0x16,0x29,0x30,0x0f,0x01,0x21,0x31,0x0f,0x16,0x29,0x20,0x0f,0x09,0x19,0x29 };


#include "common.h"
#include "graph.h"
#include "joyst.h"

#include "chesslogic.h"
#include "chessopn.c"
#include "AI.c"
#include "GUI.c"


/* Program entry */
int main()
{
waitvblank();	/* NES.h */

OnloadInitChessGame();	// prepare chess logic
	
DisplayTitle();
			//turn on screen, visibility of background and sprites
grEI();			// enable my graphics, it is disabled by default,
			//PPU_CTRL1 = 0x90 screen is on, NMI on
for(;!joySTART && !joySELECT;) readjoykey();		

// loop forever
for(;;)
 {
 grDI();
 clearscr();
 setFEN( (BYTE*) chess_sFEN );

 DisplayBoard();
 
 waitvblank();
 grEI();
 LoopBoard();
 
}

return 0;
}

