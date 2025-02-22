/*

 n2.exe - replaces tileset in a NES (Nintendo console) file

 Compiled on LCC-WIN32

*/

#include <stdio.h>
#include <string.h>


// MAIN PROGRAM
int main(int argc,char *argv[])
{
	if(argc<3)
		{
		printf("Replaces tileset in a NES file, use after cc65.\n\n");
		printf("  usage: n2 <.NES-file> <.CHR-file 8Kb or less>\n\n");
		printf("Press a key, exiting.\n\n");
		getchar();
		return(0);
		}

	char *nesfname = argv[1], *chrfname = argv[2];
	char t[1024];	// tmp - filename
	char header[16], c[10], *z;

	sprintf(t,"%s", nesfname);
	z = t; z+= strlen(t)-4;
	sprintf(z, ".tmp");

	FILE *f1 = fopen (nesfname, "r+b");
	FILE *f2 = fopen (chrfname, "r+b");
	if(f1==NULL || f2==NULL)
	 {
		printf("File read error.\n");
		getchar();
		return(0);
	 }
	FILE *f3 = fopen(t,"w+b");
	if(f3==NULL)
	 {
		printf("File write error.\n\n");
		getchar();
		return(0);
	 }
	printf("*header 16 bytes.\n");
	fread(header,1,16,f1);
	fwrite(header,1,16,f3);
	int p16 = (int)header[4];
	int flags6 = (int)header[6];
	int b=0, i=0, k=0;

/*
NES file structure:

Header (16 bytes)
Trainer, if present (0 or 512 bytes)
PRG ROM data (16384 * x bytes)
CHR ROM data, if present (8192 * y bytes)
PlayChoice INST-ROM, if present (0 or 8192 bytes)
PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing, see PC10 ROM-Images for details)

0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
4: Size of PRG ROM in 16 KB units
5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
6: Flags 6
7: Flags 7
8: Size of PRG RAM in 8 KB units (Value 0 infers 8 KB for compatibility; see PRG RAM circuit)
9: Flags 9
10: Flags 10 (unofficial)
11-15: Zero filled
*/

	if(flags6 & 4)
	{
	for(i=0;i<512;i++)
		{
		fread(c,1,1,f1); fwrite(c,1,1,f3);
		}
	printf("*trainer 512 bytes\n");
    }

/*
76543210
||||||||
||||+||+- 0xx0: vertical arrangement/horizontal mirroring (CIRAM A10 = PPU A11)
|||| ||   0xx1: horizontal arrangement/vertical mirroring (CIRAM A10 = PPU A10)
|||| ||   1xxx: four-screen VRAM
|||| |+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
|||| +--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
++++----- Lower nybble of mapper number
*/

	printf("*%d PRG banks 16Kb each\n",p16);
	while(p16--)
	   {
		for(i=0;i<16384;i++)
			{
			fread(c,1,1,f1); fwrite(c,1,1,f3);
			}
		}

	while(!feof(f2))
		{
		if( fread(c,1,1,f2) )
			{
			fwrite(c,1,1,f3);
			b++;
			if(!feof(f1)) fread(c,1,1,f1);
			}
		}
	printf("*CHR, replaced %d bytes\n",b);

	while(!feof(f1))
		{
		if(fread(c,1,1,f1))
			{
			fwrite(c,1,1,f3);
			k++;
			}
		}
	printf("*tail %d bytes\n",k);

	fclose(f1);
	fclose(f2);
	fclose(f3);
	remove(nesfname);
	rename(t,nesfname);

	printf("Done.\n" );
	return(0);
}


