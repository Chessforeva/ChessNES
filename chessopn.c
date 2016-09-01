/*
A very short chess openings database

*/

// read binary data (variable OpnBase[])
#include "prep_openings.c"

// uses small book of moves, this gets address to data

#define RND inpRND

BYTE histmoved[255];	// to save movegen indexes
BYTE movedcnt=0;

BYTE openStop=0;	// optimised, look or not
BYTE oMx_=0;
 
// returns move index+1 , or 0 if none
BYTE OpeningMove(void)
{
 BYTE h0,h1,q,i,j, *p, w,m;
 
 #define maxopeninglen 20
 
 BYTE opnblist[maxopeninglen];	// current opening list, not too long
 BYTE opnbptr=0;

 #define maxopeningvariants 20
  
 BYTE opnvariants[maxopeningvariants];	// to hold current variants
 BYTE opnvcnt=0;

 switch(movedcnt)
 {
  case 0:	// e2e4,g1f3,...
  {
  p = (BYTE *)(OpnCases[(RND%Opn1mv)]);
  return (p[0]+1);
  }
 case 1:		// 1...c7c5,1...g8f6,...
  {

   h0=histmoved[0];
   for(i=0;i<Opn1mv;i++)
    {
     p = (BYTE *)(OpnCases[i]);
     q = p[0];
     if(h0==q)
      {
       waitvblank();
       for(j=i+1; j<Opn1mv; j++)
        {
        p = (BYTE *)(OpnCases[j]);
        if(q!=p[0]) break;
	if(!(j&3)) waitvblank();
	}
       j-=i;	// diff.
	
       p = (BYTE *)(OpnCases[i+(RND%j)]);
       
       return (p[1]+1);
      }
    }
   break;
  }

 default:	// moves 2., more
  {

   if(movedcnt>maxopeninglen) break;
   
   if(movedcnt<oMx_) openStop=0;
   oMx_ = movedcnt;
   
   if(openStop) break;
   
   //other opening cases
   h0=histmoved[0]; h1=histmoved[1];
   for(i=0;i<Opn1mv;i++)
    {
     p = (BYTE *)(OpnCases[i]);
     if(h0==*(p++) && h1==*(p++))
      {
       waitvblank();
       opnvcnt=0;
       opnblist[0]=h0; opnblist[1]=h1; opnbptr=2;
       for(;;)
        {
        q=*(p++);
	if(q==0xFF) break;
	if(q<200)
	 {
	 if(!(opnbptr&3)) waitvblank();
	 
	 opnblist[opnbptr++]=q;
	 if(opnbptr==movedcnt+1)	// a new variant
	  {
	   for(j=0;j<movedcnt;j++)
	    {
	     w = opnblist[j];
	     m = histmoved[j];
	     if(w!=m) { q=0xFE; break; }
	    }
	   waitvblank(); 
	   if(q!=0xFE) opnvariants[opnvcnt++]=q;
	  }
	 }	
	else
	 {
	 opnbptr=(q-200);	// preset pointer of list
	 }
	}
	
       if(!opnvcnt) openStop = 1;	// if do not look anymore in book
       else
        {
	return (opnvariants[(RND%opnvcnt)]+1);
	}
       break;
      }
    }
  }
 }
 
  return 0;
}

