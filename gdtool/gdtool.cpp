/*====================================================================

$Id: gdtool.cpp,v 1.1 2005-08-24 22:13:34 wntrmute Exp $

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

====================================================================*/

#include <stdio.h>
#include <tchar.h>
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

int _tmain(int argc, _TCHAR* argv[])
{
	uint8 mode;
	char *fname=NULL;
	char *ofile = "a.out";
	int i;
	bool	decode_unknown = false;
	mode = M_NONE;

	gd_globals_t *gdg;
	gdg = gd_init();

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
			case 'd':
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
		gd_dis_file(gdg, fname, stdout);
		break;
	case M_ASSEMBLE:
		gd_ass_init();
		gd_ass_file(gdg, fname, 1);
		gd_ass_init();
		gd_ass_file(gdg, fname, 2);

		printf("Code size: %d\n", gdg->buffer_size * 2);

		FILE *fout;
		fout = fopen(ofile, "wb");
		fwrite(gdg->buffer, 1, gdg->buffer_size * 2, fout);
		fclose(fout);
		break;
	default:
		printf("GCDSP 0.1, Copyright (C) 2005 Duddie\nGCDSP comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome\nto redistribute it under GPL conditions\n");
		break;
	}

	if (decode_unknown)
		gd_dis_close_unkop();

	return 0;
}
