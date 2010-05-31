/*====================================================================

$Id: gdtool.cpp,v 1.4 2008-11-11 01:04:26 wntrmute Exp $

project:      GameCube DSP Tool (gcdsp)
mail:		  duddie@walla.com

Copyright (c) 2005 Duddie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

$Log: not supported by cvs2svn $
Revision 1.4  2008/10/04 10:30:00  Hermes
added function to export the code to .h file
added support for / * * / and // for comentaries
added some sintax detection when use registers

Revision 1.3  2005/09/14 02:19:29  wntrmute
added header guards
use standard main function

Revision 1.2  2005/09/14 02:06:24  wntrmute
added TCHAR defines on linux

Revision 1.1  2005/08/24 22:13:34  wntrmute
Initial import


====================================================================*/

#include <stdio.h>

#include <memory.h>
#include <stdlib.h>
#include "dtypes.h"
#include "gdsp_tool.h"

#ifdef _MSVC_
#pragma warning(disable:4996)
#endif

#define M_NONE			0
#define M_ASSEMBLE		1
#define	M_DISASSEMBLE	2

extern sint32 strtoval(char *str);

int main(int argc, char* argv[])
{
	uint8 mode;
	char *fname=NULL;
	const char *ofile = "a.out";
	int i,n;
	bool	decode_unknown = false;
	mode = M_NONE;

	FILE *fout=stdout;

	gd_globals_t *gdg;
	gdg = gd_init();
	gdg->pc = 0;
	gdg->decode_registers = false;
	gdg->decode_names = false;
	gdg->show_hex = true;
	gdg->show_pc = true;
	gdg->print_tabs = false;
	gdg->ext_separator = '\'';
	for (i = 0 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'a': // added by Hermes
				if(argv[i + 1])
				{
				char cadena[1024];
				sprintf(cadena,"#%s",argv[i + 1]);
				n=0;
				while(cadena[n]!=0)
					{
					if (cadena[n] >= 'a' && cadena[n] <= 'z')	// convert to uppercase
					cadena[n] = cadena[n] - 'a' + 'A';
					n++;
					}
				gdg->pc = strtoval(cadena);
				}
	
				i++;
				break;
			case 'd':
				ofile=NULL; // modified by Hermes
				mode = M_DISASSEMBLE;
				fname = argv[i + 1];
				i++;
				break;
			case 'c':
				mode = M_ASSEMBLE;
				fname = argv[i + 1];
				i++;
				break;
			case 'o':
				ofile = argv[i + 1];
				i+=2;
				break;
			case 's':
				gdg->show_hex = false;
				gdg->show_pc = false;
				break;
			case 'n':
				gdg->decode_names = true;
				break;
			case 'r':
				gdg->decode_registers = true;
				break;
			case 'u':
				decode_unknown = true;
			}
		}
	}

	gd_dis_open_unkop();

	switch(mode)
	{
	case M_DISASSEMBLE:
		 
		if(ofile)
			if(!(fout=fopen(ofile, "wb"))) fout=stdout;
		gd_dis_file(gdg, fname, fout);
		break;
	case M_ASSEMBLE:
		gd_ass_init();
		gd_ass_file(gdg, fname, 1);
		gd_ass_init();
		gd_ass_file(gdg, fname, 2);

		printf("Code size: %d\n", gdg->buffer_size * 2);

		
		
		char name[32];
		int n,bsize;
		char *cad,*cad2;

		fout = fopen(ofile, "wb");
		cad2=strstr((char*)ofile,".h");
		if(cad2)
			{
			const char *end[2]={"", ","};
			cad=cad2;
			while(cad>ofile)
				{
				if(*cad=='/' || *cad=='\\') {cad++;break;}
				cad--;
				}
			n=(int) (cad2-cad);
			if(n>31) n=31;
			memcpy(name,cad,n);name[n]=0;
		
			bsize = (gdg->buffer_size*2+31) & ~31;

			fprintf(fout, "/* gdtool v1.4 .h exporter by Hermes */\n\n");

			fprintf(fout, "#define %s_size %d\n\n",name,bsize); // padded to 32 bytes
		
			fprintf(fout, "unsigned short %s[%d] __attribute__ ((aligned (32))) ={\n\n",name, (bsize>>1));

			n=0;
			bsize >>= 1;
			cad=gdg->buffer;
			while(n<gdg->buffer_size)
				{
				if(n & 15) fprintf(fout, " 0x%04x%s", ((unsigned char) cad[1]) | (((unsigned char) cad[0])<<8), end[(n<bsize-1)]);
				else	fprintf(fout, "\n	0x%04x%s", ((unsigned char) cad[1]) | (((unsigned char) cad[0])<<8), end[(n<bsize-1)]);
				cad+=2;
				n++;
				}
			while(n<bsize)
				{
				if(n & 15) fprintf(fout," 0x0000%s",end[(n<bsize-1)]);
				else fprintf(fout,"\n 0x0000%s",end[(n<bsize-1)]);
				n++;
				}
			fprintf(fout, "\n\n};\n\n");
			}
        else
		fwrite(gdg->buffer, 1, gdg->buffer_size * 2, fout);
		fclose(fout);
		break;
	default:
		printf("GCDSP 1.4, Copyright (C) 2005 Duddie\nGCDSP comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome\nto redistribute it under GPL conditions\n");
		printf("\nUsage:\n\n");
		printf("	gcdsptool <param1> <param2> ....\n\n");
		printf("		-a 0x01AB  -> start address to disassemble\n");
		printf("		-s         -> don't show PC and HEX code\n");
		printf("		-n         -> decode names\n");
		printf("		-r         -> decode registers\n");
		printf("		-u         -> decode unknown\n");
		printf("		-d infile  -> disassemble file\n");
		printf("		-c infile  -> assemble file\n");
		printf("		-o outfile -> output file (ext .h to C export file)\n\n");
		printf("Examples:\n\n");
		printf("  gcdsptool -c dsp.asm -o dsp.h	-> assemble and export to C\n");
		printf("  gcdsptool -c dsp.asm -o dsp.bin	-> assemble and export to Binary\n");
		printf("  gcdsptool -a 0x8000 -n -r -s -d rom_dump.bin -o rom_dump.asm\n");
		printf("  gcdsptool -n -r -d code_dump.bin -o code_dump.lst\n\n");
		break;
	}

	if (decode_unknown)
		gd_dis_close_unkop();

	return 0;
}
