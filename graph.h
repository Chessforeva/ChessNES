/*
 This contains NES graphics functions for cc65.
 include after "common.h",<nes.h>
 
 NES graphics is nasty. Do small and fast code as can.
 Dancing screens because of rendering interrupts.
 For larger code it is almost impossible to comply with frame-rate.
 
 Use waitvblank() to force CPU waiting for some bunch of free cycles for our code.
 Kind of pause. Otherwise screen interrupt happens inside long loops.
 
 */

#define PPU_addr_tileset0 0x0000
#define PPU_addr_tileset1 0x1000
#define PPU_addr_nametable0 0x2000
#define PPU_addr_nametable1 0x2400
#define PPU_addr_nametable2 0x2800
#define PPU_addr_nametable3 0x2C00
#define PPU_addr_nametableMirror 0x3000
#define PPU_addr_palette 0x3F00
#define PPU_addr_Mirrors 0x4000

#define PPU_CTRL *((unsigned char*)0x2000)
#define PPU_MASK *((unsigned char*)0x2001)
#define PPU_STATUS *((unsigned char*)0x2002)
#define PPU_SPR_ADDR *((unsigned char*)0x2003)
#define PPU_SPR_IO *((unsigned char*)0x2004)
#define PPU_SCROLL *((unsigned char*)0x2005)
#define PPU_ADDRESS 0x2006
#define PPU_DATA *((unsigned char*)0x2007)
 

#define word_poke(A,X) {poke(A,hi2lo(X)); poke(A,loByte(X));}
#define PPU_pokeAddr(X) word_poke(PPU_ADDRESS,X)
#define PPU_poke() word_poke(PPU_ADDRESS,ppu_pos)

// current screen addresses
WORD ADDR_Nametable = PPU_addr_nametable0;
WORD ADDR_TileSet = PPU_addr_tileset0;

// sets screen position offset (for scrolling, etc.) 32x8=256, hi-byte
#define setscreenpos(x,y) PPU_SCROLL=x; PPU_SCROLL=y;
// this resets scrolls
#define scrollrefresh() setscreenpos(0,0)

// when NMI is set
#define vblank() { while( !(peek(PPU_STATUS) & 128) ); }

// disables (sets black color)
#define grDI() {PPU_MASK=0;}
// enables graphics (my case): 8 is background only, or 0x1e for all
#define grEI() {scrollrefresh(); PPU_MASK = 8;}

int ppu_pos = 0;		// contains the last setpos

// sets position where to display character from tileset table
// row [0..27], col [0..30]
#define setpos(row,col) { ppu_pos=ADDR_Nametable+0x21+(row<<5)+col; PPU_poke(); }
#define setpos_fast(delta) { ppu_pos+=delta; PPU_poke(); }

// write a string at position
void write_string(BYTE *str)
{
while(*str) PPU_DATA = *(str++);
}

// write len-bytes from a buffer
void write_buf(BYTE *buf, BYTE len)
{
while((len--)>0) PPU_DATA = *(buf++);
}

#define write_char(c) PPU_DATA=c;

// sets palette from given 16 bytes
void setpalette(BYTE* pal)
{
BYTE i;
PPU_pokeAddr(PPU_addr_palette);
for(i=0;i<16;i++) PPU_DATA = pal[i];
}

// clear screen, we put 0s into nametable 1024 bytes
void clearscr()
{
BYTE q=0,b=0;
PPU_pokeAddr(ADDR_Nametable);
while((q++)<4) { while(++b) PPU_DATA = 0; }
}


