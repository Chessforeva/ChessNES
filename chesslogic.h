/*
        Chess calculations: chess board, move generator, history
        (cc65 code for c64,NES)
*/

// somewhere in free space
BYTE MOVEGEN [500]; // (~100 x 5 max.bytes)
BYTE MOVEHIST [2400]; // (~300 x 8 max.bytes)  //Carlsen plays 122 moves, btw.                                                  

 // char* "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
static const BYTE chess_sFEN[57] = {
  14, 12, 13, 15, 16, 13, 12, 14, 47, 11, 11, 11, 11, 11, 11, 
  11, 11, 47, 56, 47, 56, 47, 56, 47, 56, 47, 1, 1, 1, 1, 1, 
  1, 1, 1, 47, 4, 2, 3, 5, 6, 3, 2, 4, 32, 119, 32, 6, 5, 16, 
  15, 32, 45, 32, 48, 32, 49, 
 0 };

 // char* "KQkq"
static const BYTE chess_cs[5] = {
  6, 5, 16, 15, 
 0 };

struct chess_stru
 {
 char B[64];    // chess board 64 squares [ 0 ,PNBRQK (1-6), pnbrq (11-16) ]
 BYTE w;                // side to move now (1-white,0 black)
 BYTE cs;               // KQkq castlings (bit3...bit0)
 BYTE ep;               // en-passant square or 0xFF

 BYTE *gList;   // pointer to list of possible moves
 BYTE gc;               // count of moves possible now after MoveGen
                                
 BYTE *mHist;   // pointer to history of moves
 BYTE mn;               // move number currently
 BYTE *mh;              // pointer to end of hist.
 
 BYTE WK;               // square of white king
 BYTE BK;               // square of black king
 
 } Chess; 


//============= MOVE AND UNDO

void MkMove(BYTE n) {   /* process move k from the list created by MoveGen */

        BYTE z,p,f,t,e,k, E1,A1,H1;
        BYTE *o = Chess.gList, *h = Chess.mh;
        o+=(n<<2)+n; Chess.mh+=8;
        memcpy( (void *)h, (void *)o, 5 ); h+=5;
        f=o[0]; t=o[1]; p=o[2]; k=o[3];
        e=o[4]; // was this en-passant capture, captured pawn square
        z=Chess.B[f];
                
        *(h++) = Chess.B[t];    // captured piece
    *(h++) = Chess.cs;  // castling info
    *(h++) = Chess.ep;  // en-passant square to save
        
        Chess.B[t]=( (p==0) ? z : (Chess.w ? p+0 : (p+10)) );
        switch(z)
                {
                case  6: { Chess.WK=t; break; }
                case 16: { Chess.BK=t; break; }
                }
        
        Chess.B[f]=0;
    if(e!=0xFF) Chess.B[e]=0;
        
        E1=(Chess.w?4:60); H1=(Chess.w?7:63); A1=(Chess.w?0:56);
    if(f==E1) Chess.cs&=(Chess.w ?3:12);
    if(f==H1 || t==H1) Chess.cs&=(Chess.w?7:13);
    if(f==A1 || t==A1) Chess.cs&=(Chess.w?11:14);
    if(k!=0)
                {
                if(k==1)
                        {
                        Chess.B[H1-2]=Chess.B[H1]; Chess.B[H1]=0;
                        Chess.cs&=(Chess.w?7:13);
                        }
                else
                        {
                        Chess.B[A1+3]=Chess.B[A1]; Chess.B[A1]=0;
                        Chess.cs&=(Chess.w?11:14);
                        }
                }

        Chess.ep=0xFF;
    if((z==1 || z==11) && ( t==(f+16) || f==(t+16) )) Chess.ep = (t+f)>>1;

        Chess.w^=1;
        if(Chess.w) Chess.mn++;
};

void UnMkMove() {       /* process unmake last move */

        BYTE f,t,k,e,b;
        BYTE *h;
        Chess.mh-=8;
        h = Chess.mh;
        f=h[0]; t=h[1]; k=h[3]; e=h[4];
        b=Chess.B[t];
        Chess.B[f]=( h[2]==0 ? b : (Chess.w?11:1) );
        Chess.B[t]=h[5];
        switch(b)
                {
                case  6: { Chess.WK=f; break; }
                case 16: { Chess.BK=f; break; }
                }
        
    if(e!=0xFF) Chess.B[e]=(Chess.w?1:11);
    if(k!=0)
                {
                if(k==1)
                        {
                        Chess.B[f+3]=Chess.B[f+1]; Chess.B[f+1]=0;
                        }
                else
                        {
                        Chess.B[f-4]=Chess.B[f-1]; Chess.B[f-1]=0;
                        }
                }
        Chess.cs = h[6];
        Chess.ep = h[7];
        if(Chess.w) Chess.mn--;
        Chess.w^=1;

};

//============= MOVE GENERATION

//------------------------- [ check? checkmate? stalemate? ]------------------
/*
 detects check right now to king to move
        
 */
BYTE CkiSq;     //Give 0xFF to scan all kings,
                        // or square of king to verify (faster way)

BYTE isCheck() { 

    BYTE t,X,Y,x,y,q,i,j,c,x1,y1, xabs,yabs, kg=(Chess.w?6:16);
        i=(CkiSq==0xFF ? 0 : CkiSq);
    for(t=0; t==0 && i<64; i++)
        {
        if(Chess.B[i]==kg || CkiSq==i)
                {
         /* directional moves */
                X=i&7; Y=i>>3;
                for(x=0;t==0 && x<3;x++)
                 for(y=0;t==0 && y<3;y++)
                  for(q=1;t==0 && q<8;q++)
                        {
                        if( ( x==0 ? (X>=q) : ((x==1)||( (X+q)<8))  ) &&
                                ( y==0 ? (Y>=q) : ((y==1)||( (Y+q)<8)) ) )
                                {
                                x1 = (x==1 ? X : X+q);
                                y1 = (y==1 ? Y : Y+q);
                                j= ((y==0 ? Y-q : y1 )<<3);
                                j+= (x==0 ? X-q : x1 );
                                c=Chess.B[j];
                                if( c!=0 && (c>10)==Chess.w )
                                        {
                                        if((x==1 || y==1) && ((Chess.w ? 14 : 4 )==c)) t=1;     //rooks
                                        else if((x!=1 && y!=1) && ((Chess.w ? 13 : 3 )==c)) t=1;        //bishops
                                        else if((Chess.w ? 15 : 5 )==c) t=1;            //queens
                                        else if((q==1) && ((Chess.w ? 16 : 6 )==c)) t=1;        // kings
                                        else if((q==1) && (x!=1) && (y!=1) &&
                                                ((y>1)==Chess.w) && ((Chess.w ? 11 : 1 )==c)) t=1;      // pawns
                                        }
                                if(c!=0) q=8;
                                }
                        else q=8;
                        }

                /*knight moves */
                for(x=0;x<5;x++)
                 for(y=0;y<5;y++)
                        {
                        xabs= (x<2?2-x:x-2);
                        yabs= (y<2?2-y:y-2);
                        if( (xabs+yabs)==3 )
                                if( (x<2 ? (X>=xabs) : (X+xabs<8)) &&
                                        (y<2 ? (Y>=yabs) : (Y+yabs<8)) )
                                        {
                                        j=((Y+y-2)<<3) + (X+x-2);
                                        if((Chess.w ? 12 : 2 )==Chess.B[j]) t=1;
                                        }
                        }
                }
                if(CkiSq!=0xFF) break;
        }
        return t;
};


// global variables for move generation and movements
BYTE *mg_L, mg_sq, mg_X, mg_Y;

void push_genml(BYTE f, BYTE t, BYTE p, BYTE k, BYTE e)
        {
        Chess.gc++;
        (*mg_L++)=f; (*mg_L++)=t; (*mg_L++)=p;
        (*mg_L++)=k; (*mg_L++)=e;
        };

/* Adds move to the list.
Parameters:
        move from square, to square, promoted piece, castling info
*/
void addmove(BYTE f, BYTE t, BYTE r, BYTE k)
        {
        BYTE g=1, p, e=0xFF, u;

        if(k==0)                // if not castling
                {
                /*
                Validate move, is our king under threat after movement.
                Simply pre-move piece, verify check and place back without saving.
                */
                p= Chess.B[f];
                u= Chess.B[t];
                Chess.B[f] = 0;
                Chess.B[t] = p;
        
                if((Chess.ep==t) && (p==1 || p==11)) e = (Chess.w ? t-8 : t+8);
                if(e!=0xFF) Chess.B[e]=0;
                
                if(Chess.w)
                        {
                        CkiSq = ((p==6) ? t : Chess.WK);
                        }
                else
                        {
                        CkiSq = ((p==16) ? t : Chess.BK);
                        }
                g = isCheck()^1;
                CkiSq = 0xFF;

                Chess.B[f] = p;
                Chess.B[t] = u;
                if(e!=0xFF) Chess.B[e]=(Chess.w?11:1);
                }
        
        if(g)   // good move, save it
                {
                if(r)
                        {
                        push_genml(f,t,5,k,e);  // Q=5
                        push_genml(f,t,4,0,e);  // R=4
                        push_genml(f,t,3,0,e);  // B=3
                        push_genml(f,t,2,0,e);  // N=2
                        }
                else push_genml(f,t,0,k,e);
                }
        };
        
        
void pawngen() {        /* generates all pawn moves */

        BYTE r=0, i;
    if(Chess.w)
        {
        if(mg_Y==6) r=1;
        if(mg_X>0 && mg_Y<7)
                {
                i = mg_sq+7;
                if( (Chess.B[i]>10) || (Chess.ep==i) ) addmove(mg_sq,i,r,0);
                }
        if(mg_X<7 && mg_Y<7)
                {
                i = mg_sq+9;
                if( (Chess.B[i]>10) || (Chess.ep==i) ) addmove(mg_sq,i,r,0);
                }
        if(mg_Y<7)
                {
                i = mg_sq+8;
                if(Chess.B[i]==0)
                        {
                        addmove(mg_sq,i,r,0);
                        i+=8;
                        if(mg_Y==1 && Chess.B[i]==0) addmove(mg_sq,i,0,0);
                        }
                }
        }
        else
        {
        BYTE c;
        if(mg_Y==1) r=1;
        if(mg_X<7 && mg_Y>0)
                {
                i = mg_sq-7; c = Chess.B[i];
                if( (c && c<10) || (Chess.ep==i) ) addmove(mg_sq,i,r,0);
                }
        if(mg_X>0 && mg_Y>0)
                {
                i = mg_sq-9; c = Chess.B[i];
                if( (c && c<10) || (Chess.ep==i) ) addmove(mg_sq,i,r,0);
                }
        if(mg_Y>0)
                {
                i = mg_sq-8;
                if(Chess.B[i]==0)
                        {
                        addmove(mg_sq,i,r,0);
                        i-=8;
                        if(mg_Y==6 && Chess.B[i]==0) addmove(mg_sq,i,0,0);
                        }
                }
        }
};
void knightgen() {      /* generates all knight moves */

        BYTE x,y,xabs,yabs,i,c;
        for(x=0;x<5;x++)
         for(y=0;y<5;y++)
                {
                xabs= (x<2?2-x:x-2);
                yabs= (y<2?2-y:y-2);
                if( (xabs+yabs)==3 )
                        if( (x<2 ? (mg_X>=xabs) : (mg_X+xabs<8)) &&
                                (y<2 ? (mg_Y>=yabs) : (mg_Y+yabs<8)) )
                                {
                                i=((mg_Y+y-2)<<3) + (mg_X+x-2);
                                c=Chess.B[i];
                                if((c==0) || ((c>10)==Chess.w) ) addmove(mg_sq,i,0,0);
                                }
                }
};

void bishopgen() {      /* generates all bishop moves */

        BYTE x,y,q,i,c;
    for(x=0;x<2;x++)
         for(y=0;y<2;y++)
          for(q=1;q<8;q++)
                {
                if( (x==0 ? (mg_X>=q) : (mg_X+q<8)) &&
                        (y==0 ? (mg_Y>=q) : (mg_Y+q<8)) )
                        {
                        i=((y==0 ? mg_Y-q : mg_Y+q)<<3) + (x==0 ? mg_X-q : mg_X+q);
                        c=Chess.B[i];
                        if((c==0) || ((c>10)==Chess.w) ) addmove(mg_sq,i,0,0);
                        if(c!=0) q=8;
                        }
                else q=8;
                }
};

void rookgen() {        /* generates all rook moves */

        BYTE x,y,q,i,c;
    for(x=0;x<3;x++)
         for(y=0;y<3;y++)
                {
                if( ((x==1) || (y==1)) && (x!=y) )
                 {
                 for(q=1;q<8;q++)
                        {
                        if( ( x==0 ? (mg_X>=q) : ((x==1)||(mg_X+q<8))  ) &&
                                ( y==0 ? (mg_Y>=q) : ((y==1)||(mg_Y+q<8)) ) )
                                {
                                i= ((y==0 ? mg_Y-q : (y==1 ? mg_Y+0 : mg_Y+q) )<<3);
                                i+= (x==0 ? mg_X-q : (x==1 ? mg_X+0 : mg_X+q));
                                c=Chess.B[i];
                                if((c==0) || ((c>10)==Chess.w) ) addmove(mg_sq,i,0,0);
                                if(c!=0) q=8;
                                }
                        else q=8;
                        }
                 }
                }
};

void kinggen() {        /* generates all king moves */

        BYTE x,y,i,c,b,E1,g;
    for(x=0;x<3;x++)
         for(y=0;y<3;y++)
          if( (x!=1) || (y!=1) )
                {
                if( ( x==0 ? (mg_X>0) : ((x==1)||(mg_X<7))  ) &&
                        ( y==0 ? (mg_Y>0) : ((y==1)||(mg_Y<7)) ) )
                        {
                        i= ((y==0 ? mg_Y-1 : (y==1 ? mg_Y+0 : mg_Y+1) )<<3);
                        i+= (x==0 ? mg_X-1 : (x==1 ? mg_X+0 : mg_X+1));
                        c=Chess.B[i];
                        if((c==0) || ((c>10)==Chess.w) ) addmove(mg_sq,i,0,0);
                        }
                }
                

    b=1<<(Chess.w ?3:1);
        for(x=0; x<2; x++,b>>=1)
         if(Chess.cs & b)
                {
                E1=(Chess.w?4:60);
                for(g=1,y=1; y<3+x; y++)
                        if(Chess.B[ (x==0 ? E1+y : E1-y) ]!=0) g=0;

                if(g)
                        {
                        // put fake kings and verify checks+
                        for(y=1; y<3; y++)
                                Chess.B[ (x==0 ? E1+y : E1-y) ]= (Chess.w ? 6 : 16);
                        CkiSq = 0xFF;
                        g=isCheck()^1;
                        for(y=1; y<3; y++)
                                Chess.B[ (x==0 ? E1+y : E1-y) ]=0;
                        if(g) addmove(E1, ( x==0 ? E1+2 :E1-2), 0,x+1);
                        }
                }

};

//Give 0 to scan for 1 possible move only (faster way for checkmate,stalemate detection)
BYTE CkMt1mv;

/*
 Generates next possible moves in current position
*/
void MoveGen()
        {
        BYTE c;
        mg_L = Chess.gList;
        Chess.gc = 0;
        
        for(mg_sq=0; mg_sq<64; mg_sq++)
                {
                c=Chess.B[mg_sq];
                if(c)
                        {
                        if(Chess.w == (c<10))
                                {
                                c%=10;
                                mg_X=mg_sq&7; mg_Y=mg_sq>>3;
                                switch(c)
                                        {
                                        case 1: { pawngen(); break; }
                                        case 2: { knightgen(); break; }
                                        case 3: { bishopgen(); break; }
                                        case 4: { rookgen(); break; }
                                        case 5: { bishopgen(); rookgen(); break; }      // if queen
                                        case 6: { kinggen(); break; }
                                        }
                                }
                        }
                if(CkMt1mv && (Chess.gc!=0)) break;
                }
        CkiSq = (Chess.w ? Chess.WK : Chess.BK);
        CkMt1mv = 0;
        };
        
//================ END of move generation

/* Sets position by FEN
                Parameter: array of chess FEN of pieces
                All data required till move number.
 */
void setFEN(BYTE *pos)
 {  
        BYTE c, i, j, y=7, x=0;
        
        memset( Chess.B, 0, 64 );
                        
        for(i=0;;i++)
        {
        c = pos[i];
        if(c==32) break;
        if(c!=47)                       // '/'
                {
                if((c>=48) && (c<=57)) x+=(c-49); else Chess.B[(y<<3)+x] = c;
                if(x>6) { x=0; y--; } else x++;
                }
        }
        c = pos[++i];
        Chess.w = (c==119?1:0); // 'w' or 'b'?
    Chess.cs = 0;
        for(i+=2;;i++)
        {
        c = pos[i];
        if(c==32) break;
        for(j=0;j<4;j++)
                if(chess_cs[j]==c) Chess.cs += (1<<(3-j));
        }
        c = pos[++i];
        Chess.ep = ((c==45) ? 0xFF : (c-97)+((pos[++i]-49)<<3) );
        for(i+=2; pos[i]!=32; i++);             // skips null-move counter
        Chess.mn = atoi( &pos[++i] );
        Chess.mh = Chess.mHist; // simply ignores garbage in hist
        for(j=0; j<64; j++)
                switch( Chess.B[j] )
                {
                case  6: { Chess.WK = j; break; }
                case 16: { Chess.BK = j; break; }
                }
        CkiSq = 0xFF; CkMt1mv = 0;
        MoveGen();
 };

/* FIRST TIME INIT ONLOAD */
void OnloadInitChessGame()
{
        Chess.gList = MOVEGEN;
        Chess.mHist = MOVEHIST;
        setFEN( (BYTE*) chess_sFEN );
}
