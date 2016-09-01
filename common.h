/*
	Some definitions to all code of this project
	cc65 for NES
*/

#define BYTE unsigned char
#define WORD unsigned int

#define poke(A,X) (*(BYTE *)A) = (X)
#define peek(A) (*(BYTE *)A)

// get highest byte
#define lo2hi(x)           \
            (__AX__ = (x),      \
             asm ("tax"),       \
             asm ("lda #$00"),  \
             __AX__)
	     
#define hi2lo(x)           \
            (__AX__ = (x),      \
             asm ("txa"),       \
             asm ("ldx #$00"),  \
             __AX__)
			 
#define loByte(x)           \
            (__AX__ = (x),      \
             asm ("ldx #$00"),       \
             __AX__)

WORD random ( WORD dw ) { return ((rand() + clock()) % dw); }


// a small code to test values
BYTE DBG_dump [1000];	// address to save dump

// seek for "DBG:" text in memory
void dbgprep()
{
 BYTE *p=DBG_dump;
 poke(p++,'D'); poke(p++,'B');
 poke(p++,'G'); poke(p++,':'); 
}
	
// (to debug 1 byte without stopping program)
void dbgByte( BYTE b )
 {
 dbgprep(); poke( (BYTE *)(DBG_dump+4),b);
 }
// (to debug a buffer)
void dbgDump( BYTE *ptr, BYTE len )
 {
 dbgprep(); memcpy(DBG_dump+4,ptr,len);
 }

// dummy pause, use waitvblank() instead
void paused()
{
int i;
while((++i)!=999);
}