
#define ADDR_Input1 0x4016
#define ADDR_Input2 0x4017

// keys pressed right now
#define joyA (joyKey&128)
#define joyB (joyKey&64)
#define joySELECT (joyKey&32)
#define joySTART (joyKey&16)
#define joyUP (joyKey&8)
#define joyDOWN (joyKey&4)
#define joyLEFT (joyKey&2)
#define joyRIGHT (joyKey&1)

BYTE preKey = 0;	// the previous input
BYTE joyKey = 0;	// current input or 0
BYTE keyTicked = 0;	// this is a ticker counter while user presses nothing
BYTE inpRND = 0;	// Random number based on user waiting

void readjoykey()
{
BYTE b=128;
joyKey = 0;
poke(ADDR_Input1,1);
poke(ADDR_Input1,0);
while(b)
	{
	if( peek(ADDR_Input1)&1 ) joyKey|=b;
	b>>=1;
	};

}

#define clearjoykey() joyKey=0;
#define clearprekey() preKey=0;

#define rdkrndifdo(act) { \
 readjoykey(); inpRND++; if(keyTicked) keyTicked++; \
 if(joyKey && (joyKey!=preKey) ) {  preKey=joyKey; act; } }

void loopTillOtherKey()
 {
 while(1) { rdkrndifdo(break); }
 }

// returns 1 if other key is pressed now, 0-nothing
BYTE inkeyOtherKey()
 {
 rdkrndifdo(return 1);
 return 0;
 }
