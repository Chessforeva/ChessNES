/*
chess AI

1st. scan all moves

*/


// use input counter as random number
#define RND inpRND

// put a "king" at square and verify check,then restore
#define isAttacked() { \
  CkiSq = t; p0 = Chess.B[t]; \
  Chess.B[t] = (Chess.w?6:16); y = isCheck(); Chess.B[t] = p0; CkiSq = myK; }
		
#define pieceAt(sq) (Chess.B[sq]%10)


BYTE *AI_o;	// pointer to move info
BYTE AI_k;	// index counter in possible moves list-1

// data about move
BYTE capt[100];	// capture flag
BYTE sfmv[100];	// safe move

BYTE AI_OPP_MOVEGEN [500];		// move generation for opponent

BYTE thChr;

// put thinking character (code 16)
void putThinker()
{
 thChr=16-thChr;	// blink it
 setpos(0,0);
 write_char(thChr);
 scrollrefresh();
}

// see, what can do the last moved piece by opponent
void seeAttackers()
{
 waitvblank();
 mg_L = AI_OPP_MOVEGEN;
 Chess.gc = 0;
 mg_X=mg_sq&7; mg_Y=mg_sq>>3;
 switch(pieceAt(mg_sq))
 {
 case 1: { pawngen(); break; }
 case 2: { knightgen(); break; }
 case 3: { bishopgen(); break; }
 case 4: { rookgen(); break; }
 case 5: { bishopgen(); rookgen(); break; }	// if queen
 case 6: { kinggen(); break; }
 }
}

void AI( BYTE LastMoveSquare )
{

/* already generated */
//MoveGen();

BYTE i, *o, j, *oj;
BYTE beat, bbest=0;	// best beat
BYTE p,pb,po, p0,t,y;
BYTE sgc,ogc;
BYTE r1 = RND&1;

#define Vbyi(I) (I>>3)
#define Hbyi(I) (I&7)
#define distVH(a,b) (a>b?a-b:b-a)

BYTE myK = (Chess.w ? Chess.WK : Chess.BK), oppK = (Chess.w ? Chess.BK : Chess.WK);
BYTE kH = Hbyi(oppK), kV = Vbyi(oppK);
BYTE pV = (Chess.w ? 7 : 0);
BYTE df,dt,tV,dV,dH,Ef,Et,E,Ep, Ebest=0;	// evaluation

AI_k=0;
CkiSq = myK;

thChr = 0;

if(!Chess.gc) return;	// can not move now

waitvblank();	// wait for sure CPU time
putThinker();

AI_k = OpeningMove();	// see book moves
if(AI_k)
 {
  for(i=1, o=Chess.gList; i<AI_k; i++,o+=5);
  AI_o=o;
  waitvblank();
  return;	// if there is an answer
 } 

waitvblank();
putThinker();

// find best capture (ignores en-passant)
for(i=0, o=Chess.gList; i<Chess.gc; i++,o+=5)
 {
 sfmv[i]=1;		// all moves safe at first
 p = pieceAt(o[1]);
 capt[i]=p;
 if(p)
  {
  beat = (10+(p==2?3:p))-pieceAt(o[0]);	// calculate best capture, Bishop is equal knight
  if(beat<10) sfmv[i]=0;	// if losing material
  else if (beat>bbest) { AI_k = i+1; AI_o=o; bbest=beat; }
  }
 }
if(AI_k) return;	// capture now!

waitvblank();
putThinker();

// see all safe captures 
for(i=0, o=Chess.gList; i<Chess.gc; i++,o+=5)
 {
 if(capt[i])
  {
  waitvblank();	// wait for sure CPU time
  t=o[1];
  isAttacked();	// can I take an unprotected piece
  if(!y) { AI_k = i+1; AI_o=o; }
  else sfmv[i]=0;	// losing piece
  }
 }
if(AI_k) return; 	// simply take piece
waitvblank();
putThinker();

// see all non-capture moves
for(i=0, o=Chess.gList; i<Chess.gc; i++,o+=5)
 {
 if(!capt[i])
  {
  waitvblank();	// wait for sure CPU time
  t=o[1];
  isAttacked();	// can I move here without losing a piece
  if(y) sfmv[i]=0;	// this is a bad move, square under threat
  }
 }
waitvblank();
putThinker();

if(LastMoveSquare)
{
waitvblank();
sgc = Chess.gc;		// save current list counter
Chess.w^=1;	//let move again
mg_sq=LastMoveSquare;	// square of last move
seeAttackers();	// generate moves for last moved piece
Chess.w^=1;
ogc = Chess.gc; Chess.gc = sgc; // swap back

waitvblank();
// look for moves attacking our pieces
for(pb=0,i=0, o=Chess.gList ; i<Chess.gc; i++,o+=5)
 {
 t=o[0]; p=pieceAt(t); po=pieceAt(LastMoveSquare); po=(po==3?2:po);
 if((r1 || p!=1) && p>pb)		// sometimes ignore pawns, faster
  {
  waitvblank();
  for(j=0, oj=AI_OPP_MOVEGEN; j<ogc; j++, oj+=5)
   {
   if(sfmv[i] && (oj[1]==t))
    {
     ogc=1;
     if(p<=po)
      {
      // see if my piece protected
      waitvblank();
      sgc = Chess.gc;		// save current list counter
      mg_sq=t;			// square of my piece
      Chess.w^=1;	//let move again
      seeAttackers();
      Chess.w^=1;	//restore
      ogc = Chess.gc; Chess.gc = sgc; // swap back
      }
     if(ogc) {AI_k = i+1; AI_o=o; pb=p; }	// should move this piece away
    }
   }
  }
 }
}
if(AI_k) return; 	// try move our piece away
waitvblank();
putThinker();

// try attack king check+
if(RND&3)
 for(ogc=1, i=0, o=Chess.gList; i<Chess.gc; i++,o+=5)
 {

 if(sfmv[i])
  {
  waitvblank();
  MkMove(i);
  CkiSq = oppK;
  if(isCheck())
   {
   AI_k = i+1; AI_o=o;
	// maybe king can not move away, checkmate possible
   waitvblank();
   sgc = Chess.gc;		// save current list counter
   mg_sq=oppK;			// square of king move
   seeAttackers();	// generate king moves
   ogc = Chess.gc; Chess.gc = sgc; // swap back
   }
  CkiSq = myK;
  waitvblank();
  UnMkMove();
  if(!ogc) return;  
  }
 }

if(AI_k) return; 	// check+
waitvblank();
putThinker();

// now see only safe moves
for(i=0, o=Chess.gList; i<Chess.gc; i++,o+=5)
 {
 if(sfmv[i])
 {
  // do without thinking!
  // promotion is excellent, castling is good, en-passant also good
  if( (o[2]==5) || o[3] || (o[4]!=0xFF)) {AI_k = i+1; AI_o=o; return; }

  if((r1+i)&1)
  {

#define Eset(m) if(E<m) E=m;
#define Eadd(n,m) if(Ep==n) Eset(m);

  p = pieceAt(df);  
  waitvblank();
  df = o[0]; dt = o[1]; tV=Vbyi(dt);
  dV = distVH(tV,kV);
  dH = distVH(Hbyi(dt),kH);
  Et = dH + dV;
  Ef = distVH(Vbyi(df),kV) + distVH(Hbyi(df),kH);
    // move closer to king, small pieces + some random
  E = 200-(Et<<3)+Ef + r1;
  if(Chess.mn<16)
   {
    E-=(p<4?dV:(p<<2)+(p==6?50:0));	// do not move king
   }   
  else
   if(p==1)
   {
    Ep = distVH(pV,tV);
    E+=40-(Ep<<3);
    // promotion near
    Eadd(1,252); Eadd(2,250); Eadd(3,240);
   }
  waitvblank();
  if(E>Ebest)
   {
    Ebest = E; AI_k = i+1; AI_o=o;
   }
  }
 }
 }
if(AI_k) return; 	// seems best move right now
waitvblank();
putThinker();

/*
// see all moves now, too slow
for(i=0, o=Chess.gList ; i<Chess.gc; i++,o+=5)
 {
  waitvblank();
  MkMove(i);
  Chess.gList = AI_OPP_MOVEGEN;
  Chess.gc=0;
  waitvblank();
  MoveGen();
  if(!Chess.gc) { AI_k = i+1; AI_o=o; }
  Chess.gList = MOVEGEN;	// restore
  Chess.gc=sgc;
  waitvblank();
  UnMkMove();
 }
waitvblank();

if(AI_k) return; 	// simply take piece

*/

 AI_k=1; AI_o=Chess.gList;

}

