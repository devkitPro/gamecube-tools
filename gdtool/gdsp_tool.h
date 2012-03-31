/*====================================================================

filename:     gdsp_tool.h
project:      GameCube DSP Tool (gcdsp)
created:      2005.03.04
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

====================================================================*/

#ifndef _gdsp_tool_h_
#define _gdsp_tool_h_

typedef struct gd_globals_t
{
	bool	print_tabs;
	bool	show_hex;
	bool	show_pc;
	bool	decode_names;
	bool	decode_registers;

	uint16	*binbuf;
	uint16	pc;
	char	*buffer;
	uint16	buffer_size;
	char	ext_separator;
} gd_globals_t;




gd_globals_t *gd_init(void);
char *gd_dis_opcode(gd_globals_t *gdg);
uint16 gd_dis_get_opcode_size(gd_globals_t *gdg);
bool gd_dis_file(gd_globals_t *gdg, char *name, FILE *output);
void gd_dis_close_unkop(void);
void gd_dis_open_unkop(void);
void gd_ass_file(gd_globals_t *gdg, char *fname, uint32 pass);
void gd_ass_init(void);

#endif
