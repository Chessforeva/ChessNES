/*
GUI of chess project
*/

/*----
 GUI for first title

*/
	    
// display "Nintendo Entertainment System"
void PrintBigLogo()
{
BYTE i,j,c = 185;
for(i=0;i<6;i++,c+=8)
 {
 setpos(0+i,10);
 for(j=0;j<8;j++,c++)
  write_char( (j==0 || j==7 || i==0 || i==5 ? 205 :  (c==205 ? 204 : c) ) );
 }
}

#define titCnt 7

BYTE *titles[] = {
 "Nintendo 8-bit NES project",
 "by Chessforeva, 2016",
 "AI plays noob chess",
 "Compiled with cc65",
 "Thanks to YY-CHR, nesst",
 "tileset editors",
 "Press start..."
};

BYTE titLoc[] = {
 9,1,
 11,3,
 14,5,
 18,5,
 21,2,
 22,5,
 26,8
};

// displays first title
void DisplayTitle()
{
BYTE i,j;

PPU_CTRL = 0;		// set tileset0
setpalette( (BYTE*)paletteTitle );	// set palette colours
PrintBigLogo();

for(i=0,j=0;i<titCnt;i++,j+=2)
 {
 setpos(titLoc[j],titLoc[j+1]);
 write_string(titles[i]);
 }

}


/*----
 GUI for chess board
*/

BYTE me2move = 1;	// 1-white,0-black

BYTE pieces_chr_bg_w[] = {145,148,151,154,157,202, 49,52,55,58,61,196};
BYTE pieces_chr_bg_b[] = {97,100,103,106,109,199, 1,4,7,10,13,193};

/*
This is an awkward thing - PPU should redraw in a small cycle (FPS thing)
Otherwise rendering glitches occur.
So, we keep some characters we have pushed to VRAM.
We render on disabled screen when more than cursor should redraw.
*/
BYTE cursbgchr[8][8][2];		// characters behind the cursor,drag or answer

#define XbyH(H) (1+(H*3))
#define YbyV(V) (1+((7-V)*3))
#define sqBgClr(V,H) ((V+H)&1)
#define IbyVH(V,H) ((V<<3)+H)
#define VbyI(I) (I>>3)
#define HbyI(I) (I&7)


// displays picture of piece on screen at position
// V,H=[0..7], p=[0-empty,
//  whites 1-pawn,2-knight,3-bishop,4-rook,5-queen,6-king,
//  blacks 11-pawn,...16-king]
void putPiecePic(BYTE V, BYTE H, BYTE p)
{
 BYTE i,j, b=sqBgClr(V,H); 
  
 #define ForEachOf9chars(a) \
	{ \
	if(!i) setpos(YbyV(V),XbyH(H)) \
	else setpos_fast(0x20); \
	for(j=0;j<3;j++) write_char(a); \
	}
 #define SaveBgChr(a,b) cursbgchr[V][H][0]= a; cursbgchr[V][H][1] = b;
 
 if(p)
  {
  BYTE *tb = (BYTE *) ( b ? pieces_chr_bg_w : pieces_chr_bg_b );
  BYTE pc = (BYTE) tb[(p ? p-1-(p>10?4:0) : 0)];
  for(i=0;i<3;i++,pc+=13) ForEachOf9chars(pc++);
  SaveBgChr(pc-14,pc-16);
  }
 else
  {
  BYTE b1=(b ? 49 : 1);
  for(i=0;i<3;i++) ForEachOf9chars(b1);
  SaveBgChr(b1,b1);
  }
}

// display board labels "A-H,1-8"
void PrintLabels()
{
BYTE i,c = 32;
for(i=0;i<8;i++,c+=16) 
 {
  setpos(25,1+XbyH(i));
  write_char(c);
 }
for(i=0;i<8;i++,c+=(i>5?1:16)) 
 {
  setpos(1+YbyV(i),0);
  write_char(c);
 }
}

// display "NES"
void PrintSmallLogo()
{
BYTE i,j, c = 205;
for(i=0;i<2;i++,c+=13)
 {
 setpos(i,28-1);
 for(j=0;j<3;j++) write_char(c++);
 }
}

// displays ToMove indicators
void PrintToMove()
{
setpos(24,25); write_char(Chess.w ? 243 : 0);
setpos(1,25); write_char(Chess.w ? 0 : 244);
scrollrefresh();
}

BYTE isCHECK;	// check status

// displays "#","+" if have to
void PrintCkMt()
{
CkiSq=0xff;
isCHECK = isCheck();
waitvblank();
setpos(5,26); write_char( isCHECK ? (Chess.gc? 245 : 246) : 0 );
scrollrefresh();
}

// displays game result
void PrintGameResult()
{
BYTE c=248;
if(isCHECK) c+=(Chess.w?1:-1);
setpos(12,26); write_char(c);
scrollrefresh();
}

//redraws square
#define PrintSquare(v,h) putPiecePic(v,h,Chess.B[ IbyVH(v,h) ])

// redraws all squares
void PrintBoard()
{
 BYTE h,v,i;
 for(i=0,v=0;v<8;v++)
  for(h=0;h<8;h++)
   putPiecePic(v,h,Chess.B[i++]);
}


/*----
 Cursor on square position
*/
struct curs_stru
{
 BYTE v,h;	// v,h=[0-none,1..8]
 BYTE r_v, r_h;	// to redraw previous character
 BYTE sq;	// square 0..63
} curs, drag, answer;

void PrintCursors()
{
BYTE V,H;
if(curs.h)
 {
 V=curs.v-1; H=curs.h-1;
 setpos(2+YbyV(V),2+XbyH(H));
 write_char( 250+sqBgClr(V,H));
 curs.sq = IbyVH(V,H);
 }
if(curs.r_h)
 {
 V=curs.r_v-1; H=curs.r_h-1;
 setpos(2+YbyV(V),2+XbyH(H));
 write_char( cursbgchr[V][H][0] );
 curs.r_v=0; curs.r_h=0;
 }
scrollrefresh();
}

void PrintDrags()
{
BYTE V,H;
if(drag.h)
 {
 V=drag.v-1; H=drag.h-1;
 setpos(2+YbyV(V),XbyH(H));
 write_char( 252+sqBgClr(V,H));
 drag.sq = IbyVH(V,H);
 }
if(drag.r_h)
 {
 V=drag.r_v-1; H=drag.r_h-1;
 setpos(2+YbyV(V),XbyH(H));
 write_char( cursbgchr[V][H][1] );
 drag.r_v=0; drag.r_h=0;
 }
scrollrefresh();
}

void PrintMoved()
{
BYTE V,H,sq;
if(answer.sq)
 {
 sq=answer.sq-1;
 V=VbyI(sq); answer.r_v=V+1;
 H=HbyI(sq); answer.r_h=H+1;
 setpos(2+YbyV(V),XbyH(H));
 write_char( 252+sqBgClr(V,H));
 }
else
 {
 if(answer.r_v)
  {
  V=answer.r_v-1; H=answer.r_h-1;
  setpos(2+YbyV(V),XbyH(H));
  write_char( cursbgchr[V][H][1] );
  }
 }
scrollrefresh();
}

void DisplayBoard()
{
PPU_CTRL = 16;			// set tileset1
setpalette( (BYTE*)paletteBoard );	// set palette colours
PrintLabels();
PrintSmallLogo();	
PrintBoard();
PrintToMove();
curs.h=5; curs.v=(me2move?2:7); //square e2 or e7
drag.h=0; drag.v=0; // none
curs.r_h=0; drag.r_h=0; answer.r_h=0; // none
PrintCursors();
}

// returns 1 if this piece is movable
BYTE ValidateDragPiece()
{
if(curs.h)
 {
 BYTE i,*o = Chess.gList;
 for(i=0;i<Chess.gc;i++,o+=5) if(o[0]==curs.sq) return 1;
 }
return 0;
}


struct move_stru_info
{
BYTE k;			// which move in the list?
BYTE f,t;		// from,to
BYTE epsq;		// en-passant square
BYTE rsq1,rsq2;		// rook square from and to
} Move;
	// if castling or en-passant then should redraw all board

void prepCsEp(BYTE k, BYTE *o)
{
 BYTE f=o[0], cs = o[3];
 if(cs)
  {
   if(cs==1) { Move.rsq1 = f+1; Move.rsq2 = f+3; }
   else { Move.rsq1 = f-1; Move.rsq2 = f-4; }
  }
 else { Move.rsq1 = 0; Move.rsq2 = 0; }
 Move.f=f; Move.t=o[1];
 Move.epsq = o[4];
 Move.k=k;
}
    
// returns 1 if this piece is movable to cursor square
BYTE ValidateMovePiece()
{
if(curs.h && drag.h)
 {
 BYTE i,k=0,*o = Chess.gList;
 for(i=0;i<Chess.gc;i++,o+=5, k++)
  {
  if(o[0]==drag.sq && o[1]==curs.sq)
    {
    prepCsEp(k,o);
    return 1;
    }
  }
 }
return 0;
}

// square drawings take a lot CPU
#define PrSq(id) { waitvblank(); PrintSquare(VbyI(id), HbyI(id)); scrollrefresh(); }
// do move and animate
void DoMOVE()
{
	    waitvblank();	// wait
	    MkMove(Move.k);
	      
	    histmoved[ movedcnt++ ]=Move.k;	// info. in openings
	    
	    waitvblank();
	    PrSq(Move.f);
	    waitvblank();
	    PrSq(Move.t);
	    
	    if(Move.epsq!=0xff) PrSq(Move.epsq);

	    if(Move.rsq1)	// castling rooks
	     {
	     waitvblank();
	     PrSq(Move.rsq1);
	     waitvblank();
	     PrSq(Move.rsq2);
	     }

	    waitvblank();
	    MoveGen();
	    waitvblank();	// wait for sure
	    PrintToMove();
	    waitvblank();
	    PrintCkMt();
	    if(!Chess.gc)		// if game is over
	     {
	     waitvblank();
	     PrintGameResult();
	     waitvblank();
	     }
}


// this is the loop in chess board screen
void LoopBoard()
{
BYTE V,H;
BYTE ndrag=0;

	 // if AI should do first move...
if(me2move!=Chess.w)
 {
	waitvblank();
	// Compute move
	AI( Move.t );
	waitvblank();
	thChr=16;
	putThinker();
	waitvblank();
	      
	if(AI_k)
	 {
	 prepCsEp(--AI_k, AI_o);
	 DoMOVE();
  
	 answer.sq = Move.t+1;
	 PrintMoved();			// display last move by opponent
	 waitvblank();
	 }
 }
		
while(1)
 {
 if(inkeyOtherKey())	// read joystick control
	{
	waitvblank();		// this makes it possible, wait for the right rendering time :))))
	
	if(!Chess.gc && (joySELECT || joySTART))
	 {
	 me2move^=1; inpRND+=keyTicked;	// if game over then swap sides
	 return;
	 }
	
	 // pause between key presses
	if( !(ndrag) && ((!keyTicked) || keyTicked>60))
	{
	keyTicked=0;
	V = curs.v; H = curs.h; 
	
	if(H>1 && joyLEFT) curs.h--;
	else if(V>1 && joyDOWN) curs.v--;
	else if(V<8 && joyUP) curs.v++;
	else if(H<8 && joyRIGHT) curs.h++;
	else if(joyA || joyB || joySELECT)
	 {

	 if(ValidateDragPiece())
	  {
	  if(!(drag.h==curs.h && drag.v==curs.v)) ndrag=1;
	  }
	 else
	  {
	  waitvblank();
	  if(ValidateMovePiece()) ndrag=150;
	  }

	 if(ndrag) keyTicked=1;
	 }
	
	// moves cursor
	if(!(V==curs.v && H==curs.h))
	 {
	 curs.r_v=V; curs.r_h=H;
	 PrintCursors();
	 keyTicked=1;
	 }
 
	}

	}
 else
	{
	// Do a lot during inactivity of user inputs 
		
	if(!(keyTicked & 31)) waitvblank();
	
	 if(ndrag)
	  {
	  ndrag++;
	  // drawings for piece drag
	  if(ndrag>25 && ndrag<80)
	   {
	    waitvblank();	// pause and wait to get CPU time
	    if(drag.h) { drag.r_v=drag.v; drag.r_h=drag.h; }
	    drag.v=curs.v; drag.h=curs.h;
	    PrintDrags();
	    ndrag=0;
	    keyTicked=1;
	   }

	  // drawings for piece movement
	  if(ndrag>80)
	   {
	    waitvblank();
	    answer.sq = 0;
	    PrintMoved();			// remove last answer	    
	    DoMOVE();
	    drag.h=0; drag.v=0; drag.sq=0;
	       
	    if(me2move!=Chess.w)
	     {
	      // Compute answer
	      AI( Move.t );
	      waitvblank();
	      thChr=16;
	      putThinker();
	      waitvblank();
	      
	      if(AI_k)
	       {
	       prepCsEp(--AI_k, AI_o);
	       DoMOVE();

	       answer.sq = Move.t+1;
	       PrintMoved();			// display last move by opponent
	       waitvblank();
	       }
	     }
	    
	    PrintCursors();
	    waitvblank();
	    
	    ndrag=0;
	    keyTicked=1;
	    clearprekey();	    
	   }	   
	   
	  }
	 
	}
 }

}